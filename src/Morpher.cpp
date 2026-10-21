#include "Morpher.h"
#include <cmath>
#include "esp_random.h"

static_assert(MODEL_COUNT >= 2, "Morphing needs at least two models");

namespace {

// Point i of src moves linearly towards point i of dst; t is in [0, 1].
void morph(const Model& src, const Model& dst, float t, Model& res) {
    res.resize(src.size());

    for (size_t i = 0; i < src.size(); i++) {
        float srcX = static_cast<float>(src[i].x);
        float srcY = static_cast<float>(src[i].y);
        float srcZ = static_cast<float>(src[i].z);

        float dstX = static_cast<float>(dst[i].x);
        float dstY = static_cast<float>(dst[i].y);
        float dstZ = static_cast<float>(dst[i].z);

        res[i] = Point3D{srcX + t * (dstX - srcX),
                         srcY + t * (dstY - srcY),
                         srcZ + t * (dstZ - srcZ)};
    }
}

int randomModel() { return static_cast<int>(esp_random() % MODEL_COUNT); }

// A random model index other than `current`.
// Terminates because MODEL_COUNT >= 2 (see static_assert above).
int randomOtherModel(int current) {
    int next;
    do {
        next = randomModel();
    } while (next == current);
    return next;
}
}  // namespace

const Model& Morpher::step() {
    if (srcIdx_ < 0) {
        srcIdx_ = randomModel();
    }

    if (!inMorph_ && frame_ == HOLD_FRAMES) {
        inMorph_ = true;
        frame_ = 0;
        dstIdx_ = randomOtherModel(srcIdx_);
    } else if (inMorph_ && frame_ == MORPH_FRAMES) {
        inMorph_ = false;
        frame_ = 0;
        srcIdx_ = dstIdx_;
    }

    frame_++;

    if (!inMorph_) {
        return models[srcIdx_];
    }

    // frame_ runs 1..MORPH_FRAMES, so the last morph frame is exactly dst
    float t = static_cast<float>(frame_) / static_cast<float>(MORPH_FRAMES);
    morph(models[srcIdx_], models[dstIdx_], t, morphed_);
    return morphed_;
}
