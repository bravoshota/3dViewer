#include "common.h"

Vertex::Vertex() : x(0), y(0), z(0)
{
}

Vertex::Vertex(float xp, float yp, float zp)
    : x(xp)
    , y(yp)
    , z(zp)
{
}

std::ostream& operator<<(std::ostream& out, const Vertex &p) {
    out << p.x << " " << p.y << " " << p.z;
    return out;
}

Triangle::Triangle(unsigned int _i1, unsigned int _i2, unsigned int _i3)
    : i1(_i1)
    , i2(_i2)
    , i3(_i3)
{
}
