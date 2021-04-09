/*
 *  Copyright © 2018 Ambareesh "Amby" Balaji <ambareeshbalaji@gmail.com>
 *  Copyright © 2018-2021 Hennadii Chernyshchyk <genaloner@gmail.com>
 *
 *  This file is part of Crow Translate.
 *
 *  Crow Translate is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Crow Translate is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a get of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "snippingarea.h"

#include "comparableqpoint.h"

#include <QGuiApplication>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QtMath>
#ifdef Q_OS_LINUX
#include <QX11Info>

#include <xcb/xproto.h>
#endif

SnippingArea::SnippingArea(bool ignoreDevicePixelRatio, QWidget *parent)
    : QWidget(parent)
    , m_devicePixelRatio(ignoreDevicePixelRatio ? 1.0 : devicePixelRatioF())
{
    setMouseTracking(true);
    setAttribute(Qt::WA_StaticContents);
    setWindowFlags(Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::Popup | Qt::WindowStaysOnTopHint);
}

SnippingArea::~SnippingArea()
{
    if (m_regionRememberType == AppSettings::RememberAlways)
        AppSettings().setCropRegion(scaledCropRegion());
}

AppSettings::RegionRememberType SnippingArea::regionRememberType() const
{
    return m_regionRememberType;
}

void SnippingArea::setCaptureOnRelese(bool onRelease)
{
    m_confirmOnRelease = onRelease;
}

void SnippingArea::setShowMagnifier(bool show)
{
    m_showMagnifier = show;
}

void SnippingArea::setApplyLightMask(bool apply)
{
    m_maskColor = apply ? QColor(255, 255, 255, 100) : QColor();
}

void SnippingArea::setRegionRememberType(AppSettings::RegionRememberType type)
{
    m_regionRememberType = type;
}

void SnippingArea::setCropRegion(QRect region)
{
    m_selection = QRect(region.x() * static_cast<int>(m_devicePixelRatioI),
                        region.y() * static_cast<int>(m_devicePixelRatioI),
                        region.width() * static_cast<int>(m_devicePixelRatioI),
                        region.height() * static_cast<int>(m_devicePixelRatioI));
}

void SnippingArea::snip(QMap<const QScreen *, QImage> images)
{
    m_images = qMove(images);
    for (auto it = m_images.constBegin(); it != m_images.constEnd(); ++it) {
        if (it.value().isNull()) {
            QMessageBox message(parentWidget());
            message.setIcon(QMessageBox::Critical);
            message.setText(tr("Unable to snip screen area"));
            message.setInformativeText(tr("Received an invalid image of screen %1.").arg(it.key()->name()));
            message.exec();
            emit cancelled();
            return;
        }
    }

    preparePaint();
    createPixmapFromScreens();
    setGeometryToScreenPixmap();

    m_mouseDragState = MouseState::None;
    if (m_regionRememberType == AppSettings::NeverRemember) {
        m_selection.setRect(0, 0, 0, 0);
        m_startPos = {};
        m_initialTopLeft = {};
        m_mousePos = {};

        m_magnifierAllowed = false;
        m_toggleMagnifier = false;
        m_disableArrowKeys = false;

        m_handleRadius = s_handleRadiusMouse;
        m_handlePositions.fill({});
        setCursor(Qt::CrossCursor);
    } else {
        m_selection = m_selection.intersected(rect());
        setMouseCursor(QCursor::pos());
    }

    setBottomHelpText();
    layoutBottomHelpText();

    update();
    show();
}

void SnippingArea::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        // Clear cached text
        m_bottomLeftHelpText.clear();
        m_bottomRightHelpText.clear();
        break;
    default:
        QWidget::changeEvent(event);
    }
}

void SnippingArea::keyPressEvent(QKeyEvent *event)
{
    const bool shiftPressed = event->modifiers() & Qt::ShiftModifier;
    if (shiftPressed)
        m_toggleMagnifier = true;
    switch (event->key()) {
    case Qt::Key_Escape:
        cancelSelection();
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        acceptSelection();
        break;
    case Qt::Key_Up: {
        if (m_disableArrowKeys) {
            update();
            break;
        }
        const qreal step = (shiftPressed ? 1 : s_magnifierLargeStep);
        const int newPos = boundsUp(qRound(m_selection.top() * m_devicePixelRatio - step), false);
        if (event->modifiers() & Qt::AltModifier) {
            m_selection.setBottom(static_cast<int>(m_devicePixelRatioI) * newPos + m_selection.height());
            m_selection = m_selection.normalized();
        } else {
            m_selection.moveTop(static_cast<int>(m_devicePixelRatioI) * newPos);
        }
        update();
        break;
    }
    case Qt::Key_Right: {
        if (m_disableArrowKeys) {
            update();
            break;
        }
        const qreal step = (shiftPressed ? 1 : s_magnifierLargeStep);
        const int newPos = boundsRight(qRound(m_selection.left() * m_devicePixelRatio + step), false);
        if (event->modifiers() & Qt::AltModifier)
            m_selection.setRight(static_cast<int>(m_devicePixelRatioI) * newPos + m_selection.width());
        else
            m_selection.moveLeft(static_cast<int>(m_devicePixelRatioI) * newPos);
        update();
        break;
    }
    case Qt::Key_Down: {
        if (m_disableArrowKeys) {
            update();
            break;
        }
        const qreal step = (shiftPressed ? 1 : s_magnifierLargeStep);
        const int newPos = boundsDown(qRound(m_selection.top() * m_devicePixelRatio + step), false);
        if (event->modifiers() & Qt::AltModifier)
            m_selection.setBottom(static_cast<int>(m_devicePixelRatioI) * newPos + m_selection.height());
        else
            m_selection.moveTop(static_cast<int>(m_devicePixelRatioI) * newPos);
        update();
        break;
    }
    case Qt::Key_Left: {
        if (m_disableArrowKeys) {
            update();
            break;
        }
        const qreal step = (shiftPressed ? 1 : s_magnifierLargeStep);
        const int newPos = boundsLeft(qRound(m_selection.left() * m_devicePixelRatio - step), false);
        if (event->modifiers() & Qt::AltModifier) {
            m_selection.setRight(static_cast<int>(m_devicePixelRatioI) * newPos + m_selection.width());
            m_selection = m_selection.normalized();
        } else {
            m_selection.moveLeft(static_cast<int>(m_devicePixelRatioI) * newPos);
        }
        update();
        break;
    }
    default:
        break;
    }
    event->accept();
}

void SnippingArea::keyReleaseEvent(QKeyEvent *event)
{
    if (m_toggleMagnifier && !(event->modifiers() & Qt::ShiftModifier)) {
        m_toggleMagnifier = false;
        update();
    }
    event->accept();
}

void SnippingArea::mousePressEvent(QMouseEvent *event)
{
    m_handleRadius = event->source() == Qt::MouseEventNotSynthesized ? s_handleRadiusMouse : s_handleRadiusTouch;

    if (event->button() & Qt::LeftButton) {
       /*
        * NOTE Workaround for https://bugs.kde.org/show_bug.cgi?id=407843
        * If we show the selection Widget when a right click menu is open we lose focus on X.
        * When the user clicks we get the mouse back. We can only grab the keyboard if we already
        * have mouse focus. So just grab it undconditionally here.
        */
        grabKeyboard();
        m_mousePos = event->pos();
        m_magnifierAllowed = true;
        m_mouseDragState = mouseLocation(m_mousePos);
        m_disableArrowKeys = true;
        switch (m_mouseDragState) {
        case MouseState::Outside:
            m_startPos = m_mousePos;
            break;
        case MouseState::Inside:
            m_startPos = m_mousePos;
            m_magnifierAllowed = false;
            m_initialTopLeft = m_selection.topLeft();
            setCursor(Qt::ClosedHandCursor);
            break;
        case MouseState::Top:
        case MouseState::Left:
        case MouseState::TopLeft:
            m_startPos = m_selection.bottomRight();
            break;
        case MouseState::Bottom:
        case MouseState::Right:
        case MouseState::BottomRight:
            m_startPos = m_selection.topLeft();
            break;
        case MouseState::TopRight:
            m_startPos = m_selection.bottomLeft();
            break;
        case MouseState::BottomLeft:
            m_startPos = m_selection.topRight();
            break;
        default:
            break;
        }
    }
    if (m_magnifierAllowed)
        update();
    event->accept();
}

