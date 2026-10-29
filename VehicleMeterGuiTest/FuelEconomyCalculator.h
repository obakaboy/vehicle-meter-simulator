#pragma once
#include <optional>
#include "VehicleState.h"
class FuelEconomyCalculator {
public:
    void update(VehicleState& state, double averageSpeedKmh, bool accelerating, double seconds);
    void reset();
    double distanceKm() const { return distanceKm_; }
    double consumedLiters() const { return consumedLiters_; }
    std::optional<double> instantKmPerLiter() const { return instant_; }
    std::optional<double> averageKmPerLiter() const;
private:
    double distanceKm_ = 0.0;
    double consumedLiters_ = 0.0;
    std::optional<double> instant_;
};

