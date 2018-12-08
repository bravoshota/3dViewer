#pragma once

#include "common.h"
#include <QString>
#include <vector>

int getStlFileFormat(const QString &path);
bool openStlBin(char *filename, std::vector<common::Vertex> &vertices,
                std::vector<common::Triangle> &faces);
bool openStlAsc(char *filename, std::vector<common::Vertex> &vertices,
                std::vector<common::Triangle> &faces);
