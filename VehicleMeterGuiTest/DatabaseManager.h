#pragma once

#include <QSqlDatabase>
#include <QString>
#include <vector>

#include "DriveRecord.h"
#include "DriveStatistics.h"

class DatabaseManager
{
public:
	DatabaseManager();
	~DatabaseManager();

	bool open(const QString& databasePath);		//DBファイルを開く
	bool initialize();							//必要なテーブルを準備

	bool saveDriveRecord(const DriveRecord& record);
	std::vector<DriveRecord> loadDriveRecords();	//走行毎のドライブレコード

	DriveStatistics loadDriveStatistics();			//全ドライブレコードから集計したやつ

private:
	QSqlDatabase m_database;
};