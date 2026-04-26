#include <iostream>
using namespace std;

class Complex
{
private:
    double real;
    double imag;
public:
    Complex(double r = 0.0, double i = 0.0) : real(r), imag(i) {}
    
    Complex operator+(const Complex& other) const
    {
        return Complex(real + other.real, imag + other.imag);
    }
    Complex operator-(const Complex& other) const
    {
        return Complex(real - other.real, imag - other.imag);
    }

    void display() const
    {
        cout << real << " + " << imag << "i" << endl;
    }
};

int main()
{
    Complex z1 = Complex(1.0);
    Complex z2(3.0, 4.0);
    z1.display();
    z2.display();
    Complex z3 = z1 + z2;
    Complex z4 = z1 - z2;
    z3.display();
    z4.display();
    Complex* z5 = new Complex();
    z5->display();
    delete z5;

    return 0;
}