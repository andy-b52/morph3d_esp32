#pragma once

#include "Models.h"

constexpr int MORPH_FRAMES = 64;  // frames a morph takes
constexpr int HOLD_FRAMES = 256;  // frames a shape is held between morphs

// Holds a random model for HOLD_FRAMES frames, then morphs it into a random
// other model over MORPH_FRAMES frames, and so on.
class Morpher {
public:
    // Advances one frame and returns the shape to draw for it.
    // The reference is valid until the next call.
    const Model& step();

private:
    Model morphed_;    // output buffer while morphing
    int srcIdx_ = -1;  // model held, or morphed from
    int dstIdx_ = -1;  // model morphed to
    int frame_ = 0;    // frames since the current phase started
    bool inMorph_ = false;
};
