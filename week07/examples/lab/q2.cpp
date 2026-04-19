#include <iostream>

using namespace std;

struct stuinfo
{
    string name;
    int age;
};

template <typename T>
int Compare(const T &a, const T &b)
{
    if (a == b)
    {
        return 0;
    }
    else if (a > b)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

template<>
int Compare<stuinfo>(const stuinfo &s1, const stuinfo &s2)
{
    if (s1.age == s2.age)
    {
        return 0;
    }
    else if (s1.age > s2.age)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

template int Compare<int>(const int&, const int&);

int main()
{
    int i1 = 1, i2 = 2;
    float f1 = 1.2f, f2 = 2.1f;
    char c1 = 'a', c2 = 'A';
    stuinfo s1 = {"Anna", 18}, s2 = {"Bob", 20};
    cout << Compare(i1, i2) << endl;
    cout << Compare(f1, f2) << endl;
    cout << Compare(c1, c2) << endl;
    cout << Compare(s1, s2) << endl;
}