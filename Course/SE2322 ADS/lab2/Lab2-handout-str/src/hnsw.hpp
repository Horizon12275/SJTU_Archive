#pragma once
#include <iostream>
#include "base.hpp"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <random>
#include <cassert>
#include <cmath>
#include <algorithm>

namespace HNSWLab {

    class HNSW : public AlgorithmInterface {
    public:
        HNSW();
        void insert(const int *item, int label);
        std::vector<int> query(const int *query, int k);
        ~HNSW() {}

    private:
        
        int efConstruction;
      
        int max_level;
        int ep;  
       
        int enterpoint;
        std::mt19937 rng;
        std::unordered_map<int, std::vector<const int*>> nodes; // label to vectors
        std::unordered_map<int, std::unordered_map<int, std::vector<int>>> layers; // level -> label -> neighbors

        int get_random_level();
        std::vector<int> search_layer(const int *q, int ep, int ef, int lc);
        std::vector<int> select_neighbors(const int *q, const std::vector<int>& W, int M, int lc);
        double euclidean_distance(const int *a, const int *b);
        int get_enter_point_level(int ep);
    };
}
