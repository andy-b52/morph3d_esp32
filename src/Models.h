#pragma once

#include <stdint.h>
#include <vector>

const int POINTS_PER_MODEL = 256;
const uint8_t MODEL_COUNT = 3;

struct Point3D {
    float x, y, z;
};

using Model = std::vector<Point3D>;

using Models = std::vector<Model>;

extern Models models;

void createModels();
