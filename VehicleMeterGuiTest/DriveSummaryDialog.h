#pragma once

#include <QDialog>
#include <optional>

#include "ui_DriveSummaryDialog.h"

class DriveSummaryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DriveSummaryDialog(
        double distanceKm,
        double elapsedSeconds,
        std::optional<double> averageFuelEconomy,
        QWidget* parent = nullptr
    );

private:
    Ui::DriveSummaryDialog ui;
};