void SnippingArea::mouseMoveEvent(QMouseEvent *event)
{
    m_mousePos = event->pos();
    m_magnifierAllowed = true;
    switch (m_mouseDragState) {
    case MouseState::None:
        setMouseCursor(m_mousePos);
        m_magnifierAllowed = false;
        break;
    case MouseState::TopLeft:
    case MouseState::TopRight:
    case MouseState::BottomRight:
    case MouseState::BottomLeft: {
        const bool afterX = m_mousePos.x() >= m_startPos.x();
        const bool afterY = m_mousePos.y() >= m_startPos.y();
        m_selection.setRect(static_cast<int>(afterX ? m_startPos.x() : m_mousePos.x()),
                           static_cast<int>(afterY ? m_startPos.y() : m_mousePos.y()),
                           static_cast<int>(qAbs(m_mousePos.x() - m_startPos.x()) + (afterX ? m_devicePixelRatioI : 0)),
                           static_cast<int>(qAbs(m_mousePos.y() - m_startPos.y()) + (afterY ? m_devicePixelRatioI : 0)));
        update();
        break;
    }
    case MouseState::Outside:
        m_selection.setRect(static_cast<int>(qMin(m_mousePos.x(), m_startPos.x())),
                           static_cast<int>(qMin(m_mousePos.y(), m_startPos.y())),
                           static_cast<int>(qAbs(m_mousePos.x() - m_startPos.x()) + m_devicePixelRatioI),
                           static_cast<int>(qAbs(m_mousePos.y() - m_startPos.y()) + m_devicePixelRatioI));
        update();
        break;
    case MouseState::Top:
    case MouseState::Bottom: {
        const bool afterY = m_mousePos.y() >= m_startPos.y();
        m_selection.setRect(m_selection.x(),
                           static_cast<int>(afterY ? m_startPos.y() : m_mousePos.y()),
                           m_selection.width(),
                           static_cast<int>(qAbs(m_mousePos.y() - m_startPos.y()) + (afterY ? m_devicePixelRatioI : 0)));
        update();
        break;
    }
    case MouseState::Right:
    case MouseState::Left: {
        const bool afterX = m_mousePos.x() >= m_startPos.x();
        m_selection.setRect(static_cast<int>(afterX ? m_startPos.x() : m_mousePos.x()),
                           m_selection.y(),
                           static_cast<int>(qAbs(m_mousePos.x() - m_startPos.x()) + (afterX ? m_devicePixelRatioI : 0)),
                           m_selection.height());
        update();
        break;
    }
    case MouseState::Inside: {
        m_magnifierAllowed = false;
        /*
         * We use some math here to figure out if the diff with which we
         * move the rectangle with moves it out of bounds,
         * in which case we adjust the diff to not let that happen
         */
        // New top left point of the rectangle
        QPoint newTopLeft = ((m_mousePos - m_startPos + m_initialTopLeft) * m_devicePixelRatio).toPoint();

        const QRect newRect(newTopLeft, m_selection.size() * m_devicePixelRatio);

        const QRect translatedScreensRect = m_screensRect.translated(-m_screensRect.topLeft());
        if (!translatedScreensRect.contains(newRect)) {
            // Keep the item inside the scene screen region bounding rect.
            newTopLeft.setX(qMin(translatedScreensRect.right() - newRect.width(), qMax(newTopLeft.x(), translatedScreensRect.left())));
            newTopLeft.setY(qMin(translatedScreensRect.bottom() - newRect.height(), qMax(newTopLeft.y(), translatedScreensRect.top())));
        }

        m_selection.moveTo(newTopLeft * m_devicePixelRatioI);
        update();
        break;
    }
    default:
        break;
    }

    event->accept();
}

