#include <iostream>
using namespace std;

int f(int n)
{
    if (n == 1)
        return 1;
    else
        return n * f(n - 1);
}

int main()
{
    cout << f(100) << endl;
}

// g++ test2.cpp -o test2 -lgmpxx -lgmp
// time ./test2