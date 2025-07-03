#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include "src/hnsw.hpp"
#include "util/util.hpp"
#include "util/vecs_io.hpp"
#include "util/ground_truth.hpp"
#include "util/parameter.hpp"

using namespace std;
using namespace HNSWLab;
namespace HNSWLab {
HNSW::HNSW()
        : efConstruction(100),  max_level(0), ep(-1),enterpoint(-1) {
        // Initialize random generator
       
    }
    int HNSW::get_random_level() {

       std::uniform_real_distribution<double> distribution(0.0, 1.0);
        double r = -log(distribution(level_generator_)) * mult_;
        return (int) r;
    }

    int HNSW::get_enter_point_level(int ep) {
       
        for (int lc =layers.size()-1; lc >= 0; --lc) {
            if(!(layers[lc].empty())){
            if (layers[lc].find(ep) != layers[lc].end()) {
                return lc;
            }
            }
        }
        return 0; // If not found, it should be the base level
    }

   void HNSW::insert(const int *item, int label) {
        int L = get_random_level();  // 获取随机层级
        nodes[label] = std::vector<const int*>(1, item);

        // std::cout << "Inserting item with label " << label << " at level " << L << std::endl;

        if (layers.empty()) {
            for (int lc = L; lc >= 0; --lc) {
            layers[lc][label] = {};
            }
        enterpoint=label;
        // std::cout << "Inserted first element at level " << L << " with label " << label << std::endl;
        return;
        }
        std::vector<int> W;
        ep=enterpoint;
         for (int lc =layers.size()-1; lc >= 0; --lc) {
            if(!(layers[lc].empty())){
            max_level=lc;
            break;
            }
        }
        // std::cout << "Initial entry point: " << ep << " at level " << max_level << std::endl;
        // if(max_level>L){
        for (int lc = max_level; lc > L; lc--) {
            std::vector<int> W_vec = search_layer(item, ep, 1, lc);
            std::priority_queue<std::pair<double, int>> W;
            for (int e : W_vec) {
            W.push({-euclidean_distance(item, nodes[e][0]), e});
            }
            ep = W.top().second;
            // std::cout << "Searching level " << lc << ", new entry point: " << ep << std::endl;
        }
        // }
        // else{
        // for (int lc = max_level; lc <= L; lc++) {
        //     std::vector<int> W_vec = search_layer(item, ep, 1, lc);
        //     std::priority_queue<std::pair<double, int>> W;
        //     for (int e : W_vec) {
        //     W.push({-euclidean_distance(item, nodes[e][0]), e});
        //     }
        //     ep = W.top().second;
        //     std::cout << "Searching level " << lc << ", new entry point: " << ep << std::endl;
        // }
        // }
   
        for (int lc = L; lc >= 0; --lc) {
            auto W1 = search_layer(item, ep, efConstruction, lc);
            
            auto neighbors = select_neighbors(item, W1, M, lc);
            layers[lc][label] = neighbors;

            // std::cout << "At level " << lc << ", inserting node " << label << " with neighbors: ";
           

            for (int neighbor : neighbors) {
                layers[lc][neighbor].push_back(label);
                if (layers[lc][neighbor].size() > M_max) {
                    layers[lc][neighbor] = select_neighbors(nodes[neighbor][0], layers[lc][neighbor], M, lc);
                }
            }
            double min_dist = std::numeric_limits<double>::max();
            int closest_node = -1;
            for (int node : W1) {
                double dist = euclidean_distance(item, nodes[node][0]);
                if (dist < min_dist) {
                    min_dist = dist;
                    closest_node = node;
                }
            }
            ep = closest_node;
        }

        if (L >max_level) {
            enterpoint = label;
            // std::cout << "Updated entry point to " << ep << " at level " << L << std::endl;
        }
    }

    std::vector<int> HNSW::search_layer(const int *q, int ep, int ef, int lc) {
       std::unordered_set<int> visited;
        std::priority_queue<std::pair<double, int>> C;
        std::priority_queue<std::pair<double, int>> W;

        double initial_distance = euclidean_distance(q, nodes[ep][0]);
        C.push({-initial_distance, ep});
        W.push({initial_distance, ep});
        visited.insert(ep);

        while (!C.empty()) {
            auto [dist_c, c] = C.top();
            C.pop();

            auto [dist_f, f] = W.top();
            if (-dist_c > dist_f) break;

            for (int e : layers[lc][c]) {
                if (visited.find(e) == visited.end()) {
                    visited.insert(e);
                    double dist_e = euclidean_distance(q, nodes[e][0]);
                    if (dist_e < dist_f || W.size() < ef) {
                        C.push({-dist_e, e});
                        W.push({dist_e, e});
                        if (W.size() > ef) {
                            W.pop();
                        }
                    }
                }
            }
        }

        std::vector<int> result;
        while (!W.empty()) {
            result.push_back(W.top().second);
            W.pop();
        }
        std::reverse(result.begin(), result.end());
        return result;
    }

