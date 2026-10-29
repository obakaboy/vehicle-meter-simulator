#include "FuelEconomyCalculator.h"
#include "Config.h"
#include <algorithm>

void FuelEconomyCalculator::update(VehicleState& state, double speed, bool accelerating, double seconds) {
    instant_.reset();
    if (state.power() == Power::Off) return;
    const double distance = speed * seconds / config::secondsPerHour;                           
    const double rate = config::idleLitersPerHour + speed / config::cruiseKmPerLiter            //消費量＝アイドル消費＋速度に応じた消費＋加速中の追加消費
        + (accelerating ? config::accelerationLitersPerHour : 0.0);
    const double available = state.fuelPercent() * config::tankLiters / 100.0;                  //残量
    const double used = std::min(available, rate * seconds / config::secondsPerHour);           //rateとavailableの少ないほうを使用量とする（0を下回らないため）
    distanceKm_ += distance;
    consumedLiters_ += used;
    state.setFuelPercent((available - used) / config::tankLiters * 100.0);
    if (used > 0.0) instant_ = distance / used;
}

std::optional<double> FuelEconomyCalculator::averageKmPerLiter() const {
    if (consumedLiters_ <= 0.0) return std::nullopt;
    return distanceKm_ / consumedLiters_;
}

void FuelEconomyCalculator::reset() { *this = FuelEconomyCalculator{}; }

