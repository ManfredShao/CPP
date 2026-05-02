#pragma once
#include <iostream>

class Complex
{
private:
    double real;
    double imag;

public:
    Complex(double r = 0.0, double i = 0.0) : real(r), imag(i) {}

    Complex operator+(const Complex &other) const
    {
        return Complex(real + other.real, imag + other.imag);
    }
    Complex operator-(const Complex &other) const
    {
        return Complex(real - other.real, imag - other.imag);
    }
    Complex operator~()
    {
        return *this=Complex(real, -imag);
    }
    Complex operator-(double scalar) const
    {
        return Complex(real - scalar, imag);
    }
    Complex operator*(const Complex &other) const
    {
        return Complex(real * other.real - imag * other.imag, real * other.imag + imag * other.real);
    }
    friend Complex operator*(double scalar, const Complex &c)
    {
        return Complex(scalar * c.real, scalar * c.imag);
    }

    bool operator==(const Complex &other) const
    {
        return real == other.real && imag == other.imag;
    }
    bool operator!=(const Complex &other) const
    {
        return !(*this == other);
    }
    Complex operator=(const Complex &other)
    {
        if (this != &other)
        {
            real = other.real;
            imag = other.imag;
        }
        return *this;
    }

    friend std::ostream &operator<<(std::ostream &os, const Complex &c)
    {
        os << c.real << (c.imag >= 0 ?  "+" : "") << c.imag << "i";
        return os;
    }

    friend std::istream &operator>>(std::istream &is, Complex &c)
    {
        is >> c.real >> c.imag;
        return is;
    }

};