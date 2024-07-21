#include <iostream>
#include <iomanip>
#include "MurmurHash3.h"
#include "bloomFilter.h"

void test(uint64_t k, uint64_t m, uint64_t n);

int main()
{
    for (int i = 1; i <= 5; i++)
        for (int j = 2; j <= 5; j++)
            test(i, j * 1000000, 1000000);
    return 0;
}

void test(uint64_t k, uint64_t m, uint64_t n)
{
    uint64_t key = 0;     // key
    uint64_t cnt = 0;     // count of false positive
    bloomFilter bf(k, m); // bloom filter
    for (int i = 0; i < n; i++)
    {
        key = i;
        bf.insert(key);
    }
    for (int i = n; i < n + n; i++)
    {
        key = i;
        if (bf.find(key))
            cnt++;
    }
    std::cout << "When k = " << k << ", m = " << m << ", n = " << n << " ";
    std::cout << std::fixed << std::setprecision(3) << "False positive rate: " << (double)cnt / n << std::endl;
}

// for (int i = 0; i < 100; i++)
// {
//     key = i;
//     uint64_t hash[k] = {0};
//     MurmurHash3_x64_128(&key, sizeof(key), 1, hash);
//     std::cout << "Hash value of "<< i << ": ";
//     for(int j = 0; j < k; j++) {
//         std::cout << hash[j] << " ";
//     }
//     std::cout << std::endl;
// }

// key = 103122;
// uint64_t hash[6] = {0};
// MurmurHash3_x64_128(&key, sizeof(key), 1, hash);
// std::cout << "Hash value: ";
// for(int i = 0; i < 6; i++) {
//     std::cout << hash[i] << " ";
// }
// std::cout << std::endl;

// uint64_t key = 522123456789;
// uint64_t hash[2] = {0};

// MurmurHash3_x64_128(&key, sizeof(key), 1, hash);

// std::cout << "Hash value: ";
// for(int i = 0; i < 2; i++) {
//     std::cout << hash[i] << " ";
// }
// std::cout << std::endl;
// //mod m-1
// //only use hash[1]
// int m = 100;
// std::cout << hash[1]%(m-1) << " ";

// uint64_t key = 0;
// uint64_t k = 4;
// uint64_t n = 100;
// uint64_t m = 2 * n;