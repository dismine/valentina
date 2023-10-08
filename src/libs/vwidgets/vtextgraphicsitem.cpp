/************************************************************************
 **
 **  @file   vtextgraphicsitem.cpp
 **  @author Bojan Kverh
 **  @date   June 16, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
 **  <https://gitlab.com/smart-pattern/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include <QColor>
#include <QDebug>
#include <QFlags>
#include <QFont>
#include <QGraphicsItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPen>
#include <QPoint>
#include <QStyleOptionGraphicsItem>
#include <QtMath>
#include <array>

#include "../ifc/exception/vexception.h"
#include "../vformat/vsinglelineoutlinechar.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/def.h"
#include "../vmisc/literals.h"
#include "../vmisc/svgfont/svgdef.h"
#include "../vmisc/svgfont/vsvgfont.h"
#include "../vmisc/svgfont/vsvgfontdatabase.h"
#include "../vmisc/vabstractvalapplication.h"
#include "theme/vscenestylesheet.h"
#include "vtextgraphicsitem.h"

namespace
{
constexpr qreal resizeSquare = MmToPixel(3.);
constexpr qreal rotateCircle = MmToPixel(2.);
constexpr int rotateRect = 60;
constexpr int rotateArc = 50;
constexpr qreal minW = MmToPixel(4.) + resizeSquare;
constexpr qreal minH = MmToPixel(4.) + resizeSquare;
constexpr int activeZ = 10;

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetBoundingRect calculates the bounding box around rectBB rectangle, rotated around its center by dRot degrees
 * @param rectBB rectangle of interest
 * @param dRot rectangle rotation
 * @return bounding box around rectBB rotated by dRot
 */
