#pragma once

#include "common.h"
#include <QString>
#include <vector>

int getStlFileFormat(const QString &path);
bool ascStl2Off(char *filename, std::vector<Vertex> &vertices, std::vector<Triangle> &faces);
bool binStl2Off(char *filename, std::vector<Vertex> &vertices, std::vector<Triangle> &faces);
