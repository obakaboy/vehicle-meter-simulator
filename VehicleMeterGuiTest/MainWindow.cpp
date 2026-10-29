#include "MainWindow.h"
#include "Config.h"
#include "DriveSummaryDialog.h"
#include "DriveRecord.h"
#include "DriveHistoryDialog.h"

#include <QKeyEvent>
#include <cmath>
#include <algorithm>
#include <optional>
#include <QCoreApplication>
#include <QDebug>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),                                          //親クラスの初期。親クラスのコンストラクタにparentを渡す
    m_controller(m_state)                                           //m_stateを参照するContorollerを作る
{
    ui.setupUi(this);                                               //thisオブジェクト(MainWindow)に.uiで設計した画面を構築させる

    const QString databasePath =
        QCoreApplication::applicationDirPath() + "/vehicle_meter.db";

    if (m_databaseManager.open(databasePath))
    {
        m_databaseManager.initialize();
    }

    ui.speedometerWidget->setMaximumSpeed(                          //UIに表示する最高車速を定数から設定する
        static_cast<int>(config::maxSpeedKmh));                     //maxSpeedKmhはdouble、setMaxSpeedはintなのでcastする

    connect(                                                        //QTimerがtimuoutしたらMainWindowのupdateSimulation()を呼ぶ
        &m_timer,                                                       //sender(どの送信元オブジェクトか)
        &QTimer::timeout,                                               //signal(そのオブジェクトのどのシグナルか)
        this,                                                           //receiver(どの受信側オブジェクトか)
        &MainWindow::updateSimulation                                   //function(そのオブジェクトのどの関数か)
    );

    m_timer.start(config::cycleMilliseconds);                       //cycleMilliseconds=100より、100msごとにtimeoutシグナルを発生させる

    updateDisplay();                                                //初回は100msを待たずにウィンドウ生成時点で初期表示をGUIへ反映
}
 
MainWindow::~MainWindow()
{
}

