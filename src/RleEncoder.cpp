#include "RleEncoder.h"

void encodeFrame(const uint8_t* frame, size_t size, std::vector<uint8_t>& out) {
    out.clear();

    if (size == 0) return;

    uint8_t color = frame[0];
    uint8_t runLength = 1;

    for (size_t i = 1; i < size; i++) {
        // If color changes OR we hit the 8-bit packet limit, flush the packet
        if (frame[i] != color || runLength == 255) {
            out.push_back(color);
            out.push_back(runLength);
            color = frame[i];
            runLength = 1;
        } else {
            runLength++;
        }
    }

    // Always cleanly flush exactly one final packet at the very end
    out.push_back(color);
    out.push_back(runLength);
}
