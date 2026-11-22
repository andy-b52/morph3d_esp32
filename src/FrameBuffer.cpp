#include <algorithm>
#include <cstdlib>
#include "FrameBuffer.h"

uint8_t* frameBuf = nullptr;

bool allocFrameBuffer() {
    frameBuf = static_cast<uint8_t*>(calloc(PIXELS, sizeof(uint8_t)));
    return frameBuf != nullptr;
}

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

void clearFrame() { std::fill(frameBuf, frameBuf + PIXELS, 0); }

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