void MainWindow::keyPressEvent(QKeyEvent* event)                        //Qtから届いたキー入力(引数)をInputFrameに変換する関数
{
    switch (event->key())                                               //eventがポインタなので「.」ではなく「->」
    {
    case Qt::Key_W:                                                     //W入力と↑入力の両方で同じ処理をするためにcaseを連続で書く
    case Qt::Key_Up:                                                    //caseはbreakがなければ次のcaseへ流れる性質を利用
        m_input.accelerate = true;
        break;

    case Qt::Key_S:
    case Qt::Key_Down:
        m_input.brake = true;
        break;

    case Qt::Key_P:
        if (!event->isAutoRepeat())                                     //AutoRepeatにしたがって何度もON/OFFを繰り返す必要がないので弾く
        {
            m_input.commands.push_back(Command::TogglePower);           //std::vectorの末尾へ要素(TogglePower)を追加する
        }                                                               //上記はVehicleContorollerで1つずつ処理
        break;

    case Qt::Key_C:
        if (!event->isAutoRepeat())
        {
            m_input.commands.push_back(Command::ToggleAcc);
        }
        break;

    case Qt::Key_M:
        if (!event->isAutoRepeat())
        {
            m_input.commands.push_back(Command::ChangeDriveMode);
        }
        break;

    case Qt::Key_R:
        if (!event->isAutoRepeat())
        {
            m_input.commands.push_back(Command::Reset);
        }
        break;

    case Qt::Key_Q:
        if (!event->isAutoRepeat())
        {
            close();                                                    //ウィンドウを閉じる（QWidgetがもってる関数）
        }
        break;

    case Qt::Key_Plus:
        if (!event->isAutoRepeat())
        {
            m_input.commands.push_back(Command::IncreaseAccTarget);
        }
        break;

    case Qt::Key_Minus:
        if (!event->isAutoRepeat())
        {
            m_input.commands.push_back(Command::DecreaseAccTarget);
        }
        break;

    case Qt::Key_T:
        if (!event->isAutoRepeat())
        {
            m_state.toggleDistanceDisplay();
        }
        break;

    case Qt::Key_F:
        if (!event->isAutoRepeat())
        {
            m_odometer.resetTrip();
        }
        break;

    case Qt::Key_H:
        if (!event->isAutoRepeat() && m_state.power() == Power::Off)
        {
            const auto records = m_databaseManager.loadDriveRecords();

            const auto statistics = m_databaseManager.loadDriveStatistics();

            DriveHistoryDialog dialog(records, statistics, this);

            dialog.exec();
        }
        break;

    default:
        QMainWindow::keyPressEvent(event);                              //どれにも当てはまらない場合、親クラスQMainWindowの標準キー処理を呼ぶ
        break;
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent* event)                      //keyPressEventの離した時Verの関数３
{
    if (event->isAutoRepeat())
    {
        return;
    }

    switch (event->key())
    {
    case Qt::Key_W:
    case Qt::Key_Up:
        m_input.accelerate = false;
        break;

    case Qt::Key_S:
    case Qt::Key_Down:
        m_input.brake = false;
        break;

    default:
        QMainWindow::keyReleaseEvent(event);
        break;
    }
}

void MainWindow::updateAccDisplay()
{
    if (m_state.power() == Power::Off)
    {
        ui.accStatusLabel->setText("OFF");
        ui.accStatusLabel->setStyleSheet(
            "color: gray;"
            "font-weight: bold;"
        );
    }
    else if (m_state.accActive())
    {
        ui.accStatusLabel->setText("ACTIVE");
        ui.accStatusLabel->setStyleSheet(
            "color: green;"
            "font-weight: bold;"
        );
    }
    else
    {
        ui.accStatusLabel->setText("STANDBY");
        ui.accStatusLabel->setStyleSheet(
            "color: orange;"
            "font-weight: bold;"
        );
    }
}

void MainWindow::updateDriveModeDisplay()                   //どのドライブモードのテーマにするか決める関数
{
    switch (m_state.driveMode())
    {
    case DriveMode::Normal:
        applyDriveModeTheme(
            "NORMAL",
            QColor(Qt::blue),
            "white",
            "#3b82f6"
        );
        break;

    case DriveMode::Sport:
        applyDriveModeTheme(
            "SPORT",
            QColor(Qt::red),
            "#f2f2f2",
            "#e53935"
        );
        break;

    case DriveMode::Eco:
        applyDriveModeTheme(
            "ECO",
            QColor(Qt::green),
            "#f1f8f1",
            "#2eaf4a"
        );
        break;
    }
}

void MainWindow::updateSimulation()
{
    const double seconds =
        static_cast<double>(config::cycleMilliseconds) / 1000.0;

    const Power previousPower = m_state.power();

    const double previousEconomyDistance =
        m_controller.economy().distanceKm();

    const double previousConsumedLiters =
        m_controller.economy().consumedLiters();

    const bool powerToggleRequested =
        std::find(
            m_input.commands.begin(),
            m_input.commands.end(),
            Command::TogglePower
        ) != m_input.commands.end();

    m_controller.update(m_input, seconds);

    const Power currentPower = m_state.power();

    m_input.commands.clear();


    // POWER OFF → ON
    if (previousPower == Power::Off &&
        currentPower == Power::On)
    {
        m_odometer.startSession();

        m_sessionElapsedSeconds = 0.0;

        m_sessionStartEconomyDistanceKm =
            previousEconomyDistance;

        m_sessionStartConsumedLiters =
            previousConsumedLiters;

        m_sessionStartedAt =
            QDateTime::currentDateTime();
    }


    // POWER ON中の経過時間
    if (currentPower == Power::On)
    {
        m_sessionElapsedSeconds += seconds;
    }


    // Drive Summary用
    bool showDriveSummary = false;

    double summaryDistanceKm = 0.0;
    double summaryElapsedSeconds = 0.0;

    std::optional<double> summaryAverageFuelEconomy;


    // Pキーによる POWER ON → OFF
    if (previousPower == Power::On &&
        currentPower == Power::Off &&
        powerToggleRequested)
    {
        const double sessionEconomyDistance =
            m_controller.economy().distanceKm()
            - m_sessionStartEconomyDistanceKm;

        const double sessionConsumedLiters =
            m_controller.economy().consumedLiters()
            - m_sessionStartConsumedLiters;

        if (sessionConsumedLiters > 0.0)
        {
            summaryAverageFuelEconomy =
                sessionEconomyDistance / sessionConsumedLiters;
        }

        summaryDistanceKm =
            m_odometer.getSinceStartDistance();

        summaryElapsedSeconds =
            m_sessionElapsedSeconds;

        DriveRecord record;

        record.startedAt = m_sessionStartedAt;
        record.endedAt = QDateTime::currentDateTime();

        record.durationSeconds =
            static_cast<int>(std::round(summaryElapsedSeconds));

        record.distanceKm =
            summaryDistanceKm;

        record.averageFuelEconomy =
            summaryAverageFuelEconomy;

        m_databaseManager.saveDriveRecord(record);

        showDriveSummary = true;
    }


    // 距離更新
    m_odometer.update(
        m_state.speedKmh(),
        seconds
    );

    // GUI更新
    updateDisplay();


    // POWER OFF表示へ更新した後にサマリーを表示
    if (showDriveSummary)
    {
        DriveSummaryDialog dialog(
            summaryDistanceKm,
            summaryElapsedSeconds,
            summaryAverageFuelEconomy,
            this
        );

        dialog.exec();
    }
}

void MainWindow::updateDisplay()                                                    //表示処理関数
{
    ui.speedometerWidget->setSpeed(                                                 //速度
        static_cast<int>(std::round(m_state.speedKmh()))                            //四捨五入してint型にcastしてSpeedmeterWidgetに渡す
    );

    switch (m_state.getDistanceDisplay())
    {
    case DistanceDisplay::Odo:
        ui.speedometerWidget->setDistanceInfo(
            "ODO",
            m_odometer.getOdoDistance()
        );
        break;

    case DistanceDisplay::Trip:
        ui.speedometerWidget->setDistanceInfo(
            "TRIP",
            m_odometer.getTripDistance()
        );
        break;

    case DistanceDisplay::SinceStart:
        ui.speedometerWidget->setDistanceInfo(
            "START",
            m_odometer.getSinceStartDistance()
        );
        break;
    }

    const int fuel =                                                                //Fuel
        static_cast<int>(std::round(m_state.fuelPercent()));                        //速度と同じ処理だが、fuelは変更しないのでconst

    ui.fuelBar->setValue(fuel);                                                     //プログレスバーへ反映

    ui.fuelValueLabel->setText(                                                     //fuelの残量を文字列表示
        QString::number(fuel) + " %"
    );

    updateFuelEconomyDisplay();

    const Power currentPower = m_state.power();
    const bool currentAccActive = m_state.accActive();
    const DriveMode currentDriveMode = m_state.driveMode();
    const double currentAccTarget = m_state.accTargetKmh();

    const bool powerChanged =
        m_firstDisplayUpdate ||
        currentPower != m_lastPower;

    const bool accChanged =
        m_firstDisplayUpdate ||
        currentAccActive != m_lastAccActive ||
        powerChanged;

    const bool driveModeChanged =                     //前回表示したDriveModeと違うかの判定
        m_firstDisplayUpdate ||
        currentDriveMode != m_lastDriveMode;

    const bool accTargetChanged =
        m_firstDisplayUpdate ||
        currentAccTarget != m_lastAccTarget ||
        accChanged;

    if (powerChanged)
    {
        updatePowerDisplay();
    }

    if (accChanged)
    {
        updateAccDisplay();
    }

    if (driveModeChanged)
    {
        updateDriveModeDisplay();
    }

    if (accTargetChanged)
    {
        updateAccTargetDisplay();
    }

    m_lastPower = currentPower;                         //今回の状態を前回値として保存
    m_lastAccActive = currentAccActive;
    m_lastDriveMode = currentDriveMode;
    m_lastAccTarget = currentAccTarget;

    m_firstDisplayUpdate = false;
}

void MainWindow::updatePowerDisplay()
{
    if (m_state.power() == Power::Off)
    {
        ui.powerStatusLabel->setText("OFF");
        ui.powerStatusLabel->setStyleSheet(
            "color: gray;"
            "font-weight: bold;"
        );
    }
    else
    {
        ui.powerStatusLabel->setText("ON");
        ui.powerStatusLabel->setStyleSheet(
            "color: green;"
            "font-weight: bold;"
        );
    }
}

void MainWindow::applyDriveModeTheme(                   //実際にGUIへテーマを反映する関数
    const QString& modeText,
    const QColor& accentColor,
    const QString& backgroundColor,
    const QString& fuelColor)
{
    ui.driveModeStatusLabel->setText(modeText);

    ui.driveModeStatusLabel->setStyleSheet(
        QString(
            "color: %1;"
            "font-weight: bold;"
        ).arg(accentColor.name())
    );

    ui.speedometerWidget->setNeedleColor(accentColor);

    ui.centralWidget->setStyleSheet(
        QString(
            "background-color: %1;"
        ).arg(backgroundColor)
    );

    ui.fuelBar->setStyleSheet(
        QString(
            "QProgressBar {"
            "  border: 1px solid gray;"
            "  background-color: #eeeeee;"
            "}"
            "QProgressBar::chunk {"
            "  background-color: %1;"
            "}"
        ).arg(fuelColor)
    );
}

void MainWindow::updateAccTargetDisplay()
{
    if (!m_state.accActive())
    {
        ui.accTargetValueLabel->setText("-- km/h");
        return;
    }

    const int target =
        static_cast<int>(std::round(m_state.accTargetKmh()));

    ui.accTargetValueLabel->setText(
        QString::number(target) + " km/h"
    );
}

void MainWindow::updateFuelEconomyDisplay()
{
    const auto instant = m_controller.economy().instantKmPerLiter();

    const auto average = m_controller.economy().averageKmPerLiter();

    if (instant.has_value())
    {
        ui.instantFuelValueLabel->setText(QString::number(instant.value(), 'f', 1) + " km/L");  //'f'は固定小数点形式、'1'は小数点以下一桁
    }
    else
    {
        ui.instantFuelValueLabel->setText("-- km/L");
    }
    if (average.has_value())
    {
        ui.averageFuelValueLabel->setText(QString::number(average.value(), 'f', 1) + " km/L");
    }
    else
    {
        ui.averageFuelValueLabel->setText("-- km/L");
    }
}