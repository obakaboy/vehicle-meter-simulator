#pragma once
namespace config {
inline constexpr double maxSpeedKmh = 180.0;
inline constexpr double accMinimumKmh = 30.0;
inline constexpr double normalAccelerationKmhPerSecond = 12.0;  //単位km/h per second
inline constexpr double sportAccelerationkmhPerSecond = 18.0; 
inline constexpr double ecoAccelerationKmhPerSecond = 8.0; 
inline constexpr double brakingKmhPerSecond = 30.0;
inline constexpr double coastingKmhPerSecond = 2.0;
inline constexpr double accGainPerSecond = 1.5;
inline constexpr double tankLiters = 40.0;
inline constexpr double lowFuelPercent = 15.0;
inline constexpr double idleLitersPerHour = 0.6;
inline constexpr double cruiseKmPerLiter = 18.0;
inline constexpr double accelerationLitersPerHour = 5.0;
inline constexpr double secondsPerHour = 3600.0;
inline constexpr int cycleMilliseconds = 100;
inline constexpr double accSetStep = 5.0;
}

