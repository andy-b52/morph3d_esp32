#pragma once

#include <cstddef>
#include <cstdint>

const int WIDTH = 320;
const int HEIGHT = 240;
const size_t PIXELS = HEIGHT * WIDTH;

const size_t FRAME_BYTES = PIXELS * sizeof(uint8_t);

// 1 byte per pixel, greyscale palette, 0 - black, 255 - white.
// Allocated on the heap by allocFrameBuffer(): a static array this large
// does not fit in the ESP32's fixed static DRAM region.
extern uint8_t* frameBuf;

// Allocates and clears frameBuf. Returns false if there is not enough memory.
bool allocFrameBuffer();

void setPixel(int x, int y, uint8_t c);
void setPixel2x2(int x, int y, uint8_t c);
void setPixel3x3(int x, int y, uint8_t c);

void clearFrame();
void fadeOutFrame();
