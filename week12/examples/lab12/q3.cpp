// dynamic allocation and polymorphism
#include <iostream>
using namespace std;
class Polygon
{
protected:
    int width, height;

public:
    Polygon(int a, int b) : width(a), height(b) {}
    virtual int area(void) = 0;
    // virtual ~Polygon() {}
    ~Polygon() { 
        cout << "Polygon (Base) destructor called." << endl; 
    }
    void printarea()
    {
        cout << this->area() << '\n';
    }
};
class Rectangle : public Polygon
{
public:
    Rectangle(int a, int b) : Polygon(a, b) {}
    int area()
    {
        return width * height;
    }
    // ~Rectangle() { 
    //     cout << "Rectangle (Derived) destructor called!" << endl; 
    // }
};
class Triangle : public Polygon
{
public:
    Triangle(int a, int b) : Polygon(a, b) {}
    int area()
    {
        return width * height / 2;
    }
};
int main()
{
    Polygon *ppoly = new Rectangle(4, 5);
    ppoly->printarea();
    delete ppoly;
    ppoly = new Triangle(2, 5);
    ppoly->printarea();
    delete ppoly;
    return 0;
}