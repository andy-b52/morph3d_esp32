#pragma once

#include <vector>

const int POINTS_PER_MODEL = 256;

struct Point3D {
    float x, y, z;
};

using Model = std::vector<Point3D>;

using Models = std::vector<Model>;

const Models& getModels();
