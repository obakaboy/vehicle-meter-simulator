#include "VehicleState.h"
#include "Config.h"
#include <algorithm>
#include <cmath>
void VehicleState::setPower(Power power) {                                                  //powerのsetter
    power_ = power;                                                                         //VehicleStateのメンバ変数(power_)へ関数が受け取った引数(power)を代入
    if (power_ == Power::Off) {                                                             //powerをoffにした場合、車速０、ACC解除、DriveModeをNormalへ変更
        speedKmh_ = 0.0;
        cancelAcc();
        driveMode_ = DriveMode::Normal;
    }
}
void VehicleState::setSpeedKmh(double speed) {                                                //speedのsetter
    if (std::isfinite(speed)) {                                                               //speedが普通の数値の場合（doubleには NaN や +∞ -∞も入る）
        speedKmh_ = power_ == Power::On ? std::clamp(speed, 0.0, config::maxSpeedKmh) : 0.0;  //speedKmh_にpower_がONならclampで処理したspeed、OFFなら０を代入
    }
}
void VehicleState::setFuelPercent(double fuel) {                                              //fuelのsetter 考え方はspeedと同じ
    if (std::isfinite(fuel)) {
        fuelPercent_ = std::clamp(fuel, 0.0, 100.0);
        if (fuelPercent_ == 0.0) cancelAcc();                                                //fuelが0ならACCをキャンセル
    }
}
bool VehicleState::engageAcc() {                                                             //ACC開始用関数 boolなので開始成功→true 失敗→false を返す
    if (power_ == Power::Off || fuelPercent_ <= 0.0 || speedKmh_ < config::accMinimumKmh)    // "||" 論理OR  
        return false;
    accActive_ = true;                                                                       //メンバ変巣accActive_をtrue(ACCをオン)
    accTargetKmh_ = speedKmh_;                                                               //メンバ変数accTarget_にspeedKmh_(現在車速)を代入
    return true;
}
void VehicleState::cancelAcc() {                                                             //ACCキャンセル用関数
    accActive_ = false;
    accTargetKmh_ = 0.0;
}
void VehicleState::reset() { *this = VehicleState{}; }                                       //リセット用関数 thisはこの関数を実行しているオブジェクトのポインタ、*はオブジェクト本体

void VehicleState::increaseAccTarget()                                                       //ACCターゲット車速増加関数
{
    if (accActive_)
    {
        const double step = config::accSetStep;

        const double nextTarget =
            std::floor(accTargetKmh_ / step) * step + step;                                  //ACC ON後のターゲット車速を10+5nにする処理

        accTargetKmh_ = std::clamp(                                                          //正常値判定
            nextTarget,
            config::accMinimumKmh,
            config::maxSpeedKmh
        );
    }
}

void VehicleState::decreaseAccTarget()                                                       //ACCターゲット車速減少関数
{
    if (accActive_)
    {
        const double step = config::accSetStep;

        const double nextTarget =
            std::ceil(accTargetKmh_ / step) * step - step;

        accTargetKmh_ = std::clamp(
            nextTarget,
            config::accMinimumKmh,
            config::maxSpeedKmh
        );
    }
}

void VehicleState::cycleDriveMode(){                                                         //DriveMode変更時のモード循環関数
    if (power_ == Power::Off){
        return;
    }
    switch (driveMode_)
    {
    case DriveMode::Normal:
        driveMode_ = DriveMode::Sport;
        break;
    case DriveMode::Sport:
        driveMode_ = DriveMode::Eco;
        break;
    case DriveMode::Eco:
        driveMode_ = DriveMode::Normal;
        break;
    }
}

DistanceDisplay VehicleState::getDistanceDisplay() const
{
    return distanceDisplay_;
}

void VehicleState::toggleDistanceDisplay()
{
    if (power_ == Power::Off) {
        return;
    }
    switch (distanceDisplay_)
    {
    case DistanceDisplay::Odo:
        distanceDisplay_ = DistanceDisplay::Trip;
        break;
    case DistanceDisplay::Trip:
        distanceDisplay_ = DistanceDisplay::SinceStart;
        break;
    case DistanceDisplay::SinceStart:
        distanceDisplay_ = DistanceDisplay::Odo;
        break;
    }
}