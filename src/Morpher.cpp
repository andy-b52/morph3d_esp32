#include "Morpher.h"
#include <cassert>
#include "esp_random.h"

namespace {

// Point i of src moves linearly towards point i of dst; t is in [0, 1].
void morph(const Model& src, const Model& dst, float t, Model& res) {
    res.resize(src.size());

    for (size_t i = 0; i < src.size(); i++) {
        float srcX = src[i].x;
        float srcY = src[i].y;
        float srcZ = src[i].z;

        float dstX = dst[i].x;
        float dstY = dst[i].y;
        float dstZ = dst[i].z;

        res[i] = Point3D{srcX + t * (dstX - srcX), srcY + t * (dstY - srcY),
                         srcZ + t * (dstZ - srcZ)};
    }
}

int randomModel() { return static_cast<int>(esp_random() % getModels().size()); }

// A random model index other than `current`.
int randomOtherModel(int current) {
    assert(getModels().size() >= 2 && "Morphing needs at least two models");
    int next;
    do {
        next = randomModel();
    } while (next == current);
    return next;
}
}  // namespace

const Model& Morpher::step() {
    const Models& models = getModels();

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
