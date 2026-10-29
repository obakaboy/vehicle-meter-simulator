#include "DriveSummaryDialog.h"

#include <QString>
#include <QChar>
#include <cmath>

DriveSummaryDialog::DriveSummaryDialog(
    double distanceKm,
    double elapsedSeconds,
    std::optional<double> averageFuelEconomy,
    QWidget* parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    // 走行距離
    ui.distanceValueLabel->setText(
        QString::number(distanceKm, 'f', 1) + " km"
    );

    // 走行時間
    const int totalSeconds =
        static_cast<int>(std::round(elapsedSeconds));

    const int hours = totalSeconds / 3600;
    const int minutes = (totalSeconds % 3600) / 60;
    const int seconds = totalSeconds % 60;

    const QString timeText =
        QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));

    ui.driveTimeValueLabel->setText(timeText);

    // 平均燃費
    if (averageFuelEconomy.has_value())
    {
        ui.averageFuelValueLabel->setText(
            QString::number(
                averageFuelEconomy.value(),
                'f',
                1
            ) + " km/L"
        );
    }
    else
    {
        ui.averageFuelValueLabel->setText("--- km/L");
    }
}