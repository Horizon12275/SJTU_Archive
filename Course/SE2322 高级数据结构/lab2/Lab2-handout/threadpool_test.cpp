#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include "src/hnsw.hpp"
#include "util/util.hpp"
#include "util/vecs_io.hpp"
#include "util/ground_truth.hpp"
#include "util/parameter.hpp"

using namespace std;
using namespace HNSWLab;

int main()
{
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
    double single_insert_time;
    TimeRecord insert_record;

    for (int i = 0; i < base_n_vec; i++)
    {
        hnsw.insert(base + base_vec_dim * i, i);

        if (i % report_every == 0)
        {
            insert_record.reset();
        }
    }

    single_insert_time = insert_record.get_elapsed_time_micro() / base_n_vec * 1e-3;
    printf("average insert time: %.3f ms\n", single_insert_time);

    printf("querying\n");
    vector<vector<int>> test_gnd_l(gnd_n_vec);
    double single_query_time;
    TimeRecord query_record;

    int num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads(num_threads);

    for (int t = 0; t < num_threads; t++)
    {
        threads[t] = std::thread([&](int start, int end)
                                 {
            for (int i = start; i < end; i++)
            {
                test_gnd_l[i] = hnsw.query(query + i * query_vec_dim, gnd_vec_dim);
            } }, t * gnd_n_vec / num_threads, (t + 1) * gnd_n_vec / num_threads);
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    single_query_time = query_record.get_elapsed_time_micro() / query_n_vec * 1e-3;

    double recall = count_recall(gnd_n_vec, gnd_vec_dim, test_gnd_l, gnd);
    printf("average recall: %.3f, single query time %.3f ms\n", recall, single_query_time);
    return 0;
}