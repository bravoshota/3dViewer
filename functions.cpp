#include "common.h"
#include "functions.h"
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QVector>
#include <QByteArray>
#include <QDataStream>
#include <QMessageBox>
#include <boost/cstdint.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>

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
    size_t fileSize = fileInfo.size();

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
    uint32_t nTriangles = *((uint32_t*)nTrianglesBytes.data());
    // check the length of file (header + facets)
    if (fileSize == (84 + (nTriangles * 50)))
        // if proper return binary type
        return STL_BINARY;

    // not proper, return error
    return STL_INVALID;
}

// convert ascii STL to OFF
bool ascStl2Off(char *filename, std::vector<Vertex> &vertices, std::vector<Triangle> &faces)
{
    // input variables:
    // filename - the path of file to load

    // open the file
    std::ifstream obj(filename, std::ios::in);
    // define the maximum size of line to process
    const int buffsize = 100;
    char str[buffsize];
    char tstr[buffsize];

    // initiate the arrays of vertices and facets
    unsigned int number_of_points = 0;
\
    // read the first line
    obj.getline(str, buffsize);

    // loop over the lines of file
    while (obj.getline(str, buffsize))
    {
        // count the leading spaces
        int b = 0;
        while (isspace(str[b])) b++;
        // skip the leading spaces and convert all chars to lowercase
        for (int tb = b; tb < buffsize; tb++) tstr[tb - b] = tolower(str[tb]);

        // set the rest of chars to space
        for (int tb = buffsize - b; tb < buffsize; tb++) tstr[tb] = ' ';

        // check do we find the start of new facet
        if (strncmp(tstr, "facet", 5) == 0)
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

                // count the leading spaces
                int b = 0;
                while (isspace(str[b])) b++;
                // skip the leading spaces and convert all chars to lowercase
                for (int tb = b; tb < buffsize; tb++) tstr[tb - b] = tolower(str[tb]);

                // set the rest of chars to space
                for (int tb = buffsize - b; tb < buffsize; tb++) tstr[tb] = ' ';

                // split the buffer by spaces
                std::vector<std::string> entities;
                boost::split(entities, tstr, boost::is_any_of(" "), boost::token_compress_on);
                // convert coordinates into float numbers (skip the "vertex" which has zero index)
                float x = stof(entities[1]);
                float y = stof(entities[2]);
                float z = stof(entities[3]);

                // initiate the checker of the vertex is already exist
                bool found_close_point = false;
                // loop over previous vertices
                for (size_t k = 0; k < vertices.size(); k++) {

                    // check does the distance by X axis lower than eps (little constant)
                    float dx = x - vertices[k].x;
                    if (dx > -1E-5 && dx < 1E-5) {
                        // check does the distance by Y axis lower than eps (little constant)
                        float dy = y - vertices[k].y;
                        if (dy > -1E-5 && dy < 1E-5) {
                            // check does the distance by Z axis lower than eps (little constant)
                            float dz = z - vertices[k].z;
                            if (dz > -1E-5 && dz < 1E-5) {
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
                    // create new vertex
                    Vertex p(x, y, z);
                    // add it to array
                    vertices.push_back(p);
                    // set the index of new vertex into facet
                    index[j] = number_of_points;
                    ++number_of_points;
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
bool binStl2Off(char *filename, std::vector<Vertex> &vertices, std::vector<Triangle> &faces)
{
    // input variables:
    // filename - the path of file to load

    // open the file
    std::ifstream obj(filename, std::ios::in | std::ios::binary);
    // skip the header
    for (int i = 0; i < 80; i++) {
        boost::uint8_t c;
        obj.read(reinterpret_cast<char*>(&c), sizeof(c));
    }

    // read the number of facets
    boost::uint32_t N32;
    obj.read(reinterpret_cast<char*>(&N32), sizeof(N32));
    unsigned int N = N32;

    // initiate the arrays of vertices and facets
    faces.reserve(N);
    unsigned int number_of_points = 0;

    // loop over facets
    for (int i = 0; i < (int)N; i++) {
        std::cout << i << std::endl;
        // initiate the new normal
        float normal[3];
        // read the floats (normal coordinates) from binary
        obj.read(reinterpret_cast<char*>(&normal[0]), sizeof(normal[0]));
        obj.read(reinterpret_cast<char*>(&normal[1]), sizeof(normal[1]));
        obj.read(reinterpret_cast<char*>(&normal[2]), sizeof(normal[2]));

        // initiate the new facet
        unsigned int index[3];
        // loop over facet's vertices
        for (int j = 0; j < 3; j++) {
            // read the floats (vertex coordinates) from binary
            float x, y, z;
            obj.read(reinterpret_cast<char*>(&x), sizeof(x));
            obj.read(reinterpret_cast<char*>(&y), sizeof(y));
            obj.read(reinterpret_cast<char*>(&z), sizeof(z));

            // initiate the checker of the vertex is already exist
            bool found_close_point = false;
            // loop over previous vertices
            for (size_t k = 0; k < vertices.size(); k++) {
                // check does the distance by X axis lower than eps (little constant)
                float dx = x - vertices[k].x;
                if (dx > -1E-5 && dx < 1E-5) {
                    // check does the distance by Y axis lower than eps (little constant)
                    float dy = y - vertices[k].y;
                    if (dy > -1E-5 && dy < 1E-5) {
                        // check does the distance by Z axis lower than eps (little constant)
                        float dz = z - vertices[k].z;
                        if (dz > -1E-5 && dz < 1E-5) {
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
                // create new vertex
                Vertex p(x, y, z);
                // add it to array
                vertices.push_back(p);
                // set the index of new vertex into facet
                index[j] = number_of_points;
                ++number_of_points;
            }
        }

        // add new facet into array
        faces.push_back({index[0], index[1], index[2]});
        // skip the end of facet
        char c;
        obj.read(reinterpret_cast<char*>(&c), sizeof(c));
        obj.read(reinterpret_cast<char*>(&c), sizeof(c));
    }

    return true;
}
