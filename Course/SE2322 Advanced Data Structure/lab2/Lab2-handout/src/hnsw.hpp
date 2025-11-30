#pragma once
#include <iostream>
#include "base.hpp"
#include <vector>
#include <cstring>

#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <random>
#include <cassert>

#include "../util/util.hpp"

// #define DEBUG 0

namespace HNSWLab
{

    class HNSW : public AlgorithmInterface
    {
    private:
        // 自定义最近邻比较函数
        struct cmp_nearest
        {
            const int *item;
            const std::unordered_map<int, const int *> &vector_map;

            cmp_nearest(const int *item, const std::unordered_map<int, const int *> &vector_map)
                : item(item), vector_map(vector_map) {}

            bool operator()(int label1, int label2) const
            {
                return l2distance(item, vector_map.at(label1), 128) > l2distance(item, vector_map.at(label2), 128);
            }
        };

        // 自定义最远邻比较函数
        struct cmp_furthest
        {
            const int *item;
            const std::unordered_map<int, const int *> &vector_map;

            cmp_furthest(const int *item, const std::unordered_map<int, const int *> &vector_map)
                : item(item), vector_map(vector_map) {}

            bool operator()(int label1, int label2) const
            {
                return l2distance(item, vector_map.at(label1), 128) < l2distance(item, vector_map.at(label2), 128);
            }
        };

        // 用unordered_map来存储label和对应的向量
        std::unordered_map<int, const int *> vector_map;

        // 用unordered_map来存储label和对应的level
        std::unordered_map<int, int> level_map;

        // 用unordered_map数组来存储在第lc层的label和对应所有邻居，lc不超过30，每一层的邻居集合用unordered_set来存储
        std::unordered_map<int, std::unordered_map<int, std::unordered_set<int>>> neighbors;

        // 入口点的label，-1表示没有入口点
        int enter_point;

    public:
        HNSW()
        {
            enter_point = -1;
        }

        // 插入一个向量
        void insert(const int *item, int label);

        // 查询k个最近邻
        std::vector<int> query(const int *query, int k);

        // SEARCH_LAYER函数，q是查询的向量，ep是入口点的label，ef是搜索的最大个数，lc是搜索的层数
        std::unordered_set<int> SEARCH_LAYER(const int *q, int ep, int ef, int lc);

        // SELECT_NEIGHBORS函数，q是查询向量的label，W是当前层的所有候选邻居节点的label集合，M是最大邻居数，lc是搜索的层数
        std::unordered_set<int> SELECT_NEIGHBORS(int q, std::unordered_set<int> W, int M, int lc);

        // 设置入口点
        void set_enter_point(int label);

        // 获取入口点
        int get_enter_point();

        ~HNSW() {}
    };

    /**
     * input:
     * iterm: the vector to be inserted
     * label: the label(id) of the vector
     */
    void HNSW::insert(const int *item, int label)
    {
        // DEBUG
        // if (DEBUG)
        //     std::cout << "BEGIN INSERT" << std::endl;

        // 将item插入到vector_map中，其中向量的维数
        vector_map[label] = item;

        // 直接用get_random_level得到L
        int L = get_random_level();

        // 将level插入到level_map中
        level_map[label] = L;

        // 这里的W是一个unordered_set，用于存储当前层的最近邻元素集合
        std::unordered_set<int> W;

        // 尝试获取入口点
        int ep = get_enter_point();

        // 如果没有入口点，那么直接跳过以下步骤、并将label设置为入口点
        if (ep == -1)
        {
            set_enter_point(label);
            return;
        }

        // 如果有入口点，从MaxL到L+1层，每层寻找当前层lc中离q最近邻的1个点，加⼊W
        int maxL = level_map[ep];
        for (int lc = maxL; lc >= L + 1; lc--)
        {
            W = SEARCH_LAYER(item, ep, 1, lc);
            ep = *W.begin();
        }

        // 从L到0层，搜索每一层的邻居
        for (int lc = std::min(maxL, L); lc >= 0; lc--)
        {
            // 每层寻找当前层q最近邻的efConstruction个点赋值到集合W
            W = SEARCH_LAYER(item, ep, ef_construction, lc);
            // 在W中选择q最近邻的M个点作为neighbors双向连接起来
            // neighbors[lc][q]表示第lc层中节点q的邻居
            neighbors[lc][label] = SELECT_NEIGHBORS(label, W, M, lc);
            // 检查每个neighbors的连接数，
            // 如果⼤于Mmax，则保留其中⻓度最短的Mmax个连接，并删去其他边
            std::unordered_set<int> temp_neighbors = neighbors[lc][label];
            for (auto &e : temp_neighbors)
            {
                neighbors[lc][e].insert(label);
                if (neighbors[lc][e].size() > M_max)
                {
                    neighbors[lc][e] = SELECT_NEIGHBORS(e, neighbors[lc][e], M_max, lc);
                }
            }
            // 从W中挑选最接近q的节点作为下⼀层搜索的⼊⼝节点
            long min_dist = l2distance(item, vector_map[*W.begin()], 128);
            ep = *W.begin();
            for (auto &e : W)
            {
                long dist = l2distance(item, vector_map[e], 128);
                if (dist < min_dist)
                {
                    min_dist = dist;
                    ep = e;
                }
            }
        }

        // 最后、如果L>maxL，则将label设置为整个图的入口点
        if (L > maxL)
        {
            set_enter_point(label);
        }
    }

