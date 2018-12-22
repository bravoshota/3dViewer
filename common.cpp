#include "common.h"
namespace common {

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

std::ostream& operator<<(std::ostream& out, const Vertex &p) {
    out << p.x << " " << p.y << " " << p.z;
    return out;
}

// dot product
double Vector::operator*(const Vector &other) const
{
    return x*other.x + y*other.y + z*other.z;
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
