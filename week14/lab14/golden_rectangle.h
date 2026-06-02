#pragma once
#include <cassert>

template <typename T>
class GoldenRectangle
{
private:
    T length;
    T width;
public:
    GoldenRectangle(T length, T width): length(length), width(width) {}

    bool isGolden() const {
        T shorter = length < width ? length : width;
        T longer = length > width ? length : width;
        assert(shorter != 0);
        double ratio = static_cast<double>(longer) / static_cast<double>(shorter);
        return ratio == 1.618;
    }
};