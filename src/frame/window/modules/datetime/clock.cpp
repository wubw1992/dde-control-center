/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     wubw <wubowen_cm@deepin.com>
 *
 * Maintainer: wubw <wubowen_cm@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "clock.h"

#include <QPainter>
#include <QPainterPath>
#include <QIcon>

using namespace DCC_NAMESPACE;
using namespace DCC_NAMESPACE::datetime;

Clock::Clock(QWidget *parent)
    : QWidget(parent)
    , m_drawTicks(true)
    , m_autoNightMode(true)
    , n_bIsUseBlackPlat(true)
{

}

Clock::~Clock()
{

}

QPixmap Clock::getPixmap(const QString name, const QSize size)
{
    const QIcon &icon = QIcon(name);
    const qreal ratio = devicePixelRatioF();
    QPixmap pixmap = icon.pixmap(size * ratio).scaled(size * ratio, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixmap.setDevicePixelRatio(ratio);
    return pixmap;
}

void Clock::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPixmap pix;
    QDateTime datetime(QDateTime::currentDateTime());
    const QTime time(datetime.time());
    QPainter painter(this);
    painter.setRenderHints(QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);

    // draw plate
    painter.save();
    if (n_bIsUseBlackPlat) {
        pix = getPixmap(":/datetime/icons/dcc_clock_black.svg", clockSize);
    } else {
        pix = getPixmap(":/datetime/icons/dcc_clock_white.svg", clockSize);
    }
    painter.translate(width() / 2.0, height() / 2.0);
    painter.drawPixmap(QPointF(-clockSize.width() / 2.0, -clockSize.height() / 2.0), pix);
    painter.restore();

    int nHour = (time.hour() >= 12) ? (time.hour() - 12) : time.hour();
    int nStartAngle = 90;//The image from 0 start , but the clock need from -90 start

    // draw hour hand
    const qreal hourAngle = qreal(nHour * 30 + time.minute() * 30 / 60 + time.second() * 30 / 60 / 60 - nStartAngle);
    painter.save();
    painter.translate(width() / 2.0, height() / 2.0);
    painter.rotate(hourAngle);
    pix = getPixmap(":/datetime/icons/dcc_noun_hour.svg", pointSize);
    painter.drawPixmap(QPointF(-pointSize.width() / 2.0, -pointSize.height() / 2.0), pix);
    painter.restore();

    // draw minute hand
    const qreal minuteAngle = qreal(time.minute() * 6 + time.second() * 6 / 60 - nStartAngle);
    painter.save();
    painter.translate(width() / 2.0, height() / 2.0);
    painter.rotate(minuteAngle);
    pix = getPixmap(":/datetime/icons/dcc_noun_minute.svg", pointSize);
    painter.drawPixmap(QPointF(-pointSize.width() / 2.0, -pointSize.height() / 2.0), pix);
    painter.restore();

    // draw second hand
    const qreal secondAngle = qreal(time.second() * 6 - nStartAngle);
    painter.save();
    painter.translate(width() / 2.0, height() / 2.0);
    painter.rotate(secondAngle);
    pix = getPixmap(":/datetime/icons/dcc_noun_second.svg", pointSize);
    painter.drawPixmap(QPointF(-pointSize.width() / 2.0, -pointSize.height() / 2.0), pix);
    painter.restore();

    painter.end();
}

bool Clock::autoNightMode() const
{
    return m_autoNightMode;
}

void Clock::setAutoNightMode(bool autoNightMode)
{
    if (m_autoNightMode != autoNightMode) {
        m_autoNightMode = autoNightMode;
        update();
    }
}

void Clock::setPlate(bool isBlack)
{
    if (n_bIsUseBlackPlat != isBlack) {
        n_bIsUseBlackPlat = isBlack;
        update();
    }
}

void Clock::setTimeZone(const ZoneInfo &timeZone)
{
    if (m_timeZone == timeZone) return;

    m_timeZone = timeZone;
    update();
}

bool Clock::drawTicks() const
{
    return m_drawTicks;
}

void Clock::setDrawTicks(bool drawTicks)
{
    if (m_drawTicks != drawTicks) {
        m_drawTicks = drawTicks;
        update();
    }
}
