#include "Renderer.h"
#include <algorithm>
#include <cmath>
#include "FrameBuffer.h"

namespace {
Model rotated;  // scratch: rotated + projected points

void makePerspective(float x, float y, float z, float& newX, float& newY) {

    const float d = 200.0f;
    const float cameraOffset = 100.0f;

    float divisor = z + d + cameraOffset;

    newX = 1.5f * x * d / divisor;
    newY = 1.5f * y * d / divisor;
}

void rotateModel(uint32_t frameNumber, const Model& m, Model& rotatedModel) {
    if (!rotatedModel.empty()) {
        rotatedModel.clear();
    }

    // calculate angels
    float a1 = (2.0f * (float)M_PI * (float)frameNumber) / 512.0f;
    float a2 = 2.0f * a1;
    float a3 = 3.0f * a1;

    // calculate sin, cos
    float cos1 = cosf(a1), sin1 = sinf(a1);
    float cos2 = cosf(a2), sin2 = sinf(a2);
    float cos3 = cosf(a3), sin3 = sinf(a3);

    float newX, newY;

    for (const Point3D& pt : m) {
        float x0 = pt.x;
        float y0 = pt.y;
        float z0 = pt.z;

        // rotate around axis z
        float x1 = x0 * cos1 - y0 * sin1;
        float y1 = x0 * sin1 + y0 * cos1;
        float z1 = z0;

        // rotate around axis y
        float x2 = x1 * cos2 - z1 * sin2;
        float z2 = x1 * sin2 + z1 * cos2;
        float y2 = y1;

        // rotate around axis x
        float y3 = y2 * cos3 - z2 * sin3;
        float z3 = y2 * sin3 + z2 * cos3;
        float x3 = x2;

        makePerspective(x3, y3, z3, newX, newY);

        rotatedModel.emplace_back(
            Point3D{newX, newY, z3});
    }
}

void drawModel(Model& m) {
    std::sort(m.begin(), m.end(),
              [](const Point3D& a, const Point3D& b) { return a.z > b.z; });

    float minZ = m.front().z;  // Farthest away
    float maxZ = m.back().z;   // Closest to the camera
    float zRange = maxZ - minZ;

    if (zRange == 0) zRange = 1;

    for (const auto& pt : m) {
        int px = static_cast<int>(std::lround(pt.x)) + WIDTH / 2;
        int py = static_cast<int>(std::lround(pt.y)) + HEIGHT / 2;

        // Ensure we are inside frame bounds before rendering
        if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {

            // Ranges from 0 (farthest) to zRange (closest)
            float relativeZ = pt.z - minZ;

            // Map linearly to a 64-255 brightness scale
            uint8_t brightness =
                static_cast<uint8_t>(((relativeZ * 191.0f) / zRange) + 64.0f);

            setPixel2x2(px, py, brightness);
            // setPixel(px, py, brightness);
        }
    }
}
}  // namespace

void renderModel(const Model& model, uint32_t frameNumber) {
    rotateModel(frameNumber, model, rotated);
    drawModel(rotated);
}
