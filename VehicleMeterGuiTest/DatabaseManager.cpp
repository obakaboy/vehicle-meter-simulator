#include "DatabaseManager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DatabaseManager::DatabaseManager()
	:m_database(
		QSqlDatabase::addDatabase(
			"QSQLITE",
			"VehicleMeterConnection"
		)
	)
{
}

DatabaseManager::~DatabaseManager()
{
	if (m_database.isOpen())
	{
		m_database.close();
	}

	//テスト用
	const QString connectionName =
		m_database.connectionName();

	m_database = QSqlDatabase();

	QSqlDatabase::removeDatabase(connectionName);
	//ここまで
}

bool DatabaseManager::open(const QString& databasePath)
{
	m_database.setDatabaseName(databasePath);

	if (!m_database.open())
	{
		qWarning()
			<< "Failed to open database:"
			<< m_database.lastError().text();

		return false;
	}

	return true;
}

bool DatabaseManager::initialize()
{
	QSqlQuery query(m_database);

	const QString sql = 
		R"(
            CREATE TABLE IF NOT EXISTS drive_history (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                started_at TEXT NOT NULL,
                ended_at TEXT NOT NULL,
                duration_seconds INTEGER NOT NULL,
                distance_km REAL NOT NULL,
                average_fuel_economy REAL
            )
        )";

	if (!query.exec(sql))
	{
		qWarning()
			<< "Failed to create drive_history table:"
			<< query.lastError().text();

		return false;
	}

	return true;

}

bool DatabaseManager::saveDriveRecord(const DriveRecord& record)
{
	QSqlQuery query(m_database);

	query.prepare(
		R"(
            INSERT INTO drive_history (
                started_at,
                ended_at,
                duration_seconds,
                distance_km,
                average_fuel_economy
            )
            VALUES (
                :started_at,
                :ended_at,
                :duration_seconds,
                :distance_km,
                :average_fuel_economy
            )
        )"
	);

	query.bindValue(
		":started_at",
		record.startedAt.toString(Qt::ISODate)
	);

	query.bindValue(
		":ended_at",
		record.endedAt.toString(Qt::ISODate)
	);

	query.bindValue(
		":duration_seconds",
		record.durationSeconds
	);

	query.bindValue(
		":distance_km",
		record.distanceKm
	);

	if (record.averageFuelEconomy.has_value())
	{
		query.bindValue(
			":average_fuel_economy",
			record.averageFuelEconomy.value()
		);
	}
	else
	{
		query.bindValue(
			":average_fuel_economy",
			QVariant()								//値がない場合にNULLを保持するためにQVariant()を渡す
		);
	}

	if (!query.exec())
	{
		qWarning()
			<< "Failed to insert drive record:"
			<< query.lastError().text();

		return false;
	}

	return true;
}

std::vector<DriveRecord> DatabaseManager::loadDriveRecords()
{
	std::vector<DriveRecord> records;
	
	QSqlQuery query(m_database);

	const QString sql =
		R"(
            SELECT
                started_at,
                ended_at,
                duration_seconds,
                distance_km,
                average_fuel_economy
            FROM drive_history
            ORDER BY id DESC
        )";

	if (!query.exec(sql))
	{
		qWarning()
			<< "Failed to load drive records:"
			<< query.lastError().text();

		return records;
	}

	while (query.next())
	{
		DriveRecord record;

		record.startedAt =
			QDateTime::fromString(
				query.value("started_at").toString(),
				Qt::ISODate
			);

		record.endedAt =
			QDateTime::fromString(
				query.value("ended_at").toString(),
				Qt::ISODate
			);

		record.durationSeconds =
			query.value("duration_seconds").toInt();

		record.distanceKm =
			query.value("distance_km").toDouble();

		if (!query.value("average_fuel_economy").isNull())
		{
			record.averageFuelEconomy =
				query.value("average_fuel_economy").toDouble();
		}

		records.push_back(record);
	}

	return records;
}

DriveStatistics DatabaseManager::loadDriveStatistics()
{
	DriveStatistics statistics;

	QSqlQuery query(m_database);

	const QString sql =
		R"(
            SELECT
                COUNT(*) AS total_trips,
                COALESCE(SUM(distance_km), 0) AS total_distance,
                COALESCE(SUM(duration_seconds), 0) AS total_duration,
                AVG(average_fuel_economy) AS average_fuel_economy,
                COALESCE(MAX(distance_km), 0) AS max_distance
            FROM drive_history
        )";

	if (!query.exec(sql))
	{
		qWarning()
			<< "Failed to load drive statistics:"
			<< query.lastError().text();

		return statistics;
	}

	if (query.next())
	{
		statistics.totalTrips = query.value("total_trips").toInt();

		statistics.totalDistanceKm = query.value("total_distance").toDouble();

		statistics.totalDurationSeconds = query.value("total_duration").toInt();

		if (!query.value("average_fuel_economy").isNull())
		{
			statistics.averageFuelEconomy = query.value("average_fuel_economy").toDouble();
		}

		statistics.maxDistanceKm = query.value("max_distance").toDouble();
	}

	return statistics;
}