    std::vector<int> HNSW::select_neighbors(const int *q, const std::vector<int>& W, int M, int lc) {
        std::priority_queue<std::pair<double, int>> pq;
        for (int e : W) {
            pq.push({euclidean_distance(q, nodes[e][0]), e});
            if (pq.size() > M) {
                pq.pop();
            }
        }

        std::vector<int> neighbors;
        while (!pq.empty()) {
            neighbors.push_back(pq.top().second);
            pq.pop();
        }
        return neighbors;
    }

    std::vector<int> HNSW::query(const int *query, int k) {
    std::vector<int> result;
    ep=enterpoint;
    for (int lc =layers.size()-1; lc >= 0; --lc) {
            if(!(layers[lc].empty())){
            max_level=lc;
            break;
            }
        }

    // std::cout << "Starting query..." << std::endl;
    // std::cout << "Max level is " << max_level << std::endl;
     std::vector<int> W; // 用于累积结果的容器
    // 从最高层开始查询
     for (int lc = max_level; lc >= 1; --lc) {
            // std::cout << "Searching at level " << lc << " starting from entry point " << ep << std::endl;
            auto candidates = search_layer(query, ep, 1, lc);
            W.insert(W.end(), candidates.begin(), candidates.end()); // 累积结果
            
            // 找到 W 中距离最近的节点作为新的入口点
            double min_dist = std::numeric_limits<double>::max();
            int closest_node = -1;
            for (int node : W) {
                double dist = euclidean_distance(query, nodes[node][0]);
                if (dist < min_dist) {
                    min_dist = dist;
                    closest_node = node;
                }
            }
            ep = closest_node;
            // std::cout << "New entry point: " << ep << std::endl;
        }

    // std::cout << "Searching at base level (0) with efConstruction = " << efConstruction << std::endl;
    auto W1 = search_layer(query, ep, ef_construction, 0);
           

    std::priority_queue<std::pair<double, int>> pq;
    for (int e : W1) {
        pq.push({euclidean_distance(query, nodes[e][0]), e});
        if (pq.size() > k) {
            pq.pop();
        }
    }

    std::vector<std::pair<double, int>> sorted_results;
    while (!pq.empty()) {
        sorted_results.push_back(pq.top());
        pq.pop();
    }

    std::sort(sorted_results.begin(), sorted_results.end());

    for (const auto& pair : sorted_results) {
        result.push_back(pair.second);
        // std::cout << "Found neighbor: " << pair.second << " with distance: " << pair.first << std::endl;
    }

    return result;
}


    double HNSW::euclidean_distance(const int *a, const int *b) {
        double dist = 0.0;
        for (int i = 0; i < 128; ++i) { // Assuming vectors are 128-dimensional
            dist += (a[i] - b[i]) * (a[i] - b[i]);
        }
        return dist;
    }
}
void print_query_result(const std::vector<int>& result) {
    for (int label : result) {
        std::cout << label << " ";
    }
    std::cout << std::endl;
}

void query_thread(HNSW& hnsw, const int* query, int query_vec_dim, int k, vector<vector<int>>& results, int idx) {
    results[idx] = hnsw.query(query, k);
}

int main() {
    std::printf("load ground truth\n");
    int gnd_n_vec = 100;
    int gnd_vec_dim = 10;
    char *path = "./data/siftsmall/gnd.ivecs";
    int *gnd = read_ivecs(gnd_n_vec, gnd_vec_dim, path);

    std::printf("load query\n");
    int query_n_vec = 100;
    int query_vec_dim = 128;
    path = "./data/siftsmall/query.bvecs";
    int *query = read_bvecs(query_n_vec, query_vec_dim, path);

    std::printf("load base\n");
    int base_n_vec = 10000;
    int base_vec_dim = 128;
    path = "./data/siftsmall/base.bvecs";
    int *base = read_bvecs(base_n_vec, base_vec_dim, path);

    HNSW hnsw;

    size_t report_every = 1000;
    TimeRecord insert_record;

    auto insert_start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < base_n_vec; i++) {
        hnsw.insert(base + base_vec_dim * i, i);

        if (i % report_every == 0) {
            insert_record.reset();
        }
    }
    auto insert_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> insert_duration = insert_end - insert_start;
    
    std::cout << "Average insertion time per vector: " << (insert_duration.count() / base_n_vec) * 1e3 << " ms" << std::endl;

    printf("querying\n");
    vector<vector<int>> test_gnd_l(query_n_vec);
    double single_query_time;
    TimeRecord query_record;

    std::vector<std::thread> threads;
    for (int i = 0; i < query_n_vec; ++i) {
        threads.emplace_back(query_thread, std::ref(hnsw), query + i * query_vec_dim, query_vec_dim, gnd_vec_dim, std::ref(test_gnd_l), i);
    }

    for (auto& th : threads) {
        th.join();
    }

    single_query_time = query_record.get_elapsed_time_micro() / query_n_vec * 1e-3;

    double recall = count_recall(gnd_n_vec, gnd_vec_dim, test_gnd_l, gnd);
    printf("average recall parallel: %.3f, single query time %.4f ms\n", recall, single_query_time);

    return 0;
}