void SnippingArea::mouseReleaseEvent(QMouseEvent *event)
{
    switch (event->button()) {
    case Qt::LeftButton:
        if (m_mouseDragState == MouseState::Outside && m_confirmOnRelease) {
            acceptSelection();
            return;
        }
        m_disableArrowKeys = false;
        if (m_mouseDragState == MouseState::Inside)
            setCursor(Qt::OpenHandCursor);
        break;
    case Qt::RightButton:
        m_selection.setWidth(0);
        m_selection.setHeight(0);
        break;
    default:
        break;
    }
    event->accept();
    m_mouseDragState = MouseState::None;
    update();
}

void SnippingArea::mouseDoubleClickEvent(QMouseEvent *event)
{
    event->accept();
    if (event->button() == Qt::LeftButton && m_selection.contains(event->pos()))
        acceptSelection();
}

void SnippingArea::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.eraseRect(rect());

    for (auto i = m_images.constBegin(); i != m_images.constEnd(); ++i) {
        const QImage &screenImage = i.value();
        const QScreen* screen = i.key();

        QRect rectToDraw = screen->geometry().translated(-m_screensRect.topLeft());
        const qreal dpr = screenImage.width() / static_cast<qreal>(rectToDraw.width());
        const qreal dprI = 1.0 / dpr;

        QBrush brush(screenImage);
        brush.setTransform(QTransform::fromScale(dprI, dprI));

        rectToDraw.moveTopLeft(rectToDraw.topLeft() / m_devicePixelRatio);
        rectToDraw.setSize(rectToDraw.size() * m_devicePixelRatio);

        painter.setBrushOrigin(rectToDraw.topLeft());
        painter.fillRect(rectToDraw, brush);
    }

    if (!m_selection.size().isEmpty() || m_mouseDragState != MouseState::None) {
        const QRectF innerRect = m_selection.adjusted(1, 1, -1, -1);
        if (innerRect.width() > 0 && innerRect.height() > 0) {
            painter.setPen(m_strokeColor);
            painter.drawLine(m_selection.topLeft(), m_selection.topRight());
            painter.drawLine(m_selection.bottomRight(), m_selection.topRight());
            painter.drawLine(m_selection.bottomRight(), m_selection.bottomLeft());
            painter.drawLine(m_selection.bottomLeft(), m_selection.topLeft());
        }

        QRectF top(0, 0, width(), m_selection.top());
        QRectF right(m_selection.right(), m_selection.top(), width() - m_selection.right(), m_selection.height());
        QRectF bottom(0, m_selection.bottom() + 1, width(), height() - m_selection.bottom());
        QRectF left(0, m_selection.top(), m_selection.left(), m_selection.height());
        for (const QRectF &rect : {top, right, bottom, left})
            painter.fillRect(rect, m_maskColor);

        bool dragHandlesVisible = false;
        if (m_mouseDragState == MouseState::None) {
            dragHandlesVisible = true;
            drawDragHandles(painter);
        } else if (m_magnifierAllowed && (m_showMagnifier ^ m_toggleMagnifier)) {
            drawMagnifier(painter);
        }
        drawSelectionSizeTooltip(painter, dragHandlesVisible);
        drawBottomHelpText(painter);
    } else {
        drawMidHelpText(painter);
    }
}

int SnippingArea::boundsLeft(int newTopLeftX, bool mouse)
{
    if (newTopLeftX < 0) {
        if (mouse)
            m_startPos.setX(m_startPos.x() + newTopLeftX * m_devicePixelRatioI); // Tweak startPos to prevent rectangle from getting stuck
        newTopLeftX = 0;
    }

    return newTopLeftX;
}

int SnippingArea::boundsRight(int newTopLeftX, bool mouse)
{
    // The max X coordinate of the top left point
    const int realMaxX = qRound((width() - m_selection.width()) * devicePixelRatioF());
    const int xOffset = newTopLeftX - realMaxX;
    if (xOffset > 0) {
        if (mouse)
            m_startPos.setX(m_startPos.x() + xOffset * m_devicePixelRatioI);
        newTopLeftX = realMaxX;
    }

    return newTopLeftX;
}

int SnippingArea::boundsUp(int newTopLeftY, bool mouse)
{
    if (newTopLeftY < 0) {
        if (mouse)
            m_startPos.setY(m_startPos.y() + newTopLeftY * m_devicePixelRatioI);
        newTopLeftY = 0;
    }

    return newTopLeftY;
}

