#pragma once
#include "VehicleState.h"
#include "InputTypes.h"
#include "FuelEconomyCalculator.h"

class VehicleController {                                                   //入力に応じて車両の挙動を判断・制御するクラス
public:
    explicit VehicleController(VehicleState& state);
    void update(const InputFrame& input, double seconds);
    const FuelEconomyCalculator& economy() const { return economy_; }
private:
    VehicleState& state_;
    FuelEconomyCalculator economy_;
    double accelerationForMode(DriveMode mode) const;
};

