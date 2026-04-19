#include <iostream>
#include "vabs.h"

using namespace std;

int main() {
    bool (*f1)(int *, size_t) = vabs; 
    bool (*f2)(float *, size_t) = vabs;
    bool (*f3)(double *, size_t) = vabs;
    cout << "address of func \"(int) vabs\": " << (void*)f1 << "\n";
    cout << "address of func \"(float) vabs\": " << (void*)f2 << "\n";
    cout << "address of func \"(double) vabs\": " << (void*)f3 << "\n";
}