#include "common.h"
#include <math.h>

namespace common {

Matrix::Matrix()
{
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            coord[i][j] = 0.0;
}

Vertex::Vertex() : x(0), y(0), z(0)
{
}

Vertex::Vertex(double xp, double yp, double zp)
    : x(xp)
    , y(yp)
    , z(zp)
{
}

Vertex::Vertex(float *coords)
    : x(static_cast<double>(coords[0]))
    , y(static_cast<double>(coords[1]))
    , z(static_cast<double>(coords[2]))
{
}

Vertex Vertex::operator +(const Vertex &other) const
{
    return {x + other.x, y + other.y, z + other.z};
}

Vertex Vertex::operator -(const Vertex &other) const
{
    return {x - other.x, y - other.y, z - other.z};
}

Vertex Vertex::operator *(double val) const
{
    return {x * val, y * val, z * val};
}

Vertex Vertex::operator *(const Matrix &mat) const
{
    Vertex res;
    res.x = x * mat.coord[0][0] + y * mat.coord[0][1] + z * mat.coord[0][2];
    res.y = x * mat.coord[1][0] + y * mat.coord[1][1] + z * mat.coord[1][2];
    res.z = x * mat.coord[2][0] + y * mat.coord[2][1] + z * mat.coord[2][2];
    return res;
}

Vertex Vertex::operator /(double val) const
{
    return {x / val, y / val, z / val};
}

Vertex &Vertex::operator +=(const Vertex &other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

Vertex &Vertex::operator -=(const Vertex &other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

Vertex &Vertex::operator *=(double val)
{
    x *= val;
    y *= val;
    z *= val;
    return *this;
}

Vertex &Vertex::operator /=(double val)
{
    x /= val;
    y /= val;
    z /= val;
    return *this;
}

std::ostream& operator<<(std::ostream& out, const Vertex &p) {
    out << p.x << " " << p.y << " " << p.z;
    return out;
}

Vector::Vector(const Vertex &v1, const Vertex &v2)
{
    x = v2.x - v1.x;
    y = v2.y - v1.y;
    z = v2.z - v1.z;
}

Vector Vector::operator *(double val)
{
    return {x * val, y * val, z * val};
}

// dot product
double Vector::operator*(const Vector &other) const
{
    return x*other.x + y*other.y + z*other.z;
}

// cross product
Vector Vector::operator %(const Vector &other) const
{
    return {y*other.z - z*other.y, z*other.x - x*other.z, x*other.y - y*other.x};
}

double Vector::length() const
{
    return sqrt(x * x + y * y + z * z);
}

Triangle::Triangle(uint32_t i1, uint32_t i2, uint32_t i3)
{
    coord[0] = i1;
    coord[1] = i2;
    coord[2] = i3;
}

Edge::Edge(unsigned int i1, unsigned int i2)
{
    coord[0] = i1;
    coord[1] = i2;
}

}
