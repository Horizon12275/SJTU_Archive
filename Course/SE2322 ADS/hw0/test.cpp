#include <iostream>
#include <gmpxx.h>
using namespace std;

mpz_class f(int n){
    if (n == 1)
        return 1;
    else
        return n * f(n - 1);
}

int main()
{
    cout << f(100) << endl;
}

// g++ test.cpp -o test -lgmpxx -lgmp
// time ./test