int SnippingArea::boundsDown(int newTopLeftY, bool mouse)
{
    // The max Y coordinate of the top left point
    const int realMaxY = qRound((height() - m_selection.height()) * m_devicePixelRatio);
    const int yOffset = newTopLeftY - realMaxY;
    if (yOffset > 0) {
        if (mouse)
            m_startPos.setY(m_startPos.y() + yOffset * m_devicePixelRatioI);
        newTopLeftY = realMaxY;
    }

    return newTopLeftY;
}

void SnippingArea::drawBottomHelpText(QPainter &painter) const
{
    if (m_selection.intersects(m_bottomHelpBorderBox))
        return;

    painter.setBrush(m_labelBackgroundColor);
    painter.setPen(m_labelForegroundColor);
    painter.setFont(font());
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.drawRect(m_bottomHelpBorderBox);
    painter.setRenderHint(QPainter::Antialiasing, true);

    int topOffset = m_bottomHelpContentPos.y();
    for (int i = 0; i < m_bottomLeftHelpText.size(); i++) {
        const QStaticText &leftText = m_bottomLeftHelpText[i];
        const QSize leftSize = leftText.size().toSize();
        painter.drawStaticText(m_bottomHelpGridLeftWidth - leftSize.width(), topOffset, leftText);
        for (const QStaticText &rightTextPart : m_bottomRightHelpText[i]) {
            const QSize rightItemSize = rightTextPart.size().toSize();
            painter.drawStaticText(m_bottomHelpGridLeftWidth + s_bottomHelpBoxPairSpacing, topOffset, rightTextPart);
            topOffset += rightItemSize.height();
        }
        if (i != s_bottomHelpMaxLength)
            topOffset += s_bottomHelpBoxMarginBottom;
    }
}

void SnippingArea::drawDragHandles(QPainter &painter)
{
    // Rectangular region
    const qreal left = m_selection.x();
    const qreal centerX = left + m_selection.width() / 2.0;
    const qreal right = left + m_selection.width();
    const qreal top = m_selection.y();
    const qreal centerY = top + m_selection.height() / 2.0;
    const qreal bottom = top + m_selection.height();

    // Rectangle too small: make handles free-floating
    qreal offset = 0;
    // Rectangle too close to screen edges: move handles on that edge inside the rectangle, so they're still visible
    qreal offsetTop = 0;
    qreal offsetRight = 0;
    qreal offsetBottom = 0;
    qreal offsetLeft = 0;

    const qreal minDragHandleSpace = 4 * m_handleRadius + 2 * s_minSpacingBetweenHandles;
    const qreal minEdgeLength = qMin(m_selection.width(), m_selection.height());
    if (minEdgeLength < minDragHandleSpace) {
        offset = (minDragHandleSpace - minEdgeLength) / 2.0;
    } else {
        const QRect translatedScreensRect = m_screensRect.translated(-m_screensRect.topLeft());
        const int penWidth = painter.pen().width();

        offsetTop = top - translatedScreensRect.top() - m_handleRadius;
        offsetTop = (offsetTop >= 0) ? 0 : offsetTop;

        offsetRight = translatedScreensRect.right() - right - m_handleRadius + penWidth;
        offsetRight = (offsetRight >= 0) ? 0 : offsetRight;

        offsetBottom = translatedScreensRect.bottom() - bottom - m_handleRadius + penWidth;
        offsetBottom = (offsetBottom >= 0) ? 0 : offsetBottom;

        offsetLeft = left - translatedScreensRect.left() - m_handleRadius;
        offsetLeft = (offsetLeft >= 0) ? 0 : offsetLeft;
    }

    // Top-left handle
    m_handlePositions[0] = QPointF{left - offset - offsetLeft, top - offset - offsetTop};
    // Top-right handle
    m_handlePositions[1] = QPointF{right + offset + offsetRight, top - offset - offsetTop};
    // Bottom-right handle
    m_handlePositions[2] = QPointF{right + offset + offsetRight, bottom + offset + offsetBottom};
    // Bottom-left
    m_handlePositions[3] = QPointF{left - offset - offsetLeft, bottom + offset + offsetBottom};
    // Top-center handle
    m_handlePositions[4] = QPointF{centerX, top - offset - offsetTop};
    // Right-center handle
    m_handlePositions[5] = QPointF{right + offset + offsetRight, centerY};
    // Bottom-center handle
    m_handlePositions[6] = QPointF{centerX, bottom + offset + offsetBottom};
    // Left-center handle
    m_handlePositions[7] = QPointF{left - offset - offsetLeft, centerY};

    // Start path
    QPainterPath path;

    // Add handles to the path
    for (QPointF handlePosition : qAsConst(m_handlePositions))
        path.addEllipse(handlePosition, m_handleRadius, m_handleRadius);

    // Draw the path
    painter.fillPath(path, m_strokeColor);
}

