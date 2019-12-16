#pragma once

#include <vector>
#include <utility>
#include <algorithm>

/**
 * Wrapper around std::vector
 * 
 * Defines operator (int, int) so we can access matrix positions by coordinate
 */

template <class T>
struct Matrix {
    using Container = std::vector<T>;

    Container arr;

    int height;
    int width;
    int size;

    Matrix()  = delete;
    ~Matrix() = default;

    Matrix(int h, int w) : height(h), width(w), size(h * w) {
        arr.resize(size);
    }

    inline T& operator()(int row, int col) { 
        return arr[row * width + col]; 
    }

    inline const T& operator()(int row, int col) const {
        return arr[row * width + col];
    }

    inline void operator=(const Matrix<T> &other) {
        arr = other.arr;
    }
};

// template <class T>
// struct Matrix {
//     using ArrayType = T **;

//     ArrayType arr;
    
//     int height;
//     int width;
//     int size;

//     Matrix() = delete;
//     ~Matrix() {
//         delete[] arr[0];
//         delete[] arr;
//     }

//     explicit Matrix(const int h, const int w) : height(h), width(w), size(h * w) {
//         arr = new T *[height];
//         arr[0] = new T[height * width];
//         for (int i = 1; i != height; ++i)
//             arr[i] = &arr[0][i * width];
//     }

//     inline T &operator()(int row, int col) { 
//         return arr[row][col]; 
//     }

//     inline const T &operator()(int row, int col) const {
//         return arr[row][col];
//     }

//     inline void operator=(const Matrix<T> &other) {
//         #pragma omp parallel for simd collapse(2)
//         for (int i = 0; i != height; ++i)
//             for (int j = 0; j != width; ++j) 
//                 arr[i][j] = std::forward<const T>(other(i,j));
//     }
// };