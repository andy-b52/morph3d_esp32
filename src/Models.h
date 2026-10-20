#include <stdint.h>
#include <vector>

const int POINTS_PER_MODEL = 256;
const uint8_t MODEL_COUNT = 3;

struct Point3D {
    int32_t x, y, z; // fixed point (multiplied by 4096) coordinates
};

using Model = std::vector<Point3D>;

using Models = std::vector<Model>;

extern Models models;

extern void createModels();