void SnippingArea::drawMagnifier(QPainter &painter) const
{
    const int pixels = 2 * s_magPixels + 1;
    auto magX = static_cast<int>(m_mousePos.x() * m_devicePixelRatio - s_magPixels);
    int offsetX = 0;
    if (magX < 0) {
        offsetX = magX;
        magX = 0;
    } else {
        const int maxX = m_screenPixmap.width() - pixels;
        if (magX > maxX) {
            offsetX = magX - maxX;
            magX = maxX;
        }
    }
    auto magY = static_cast<int>(m_mousePos.y() * m_devicePixelRatio - s_magPixels);
    int offsetY = 0;
    if (magY < 0) {
        offsetY = magY;
        magY = 0;
    } else {
        const int maxY = m_screenPixmap.height() - pixels;
        if (magY > maxY) {
            offsetY = magY - maxY;
            magY = maxY;
        }
    }
    QRectF magniRect(magX, magY, pixels, pixels);

    qreal drawPosX = m_mousePos.x() + s_magOffset + pixels * static_cast<qreal>(s_magZoom) / 2;
    if (drawPosX > width() - pixels * static_cast<qreal>(s_magZoom) / 2)
        drawPosX = m_mousePos.x() - s_magOffset - pixels * static_cast<qreal>(s_magZoom) / 2;
    qreal drawPosY = m_mousePos.y() + s_magOffset + pixels * static_cast<qreal>(s_magZoom) / 2;
    if (drawPosY > height() - pixels * static_cast<qreal>(s_magZoom) / 2)
        drawPosY = m_mousePos.y() - s_magOffset - pixels * static_cast<qreal>(s_magZoom) / 2;
    QPointF drawPos(drawPosX, drawPosY);
    QRectF crossHairTop(drawPos.x() + s_magZoom * (offsetX - 0.5), drawPos.y() - s_magZoom * (s_magPixels + 0.5), s_magZoom, s_magZoom * (s_magPixels + offsetY));
    QRectF crossHairRight(drawPos.x() + s_magZoom * (0.5 + offsetX), drawPos.y() + s_magZoom * (offsetY - 0.5), s_magZoom * (s_magPixels - offsetX), s_magZoom);
    QRectF crossHairBottom(drawPos.x() + s_magZoom * (offsetX - 0.5), drawPos.y() + s_magZoom * (0.5 + offsetY), s_magZoom, s_magZoom * (s_magPixels - offsetY));
    QRectF crossHairLeft(drawPos.x() - s_magZoom * (s_magPixels + 0.5), drawPos.y() + s_magZoom * (offsetY - 0.5), s_magZoom * (s_magPixels + offsetX), s_magZoom);
    QRectF crossHairBorder(drawPos.x() - s_magZoom * (s_magPixels + 0.5) - 1, drawPos.y() - s_magZoom * (s_magPixels + 0.5) - 1, pixels * s_magZoom + 2, pixels * s_magZoom + 2);
    const auto frag = QPainter::PixmapFragment::create(drawPos, magniRect, s_magZoom, s_magZoom);

    painter.fillRect(crossHairBorder, m_labelForegroundColor);
    painter.drawPixmapFragments(&frag, 1, m_screenPixmap, QPainter::OpaqueHint);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    for (const QRectF &rect : {crossHairTop, crossHairRight, crossHairBottom, crossHairLeft})
        painter.fillRect(rect, m_crossColor);
}

void SnippingArea::drawMidHelpText(QPainter &painter) const
{
    painter.fillRect(rect(), m_maskColor);

    QFont midHelpTextFont = font();
    midHelpTextFont.setPointSize(s_midHelpTextFontSize);
    painter.setFont(midHelpTextFont);

    const QString midHelpText = tr("Click and drag to draw a selection rectangle,\nor press Esc to quit");
    QRect textSize = painter.boundingRect(QRect(), Qt::AlignCenter, midHelpText);
    const QRect primaryGeometry = QGuiApplication::primaryScreen()->geometry().translated(-m_screensRect.topLeft());
    QPoint pos((primaryGeometry.width() - textSize.width()) / 2 + primaryGeometry.x() / static_cast<int>(m_devicePixelRatio),
               (primaryGeometry.height() - textSize.height()) / 2 + primaryGeometry.y() / static_cast<int>(m_devicePixelRatio));

    painter.setBrush(m_labelBackgroundColor);
    QPen pen(m_labelForegroundColor);
    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawRoundedRect(QRect(pos.x() - 20, pos.y() - 20, textSize.width() + 40, textSize.height() + 40), 4, 4);

    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawText(QRect(pos, textSize.size()), Qt::AlignCenter, midHelpText);
}

void SnippingArea::drawSelectionSizeTooltip(QPainter &painter, bool dragHandlesVisible) const
{
    /*
     * Set the selection size and finds the most appropriate position:
     * - vertically centered inside the selection if the box is not covering the a large part of selection
     * - on top of the selection if the selection x position fits the box height plus some margin
     * - at the bottom otherwise
     */
    const QString selectionSizeText = QString::fromUtf8(u8"%1\u00D7%2").arg(qRound(m_selection.width() * m_devicePixelRatio)).arg(qRound(m_selection.height() * m_devicePixelRatio));
    const QRect selectionSizeTextRect = painter.boundingRect(QRect(), 0, selectionSizeText);

    const int selectionBoxWidth = selectionSizeTextRect.width() + s_selectionBoxPaddingX * 2;
    const int selectionBoxHeight = selectionSizeTextRect.height() + s_selectionBoxPaddingY * 2;
    const int selectionBoxX = qBound(0,
                                     static_cast<int>(m_selection.x()) + (static_cast<int>(m_selection.width()) - selectionSizeTextRect.width()) / 2 - s_selectionBoxPaddingX,
                                     width() - selectionBoxWidth);
    int selectionBoxY;
    if ((m_selection.width() >= s_selectionSizeThreshold) && (m_selection.height() >= s_selectionSizeThreshold)) {
        // Show inside the box
        selectionBoxY = static_cast<int>(m_selection.y() + (m_selection.height() - selectionSizeTextRect.height()) / 2);
    } else {
        // Show on top by default, above the drag Handles if they're visible
        if (dragHandlesVisible) {
            selectionBoxY = static_cast<int>(m_handlePositions[4].y() - m_handleRadius - selectionBoxHeight - s_selectionBoxMarginY);
            if (selectionBoxY < 0)
                selectionBoxY = static_cast<int>(m_handlePositions[6].y() + m_handleRadius + s_selectionBoxMarginY);
        } else {
            selectionBoxY = static_cast<int>(m_selection.y() - selectionBoxHeight - s_selectionBoxMarginY);
            if (selectionBoxY < 0)
                selectionBoxY = static_cast<int>(m_selection.y() + m_selection.height() + s_selectionBoxMarginY);
        }
    }

    // Now do the actual box, border, and text drawing
    painter.setBrush(m_labelBackgroundColor);
    painter.setPen(m_labelForegroundColor);
    const QRect selectionBoxRect(selectionBoxX,
                                 selectionBoxY,
                                 selectionBoxWidth,
                                 selectionBoxHeight);

    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.drawRect(selectionBoxRect);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawText(selectionBoxRect, Qt::AlignCenter, selectionSizeText);
}

