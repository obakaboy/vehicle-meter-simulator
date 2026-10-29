#include <gtest/gtest.h>

#include "DatabaseManager.h"

class DatabaseManagerTest
    : public ::testing::Test
{
protected:
    DatabaseManager m_databaseManager;

    void SetUp() override
    {
        ASSERT_TRUE(
            m_databaseManager.open(":memory:")
        );

        ASSERT_TRUE(
            m_databaseManager.initialize()
        );
    }
};

TEST_F(
    DatabaseManagerTest,
    EmptyDatabaseReturnsNoRecords
)
{
    const auto records =
        m_databaseManager.loadDriveRecords();

    EXPECT_TRUE(records.empty());
}

TEST_F(
    DatabaseManagerTest,
    EmptyDatabaseReturnsDefaultStatistics
)
{
    const auto statistics =
        m_databaseManager.loadDriveStatistics();

    EXPECT_EQ(statistics.totalTrips, 0);
    EXPECT_DOUBLE_EQ(statistics.totalDistanceKm, 0.0);
    EXPECT_EQ(statistics.totalDurationSeconds, 0);
    EXPECT_FALSE(statistics.averageFuelEconomy.has_value());
    EXPECT_DOUBLE_EQ(statistics.maxDistanceKm, 0.0);
}

TEST_F(
    DatabaseManagerTest,
    SaveAndLoadSingleRecord
)
{
    DriveRecord record;

    record.startedAt =
        QDateTime::fromString(
            "2026-10-28T10:00:00",
            Qt::ISODate
        );

    record.endedAt =
        QDateTime::fromString(
            "2026-10-28T10:01:00",
            Qt::ISODate
        );

    record.durationSeconds = 60;
    record.distanceKm = 1.5;
    record.averageFuelEconomy = 12.3;

    ASSERT_TRUE(
        m_databaseManager.saveDriveRecord(record)
    );

    const auto records =
        m_databaseManager.loadDriveRecords();

    ASSERT_EQ(records.size(), 1);

    EXPECT_EQ(
        records[0].startedAt,
        record.startedAt
    );

    EXPECT_EQ(
        records[0].endedAt,
        record.endedAt
    );

    EXPECT_EQ(
        records[0].durationSeconds,
        60
    );

    EXPECT_DOUBLE_EQ(
        records[0].distanceKm,
        1.5
    );

    ASSERT_TRUE(
        records[0].averageFuelEconomy.has_value()
    );

    EXPECT_DOUBLE_EQ(
        records[0].averageFuelEconomy.value(),
        12.3
    );
}

TEST_F(
    DatabaseManagerTest,
    SaveAndLoadRecordWithNullFuelEconomy
)
{
    DriveRecord record;

    record.startedAt =
        QDateTime::fromString(
            "2026-10-28T11:00:00",
            Qt::ISODate
        );

    record.endedAt =
        QDateTime::fromString(
            "2026-10-28T11:00:30",
            Qt::ISODate
        );

    record.durationSeconds = 30;
    record.distanceKm = 0.2;

    // averageFuelEconomy には値を入れない

    ASSERT_TRUE(
        m_databaseManager.saveDriveRecord(record)
    );

    const auto records =
        m_databaseManager.loadDriveRecords();

    ASSERT_EQ(records.size(), 1);

    EXPECT_EQ(
        records[0].startedAt,
        record.startedAt
    );

    EXPECT_EQ(
        records[0].endedAt,
        record.endedAt
    );

    EXPECT_EQ(
        records[0].durationSeconds,
        30
    );

    EXPECT_DOUBLE_EQ(
        records[0].distanceKm,
        0.2
    );

    EXPECT_FALSE(
        records[0].averageFuelEconomy.has_value()
    );
}

