#include "SpeedometerWidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QFont>

#include <algorithm>
#include <cmath>

SpeedometerWidget::SpeedometerWidget(QWidget* parent)
    : QWidget(parent)                                           //親クラスのコンストラクタを呼ぶ
{
    setMinimumSize(300, 300);
}

void SpeedometerWidget::setSpeed(int speed)                     //車速の状態を変更する関数
{
    m_speed = std::clamp(speed, 0, m_maximumSpeed);

    update();
}

int SpeedometerWidget::speed() const
{
    return m_speed;
}

void SpeedometerWidget::paintEvent(QPaintEvent* event)          //状態を見て描画する関数
{
    Q_UNUSED(event);

    QPainter painter(this);                                     //このwidgetをキャンパスとして描画するpainterを用意する
    painter.setRenderHint(QPainter::Antialiasing);              //アンチエイリアシングで描画補正

    const int side = std::min(width(), height());               //width()→現在widgetの横幅、height()→縦幅のうち小さいほうを使う
    const QPointF center(width() / 2.0, height() / 2.0);        //widgetの中心点
    const double radius = side * 0.40;                          //辺の40%を半径に設定

    // 外周
    QPen outerPen(Qt::black);
    outerPen.setWidth(6);

    painter.setPen(outerPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(center, radius, radius);

    // 目盛り
    QPen tickPen(Qt::black);
    tickPen.setWidth(3);
    painter.setPen(tickPen);

    // 目盛り数字用フォント
    QFont tickFont;
    tickFont.setPointSize(10);
    tickFont.setBold(true);
    painter.setFont(tickFont);

    for (int speed = 0; speed <= m_maximumSpeed; speed += 20)
    {
        const double ratio =                                    //最大車速に対して何割か
            static_cast<double>(speed) / m_maximumSpeed;

        const double angleDeg =                                 //速度の割合をメータ上の角度へ変換
            135.0 + ratio * 270.0;

        const double angleRad =                                 //ラジアンへ変換
            angleDeg * 3.14159265358979323846 / 180.0;

        // 目盛り線
        const QPointF outer(                                    //円周上の点の座標
            center.x() + radius * std::cos(angleRad),
            center.y() + radius * std::sin(angleRad));

        const QPointF inner(                                    //上を少し短くした座標
            center.x() + radius * 0.88 * std::cos(angleRad),
            center.y() + radius * 0.88 * std::sin(angleRad));

        painter.drawLine(inner, outer);                         //上2個の座標を線で繋ぎ目盛り線にする

        // 目盛り数字
        const double textRadius = radius * 0.72;                //目盛線より中心点に近い位置を作る      

        const QPointF textCenter(                               //目盛り数字用の座標
            center.x() + textRadius * std::cos(angleRad),
            center.y() + textRadius * std::sin(angleRad));

        const QRectF textRect(                                  //textCenterを中心とした長方形領域
            textCenter.x() - 20,                                //x位置
            textCenter.y() - 12,                                //y位置
            40,                                                 //横幅
            24);                                                //高さ

        painter.drawText(                                       //長方形の中央に目盛り数字を書く
            textRect,
            Qt::AlignCenter,
            QString::number(speed));

    }

    //ODO/Trip表示
    QFont distanceFont;
    distanceFont.setPointSize(11);
    distanceFont.setBold(true);
    painter.setFont(distanceFont);

    const QRectF distanceRect(
        center.x() - 100,
        center.y() + radius * 0.75,
        200,
        30);

    const QString distanceText =
        m_distanceLabel + " " + QString::number(m_distanceKm, 'f', 1) + " km";

    painter.drawText(
        distanceRect,
        Qt::AlignCenter,
        distanceText);

    // 速度に応じた針
    const double ratio = static_cast<double>(m_speed) / m_maximumSpeed;
    const double angleDeg = 135.0 + ratio * 270.0;
    const double angleRad = angleDeg * 3.14159265358979323846 / 180.0;

    const QPointF needleEnd(                                            //針の先端座標
        center.x() + radius * 0.75 * std::cos(angleRad),
        center.y() + radius * 0.75 * std::sin(angleRad));

    QPen needlePen(m_needleColor);                                      //針の描画設定（色、幅、先端形状）
    needlePen.setWidth(5);
    needlePen.setCapStyle(Qt::RoundCap);

    painter.setPen(needlePen);
    painter.drawLine(center, needleEnd);                                //針の描画

    // 中心点
    painter.setBrush(Qt::black);                                        //内側カラー
    painter.setPen(Qt::NoPen);                                          //輪郭線
    painter.drawEllipse(center, 8, 8);                                  //中心点描画

    // デジタル速度表示
    painter.setPen(Qt::black);

    QFont speedFont;
    speedFont.setPointSize(28);
    speedFont.setBold(true);

    painter.setFont(speedFont);

    const QRectF speedRect(                                             //表示領域作り
        center.x() - 100,
        center.y() + radius * 0.30,
        200,
        50);

    painter.drawText(                                                   //上の領域の中心に描画
        speedRect,
        Qt::AlignCenter,
        QString::number(m_speed));

    QFont unitFont;                                                     //単位はここで別に描画
    unitFont.setPointSize(12);
    painter.setFont(unitFont);

    const QRectF unitRect(
        center.x() - 100,
        center.y() + radius * 0.55,
        200,
        30);

    painter.drawText(
        unitRect,
        Qt::AlignCenter,
        "km/h");
}

void SpeedometerWidget::setNeedleColor(const QColor& color)
{
    m_needleColor = color;
    update();
}

void SpeedometerWidget::setMaximumSpeed(int maximumSpeed)
{
    if (maximumSpeed <= 0)
    {
        return;
    }

    m_maximumSpeed = maximumSpeed;

    m_speed = std::clamp(
        m_speed,
        0,
        m_maximumSpeed
    );

    update();
}

void SpeedometerWidget::setDistanceInfo(const QString& label, double distanceKm)
{
    m_distanceLabel = label;
    m_distanceKm = distanceKm;

    update();
}