void SnippingArea::setMouseCursor(QPointF pos)
{
    const MouseState mouseState = mouseLocation(pos);
    if (mouseState == MouseState::Outside)
        setCursor(Qt::CrossCursor);
    else if (MouseState::TopLeftOrBottomRight & mouseState)
        setCursor(Qt::SizeFDiagCursor);
    else if (MouseState::TopRightOrBottomLeft & mouseState)
        setCursor(Qt::SizeBDiagCursor);
    else if (MouseState::TopOrBottom & mouseState)
        setCursor(Qt::SizeVerCursor);
    else if (MouseState::RightOrLeft & mouseState)
        setCursor(Qt::SizeHorCursor);
    else
        setCursor(Qt::OpenHandCursor);
}

SnippingArea::MouseState SnippingArea::mouseLocation(QPointF pos) const
{
    if (isPointInsideCircle(m_handlePositions[0], m_handleRadius * s_increaseDragAreaFactor, pos))
        return MouseState::TopLeft;
    if (isPointInsideCircle(m_handlePositions[1], m_handleRadius * s_increaseDragAreaFactor, pos))
        return MouseState::TopRight;
    if (isPointInsideCircle(m_handlePositions[2], m_handleRadius * s_increaseDragAreaFactor, pos))
        return MouseState::BottomRight;
    if (isPointInsideCircle(m_handlePositions[3], m_handleRadius * s_increaseDragAreaFactor, pos))
        return MouseState::BottomLeft;
    if (isPointInsideCircle(m_handlePositions[4], m_handleRadius * s_increaseDragAreaFactor, pos))
        return MouseState::Top;
    if (isPointInsideCircle(m_handlePositions[5], m_handleRadius * s_increaseDragAreaFactor, pos))
        return MouseState::Right;
    if (isPointInsideCircle(m_handlePositions[6], m_handleRadius * s_increaseDragAreaFactor, pos))
        return MouseState::Bottom;
    if (isPointInsideCircle(m_handlePositions[7], m_handleRadius * s_increaseDragAreaFactor, pos))
        return MouseState::Left;

    // Rectangle can be resized when border is dragged, if it's big enough
    if (m_selection.width() >= 100 && m_selection.height() >= 100) {
        if (isInRange(m_selection.x(), m_selection.x() + m_selection.width(), pos.x())) {
            if (isWithinThreshold(pos.y() - m_selection.y(), s_borderDragAreaSize))
                return MouseState::Top;
            if (isWithinThreshold(pos.y() - m_selection.y() - m_selection.height(), s_borderDragAreaSize))
                return MouseState::Bottom;
        }
        if (isInRange(m_selection.y(), m_selection.y() + m_selection.height(), pos.y())) {
            if (isWithinThreshold(pos.x() - m_selection.x(), s_borderDragAreaSize))
                return MouseState::Left;
            if (isWithinThreshold(pos.x() - m_selection.x() - m_selection.width(), s_borderDragAreaSize))
                return MouseState::Right;
        }
    }

    if (m_selection.contains(pos.toPoint()))
        return MouseState::Inside;
    return MouseState::Outside;
}

QRect SnippingArea::scaledCropRegion() const
{
    return {qRound(m_selection.x() * m_devicePixelRatio),
            qRound(m_selection.y() * m_devicePixelRatio),
            qRound(m_selection.width() * m_devicePixelRatio),
            qRound(m_selection.height() * m_devicePixelRatio)};
}

QPixmap SnippingArea::selectedPixmap() const
{
#ifdef Q_OS_LINUX
    if (!QX11Info::isPlatformX11()) {
        // Wayland case
        qreal maxDpr = 1.0;
        for (const QScreen *screen: QGuiApplication::screens()) {
            if (screen->devicePixelRatio() > maxDpr)
                maxDpr = screen->devicePixelRatio();
        }

        QPixmap output(m_selection.size() * maxDpr);
        QPainter painter(&output);

        for (auto it = m_screenToDpr.constBegin(); it != m_screenToDpr.constEnd(); ++it) {
            const QScreen *screen = it.key();
            const QRect screenRect = screen->geometry();

            if (m_selection.intersects(screenRect)) {
                const QPoint pos = screenRect.topLeft();
                qreal dpr = it.value();

                QRect intersected = screenRect.intersected(m_selection);

                // Converts to screen size & position
                QRect pixelOnScreenIntersected;
                pixelOnScreenIntersected.moveTopLeft((intersected.topLeft() - pos) * dpr);
                pixelOnScreenIntersected.setWidth(intersected.width() * static_cast<int>(dpr));
                pixelOnScreenIntersected.setHeight(intersected.height() * static_cast<int>(dpr));

                QPixmap screenOutput = QPixmap::fromImage(m_images.value(screen).copy(pixelOnScreenIntersected));

                /*
                 * Short path when single screen
                 * Keep native screen resolution
                 */
                if (intersected.size() == m_selection.size())
                    return screenOutput;

                // Upscale the image according to max screen dpr, to keep the image not distorted
                const double dprI = maxDpr / dpr;
                QBrush brush(screenOutput);
                brush.setTransform(QTransform::fromScale(dprI, dprI));
                intersected.moveTopLeft((intersected.topLeft() - m_selection.topLeft()) * maxDpr);
                intersected.setSize(intersected.size() * maxDpr);
                painter.setBrushOrigin(intersected.topLeft());
                painter.fillRect(intersected, brush);
            }
        }
        return output;
    }
#endif
    return m_screenPixmap.copy(scaledCropRegion());
}