TEST_F(
    DatabaseManagerTest,
    LoadRecordsInDescendingIdOrder
)
{
    DriveRecord firstRecord;
    firstRecord.startedAt =
        QDateTime::fromString(
            "2026-10-28T10:00:00",
            Qt::ISODate
        );
    firstRecord.endedAt =
        QDateTime::fromString(
            "2026-10-28T10:01:00",
            Qt::ISODate
        );
    firstRecord.durationSeconds = 60;
    firstRecord.distanceKm = 1.0;
    firstRecord.averageFuelEconomy = 10.0;

    DriveRecord secondRecord;
    secondRecord.startedAt =
        QDateTime::fromString(
            "2026-10-28T11:00:00",
            Qt::ISODate
        );
    secondRecord.endedAt =
        QDateTime::fromString(
            "2026-10-28T11:02:00",
            Qt::ISODate
        );
    secondRecord.durationSeconds = 120;
    secondRecord.distanceKm = 2.0;
    secondRecord.averageFuelEconomy = 20.0;

    DriveRecord thirdRecord;
    thirdRecord.startedAt =
        QDateTime::fromString(
            "2026-10-28T12:00:00",
            Qt::ISODate
        );
    thirdRecord.endedAt =
        QDateTime::fromString(
            "2026-10-28T12:03:00",
            Qt::ISODate
        );
    thirdRecord.durationSeconds = 180;
    thirdRecord.distanceKm = 3.0;
    thirdRecord.averageFuelEconomy = 30.0;

    ASSERT_TRUE(
        m_databaseManager.saveDriveRecord(firstRecord)
    );

    ASSERT_TRUE(
        m_databaseManager.saveDriveRecord(secondRecord)
    );

    ASSERT_TRUE(
        m_databaseManager.saveDriveRecord(thirdRecord)
    );

    const auto records =
        m_databaseManager.loadDriveRecords();

    ASSERT_EQ(records.size(), 3);

    EXPECT_DOUBLE_EQ(records[0].distanceKm, 3.0);
    EXPECT_DOUBLE_EQ(records[1].distanceKm, 2.0);
    EXPECT_DOUBLE_EQ(records[2].distanceKm, 1.0);
}

TEST_F(
    DatabaseManagerTest,
    LoadDriveStatisticsCalculatesValuesCorrectly
)
{
    DriveRecord firstRecord;
    firstRecord.startedAt =
        QDateTime::fromString(
            "2026-10-28T10:00:00",
            Qt::ISODate
        );
    firstRecord.endedAt =
        QDateTime::fromString(
            "2026-10-28T10:00:10",
            Qt::ISODate
        );
    firstRecord.durationSeconds = 10;
    firstRecord.distanceKm = 1.0;
    firstRecord.averageFuelEconomy = 10.0;

    DriveRecord secondRecord;
    secondRecord.startedAt =
        QDateTime::fromString(
            "2026-10-28T11:00:00",
            Qt::ISODate
        );
    secondRecord.endedAt =
        QDateTime::fromString(
            "2026-10-28T11:00:20",
            Qt::ISODate
        );
    secondRecord.durationSeconds = 20;
    secondRecord.distanceKm = 2.0;
    secondRecord.averageFuelEconomy = 20.0;

    DriveRecord thirdRecord;
    thirdRecord.startedAt =
        QDateTime::fromString(
            "2026-10-28T12:00:00",
            Qt::ISODate
        );
    thirdRecord.endedAt =
        QDateTime::fromString(
            "2026-10-28T12:00:30",
            Qt::ISODate
        );
    thirdRecord.durationSeconds = 30;
    thirdRecord.distanceKm = 3.0;

    ASSERT_TRUE(
        m_databaseManager.saveDriveRecord(firstRecord)
    );

    ASSERT_TRUE(
        m_databaseManager.saveDriveRecord(secondRecord)
    );

    ASSERT_TRUE(
        m_databaseManager.saveDriveRecord(thirdRecord)
    );

    const auto statistics =
        m_databaseManager.loadDriveStatistics();

    EXPECT_EQ(
        statistics.totalTrips,
        3
    );

    EXPECT_DOUBLE_EQ(
        statistics.totalDistanceKm,
        6.0
    );

    EXPECT_EQ(
        statistics.totalDurationSeconds,
        60
    );

    ASSERT_TRUE(
        statistics.averageFuelEconomy.has_value()
    );

    EXPECT_DOUBLE_EQ(
        statistics.averageFuelEconomy.value(),
        15.0
    );

    EXPECT_DOUBLE_EQ(
        statistics.maxDistanceKm,
        3.0
    );
}

TEST(
    DatabaseManagerErrorTest,
    SaveRecordFailsWithoutInitialize
)
{
    DatabaseManager databaseManager;

    ASSERT_TRUE(
        databaseManager.open(":memory:")
    );

    DriveRecord record;

    record.startedAt =
        QDateTime::fromString(
            "2026-10-28T13:00:00",
            Qt::ISODate
        );

    record.endedAt =
        QDateTime::fromString(
            "2026-10-28T13:01:00",
            Qt::ISODate
        );

    record.durationSeconds = 60;
    record.distanceKm = 1.0;
    record.averageFuelEconomy = 10.0;

    EXPECT_FALSE(
        databaseManager.saveDriveRecord(record)
    );
}