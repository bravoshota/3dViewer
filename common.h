#pragma once

#include <iostream>

// the constants of STL file types
#define STL_INVALID -1
#define STL_BINARY 0
#define STL_ASCII 1

namespace common {

struct Vertex {
    double x;
    double y;
    double z;

    Vertex();
    Vertex(double xp, double yp, double zp);
    Vertex(float *coords);

    Vertex operator + (const Vertex &other) const;
    Vertex operator * (double val) const;
    Vertex operator / (double val) const;
    Vertex &operator += (const Vertex &other);
    Vertex &operator *= (double val);
    Vertex &operator /= (double val);
};
std::ostream& operator<<(std::ostream& out, const Vertex &p);

struct Vector : Vertex
{
    Vector() : Vertex() {}
    Vector(double xp, double yp, double zp) : Vertex(xp, yp, zp) {}
    Vector(const Vertex &v1, const Vertex &v2);

    Vector operator * (double val);
    // dot product
    double operator *(const Vector &other) const;
    // cross product
    Vector operator %(const Vector &other) const;
};

struct Triangle
{
    Triangle(uint32_t i1, uint32_t i2, uint32_t i3);
    uint32_t coord[3];
};

struct Edge
{
    Edge(unsigned int i1, unsigned int i2);
    uint32_t coord[2];
};
}
