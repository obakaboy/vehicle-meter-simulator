#pragma once

#include <QtWidgets/QMainWindow>
#include <QTimer>
#include <QColor>
#include <QString>
#include <QDateTime>

#include "ui_MainWindow.h"
#include "VehicleState.h"
#include "VehicleController.h"
#include "InputTypes.h"
#include "Odometer.h"
#include "DatabaseManager.h"

class QKeyEvent;                                                    //下のポインタを使うための前方宣言

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);                          //コンストラクタ
    ~MainWindow();                                                  //デストラクタ

protected:
    void keyPressEvent(QKeyEvent* event) override;                  //キーが押された時の情報が入ったQkeyEventオブジェクトへのポインタを引数としている
    void keyReleaseEvent(QKeyEvent* event) override;                //QMainWindowに仮想関数として存在するのでoverride

private:
    void updateSimulation();
    void updateDisplay();
    void updateAccDisplay();
    void updateDriveModeDisplay();
    void updatePowerDisplay();
    void updateFuelEconomyDisplay();

    void applyDriveModeTheme(                                       //渡された設定を実際のGUIへ反映する関数
        const QString& modeText,
        const QColor& accentColor,
        const QString& backgroundColor,
        const QString& fuelColor
    );

    void updateAccTargetDisplay();
    double m_lastAccTarget = -1.0;

    Ui::MainWindowClass ui;

    VehicleState m_state;
    VehicleController m_controller;                                 //この順番にすることでVehicleController(VehicleState& state)が成立する
    InputFrame m_input;

    QTimer m_timer;

    Power m_lastPower = Power::Off;                                  //「m_last...」は前回GUIに表示したときの状態を覚えておく変数
    bool m_lastAccActive = false;
    DriveMode m_lastDriveMode = DriveMode::Normal;

    bool m_firstDisplayUpdate = true;                               //初回表示時に前回値と同じでも強制的に全部表示するためにtrue
    Odometer m_odometer;

    double m_sessionElapsedSeconds = 0.0;
    double m_sessionStartEconomyDistanceKm = 0.0;
    double m_sessionStartConsumedLiters = 0.0;

    QDateTime m_sessionStartedAt;

    DatabaseManager m_databaseManager;
};