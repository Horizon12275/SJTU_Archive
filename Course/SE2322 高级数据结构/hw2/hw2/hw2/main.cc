#include <iostream>
#include "MurmurHash3.h"

int main() {
    uint64_t key = 522123456789;
    uint64_t hash[2] = {0};

    MurmurHash3_x64_128(&key, sizeof(key), 1, hash);

    std::cout << "Hash value: ";
    for(int i = 0; i < 2; i++) {
        std::cout << hash[i] << " ";
    }
    std::cout << std::endl;
    //mod m-1
    //only use hash[1] 
    int m = 100;
    std::cout << hash[1]%(m-1) << " ";
    
    return 0;
}
