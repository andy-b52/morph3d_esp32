#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

const int WIDTH = 320;
const int HEIGHT = 240;
const size_t PIXELS = HEIGHT * WIDTH;

const size_t FRAME_BYTES = PIXELS * sizeof(uint8_t);

const int MORPH_STEP_CNT = 64;
const int NO_MORPH_STEP_CNT = 256;

// 1 byte per pixel, greyscale palette, 0 - black, 255 - white
extern uint8_t frameBuf[PIXELS];

extern void setPixel(int x, int y, uint8_t c);

extern void drawFrame(uint32_t frameNumber);

extern void encodeFrame(std::vector<uint16_t>& v);