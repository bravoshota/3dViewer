#pragma once

#include "common.h"
#include <QString>
#include <vector>

int getStlFileFormat(const QString &path);
bool openStlBin(char *filename, std::vector<Vertex> &vertices, std::vector<Triangle> &faces);
bool openStlAsc(char *filename, std::vector<Vertex> &vertices, std::vector<Triangle> &faces);
