#pragma once

#include <QDateTime>
#include <optional>

struct DriveRecord									//1件（毎）の走行データ
{
	QDateTime startedAt;							//開始時間
	QDateTime endedAt;								//終了時間

	int durationSeconds = 0;						//走行時間
	double distanceKm = 0.0;						//走行距離
	std::optional<double> averageFuelEconomy;		//平均燃費
};