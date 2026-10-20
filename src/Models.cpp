#include "Models.h"
#include <cmath>

Models models;

void createModels() {
    models.reserve(MODEL_COUNT);

    double totalPoints = static_cast<double>(POINTS_PER_MODEL);

    // Build Sphere Model #0
    models.emplace_back();

    Model& model1 = models.back();
    model1.reserve(POINTS_PER_MODEL);

    for (int i = 0; i < POINTS_PER_MODEL; i++) {
        double idx = static_cast<double>(i);

        double angle1 = (M_PI * idx) / totalPoints;
        double angle2 = (20.0 * M_PI * idx) / totalPoints;  // 10 * 2 * M_PI

        double r = 100.0 * std::sin(angle1);

        int32_t exactX =
            static_cast<int32_t>(std::lround(r * std::sin(angle2) * 4096.0));
        int32_t exactY =
            static_cast<int32_t>(std::lround(r * std::cos(angle2) * 4096.0));
        int32_t exactZ = static_cast<int32_t>(
            std::lround(100.0 * std::cos(angle1) * 4096.0));

        model1.emplace_back(Point3D{exactX, exactY, exactZ});
    }

    // Build Lissajous Model #1
    models.emplace_back();

    Model& model2 = models.back();
    model2.reserve(POINTS_PER_MODEL);

    for (int i = 0; i < POINTS_PER_MODEL; i++) {
        double idx = static_cast<double>(i);

        double angle1 = (2.0 * 2.0 * M_PI * idx) / totalPoints;
        double angle2 = (1.0 * 2.0 * M_PI * idx) / totalPoints;
        double angle3 = (3.0 * 2.0 * M_PI * idx) / totalPoints;

        int32_t x = static_cast<int32_t>(
            std::lround(100.0 * std::sin(angle1) * 4096.0));
        int32_t y = static_cast<int32_t>(
            std::lround(100.0 * std::cos(angle2) * 4096.0));
        int32_t z = static_cast<int32_t>(
            std::lround(100.0 * std::sin(angle3) * 4096.0));

        model2.emplace_back(Point3D{x, y, z});
    }

    // Build Torus Model #2
    models.emplace_back();
    Model& torusModel = models.back();
    torusModel.reserve(POINTS_PER_MODEL);

    // Torus dimensions (scaled up to match your ~100.0 radius boundaries)
    const double R = 75.0;  // Major radius: distance from the center of the torus to the center of the tube
    const double r = 30.0;  // Minor radius: radius of the tube itself

    for (int i = 0; i < POINTS_PER_MODEL; i++) {
        double idx = static_cast<double>(i);

        // We use asymmetric multiplier cycles to smoothly distribute points 
        // around the tube and the main axis without overlapping paths.
        // 26 and 1 are coprime, meaning the winding path fills out the shape beautifully.
        double theta = (26.0 * 2.0 * M_PI * idx) / totalPoints; // Winding around the tube
        double phi   = (1.0 * 2.0 * M_PI * idx) / totalPoints;  // Winding around the center ring

        // Standard parametric equations for a 3D Torus
        double x_unit = (R + r * std::cos(theta)) * std::cos(phi);
        double y_unit = (R + r * std::cos(theta)) * std::sin(phi);
        double z_unit = r * std::sin(theta);

        // Apply high-precision delayed fixed-point scaling at the absolute last microsecond
        int32_t x = static_cast<int32_t>(std::lround(x_unit * 4096.0));
        int32_t y = static_cast<int32_t>(std::lround(y_unit * 4096.0));
        int32_t z = static_cast<int32_t>(std::lround(z_unit * 4096.0));

        torusModel.emplace_back(Point3D{x, y, z});
    }    
}