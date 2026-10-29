#include "VehicleController.h"
#include "Config.h"
#include <algorithm>
#include <cmath>
VehicleController::VehicleController(VehicleState& state) : state_(state) {}
void VehicleController::update(const InputFrame& input, double seconds) {
    if (!std::isfinite(seconds) || seconds <= 0.0) return;                                          //有効値以外を除外（update関数を終了する）
    for (const auto command : input.commands) {                                                     //commandsの中身を一つずつ処理
        switch (command) {
        case Command::TogglePower:
            state_.setPower(state_.power() == Power::On ? Power::Off : Power::On);
            break;
        case Command::ToggleAcc:
            if (state_.accActive()) state_.cancelAcc();
            else state_.engageAcc();
            break;
        case Command::Reset:
            state_.reset();
            economy_.reset();
            return; // Reset defines the entire frame, including held pedal inputs.
        case Command::Quit:
            break;
        case Command::IncreaseAccTarget:
            state_.increaseAccTarget();
            break;
        case Command::DecreaseAccTarget:
            state_.decreaseAccTarget();
            break;    
        case Command::ChangeDriveMode:
            state_.cycleDriveMode();
            break;
        }
    }
    const double oldSpeed = state_.speedKmh();
    double acceleration = 0.0;
    if (state_.power() == Power::On) {
        if (input.brake) {
            state_.cancelAcc();
            acceleration = -config::brakingKmhPerSecond;
        } else if (state_.fuelPercent() <= 0.0) {
            acceleration = -config::coastingKmhPerSecond;
        } else if (input.accelerate) {
            acceleration = accelerationForMode(state_.driveMode());                         //ドライブモードに合った加速度を取得
        } else if (state_.accActive()) {
            acceleration = std::clamp(
                (state_.accTargetKmh() - oldSpeed) * config::accGainPerSecond,
                -config::brakingKmhPerSecond, config::normalAccelerationKmhPerSecond
            );
        } else {
            acceleration = -config::coastingKmhPerSecond;
        }
        state_.setSpeedKmh(oldSpeed + acceleration * seconds);                              //速度へ反映
    }
    const double averageSpeed = (oldSpeed + state_.speedKmh()) / 2.0;
    economy_.update(state_, averageSpeed, acceleration > 0.0, seconds);
}

double VehicleController::accelerationForMode(DriveMode mode) const{                        //対応するドライブモード毎の加速度を返す関数
    switch (mode)
    {
    case DriveMode::Eco:
        return config::ecoAccelerationKmhPerSecond;
        
    case DriveMode::Normal:
        return config::normalAccelerationKmhPerSecond;
        
    case DriveMode::Sport:
        return config::sportAccelerationkmhPerSecond;
    }
    return config::normalAccelerationKmhPerSecond;
}