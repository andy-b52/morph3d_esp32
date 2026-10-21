#include "RleEncoder.h"
#include "FrameBuffer.h"

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