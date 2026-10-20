#include "DrawUtils.h"
#include <math.h>
#include <algorithm>
#include "Models.h"

uint8_t frameBuf[PIXELS];

void setPixel(int x, int y, uint8_t c) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) {
        return;
    }

    frameBuf[y * WIDTH + x] = c;
}

void setPixel3x3(int x, int y, uint8_t c) {
    // Loop through a -1 to +1 grid offset around the center (x, y)
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            int targetX = x + dx;
            int targetY = y + dy;

            // Strict bounds check before modifying the frame buffer
            if (targetX >= 0 && targetX < WIDTH && targetY >= 0 &&
                targetY < HEIGHT) {
                frameBuf[targetY * WIDTH + targetX] = c;
            }
        }
    }
}

void setPixel2x2(int x, int y, uint8_t c) {
    // Loop through a 2x2 grid where dx and dy are 0 and 1
    for (int dy = 0; dy <= 1; ++dy) {
        for (int dx = 0; dx <= 1; ++dx) {
            int targetX = x + dx;
            int targetY = y + dy;

            // Strict bounds check against your frame buffer dimensions
            if (targetX >= 0 && targetX < WIDTH && targetY >= 0 &&
                targetY < HEIGHT) {
                frameBuf[targetY * WIDTH + targetX] = c;
            }
        }
    }
}

void clearFrame() { std::fill(std::begin(frameBuf), std::end(frameBuf), 0); }

void fadeOutFrame() {
    for (uint32_t i = 0; i < FRAME_BYTES; i++) {
        uint8_t v = frameBuf[i];

        if (v > 32) {
            frameBuf[i] = v - 32;
        } else {
            frameBuf[i] = 0;
        }
    }
}

void makePerspective(float x, float y, float z,
                     float& newX, float& newY) {

    const float d = 200.0 * 4096.0;
    const float cameraOffset = 100.0 * 4096.0;

    float divisor = z + d + cameraOffset;

    newX = 1.5 * x * d / divisor;
    newY = 1.5 * y * d / divisor;
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

    for (const Point3D& pt : m){
        float x0 = pt.x;  // * (1.0f / 4096.0f);
        float y0 = pt.y;  // * (1.0f / 4096.0f);
        float z0 = pt.z;  // * (1.0f / 4096.0f);

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

        rotatedModel.emplace_back(Point3D{static_cast<int32_t>(newX + 0.5f),
                                          static_cast<int32_t>(newY + 0.5f),
                                          static_cast<int32_t>(z3 + 0.5f)});
    }
}

void morphModels(const Model& srcModel, const Model& dstModel, float stage,
                 Model& resModel) {
    if (srcModel.size() != POINTS_PER_MODEL) {
        return;
    }

    if (srcModel.size() != dstModel.size()) {
        return;
    }

    if (!resModel.empty()) {
        resModel.clear();
    }

    float s = std::min(stage, 1.0f);
    s = std::max(s, 0.0f);

    for (int i = 0; i < POINTS_PER_MODEL; i++) {
        float srcX = static_cast<float>(srcModel[i].x);
        float srcY = static_cast<float>(srcModel[i].y);
        float srcZ = static_cast<float>(srcModel[i].z);

        float dstX = static_cast<float>(dstModel[i].x);
        float dstY = static_cast<float>(dstModel[i].y);
        float dstZ = static_cast<float>(dstModel[i].z);

        resModel.emplace_back(
            Point3D{static_cast<int32_t>(srcX + s * (dstX - srcX) + 0.5f),
                    static_cast<int32_t>(srcY + s * (dstY - srcY) + 0.5f),
                    static_cast<int32_t>(srcZ + s * (dstZ - srcZ) + 0.5f)});
    }
}

void drawModel(Model& m) {
    std::sort(m.begin(), m.end(),
              [](const Point3D& a, const Point3D& b) { return a.z > b.z; });

    int32_t minZ = m.front().z;  // Farthest away
    int32_t maxZ = m.back().z;   // Closest to the camera
    int32_t zRange = maxZ - minZ;

    if (zRange == 0) zRange = 1;

    for (const auto& pt : m) {
        int px = (pt.x >> 12) + (WIDTH / 2);
        int py = (pt.y >> 12) + (HEIGHT / 2);

        // Ensure we are inside frame bounds before rendering
        if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {

            // Ranges from 0 (farthest) to zRange (closest)
            int32_t relativeZ = pt.z - minZ;

            // Map linearly to a 64-255 brightness scale
            uint8_t brightness =
                static_cast<uint8_t>(((relativeZ * 191) / zRange) + 64);

            setPixel2x2(px, py, brightness);
            // setPixel(px, py, brightness);
        }
    }
}

void drawFrame(uint32_t frameNumber) {

    if (models.size() < 3) {
        return;
    }

    static Model morphedModel;
    if (morphedModel.capacity() == 0) {
        morphedModel.reserve(POINTS_PER_MODEL);
    }

    static Model rotatedModel;
    if (rotatedModel.capacity() == 0) {
        rotatedModel.reserve(POINTS_PER_MODEL);
    }

    // main logic
    fadeOutFrame();

    static uint32_t noMorphFrameCnt = 0;
    static uint32_t morphFrameCnt = 0;
    static bool inMorph = false;
    static int srcModelIndex = -1;
    static int dstModelIndex = -1;

    if (!inMorph && noMorphFrameCnt == NO_MORPH_STEP_CNT) {
        inMorph = true;
        noMorphFrameCnt = 0;        
        morphFrameCnt = 0;
        do {
            dstModelIndex = rand() % MODEL_COUNT;
        } while (dstModelIndex == srcModelIndex);
    } else if (inMorph && morphFrameCnt == MORPH_STEP_CNT) {
        inMorph = false;
        morphFrameCnt = 0;
        noMorphFrameCnt = 0;
        srcModelIndex = dstModelIndex;
    }

    if (!inMorph) {
        if (srcModelIndex == -1) {
            srcModelIndex = rand() % MODEL_COUNT;
        }

        Model& srcModel = models[srcModelIndex];

        rotateModel(frameNumber, srcModel, rotatedModel);

        noMorphFrameCnt++;
    } else {
        Model& srcModel = models[srcModelIndex];
        Model& dstModel = models[dstModelIndex];

        morphModels(srcModel, dstModel, (float)(morphFrameCnt % MORPH_STEP_CNT) / (float)MORPH_STEP_CNT,
                    morphedModel);

        rotateModel(frameNumber, morphedModel, rotatedModel);

        morphFrameCnt++;
    }

    drawModel(rotatedModel);
}

void encodeFrame(std::vector<uint16_t>& v) {
    v.clear();
    if (FRAME_BYTES == 0) return;

    uint8_t lastColor = frameBuf[0];
    uint8_t packetSize = 1;

    for (size_t offset = 1; offset < FRAME_BYTES; offset++) {
        uint8_t currentColor = frameBuf[offset];

        // If color changes OR we hit the 8-bit packet limit, flush the packet
        if (currentColor != lastColor || packetSize == 255) {
            v.push_back(static_cast<uint16_t>(lastColor | (packetSize << 8)));
            lastColor = currentColor;
            packetSize = 1;
        } else {
            packetSize++;
        }
    }

    // Always cleanly flush exactly one final packet at the very end
    v.push_back(static_cast<uint16_t>(lastColor | (packetSize << 8)));
}