auto GetBoundingRect(const QRectF &rectBB, qreal dRot) -> QRectF
{
    std::array<QPointF, 4> apt = {rectBB.topLeft(), rectBB.topRight(), rectBB.bottomLeft(), rectBB.bottomRight()};
    QPointF ptCenter = rectBB.center();

    qreal dX1 = 0;
    qreal dX2 = 0;
    qreal dY1 = 0;
    qreal dY2 = 0;

    double dAng = qDegreesToRadians(dRot);
    for (std::size_t i = 0; i < 4; ++i)
    {
        QPointF pt = apt.at(i) - ptCenter;
        qreal dX = pt.x() * cos(dAng) + pt.y() * sin(dAng);
        qreal dY = -pt.x() * sin(dAng) + pt.y() * cos(dAng);

        if (i == 0)
        {
            dX1 = dX2 = dX;
            dY1 = dY2 = dY;
        }
        else
        {
            if (dX < dX1)
            {
                dX1 = dX;
            }
            else if (dX > dX2)
            {
                dX2 = dX;
            }
            if (dY < dY1)
            {
                dY1 = dY;
            }
            else if (dY > dY2)
            {
                dY2 = dY;
            }
        }
    }
    QRectF rect;
    rect.setTopLeft(ptCenter + QPointF(dX1, dY1));
    rect.setWidth(dX2 - dX1);
    rect.setHeight(dY2 - dY1);
    return rect;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::VTextGraphicsItem constructor
 * @param pParent pointer to the parent item
 */
VTextGraphicsItem::VTextGraphicsItem(ItemType type, QGraphicsItem *pParent)
  : VPieceItem(pParent),
    m_itemType(type)
{
    m_inactiveZ = 2;
    SetSize(minW, minH);
    setZValue(m_inactiveZ);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::SetFont sets the item font
 * @param fnt font to be used in item
 */
void VTextGraphicsItem::SetFont(const QFont &fnt)
{
    m_tm.SetFont(fnt);
}

//---------------------------------------------------------------------------------------------------------------------
void VTextGraphicsItem::SetSVGFontFamily(const QString &fntFamily)
{
    m_tm.SetSVGFontFamily(fntFamily);
}

//---------------------------------------------------------------------------------------------------------------------
void VTextGraphicsItem::SetSVGFontPointSize(int pointSize)
{
    m_tm.SetSVGFontPointSize(pointSize);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::paint redraws the item content
 * @param painter pointer to the QPainter in use
 * @param option pointer to the object containing the actual label rectangle
 * @param widget not used
 */
void VTextGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)
    painter->fillRect(m_rectBoundingBox, VSceneStylesheet::PatternPieceStyle().LabelBackgroundColor());
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter->setPen(VSceneStylesheet::PatternPieceStyle().LabelTextColor());

    PaintLabel(painter);

    // now draw the features specific to non-normal modes
    if (m_eMode != mNormal)
    {
        // outline the rectangle
        painter->setPen(QPen(VSceneStylesheet::PatternPieceStyle().LabelModeColor(), 2, Qt::DashLine));
        painter->drawRect(boundingRect().adjusted(1, 1, -1, -1));

        if (m_eMode != mRotate)
        {
            // draw the resize square
            painter->setPen(VSceneStylesheet::PatternPieceStyle().LabelModeColor());
            painter->setBrush(VSceneStylesheet::PatternPieceStyle().LabelModeColor());
            painter->drawRect(m_rectResize.adjusted(-1, -1, -1, -1));

            if (m_eMode == mResize)
            {
                // draw the resize diagonal lines
                painter->drawLine(1, 1, qFloor(m_rectBoundingBox.width()) - 1, qFloor(m_rectBoundingBox.height()) - 1);
                painter->drawLine(1, qFloor(m_rectBoundingBox.height()) - 1, qFloor(m_rectBoundingBox.width()) - 1, 1);
            }
        }
        else
        {
            // in rotate mode, draw the circle in the middle
            painter->setPen(VSceneStylesheet::PatternPieceStyle().LabelModeColor());
            painter->setBrush(VSceneStylesheet::PatternPieceStyle().LabelModeColor());
            painter->drawEllipse(QPointF(m_rectBoundingBox.width() / 2, m_rectBoundingBox.height() / 2), rotateCircle,
                                 rotateCircle);
            if (m_rectBoundingBox.width() > minW * 3 && m_rectBoundingBox.height() > minH * 3)
            {
                painter->setPen(QPen(VSceneStylesheet::PatternPieceStyle().LabelModeColor(), 3));
                painter->setBrush(Qt::NoBrush);
                // and then draw the arc in each of the corners
                int iTop = rotateRect - rotateArc;
                int iLeft = rotateRect - rotateArc;
                int iRight = qRound(m_rectBoundingBox.width()) - rotateRect;
                int iBottom = qRound(m_rectBoundingBox.height()) - rotateRect;
                painter->drawArc(iLeft, iTop, rotateArc, rotateArc, 180 * 16, -90 * 16);
                painter->drawArc(iRight, iTop, rotateArc, rotateArc, 90 * 16, -90 * 16);
                painter->drawArc(iLeft, iBottom, rotateArc, rotateArc, 270 * 16, -90 * 16);
                painter->drawArc(iRight, iBottom, rotateArc, rotateArc, 0 * 16, -90 * 16);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::SetSize Tries to set the label size to (fW, fH). If any of those is too small, the label
 * size does not change.
 * @param fW label width
 * @param fH label height
 */
void VTextGraphicsItem::SetSize(qreal fW, qreal fH)
{
    prepareGeometryChange();
    m_rectBoundingBox.setTopLeft(QPointF(0, 0));
    m_rectBoundingBox.setWidth(fW);
    m_rectBoundingBox.setHeight(fH);
    m_rectResize.setTopLeft(QPointF(fW - resizeSquare, fH - resizeSquare));
    m_rectResize.setWidth(resizeSquare);
    m_rectResize.setHeight(resizeSquare);
    setTransformOriginPoint(m_rectBoundingBox.center());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::Update sets the correct size and font size and redraws the label
 */
void VTextGraphicsItem::Update()
{
    CorrectLabel();
    UpdateBox();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::IsContained checks if the bounding box around rotated rectBB is contained in
 * the parent. If that is not the case, it calculates the amount of movement needed to put it inside the parent
 * and write it into dX, dY
 * @param rectBB bounding box in question
 * @param dRot bounding box rotation in degrees
 * @param dX horizontal translation needed to put the box inside parent item
 * @param dY vertical translation needed to put the box inside parent item
 * @return true, if rectBB is contained in parent item and false otherwise
 */
auto VTextGraphicsItem::IsContained(QRectF rectBB, qreal dRot, qreal &dX, qreal &dY) const -> bool
{
    QRectF rectParent = parentItem()->boundingRect();
    rectBB = GetBoundingRect(rectBB, dRot);
    dX = 0;
    dY = 0;

    if (not rectParent.contains(rectBB))
    {
        if (rectParent.left() - rectBB.left() > fabs(dX))
        {
            dX = rectParent.left() - rectBB.left();
        }
        else if (rectBB.right() - rectParent.right() > fabs(dX))
        {
            dX = rectParent.right() - rectBB.right();
        }

        if (rectParent.top() - rectBB.top() > fabs(dY))
        {
            dY = rectParent.top() - rectBB.top();
        }
        else if (rectBB.bottom() - rectParent.bottom() > fabs(dY))
        {
            dY = rectParent.bottom() - rectBB.bottom();
        }

        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::UpdateData Updates the detail label
 * @param qsName name of detail
 * @param data reference to VPatternPieceData
 */
void VTextGraphicsItem::UpdateData(const QString &qsName, const VPieceLabelData &data, const VContainer *pattern)
{
    m_tm.Update(qsName, data, pattern);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::UpdateData Updates the pattern label
 * @param pDoc pointer to the pattern object
 */
void VTextGraphicsItem::UpdateData(VAbstractPattern *pDoc, const VContainer *pattern)
{
    m_tm.Update(pDoc, pattern);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::GetTextLines returns the number of lines of text to show
 * @return number of lines of text
 */
auto VTextGraphicsItem::GetTextLines() const -> vsizetype
{
    return m_tm.GetSourceLinesCount();
}

//---------------------------------------------------------------------------------------------------------------------
void VTextGraphicsItem::SetPieceName(const QString &name)
{
    m_pieceName = name;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::GetFontSize returns the currently used text base font size
 * @return current text base font size
 */
auto VTextGraphicsItem::GetFontSize() const -> int
{
    int size = m_tm.GetFont().pixelSize();
    if (size == -1)
    {
        QFontMetrics fontMetrics(m_tm.GetFont());
        size = fontMetrics.height();
    }

    return size;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::mousePressEvent handles left button mouse press events
 * @param pME pointer to QGraphicsSceneMouseEvent object
 */
void VTextGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *pME)
{
    if (pME->button() == Qt::LeftButton && pME->type() != QEvent::GraphicsSceneMouseDoubleClick &&
        (flags() & QGraphicsItem::ItemIsMovable))
    {
        if (m_moveType == NotMovable)
        {
            pME->ignore();
            return;
        }

        pME->accept();
        // record the parameters of the mouse press. Specially record the position
        // of the press as the origin for the following operations
        m_ptStartPos = pos();
        m_ptStart = pME->scenePos();
        m_szStart = m_rectBoundingBox.size();
        m_ptRotCenter = mapToParent(m_rectBoundingBox.center());
        m_dAngle = GetAngle(mapToParent(pME->pos()));
        m_dRotation = rotation();
        // in rotation mode, do not do any changes here, because user might want to
        // rotate the label more.

        if ((m_moveType & AllModifications) == AllModifications)
        {
            AllUserModifications(pME->pos());
            setZValue(activeZ);
            Update();
        }
        else if (m_moveType & IsRotatable)
        {
            if (m_moveType & IsResizable)
            {
                AllUserModifications(pME->pos());
            }
            else if (m_moveType & IsMovable)
            {
                UserRotateAndMove();
            }
            else
            {
                m_eMode = mRotate;
                SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
            }
            setZValue(activeZ);
            Update();
        }
        else if (m_moveType & IsResizable)
        {
            if (m_moveType & IsRotatable)
            {
                AllUserModifications(pME->pos());
            }
            else if (m_moveType & IsMovable)
            {
                UserMoveAndResize(pME->pos());
            }
            setZValue(activeZ);
            Update();
        }
        else if (m_moveType & IsMovable)
        {
            if (m_moveType & IsRotatable)
            {
                UserRotateAndMove();
            }
            else if (m_moveType & IsResizable)
            {
                UserMoveAndResize(pME->pos());
            }
            else
            {
                m_eMode = mMove;
                SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
            }

            setZValue(activeZ);
            Update();
        }
        else
        {
            pME->ignore();
        }
    }
    else
    {
        pME->ignore();
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::mouseMoveEvent handles mouse move events
 * @param pME pointer to QGraphicsSceneMouseEvent object
 */
void VTextGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *pME)
{
    if (m_eMode == mMove && m_moveType & IsMovable)
    {
        MoveLabel(pME);
    }
    else if (m_eMode == mResize && m_moveType & IsResizable)
    {
        ResizeLabel(pME);
    }
    else if (m_eMode == mRotate && m_moveType & IsRotatable)
    {
        RotateLabel(pME);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::mouseReleaseEvent handles left button mouse release events
 * @param pME pointer to QGraphicsSceneMouseEvent object
 */
void VTextGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *pME)
{
    if (pME->button() != Qt::LeftButton)
    {
        return;
    }

    // restore the cursor
    if ((m_eMode == mMove || m_eMode == mRotate || m_eMode == mResize) && (flags() & QGraphicsItem::ItemIsMovable))
    {
        SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
    }

    double dDist = fabs(pME->scenePos().x() - m_ptStart.x()) + fabs(pME->scenePos().y() - m_ptStart.y());
    // determine if this was just press/release (bShort == true) or user did some operation between press and
    // release
    bool bShort = (dDist < 2);

    if (m_eMode == mMove || m_eMode == mResize)
    { // if user just pressed and released the button, we must switch the mode to rotate
        // but if user did some operation (move/resize), emit the proper signal and update the label
        if (bShort)
        {
            if (m_bReleased && m_moveType & IsRotatable)
            {
                m_eMode = mRotate;
                UpdateBox();
            }
        }
        else if (m_eMode == mMove && m_moveType & IsMovable)
        {
            emit SignalMoved(pos());
            UpdateBox();
        }
        else if (m_moveType & IsResizable)
        {
            emit SignalResized(m_rectBoundingBox.width());
            Update();
        }
    }
    else
    { // in rotate mode, if user did just press/release, switch to move mode
        if (bShort && (m_moveType & IsMovable || m_moveType & IsResizable))
        {
            m_eMode = mMove;
        }
        else if (m_moveType & IsRotatable)
        {
            // if user rotated the item, emit proper signal and update the label
            emit SignalRotated(rotation());
        }
        UpdateBox();
    }
    m_bReleased = true;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::hoverMoveEvent checks if cursor has to be changed
 * @param pHE pointer to the scene hover event
 */
void VTextGraphicsItem::hoverMoveEvent(QGraphicsSceneHoverEvent *pHE)
{
    if (m_eMode == mResize && m_moveType & IsResizable)
    {
        if (m_rectResize.contains(pHE->pos()))
        {
            setCursor(Qt::SizeFDiagCursor);
        }
        else
        {
            SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
        }
    }
    VPieceItem::hoverMoveEvent(pHE);
}

//---------------------------------------------------------------------------------------------------------------------
void VTextGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent *pME)
{
    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
    }
    VPieceItem::hoverEnterEvent(pME);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::UpdateBox redraws the label content
 */
void VTextGraphicsItem::UpdateBox()
{
    update(m_rectBoundingBox);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::UpdateFont sets the text font size, so that the entire text will
 *  just fit into the label bounding box
 */
void VTextGraphicsItem::CorrectLabel()
{
    qreal dX;
    qreal dY;
    QRectF rectBB;
    rectBB.setTopLeft(pos());
    rectBB.setSize(m_rectBoundingBox.size());
    if (not IsContained(rectBB, rotation(), dX, dY))
    {
        // put the label inside the pattern
        setPos(pos().x() + dX, pos().y() + dY);
    }
    UpdateBox();
}

//---------------------------------------------------------------------------------------------------------------------
void VTextGraphicsItem::AllUserModifications(const QPointF &pos)
{
    if (m_eMode != mRotate)
    {
        UserMoveAndResize(pos);
    }
    else
    {
        SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VTextGraphicsItem::UserRotateAndMove()
{
    if (m_eMode != mRotate)
    {
        m_eMode = mMove;
    }
    SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
}

//---------------------------------------------------------------------------------------------------------------------
void VTextGraphicsItem::UserMoveAndResize(const QPointF &pos)
{
    if (m_rectResize.contains(pos))
    {
        m_eMode = mResize;
        setCursor(Qt::SizeFDiagCursor);
    }
    else
    {
        m_eMode = mMove; // block later if need
        SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VTextGraphicsItem::MoveLabel(QGraphicsSceneMouseEvent *pME)
{
    const QPointF ptDiff = pME->scenePos() - m_ptStart;
    // in move mode move the label along the mouse move from the origin
    QPointF pt = m_ptStartPos + ptDiff;
    QRectF rectBB;
    rectBB.setTopLeft(pt);
    rectBB.setWidth(m_rectBoundingBox.width());
    rectBB.setHeight(m_rectBoundingBox.height());
    // before moving label to a new position, check if it will still be inside the parent item
    qreal dX;
    qreal dY;
    if (not IsContained(rectBB, rotation(), dX, dY))
    {
        pt.setX(pt.x() + dX);
        pt.setY(pt.y() + dY);
    }
    setPos(pt);
    UpdateBox();
}

//---------------------------------------------------------------------------------------------------------------------
void VTextGraphicsItem::ResizeLabel(QGraphicsSceneMouseEvent *pME)
{
    QLineF vectorDiff(m_ptStart, pME->scenePos());
    vectorDiff.setAngle(vectorDiff.angle() + m_dRotation);
    const QPointF ptDiff = vectorDiff.p2() - m_ptStart;

    // in resize mode, resize the label along the mouse move from the origin
    QPointF pt;
    QSizeF sz;

    if (m_moveType & IsMovable)
    {
        const qreal newWidth = m_szStart.width() + ptDiff.x();
        const qreal newHeight = m_szStart.height() + ptDiff.y();
        if (newWidth <= minW || newHeight <= minH)
        {
            return;
        }

        pt = m_ptStartPos;
        sz = QSizeF(newWidth, newHeight);
    }
    else
    {
        const qreal newWidth = m_szStart.width() + ptDiff.x() * 2.0;
        const qreal newHeight = m_szStart.height() + ptDiff.y() * 2.0;
        if (newWidth <= minW || newHeight <= minH)
        {
            return;
        }

        pt = QPointF(m_ptRotCenter.x() - newWidth / 2.0, m_ptRotCenter.y() - newHeight / 2.0);
        sz = QSizeF(m_szStart.width() + ptDiff.x() * 2.0, m_szStart.height() + ptDiff.y() * 2.0);
    }

    QRectF rectBB;
    rectBB.setTopLeft(pt);
    rectBB.setSize(sz);
    // before resizing the label to a new size, check if it will still be inside the parent item
    qreal dX;
    qreal dY;
    if (IsContained(rectBB, rotation(), dX, dY))
    {
        if (not(m_moveType & IsMovable))
        {
            setPos(pt);
        }
    }
    else
    {
        return;
    }

    SetSize(sz.width(), sz.height());
    Update();
    emit SignalShrink();
}

//---------------------------------------------------------------------------------------------------------------------
void VTextGraphicsItem::RotateLabel(QGraphicsSceneMouseEvent *pME)
{
    // if the angle from the original position is small (0.5 degrees), just remeber the new angle
    // new angle will be the starting angle for rotation
    if (fabs(m_dAngle) < 0.01)
    {
        m_dAngle = GetAngle(mapToParent(pME->pos()));
        return;
    }

    QRectF rectBB;
    rectBB.setTopLeft(m_ptStartPos);
    rectBB.setWidth(m_rectBoundingBox.width());
    rectBB.setHeight(m_rectBoundingBox.height());

    // calculate the angle difference from the starting angle
    double dAng = qRadiansToDegrees(GetAngle(mapToParent(pME->pos())) - m_dAngle);

    // check if the rotated label will be inside the parent item and then rotate it
    qreal dX;
    qreal dY;
    if (IsContained(rectBB, m_dRotation + dAng, dX, dY))
    {
        setRotation(m_dRotation + dAng);
        Update();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VTextGraphicsItem::PaintLabel(QPainter *painter)
{
    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    if (settings->GetSingleLineFonts())
    {
        PaintLabelSVGFont(painter);
    }
    else
    {
        PaintLabelOutlineFont(painter);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VTextGraphicsItem::PaintLabelOutlineFont(QPainter *painter)
{
    const QRectF boundingRect = this->boundingRect();
    const int iW = qFloor(boundingRect.width());
    QFont fnt = m_tm.GetFont();
    const QVector<TextLine> labelLines = m_tm.GetLabelSourceLines(iW, fnt);

    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    bool textAsPaths = settings->GetSingleStrokeOutlineFont();

    // draw text lines
    qreal iY = 0;
    qreal penWidth = VAbstractApplication::VApp()->Settings()->WidthHairLine();

    if (textAsPaths)
    {
        QPen pen = painter->pen();
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        pen.setWidthF(penWidth);
        painter->setPen(pen);

        iY += penWidth * 2;
    }

    painter->setClipRect(boundingRect);

    for (const auto &tl : labelLines)
    {
        fnt.setPointSize(qMax(m_tm.GetFont().pointSize() + tl.m_iFontSize, 1));
        fnt.setBold(tl.m_bold);
        fnt.setItalic(tl.m_italic);

        VSingleLineOutlineChar corrector(fnt);
        if (!corrector.IsPopulated())
        {
            corrector.LoadCorrections(settings->GetPathFontCorrections());
        }

        QString qsText = tl.m_qsText;
        QFontMetrics fm(fnt);
        qreal lineHeight = fm.height();

        if (iY + lineHeight > boundingRect.height())
        {
            lineHeight = boundingRect.height() - iY;
        }

        if (textAsPaths)
        {
            QString qsText = tl.m_qsText;
            qreal dX = 0;
            if (tl.m_eAlign == 0 || (tl.m_eAlign & Qt::AlignLeft) > 0)
            {
                dX = 0;
            }
            else if ((tl.m_eAlign & Qt::AlignHCenter) > 0)
            {
                dX = (boundingRect.width() - TextWidth(fm, qsText)) / 2;
            }
            else if ((tl.m_eAlign & Qt::AlignRight) > 0)
            {
                dX = boundingRect.width() - TextWidth(fm, qsText);
            }

            QPainterPath path;
            int w = 0;
            for (auto c : qAsConst(qsText))
            {
                path.addPath(corrector.DrawChar(w, static_cast<qreal>(fm.ascent()), c));
                w += TextWidth(fm, c);
            }

            QTransform matrix;
            matrix.translate(dX, iY);
            path = matrix.map(path);

            painter->save();
            painter->setBrush(QBrush(Qt::NoBrush));
            painter->drawPath(path);
            painter->restore();
        }
        else
        {
            painter->save();
            painter->setFont(fnt);
            painter->drawText(QRectF(0, iY, iW, lineHeight * 2), static_cast<int>(tl.m_eAlign), qsText);
            painter->restore();
        }

        // check if the next line will go out of bounds
        qreal nextStep = textAsPaths ? iY + fm.height() + penWidth * 2 : iY + fm.height();
        if (nextStep > boundingRect.height())
        {
            NotEnoughSpace();
            break;
        }

        iY = nextStep + m_tm.GetSpacing();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VTextGraphicsItem::PaintLabelSVGFont(QPainter *painter)
{
    VSvgFontDatabase *db = VAbstractApplication::VApp()->SVGFontDatabase();
    VSvgFontEngine engine = db->FontEngine(m_tm.GetSVGFontFamily(), SVGFontStyle::Normal, SVGFontWeight::Normal,
                                           m_tm.GetSVGFontPointSize());

    VSvgFont svgFont = engine.Font();
    if (!svgFont.IsValid())
    {
        QString errorMsg = tr("Invalid SVG font '%1'. Fallback to outline font.").arg(svgFont.Name());
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        PaintLabelOutlineFont(painter);
        return;
    }

    qreal penWidth = VAbstractApplication::VApp()->Settings()->WidthHairLine();
    QPen pen = painter->pen();
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setWidthF(penWidth);
    painter->setPen(pen);

    const QRectF boundingRect = this->boundingRect().adjusted(-penWidth, -penWidth, -penWidth, -penWidth);

    const qreal iW = boundingRect.width();
    const QVector<TextLine> labelLines = m_tm.GetLabelSourceLines(qFloor(iW), svgFont, penWidth / 2);

    // draw text lines
    qreal iY = 0;
    for (const auto &tl : labelLines)
    {
        VSvgFont lineFont = svgFont;
        lineFont.SetBold(tl.m_bold);
        lineFont.SetItalic(tl.m_italic);
        lineFont.SetPointSize(svgFont.PointSize() + tl.m_iFontSize);

        engine = db->FontEngine(lineFont);

        QString qsText = tl.m_qsText;
        qreal lineHeight = engine.FontHeight() + painter->pen().widthF() * 2;
        if (iY + lineHeight > boundingRect.height())
        {
            lineHeight = boundingRect.height() - iY;
        }

        engine.Draw(painter, QRectF(0, iY, iW, lineHeight), qsText, tl.m_eAlign);

        // check if the next line will go out of bounds
        qreal nextStep = iY + engine.FontHeight() + painter->pen().widthF() * 2;
        if (nextStep > boundingRect.height())
        {
            NotEnoughSpace();
            break;
        }

        iY = nextStep + m_tm.GetSpacing();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VTextGraphicsItem::NotEnoughSpace() const
{
    QString errorMsg;
    switch (m_itemType)
    {
        case PatternLabel:
            errorMsg = tr("Piece '%1'. Not enough space for pattern info label.").arg(m_pieceName);
            break;
        case PieceLabel:
            errorMsg = tr("Piece '%1'. Not enough space for piece info label.").arg(m_pieceName);
            break;
        case Unknown:
        default:
            errorMsg = tr("Piece '%1'. Not enough space for label.").arg(m_pieceName);
            break;
    };

    VAbstractApplication::VApp()->IsPedantic()
        ? throw VException(errorMsg)
        : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
}
