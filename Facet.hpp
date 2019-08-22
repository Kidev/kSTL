#pragma once

#include <array>

namespace kSTL {

class Facet {

public:

    typedef std::array<float, 3> Point;
    typedef std::array<char, 2> Tabbing;

    Facet(Point A, Point B, Point C, Point N, Tabbing T);

    void translate(float x, float y, float z);

    void scale(float x, float y, float z);

    void rotateZYX(float x, float y, float z);

    const Point* get_corner(const std::size_t ci) const;

    const Point* get_normal() const;

    float get_min(const std::size_t ci) const;

    float get_max(const std::size_t ci) const;

private:

    void compute_normal();

    void rotateX(float x);

    void rotateY(float y);

    void rotateZ(float z);

    void rotate_point(float& x, float& y, float angle_u);

    std::array<Point, 3> mPoints;
    Point mNormal;
    Tabbing mTabbing;
    Point mMin = { std::numeric_limits<float>::max() };
    Point mMax = { std::numeric_limits<float>::min() };

};

}
