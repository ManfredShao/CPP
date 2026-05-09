#include <iostream>

using namespace std;

class B;

class A
{
    public:
        weak_ptr<B> ptrB;
        A() { cout << "A constructor" << endl; }
        ~A() { cout << "A destructor" << endl; }
};

class B
{
    public:
        shared_ptr<A> ptrA;
        B() { cout << "B constructor" << endl; }
        ~B() { cout << "B destructor" << endl; }
};

int main()
{
    shared_ptr<A> a(new A());
    shared_ptr<B> b(new B());
    a->ptrB = b;
    b->ptrA = a;
    return 0;
}