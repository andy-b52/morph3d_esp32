#pragma once

#include <cstddef>
#include <cstdint>

const int WIDTH = 320;
const int HEIGHT = 240;
const size_t PIXELS = HEIGHT * WIDTH;

const size_t FRAME_BYTES = PIXELS * sizeof(uint8_t);

// 1 byte per pixel, greyscale palette, 0 - black, 255 - white
extern uint8_t frameBuf[PIXELS];

void setPixel(int x, int y, uint8_t c);
void setPixel2x2(int x, int y, uint8_t c);
void setPixel3x3(int x, int y, uint8_t c);

void clearFrame();
void fadeOutFrame();
