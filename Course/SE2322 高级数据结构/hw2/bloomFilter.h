#pragma once
#include <stdint.h>
#include <vector>
#include "MurmurHash3.h"

class bloomFilter {
private:
    std::vector<bool> filter; // filter
    uint64_t k; // number of hash functions
    uint64_t m; // size of filter
public:
    bloomFilter(uint64_t k, uint64_t m) : k(k), m(m) {
        filter.resize(m); // initialize filter
        std::fill(filter.begin(), filter.end(), false);
    }
    ~bloomFilter() {}
    void insert(uint64_t key);
    bool find(uint64_t key);
};

void bloomFilter::insert(uint64_t key) {
    for (int i = 0; i < k; i++) {
        uint64_t hash[2] = {0};
        MurmurHash3_x64_128(&key, sizeof(key), 1, hash);
        key = key + m * 10;
        filter[hash[1] % m] = true;
    }
}

bool bloomFilter::find(uint64_t key) {
    for (int i = 0; i < k; i++) {
        uint64_t hash[2] = {0};
        MurmurHash3_x64_128(&key, sizeof(key), 1, hash);
        key = key + 1000;
        if (!filter[hash[1] % m]) {
            return false;
        }
    }
    return true;
}