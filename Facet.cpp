#include "Facet.hpp"
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>

using namespace kSTL;

Facet::Facet(Point A, Point B, Point C, Point N, Tabbing T)
    : mPoints({A, B, C}), mNormal(N), mTabbing(T) {
    for (std::size_t j = 0; j < 3; ++j) {
        this->mMin[j] = std::min({ A[j], B[j], C[j] });
        this->mMax[j] = std::max({ A[j], B[j], C[j] });
    }
}

void Facet::translate(float x, float y, float z) {
    for (std::size_t i = 0; i < 3; ++i) {
        this->mPoints[i][0] += x;
        this->mPoints[i][1] += y;
        this->mPoints[i][2] += z;
    }
}

void Facet::scale(float x, float y, float z) {
    for (std::size_t i = 0; i < 3; ++i) {
        this->mPoints[i][0] *= x;
        this->mPoints[i][1] *= y;
        this->mPoints[i][2] *= z;
    }
}

void Facet::rotateZYX(float x, float y, float z) {
    this->rotateZ(z);
    this->rotateY(y);
    this->rotateZ(x);
    this->compute_normal();
}

const Facet::Point* Facet::get_corner(const std::size_t ci) const {
    return &this->mPoints[ci];
}

const Facet::Point* Facet::get_normal() const {
    return &this->mNormal;
}

float Facet::get_min(const std::size_t ci) const {
    return this->mMin[ci];
}

float Facet::get_max(const std::size_t ci) const {
    return this->mMax[ci];
}

void Facet::compute_normal() {
    float v1[3];
    float v2[3];

    Point A = this->mPoints[0];
    Point B = this->mPoints[1];
    Point C = this->mPoints[2];

    v1[0] = B[0] - A[0];
    v1[1] = B[1] - A[1];
    v1[2] = B[2] - A[2];
    v2[0] = C[0] - A[0];
    v2[1] = C[1] - A[1];
    v2[2] = C[2] - A[2];

    this->mNormal[0] = static_cast<float>((static_cast<double>(v1[1]) * static_cast<double>(v2[2])) - (static_cast<double>(v1[2]) * static_cast<double>(v2[1])));
    this->mNormal[1] = static_cast<float>((static_cast<double>(v1[2]) * static_cast<double>(v2[0])) - (static_cast<double>(v1[0]) * static_cast<double>(v2[2])));
    this->mNormal[2] = static_cast<float>((static_cast<double>(v1[0]) * static_cast<double>(v2[1])) - (static_cast<double>(v1[1]) * static_cast<double>(v2[0])));
}

void Facet::rotateX(float x) {
    for (std::size_t i = 0; i < 3; ++i) {
        this->rotate_point(this->mPoints[i][1],
                           this->mPoints[i][2], x);
    }
}

void Facet::rotateY(float y) {
    for (std::size_t i = 0; i < 3; ++i) {
        this->rotate_point(this->mPoints[i][2],
                           this->mPoints[i][0], y);
    }
}

void Facet::rotateZ(float z) {
    for (std::size_t i = 0; i < 3; ++i) {
        this->rotate_point(this->mPoints[i][0],
                           this->mPoints[i][1], z);
    }
}

void Facet::rotate_point(float& x, float& y, float angle_u) {
    double dx = static_cast<double>(x);
    double dy = static_cast<double>(y);
    double angle = static_cast<double>(angle_u);
    double angle_r = (angle / 180.0) * M_PI;
    double rad = sqrt((dx * dx) + (dy * dy));
    double theta = atan2(dy, dx);
    dx = rad * cos(theta + angle_r);
    dy = rad * sin(theta + angle_r);
    x = static_cast<float>(dx);
    y = static_cast<float>(dy);
}
