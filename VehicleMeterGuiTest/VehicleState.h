#pragma once
enum class Power { Off, On };                  //Power型がとりえる値をOFF/ONの2つに制限
enum class DriveMode {Eco, Normal, Sport};     //DriveMode型がとりえる値をEco/Normal/Sportの3つに制限
enum class DistanceDisplay {Odo, Trip, SinceStart};        //ODO/Tripの始動後の3値へ制限

class VehicleState {                                         //車両状態を保持し、状態変更時の制約も管理するクラス
public:
    VehicleState() = default;                                //デフォルトコンストラクタ。初期値はprivateで定義
    Power power() const { return power_; }                   //パワー状態のgetter
    DriveMode driveMode() const {return driveMode_;}         //DriveModeのgetter
    double speedKmh() const { return speedKmh_; }            //車速のgetter
    double fuelPercent() const { return fuelPercent_; }      //燃料のgetter
    bool accActive() const { return accActive_; }            //ACC状態のgetter
    double accTargetKmh() const { return accTargetKmh_; }    //ACC目標車速のgetter
    void setPower(Power power);
    void setSpeedKmh(double speed);
    void setFuelPercent(double fuel);
    bool engageAcc();
    void cancelAcc();
    void reset();
    void increaseAccTarget();
    void decreaseAccTarget();
    void cycleDriveMode();
    DistanceDisplay getDistanceDisplay() const;  
    void toggleDistanceDisplay();
private:
    Power power_ = Power::Off;
    DriveMode driveMode_ = DriveMode::Normal;
    double speedKmh_ = 0.0;
    double fuelPercent_ = 100.0;
    bool accActive_ = false;
    double accTargetKmh_ = 0.0;
    DistanceDisplay distanceDisplay_ = DistanceDisplay::Odo;
};

