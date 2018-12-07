#pragma once

#include <iostream>

// the constants of STL file types
#define STL_INVALID -1
#define STL_BINARY 0
#define STL_ASCII 1

// utility object (the vertex)
struct Vertex {
    double x;
    double y;
    double z;
    // initiate the point with coordinates
    Vertex();
    Vertex(double xp, double yp, double zp);
    Vertex(float *coords);
};

// output method of point
std::ostream& operator<<(std::ostream& out, const Vertex &p);

struct Triangle
{
    Triangle(unsigned int _i1, unsigned int _i2, unsigned int _i3);
    unsigned int i1;
    unsigned int i2;
    unsigned int i3;
};