void SnippingArea::createPixmapFromScreens()
{
    QMap<ComparableQPoint, QPair<qreal, QSize>> input;
    for (auto it = m_images.constBegin(); it != m_images.constEnd(); ++it) {
        const QScreen* screen = it.key();
        const QImage &screenImage = it.value();
        input.insert(screen->geometry().topLeft(), {screenImage.width() / static_cast<qreal>(screen->size().width()), screenImage.size()});
    }
    const QMap<ComparableQPoint, ComparableQPoint> pointsTranslationMap = computeCoordinatesAfterScaling(input);

    m_screenPixmap = QPixmap(m_screensRect.width(), m_screensRect.height());
    QPainter painter(&m_screenPixmap);
    // Geometry can have negative coordinates, so it is necessary to subtract the upper left point, because coordinates on the widget are counted from 0
    for (auto it = m_images.constBegin(); it != m_images.constEnd(); ++it)
        painter.drawImage(pointsTranslationMap.value(it.key()->geometry().topLeft()) - m_screensRect.topLeft(), it.value());
}

void SnippingArea::setGeometryToScreenPixmap()
{
#ifdef Q_OS_LINUX
    if (QX11Info::isPlatformX11()) {
        /*
         * Even though we want the quick editor window to be placed at (0, 0) in the native
         * pixels, we cannot really specify a window position of (0, 0) if HiDPI support is on.
         *
         * The main reason for that is that Qt will scale the window position relative to the
         * upper left corner of the screen where the quick editor is on in order to perform
         * a conversion from the device-independent coordinates to the native pixels.
         *
         * Since (0, 0) in the device-independent pixels may not correspond to (0, 0) in the
         * native pixels, we use XCB API to place the quick editor window at (0, 0).
         */
        const uint32_t coordinates[] = {0, 0};
        xcb_configure_window(QX11Info::connection(), winId(), XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, coordinates);
        resize(m_screenPixmap.width(), m_screenPixmap.height());
        return;
    }
#endif
    setGeometry(m_screensRect);
}

void SnippingArea::layoutBottomHelpText()
{
    int maxRightWidth = 0;
    int contentWidth = 0;
    int contentHeight = 0;
    m_bottomHelpGridLeftWidth = 0;
    for (int i = 0; i < m_bottomLeftHelpText.size(); i++) {
        const QSize leftSize = m_bottomLeftHelpText[i].size().toSize();
        m_bottomHelpGridLeftWidth = qMax(m_bottomHelpGridLeftWidth, leftSize.width());
        for (const QStaticText &rightTextPart : qAsConst(m_bottomRightHelpText[i])) {
            const QSize rightItemSize = rightTextPart.size().toSize();
            maxRightWidth = qMax(maxRightWidth, rightItemSize.width());
            contentHeight += rightItemSize.height();
        }
        contentWidth = qMax(contentWidth, m_bottomHelpGridLeftWidth + maxRightWidth + s_bottomHelpBoxPairSpacing);
        contentHeight += (i != s_bottomHelpMaxLength ? s_bottomHelpBoxMarginBottom : 0);
    }
    const QRect primaryGeometry = QGuiApplication::primaryScreen()->geometry().translated(-m_screensRect.topLeft());
    m_bottomHelpContentPos.setX((primaryGeometry.width() - contentWidth) / 2 + primaryGeometry.x() / static_cast<int>(m_devicePixelRatio));
    m_bottomHelpContentPos.setY((primaryGeometry.height() + primaryGeometry.y() / static_cast<int>(m_devicePixelRatio)) - contentHeight - 8);
    m_bottomHelpGridLeftWidth += m_bottomHelpContentPos.x();
    m_bottomHelpBorderBox.setRect(m_bottomHelpContentPos.x() - s_bottomHelpBoxPaddingX,
                                  m_bottomHelpContentPos.y() - s_bottomHelpBoxPaddingY,
                                  contentWidth + s_bottomHelpBoxPaddingX * 2,
                                  contentHeight + s_bottomHelpBoxPaddingY * 2 - 1);
}

