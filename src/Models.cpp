#include "Models.h"
#include <cmath>

Models models;

namespace {
constexpr double FIXED_SCALE = 4096.0;

struct Vec3 {
    double x, y, z;
};

int32_t toFixed(double v) {
    return static_cast<int32_t>(std::lround(v * FIXED_SCALE));
}

template <typename Generator>
void addModel(Generator gen) {
    Model& model = models.emplace_back();
    model.reserve(POINTS_PER_MODEL);

    double totalPoints = static_cast<double>(POINTS_PER_MODEL);

    for (int i = 0; i < POINTS_PER_MODEL; i++) {
        double t = 2.0 * M_PI * i / totalPoints;
        Vec3 p = gen(t);
        model.emplace_back(Point3D{toFixed(p.x), toFixed(p.y), toFixed(p.z)});
    }
}
}  // namespace

void createModels() {
    models.reserve(MODEL_COUNT);

    // Sphere: spiral from pole to pole, 10 turns
    addModel([](double t) {
        double r = 100.0 * std::sin(t / 2);

        return Vec3{r * std::sin(10 * t), r * std::cos(10 * t),
                    100.0 * std::cos(t / 2)};
    });

    // 3D Lissajous curve
    addModel([](double t) {
        return Vec3{100.0 * std::sin(2 * t), 100.0 * std::cos(t),
                    100.0 * std::sin(3 * t)};
    });

    // Torus: 26 turns around the tube per turn around the ring
    addModel([](double t) {
        const double R = 75.0;  // ring radius
        const double r = 30.0;  // tube radius
        double theta = 26 * t;
        return Vec3{(R + r * std::cos(theta)) * std::cos(t),
                    (R + r * std::cos(theta)) * std::sin(t),
                    r * std::sin(theta)};
    });
}
