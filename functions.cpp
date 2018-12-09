#include "common.h"
#include "functions.h"
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QByteArray>
#include <fstream>
#include <float.h>

// define the type of STL file (ascii or binary)
int getStlFileFormat(const QString &path)
{
    // input variables:
    // path - the path of file to load

    // get the file
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        // can't open, return error
        qDebug("\n\tUnable to open \"%s\"", qPrintable(path));
        return STL_INVALID;
    }
    // get the size of file (bytes)
    QFileInfo fileInfo(path);
    size_t fileSize = static_cast<size_t>(fileInfo.size());

    // check the size of file
    if (fileSize < 15)
    {
        // the size is too short, return error
        qDebug("\n\tThe STL file is not long enough (%u bytes).", uint(fileSize));
        return STL_INVALID;
    }

    // check the header of ascii STL
    QByteArray sixBytes = file.read(6);
    if (sixBytes.startsWith("solid "))
    {
        // loop over lines of file
        QByteArray buf;
        QByteArray prev;
        do
        {
            prev = buf;
            buf = file.readLine();
        } while (!buf.isEmpty());
        // check the ending of ascii STL
        if (prev.startsWith("endsolid"))
            // header and ending are proper, return ascii type
            return STL_ASCII;
    }

    // come back to start of file
    if (!file.reset())
    {
        // can't do, return error
        qDebug("\n\tCannot seek to the 0th byte (before the header)");
        return STL_INVALID;
    }

    // check the size of file (the minimum is longer for binary)
    if (fileSize < 84)
    {
        // the size is too short, return error
        qDebug("\n\tThe STL file is not long enough (%u bytes).", uint(fileSize));
        return STL_INVALID;
    }

    // skip the header of binary STL
    if (!file.seek(80))
    {
        // can't do, return error
        qDebug("\n\tCannot seek to the 80th byte (after the header)");
        return STL_INVALID;
    }

    // read the number of facets
    QByteArray nTrianglesBytes = file.read(4);
    if (nTrianglesBytes.size() != 4)
    {
        // can't do, return error
        qDebug("\n\tCannot read the number of triangles (after the header)");
        return STL_INVALID;
    }

    // convert to int
    uint32_t nTriangles = *reinterpret_cast<uint32_t*>(nTrianglesBytes.data());
    // check the length of file (header + facets)
    if (fileSize == (84 + (nTriangles * 50)))
        // if proper return binary type
        return STL_BINARY;

    // not proper, return error
    return STL_INVALID;
}

// open STL asci file format
bool openStlAsc(char *filename, std::vector<common::Vertex> &vertices,
                std::vector<common::Triangle> &faces)
{
    // input variables:
    // filename - the path of file to load

    // open the file
    std::ifstream obj(filename, std::ios::in);
    // define the maximum size of line to process
    const int buffsize = 100;
    char str[buffsize];

    // read the first line
    obj.getline(str, buffsize);

    // loop over the lines of file
    while (obj.getline(str, buffsize))
    {
        QString tstr = QString(str).trimmed().toLower();
        // check do we find the start of new facet
        if (tstr.left(5) == "facet")
        {
            // initiate the new facet
            unsigned int index[3];
            // skip the line with normal data
            obj.getline(str, buffsize);

            // loop over facet's vertices
            for (int j = 0; j < 3; j++)
            {
                // read the line with vertex data
                obj.getline(str, buffsize);
                tstr = QString(str).trimmed().toLower();

                // split the buffer by spaces
                QStringList entities = tstr.split(' ');
                if (entities.size() != 4 || entities[0] != "vertex")
                    return false;

                bool ok[3];
                double x = entities[1].toDouble(&ok[0]);
                double y = entities[2].toDouble(&ok[1]);
                double z = entities[3].toDouble(&ok[2]);
                if (!ok[0] || !ok[1] || !ok[2])
                    return false;

                // initiate the checker of the vertex is already exist
                bool found_close_point = false;
                // loop over previous vertices
                for (size_t k = 0; k < vertices.size(); k++)
                {
                    // check does the distance by X axis lower than eps (little constant)
                    double dx = fabs(x - vertices[k].x);
                    if (dx < 1E-5)
                    {
                        // check does the distance by Y axis lower than eps (little constant)
                        double dy = fabs(y - vertices[k].y);
                        if (dy < 1E-5)
                        {
                            // check does the distance by Z axis lower than eps (little constant)
                            double dz = fabs(z - vertices[k].z);
                            if (dz < 1E-5)
                            {
                                // the current vertex is close to existing one, set it into facet
                                index[j] = k;
                                found_close_point = true;
                                break;
                            }
                        }
                    }
                }

                // the current vertex is not close to any existing
                if (!found_close_point)
                {
                    // set the index of new vertex into facet
                    index[j] = vertices.size();
                    // add it to array
                    vertices.push_back({x, y, z});
                }
            }

            // add new facet into array
            faces.push_back({index[0], index[1], index[2]});
            // skip the end of facet
            obj.getline(str, buffsize);
            obj.getline(str, buffsize);
        }
    }

    return true;
}