void SnippingArea::setBottomHelpText()
{
    Q_ASSERT_X(m_bottomLeftHelpText.size() == m_bottomRightHelpText.size(), "setButtomHelpText", "The left and right columns must be the same size");

    const int expectedLinesCount = m_confirmOnRelease && m_selection.size().isEmpty() ? s_bottomHelpOnReleaseLength : s_bottomHelpNormalLength;

    // The text is already set
    if (m_bottomLeftHelpText.size() == expectedLinesCount)
        return;

    m_bottomLeftHelpText.clear();
    m_bottomRightHelpText.clear();
    m_bottomLeftHelpText.reserve(expectedLinesCount);
    m_bottomRightHelpText.reserve(expectedLinesCount);

    m_bottomLeftHelpText.append(QStaticText(tr("Confirm:")));
    if (expectedLinesCount == s_bottomHelpOnReleaseLength)
        m_bottomRightHelpText.append({QStaticText(tr("Release left-click")), QStaticText(tr("Enter"))});
    else
        m_bottomRightHelpText.append({QStaticText(tr("Double-click")), QStaticText(tr("Enter"))});

    m_bottomLeftHelpText.append(QStaticText(tr("Create new selection rectangle:")));
    m_bottomRightHelpText.append({QStaticText(tr("Drag outside selection rectangle")), QStaticText(tr("+ Shift: Magnifier"))});

    if (expectedLinesCount == s_bottomHelpNormalLength) {
        m_bottomLeftHelpText.append(QStaticText(tr("Move selection rectangle:")));
        m_bottomRightHelpText.append({QStaticText(tr("Drag inside selection rectangle")), QStaticText(tr("Arrow keys")), QStaticText(tr("+ Shift: Move in 1 pixel steps"))});

        m_bottomLeftHelpText.append(QStaticText(tr("Resize selection rectangle:")));
        m_bottomRightHelpText.append({QStaticText(tr("Drag handles")), QStaticText(tr("Arrow keys + Alt")), QStaticText(tr("+ Shift: Resize in 1 pixel steps"))});

        m_bottomLeftHelpText.append(QStaticText(tr("Reset selection:")));
        m_bottomRightHelpText.append({QStaticText(tr("Right-click"))});
    }

    m_bottomLeftHelpText.append(QStaticText(tr("Cancel:")));
    m_bottomRightHelpText.append({QStaticText(tr("Esc key"))});

    for (int i = 0; i < m_bottomLeftHelpText.size(); ++i) {
        prepare(m_bottomLeftHelpText[i]);
        for (QStaticText &rightPart : m_bottomRightHelpText[i])
            prepare(rightPart);
    }
}

void SnippingArea::preparePaint()
{
    m_screenToDpr.clear();
    m_screensRect = {};
    for (auto i = m_images.constBegin(); i != m_images.constEnd(); ++i) {
        const QImage &screenImage = i.value();
        const QScreen* screen = i.key();

        const qreal dpr = screenImage.width() / static_cast<qreal>(screen->geometry().width());
        m_screenToDpr.insert(screen, dpr);

#ifdef Q_OS_LINUX
        const QRect virtualScreenRect(screen->geometry().topLeft(), QX11Info::isPlatformX11() ? screenImage.size() : screenImage.size() / dpr);
#else
        const QRect virtualScreenRect(screen->geometry().topLeft(), screenImage.size());
#endif
        m_screensRect = m_screensRect.united(virtualScreenRect);
    }
}

void SnippingArea::acceptSelection()
{
    if (!m_selection.isEmpty()) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
        const qreal dpi = QGuiApplication::screenAt(QCursor::pos())->logicalDotsPerInch();
#else
        // Until Qt 5.10 there was no way to get the screen with the current cursor
        const qreal dpi = QGuiApplication::primaryScreen()->logicalDotsPerInch();
#endif
        emit snipped(selectedPixmap(), static_cast<int>(dpi));
    }
    hide();
    releaseKeyboard();
}

void SnippingArea::cancelSelection()
{
    releaseKeyboard();
    hide();
    emit cancelled();
}

QMap<ComparableQPoint, ComparableQPoint> SnippingArea::computeCoordinatesAfterScaling(const QMap<ComparableQPoint, QPair<qreal, QSize>> &outputsRect)
{
    QMap<ComparableQPoint, ComparableQPoint> translationMap;

    for (auto it = outputsRect.keyBegin(); it != outputsRect.keyEnd(); ++it)
        translationMap.insert(*it, *it);

    for (auto i = outputsRect.constBegin(); i != outputsRect.constEnd(); ++i) {
        const ComparableQPoint p = i.key();
        const QSize &size = i.value().second;
        const double dpr = i.value().first;
        if (!qFuzzyCompare(dpr, 1.0)) {
            // Must update all coordinates of next rects
            int newWidth = size.width();
            int newHeight = size.height();

            int deltaX = newWidth - (size.width());
            int deltaY = newHeight - (size.height());

            // For the next size
            for (auto i2 = outputsRect.constFind(p); i2 != outputsRect.constEnd(); ++i2) {
                ComparableQPoint point = i2.key();
                ComparableQPoint finalPoint = translationMap.value(point);

                if (point.x() >= newWidth + p.x() - deltaX)
                    finalPoint.setX(finalPoint.x() + deltaX);
                if (point.y() >= newHeight + p.y() - deltaY)
                    finalPoint.setY(finalPoint.y() + deltaY);
                // Update final position point with the necessary deltas
                translationMap.insert(point, finalPoint);
            }
        }
    }

    return translationMap;
}

bool SnippingArea::isPointInsideCircle(QPointF circleCenter, qreal radius, QPointF point)
{
    return qPow(point.x() - circleCenter.x(), 2) + qPow(point.y() - circleCenter.y(), 2) <= qPow(radius, 2);
}

bool SnippingArea::isInRange(qreal low, qreal high, qreal value)
{
    return value >= low && value <= high;
}

bool SnippingArea::isWithinThreshold(qreal offset, qreal threshold)
{
    return qFabs(offset) <= threshold;
}

void SnippingArea::prepare(QStaticText &text) const
{
    text.prepare(QTransform(), font());
    text.setPerformanceHint(QStaticText::AggressiveCaching);
}
