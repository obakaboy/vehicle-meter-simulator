#pragma once

#include <optional>

struct DriveStatistics							//各走行から集計した結果
{
	int totalTrips = 0;							//総走行回数
	double totalDistanceKm = 0.0;				//総走行距離
	int totalDurationSeconds = 0;				//総走行時間

	std::optional<double> averageFuelEconomy;	//平均燃費の平均

	double maxDistanceKm = 0.0;					//最高走行距離
};