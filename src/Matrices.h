#ifndef MATRICES_H
#define MATRICES_H

#include <math.h>
#include <vector>

#pragma once
#include <initializer_list>
#include <cassert>

struct Mat3x3 {
    double data[9];  // row-major: data[i * 3 + j]

    // Default constructor: zero-initialized
    Mat3x3() {
        for (int i = 0; i < 9; ++i) data[i] = 0.0;
    }

    // Initializer list constructor
    Mat3x3(std::initializer_list<double> list) {
        assert(list.size() == 9 && "Mat3x3 requires 9 elements");
        int i = 0;
        for (auto val : list) data[i++] = val;
    }

    // Copy constructor and assignment operator (defaulted)
    Mat3x3(const Mat3x3&) = default;
    Mat3x3& operator=(const Mat3x3&) = default;

    // Arithmetic operators
    Mat3x3 operator*(double n) const {
        Mat3x3 result;
        for (int i = 0; i < 9; ++i) result.data[i] = data[i] * n;
        return result;
    }

    Mat3x3 operator+(const Mat3x3& M) const {
        Mat3x3 result;
        for (int i = 0; i < 9; ++i) result.data[i] = data[i] + M.data[i];
        return result;
    }

    Mat3x3 operator-(const Mat3x3& M) const {
        Mat3x3 result;
        for (int i = 0; i < 9; ++i) result.data[i] = data[i] - M.data[i];
        return result;
    }

    Mat3x3 operator-() const {
        Mat3x3 result;
        for (int i = 0; i < 9; ++i) result.data[i] = -data[i];
        return result;
    }

    // Access operators
    double& operator()(int i, int j) {
        assert(i >= 0 && i < 3 && j >= 0 && j < 3);
        return data[i * 3 + j];
    }

    double operator()(int i, int j) const {
        assert(i >= 0 && i < 3 && j >= 0 && j < 3);
        return data[i * 3 + j];
    }

    double& operator[](int index) {
        assert(index >= 0 && index < 9);
        return data[index];
    }

    double operator[](int index) const {
        assert(index >= 0 && index < 9);
        return data[index];
    }

    static Mat3x3 identity() {
        return Mat3x3{
            1.0, 0.0, 0.0,
            0.0, 1.0, 0.0,
            0.0, 0.0, 1.0
        };
    }
};


struct Mat {
	std::vector<double> data;

	Mat(int rows, int cols) : data(rows* cols, 0.0) {}



	~Mat() {}



};



#endif// MATRICES_H