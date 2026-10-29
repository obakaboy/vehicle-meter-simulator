#pragma once

#include <QWidget>

#include<QColor>

#include<QString>

class SpeedometerWidget : public QWidget                        //QWidgetがもつサイズ,表示,再描画,イベント処理などの機能を継承
{
public:
    explicit SpeedometerWidget(QWidget* parent = nullptr);

    void setSpeed(int speed);
    int speed() const;

    void setNeedleColor(const QColor& color);

    void setMaximumSpeed(int maximumSpeed);

    void setDistanceInfo(const QString& label, double distanceKm);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    int m_speed = 0;
    int m_maximumSpeed = 240;
    QColor m_needleColor = Qt::blue;
    QString m_distanceLabel = "ODO";
    double m_distanceKm = 0.0;
};