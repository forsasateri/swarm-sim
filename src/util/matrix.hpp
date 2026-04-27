#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <vector>

template<typename T>
class Matrix {

public:

    Matrix(size_t width, size_t height) : width(width), height(height), data(width * height) {}

    T& operator()(size_t x, size_t y) {
        return data[y * width + x];
    }

    const T& operator()(size_t x, size_t y) const {
        return data[y * width + x];
    }


    size_t getWidth() const { return width; }
    size_t getHeight() const { return height; }

private:
    size_t width, height;
    std::vector<T> data;

};

#endif // MATRIX_HPP