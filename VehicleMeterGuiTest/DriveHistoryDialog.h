#pragma once

#include <QDialog>
#include <vector>

#include "DriveRecord.h"
#include "DriveStatistics.h"
#include "ui_DriveHistoryDialog.h"

class DriveHistoryDialog : public QDialog
{
	Q_OBJECT

public:
	explicit DriveHistoryDialog(
		const std::vector<DriveRecord>& records,
		const DriveStatistics& statistics,
		QWidget* parent = nullptr
	);

private:
	Ui::DriveHistoryDialog ui;
};