    // SELECT_NEIGHBORS函数，q是查询向量的label，W是当前层的所有候选邻居节点的label集合，M是最大邻居数，lc是搜索的层数
    std::unordered_set<int> HNSW::SELECT_NEIGHBORS(int q, std::unordered_set<int> W, int M, int lc)
    {
        // DEBUG
        // if (DEBUG)
        //     std::cout << "BEGIN SELECT_NEIGHBORS" << std::endl;

        // 定义一个最小堆，用于存储q的最近邻
        std::priority_queue<int, std::vector<int>, cmp_nearest> closest(cmp_nearest(vector_map[q], vector_map));

        // 将W中的所有元素插入到closest中
        for (auto &e : W)
        {
            closest.push(e);
        }

        // 定义一个unordered_set，用于存储最终的邻居节点
        std::unordered_set<int> result;

        // 从closest中取出M个最近邻
        while (!closest.empty() && result.size() < M)
        {
            result.insert(closest.top());
            closest.pop();
        }

        return result;
    }

    // SEARCH_LAYER函数，q是查询的向量，ep是入口点的label，ef是搜索的最大个数，lc是搜索的层数
    std::unordered_set<int> HNSW::SEARCH_LAYER(const int *q, int ep, int ef, int lc)
    {
        // DEBUG
        // if (DEBUG)
        //     std::cout << "BEGIN SEARCH_LAYER" << std::endl;

        // v：访问过的节点集合
        std::unordered_set<int> v;
        // C: 候选节点集合，其邻居将会被探索，需要取出C中q的最近邻
        std::priority_queue<int, std::vector<int>, cmp_nearest> C(cmp_nearest(q, vector_map));
        // W: 现在发现的最近邻元素集合，需要取出W中距离q最远的元素
        std::priority_queue<int, std::vector<int>, cmp_furthest> W(cmp_furthest(q, vector_map));

        // 将入口点加入到v，C，W中
        v.insert(ep);
        C.push(ep);
        W.push(ep);

        while (!C.empty())
        {
            // 取出C中q的最近邻c
            auto c = C.top();
            C.pop();

            // 取出W中q的最远点f
            auto f = W.top();

            // 如果c已经比最近邻所有节点离目标节点更远，则无需探索
            if (l2distance(vector_map[c], q, 128) > l2distance(vector_map[f], q, 128))
                break;

            // 当c比f距离q更近时，则将c的每一个邻居e都进行遍历
            for (auto e : neighbors[lc][c])
            {
                if (v.count(e) == 0)
                {
                    v.insert(e);
                    f = W.top();
                    // 如果e比w中距离q最远的f要更接近q，那就把e加入到W和候选元素C中
                    if (l2distance(vector_map[e], q, 128) < l2distance(vector_map[f], q, 128) || W.size() < ef)
                    {
                        C.push(e);
                        W.push(e);
                        if (W.size() > ef) // 保证返回的数目不大于ef
                            W.pop();
                    }
                }
            }
        }

        // 定义结果result std::unordered_set<int>，用于最终返回
        std::unordered_set<int> result;
        while (!W.empty())
        {
            result.insert(W.top());
            W.pop();
        }
        return result;
    }

    /**
     * input:
     * query: the vector to be queried
     * k: the number of nearest neighbors to be returned
     *
     * output:
     * a vector of labels of the k nearest neighbors
     */
    std::vector<int> HNSW::query(const int *query, int k)
    {
        // DEBUG
        // if (DEBUG)
        //     std::cout << "BEGIN QUERY" << std::endl;

        // ⾃顶层向第1层逐层搜索,每层寻找当前层与⽬标节点q最近邻的1个点赋值到集合W（可以复⽤
        // SEARCH_LAYER函数）然后从集合W中选择最接近q的点作为下⼀层的搜索⼊⼝点。

        // 从最顶层开始搜索
        int ep = get_enter_point();
        int maxL = level_map[ep];
        std::unordered_set<int> W;
        for (int lc = maxL; lc >= 1; lc--)
        {
            W = SEARCH_LAYER(query, ep, 1, lc);
            ep = *W.begin();
        }

        // 在第0层使⽤SEARCH_LAYER函数查找与⽬标节点q临近的efConstruction个节点，并根据需要返回的
        // 节点数量从中挑选离⽬标节点的最近的节点集（在该Lab中efConstruction为100，需要返回的节点数
        // 为10个）。
        W = SEARCH_LAYER(query, ep, ef_construction, 0);

        std::vector<int> res;
        std::priority_queue<int, std::vector<int>, cmp_nearest> closest(cmp_nearest(query, vector_map));
        for (auto &e : W)
        {
            closest.push(e);
        }
        while (!closest.empty() && res.size() < k)
        {
            res.push_back(closest.top());
            closest.pop();
        }

        // 打印一下结果
        // if (DEBUG)
        // {
        //     std::cout << "The result of query is: ";
        //     for (auto &e : res)
        //     {
        //         std::cout << e << " ";
        //     }
        //     std::cout << std::endl;
        // }

        return res;
    }

    void HNSW::set_enter_point(int label)
    {
        enter_point = label;
    }

    int HNSW::get_enter_point()
    {
        return enter_point;
    }
}