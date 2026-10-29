#include "DriveHistoryDialog.h"

#include <QTableWidgetItem>
#include <QString>

DriveHistoryDialog::DriveHistoryDialog(
    const std::vector<DriveRecord>& records,
    const DriveStatistics& statistics,
    QWidget* parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    ui.totalTripsValueLabel->setText(
        QString::number(statistics.totalTrips)
    );

    ui.totalDistanceValueLabel->setText(
        QString::number(statistics.totalDistanceKm, 'f', 2)
        + " km"
    );

    const int totalHours =
        statistics.totalDurationSeconds / 3600;

    const int totalMinutes =
        (statistics.totalDurationSeconds % 3600) / 60;

    const int totalSeconds =
        statistics.totalDurationSeconds % 60;

    const QString totalTimeText =
        QString("%1:%2:%3")
        .arg(totalHours, 2, 10, QChar('0'))
        .arg(totalMinutes, 2, 10, QChar('0'))
        .arg(totalSeconds, 2, 10, QChar('0'));

    ui.totalTimeValueLabel->setText(totalTimeText);

    if (statistics.averageFuelEconomy.has_value())
    {
        ui.averageFuelValueLabel->setText(
            QString::number(
                statistics.averageFuelEconomy.value(),
                'f',
                1
            )
            + " km/L"
        );
    }
    else
    {
        ui.averageFuelValueLabel->setText("---");
    }

    ui.maxDistanceValueLabel->setText(
        QString::number(statistics.maxDistanceKm, 'f', 2)
        + " km"
    );

    ui.historyTable->setRowCount(
        static_cast<int>(records.size())
    );

    for (int row = 0;
        row < static_cast<int>(records.size());
        ++row)
    {
        const DriveRecord& record = records[row];

        // 日時
        ui.historyTable->setItem(
            row,
            0,
            new QTableWidgetItem(
                record.startedAt.toString("yyyy/MM/dd HH:mm:ss")
            )
        );

        // 走行距離
        ui.historyTable->setItem(
            row,
            1,
            new QTableWidgetItem(
                QString::number(record.distanceKm, 'f', 2)
                + " km"
            )
        );

        // 走行時間
        const int minutes =
            record.durationSeconds / 60;

        const int seconds =
            record.durationSeconds % 60;

        const QString timeText =
            QString("%1:%2")
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));

        ui.historyTable->setItem(
            row,
            2,
            new QTableWidgetItem(timeText)
        );

        // 平均燃費
        QString fuelEconomyText = "---";

        if (record.averageFuelEconomy.has_value())
        {
            fuelEconomyText =
                QString::number(
                    record.averageFuelEconomy.value(),
                    'f',
                    1
                )
                + " km/L";
        }

        ui.historyTable->setItem(
            row,
            3,
            new QTableWidgetItem(fuelEconomyText)
        );
    }

    ui.historyTable->resizeColumnsToContents();
}