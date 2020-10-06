/*
 *  Copyright (C) 2018 Ambareesh "Amby" Balaji <ambareeshbalaji@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef QUICKEDITOR_H
#define QUICKEDITOR_H

#include "settings/appsettings.h"

#include <QKeyEvent>
#include <QPainter>
#include <QStaticText>
#include <QWidget>

#include <vector>

class QMouseEvent;
class AppSettings;

class QuickEditor : public QWidget
{
    Q_OBJECT

public:
    explicit QuickEditor(QWidget *parent = nullptr);

    void loadSettings(const AppSettings &settings);
    void capture();

signals:
    void grabDone(const QPixmap &thePixmap);
    void grabCancelled();

private:
    enum MouseState : short {
        None = 0, // 0000
        Inside = 1 << 0, // 0001
        Outside = 1 << 1, // 0010
        TopLeft = 5, //101
        Top = 17, // 10001
        TopRight = 9, // 1001
        Right = 33, // 100001
        BottomRight = 6, // 110
        Bottom = 18, // 10010
        BottomLeft = 10, // 1010
        Left = 34, // 100010
        TopLeftOrBottomRight = TopLeft & BottomRight, // 100
        TopRightOrBottomLeft = TopRight & BottomLeft, // 1000
        TopOrBottom = Top & Bottom, // 10000
        RightOrLeft = Right & Left, // 100000
    };

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *) override;

    int boundsLeft(int newTopLeftX, const bool mouse = true);
    int boundsRight(int newTopLeftX, const bool mouse = true);
    int boundsUp(int newTopLeftY, const bool mouse = true);
    int boundsDown(int newTopLeftY, const bool mouse = true);

    void drawBottomHelpText(QPainter &painter);
    void drawDragHandles(QPainter &painter);
    void drawMagnifier(QPainter &painter);
    void drawMidHelpText(QPainter &painter);
    void drawSelectionSizeTooltip(QPainter &painter, bool dragHandlesVisible);

    void setMouseCursor(const QPointF &pos);
    MouseState mouseLocation(const QPointF &pos);

    void prepare(QStaticText &text);
    void setBottomHelpText();
    void layoutBottomHelpText();

    void acceptSelection();

    static QPoint fromNative(const QPoint &point, const QScreen *screen);
    static QSize fromNative(const QSize &size, const QScreen *screen);
    static QRect fromNativePixels(const QRect &rect, const QScreen *screen);
    static bool isPointInsideCircle(const QPointF &circleCenter, qreal radius, const QPointF &point);
    static bool isInRange(qreal low, qreal high, qreal value);
    static bool isWithinThreshold(qreal offset, qreal threshold);

    static constexpr int s_handleRadiusMouse = 9;
    static constexpr int s_handleRadiusTouch = 12;
    static constexpr qreal s_increaseDragAreaFactor = 2.0;
    static constexpr int s_minSpacingBetweenHandles = 20;
    static constexpr int s_borderDragAreaSize = 10;

    static constexpr int s_selectionSizeThreshold = 100;

    static constexpr int s_selectionBoxPaddingX = 5;
    static constexpr int s_selectionBoxPaddingY = 4;
    static constexpr int s_selectionBoxMarginY = 5;

    static constexpr int s_bottomHelpMaxLength = 6;

    static constexpr int s_bottomHelpBoxPaddingX = 12;
    static constexpr int s_bottomHelpBoxPaddingY = 8;
    static constexpr int s_bottomHelpBoxPairSpacing = 6;
    static constexpr int s_bottomHelpBoxMarginBottom = 5;
    static constexpr int s_midHelpTextFontSize = 12;

    static constexpr int s_magnifierLargeStep = 15;

    static constexpr int s_magZoom = 5;
    static constexpr int s_magPixels = 16;
    static constexpr int s_magOffset = 32;

    static bool s_bottomHelpTextPrepared;

    const qreal m_dprI = 1.0 / devicePixelRatioF();
    const QColor m_strokeColor = palette().highlight().color();
    const QColor m_crossColor = QColor::fromRgbF(m_strokeColor.redF(), m_strokeColor.greenF(), m_strokeColor.blueF(), 0.7);
    const QColor m_labelForegroundColor = palette().windowText().color();
    const QColor m_labelBackgroundColor = QColor::fromRgbF(palette().light().color().redF(),
                                                     palette().light().color().greenF(),
                                                     palette().light().color().blueF(),
                                                     0.85);

    QColor m_maskColor;
    QRectF m_selection;
    QPointF m_startPos;
    QPointF m_initialTopLeft;
    std::pair<QStaticText, std::vector<QStaticText>> m_bottomHelpText[s_bottomHelpMaxLength];
    QRect m_bottomHelpBorderBox;
    QPoint m_bottomHelpContentPos;
    int m_bottomHelpGridLeftWidth = 0;
    MouseState m_mouseDragState = MouseState::None;
    QPixmap m_pixmap;
    QPointF m_mousePos;
    bool m_magnifierAllowed = false;
    bool m_showMagnifier = AppSettings::defaultShowMagnifier();
    bool m_toggleMagnifier = false;
    bool m_releaseToCapture = AppSettings::defaultCaptureOnRelease();
    AppSettings::RegionRememberType m_rememberRegion = AppSettings::defaultRegionRememberType();
    bool m_disableArrowKeys = false;
    int m_bottomHelpLength = s_bottomHelpMaxLength;

    // Midpoints of handles
    QVector<QPointF> m_handlePositions = QVector<QPointF>{8};
    // Radius of handles is either s_handleRadiusMouse or s_handleRadiusTouch
    int m_handleRadius = s_handleRadiusMouse;
};

#endif // QUICKEDITOR_H