// convert binary STL to OFF
bool openStlBin(char *filename, std::vector<common::Vertex> &vertices,
                std::vector<common::Triangle> &faces)
{
    // input variables:
    // filename - the path of file to load

    // open the file
    std::ifstream obj(filename, std::ios::in | std::ios::binary);
    // skip the header
    for (int i = 0; i < 80; i++) {
        uint8_t c;
        obj.read(reinterpret_cast<char*>(&c), sizeof(c));
    }

    // read the number of facets
    uint32_t N32;
    obj.read(reinterpret_cast<char*>(&N32), sizeof(N32));
    unsigned int N = N32;

    // initiate the arrays of vertices and facets
    faces.reserve(N);

    // loop over facets
    for (unsigned int i = 0; i < N; i++) {
        // initiate the new normal
        float normal[3];
        // read the floats (normal coordinates) from binary
        obj.read(reinterpret_cast<char*>(normal), sizeof(normal));

        // initiate the new facet
        unsigned int index[3];
        // loop over facet's vertices
        for (int j = 0; j < 3; j++) {
            // read the floats (vertex coordinates) from binary
            float coords[3];
            obj.read(reinterpret_cast<char*>(coords), sizeof(coords));

            // initiate the checker of the vertex is already exist
            bool found_close_point = false;
            // loop over previous vertices
            for (size_t k = 0; k < vertices.size(); k++) {
                // check does the distance by X axis lower than eps (little constant)
                double dx = fabs(static_cast<double>(coords[0]) - vertices[k].x);
                if (dx < 1E-5) {
                    // check does the distance by Y axis lower than eps (little constant)
                    double dy = fabs(static_cast<double>(coords[1]) - vertices[k].y);
                    if (dy < 1E-5) {
                        // check does the distance by Z axis lower than eps (little constant)
                        double dz = fabs(static_cast<double>(coords[2]) - vertices[k].z);
                        if (dz < 1E-5) {
                            // the current vertex is close to existing one, set it into facet
                            index[j] = k;
                            found_close_point = true;
                            break;
                        }
                    }
                }
            }

            // the current vertex is not close to any existing
            if (!found_close_point) {
                // set the index of new vertex into facet
                index[j] = vertices.size();
                // add it to array
                vertices.push_back(coords);
            }
        }

        // add new facet into array
        faces.push_back({index[0], index[1], index[2]});
        // skip the end of facet
        char c[2];
        obj.read(reinterpret_cast<char*>(c), sizeof(c));
    }

    return true;
}

bool normalize(common::Vector &nor)
{
    double magn = sqrt(nor.x*nor.x + nor.y*nor.y + nor.z*nor.z);
    if (magn <= DBL_EPSILON)
        return false;

    nor.x /= magn;
    nor.y /= magn;
    nor.z /= magn;
    return true;
}

bool calculateNormal(const common::Vertex &v1,
                     const common::Vertex &v2,
                     const common::Vertex &v3,
                     common::Vector &nor)
{
    double xVect1 = v2.x - v1.x;
    double yVect1 = v2.y - v1.y;
    double zVect1 = v2.z - v1.z;
    double xVect2 = v3.x - v1.x;
    double yVect2 = v3.y - v1.y;
    double zVect2 = v3.z - v1.z;

    nor.x = yVect1*zVect2 - zVect1*yVect2;
    nor.y = zVect1*xVect2 - xVect1*zVect2;
    nor.z = xVect1*yVect2 - yVect1*xVect2;

    return normalize(nor);
}
