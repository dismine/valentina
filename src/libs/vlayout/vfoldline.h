/************************************************************************
 **
 **  @file   vfoldline.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 12, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#ifndef VFOLDLINE_H
#define VFOLDLINE_H

#include <QCoreApplication>
#include <QFont>
#include <QLineF>
#include <QTransform>

#include "../vmisc/def.h"

class QPainterPath;
class VSvgFontEngine;
class QGraphicsSimpleTextItem;

struct TextPosData
{
    qreal labelWidth{0};
    qreal labelHeight{0};
    QLineF base{};
};

struct ArrowsTextPosData : public TextPosData
{
    qreal arrowsWidth{0};
    qreal arrowsHeight{0};
};

struct ThreeDotsPosData
{
    qreal width{0};
    qreal defHeight{0};
    qreal height{0};
    qreal radius{0};
    qreal margin{0};
};

struct FoldLabelPosData
{
    QFont font{};    // NOLINT(misc-non-private-member-variables-in-classes)
    QPointF pos{};   // NOLINT(misc-non-private-member-variables-in-classes)
    QString label{}; // NOLINT(misc-non-private-member-variables-in-classes)
    qreal angle{0};  // NOLINT(misc-non-private-member-variables-in-classes)
};

class VFoldLine
{
    Q_DECLARE_TR_FUNCTIONS(VFoldLine) // NOLINT

public:
    VFoldLine() = default;
    VFoldLine(const QLineF &line, FoldLineType type);
    VFoldLine(VFoldLine &&) noexcept = default;
    auto operator=(VFoldLine &&) noexcept -> VFoldLine & = default;
    VFoldLine(const VFoldLine &) = default;
    auto operator=(const VFoldLine &) -> VFoldLine & = default;
    ~VFoldLine() = default;

    void SetHeight(qreal newHeight);
    void SetWidth(qreal newWidth);
    void SetCenterPosition(qreal newCenter);
    void SetLabel(const QString &newLabel);
    void SetLabelSvgFontSize(unsigned int newFontSize);
    void SetLabelFontItalic(bool newItalic);
    void SetLabelFontBold(bool newBold);
    void SetLabelAlignment(int alignment);
    void SetMatrix(const QTransform &matrix);
    void SetVerticallyFlipped(bool value);
    void SetHorizontallyFlipped(bool value);
    void SetXScale(qreal xs);
    void SetYScale(qreal ys);
    void SetOutlineFont(const QFont &font);
    void SetSvgFont(const QString &font);

    auto FoldLinePath() const -> QVector<QPainterPath>;
    auto FoldLineMarkPoints() const -> QVector<QVector<QPointF>>;

    auto LabelPosition(bool &ok) const -> FoldLabelPosData;

    void UpdateFoldLineLabel(QGraphicsSimpleTextItem *item) const;

    auto ThreeDotsRadius() const -> qreal;

    static auto DefaultFoldLineLabel() -> QString;

private:
    QLineF m_foldLine{};
    FoldLineType m_type{FoldLineType::TwoArrowsTextAbove};
    qreal m_height{0};
    qreal m_width{0};
    qreal m_center{0.5};
    QString m_label{};
    unsigned int m_svgFontSize{defFoldLineFontSize};
    bool m_italic{false};
    bool m_bold{false};
    int m_alignment{Qt::AlignHCenter};
    QTransform m_matrix{};
    bool m_verticallyFlipped{false};
    bool m_horizontallyFlipped{false};
    qreal m_xScale{1.0};
    qreal m_yScale{1.0};
    QFont m_outlineFont{};
    QString m_svgFont{};

    auto FoldLineTwoArrowsPath() const -> QVector<QPainterPath>;
    auto FoldLineTwoArrowsTextAbovePath() const -> QVector<QPainterPath>;
    auto FoldLineTwoArrowsTextUnderPath() const -> QVector<QPainterPath>;
    auto FoldLineTextPath() const -> QVector<QPainterPath>;
    auto FoldLineThreeDotsPath() const -> QVector<QPainterPath>;
    auto FoldLineThreeXPath() const -> QVector<QPainterPath>;

    auto TrueCenter(const QLineF &base, qreal width) const -> QPointF;

    auto FoldLineLabel() const -> QString;

    auto OutlineFontLabel(const QLineF &base, qreal width, qreal textHeight) const -> QPainterPath;
    auto SVGFontLabel(const QLineF &base, qreal width, qreal textHeight) const -> QPainterPath;

    auto LabelTextHeight() const -> qreal;
    auto LabelSVGFontEngine(VSvgFontEngine &engine) const -> bool;
    auto LabelOutlineFont() const -> QFont;

    auto TwoArrowsPath(qreal width, qreal height) const -> QPainterPath;

    auto TwoArrowsPoints(qreal width, qreal height) const -> QVector<QVector<QPointF>>;
    auto ThreeDotsPoints() const -> QVector<QVector<QPointF>>;
    auto ThreeXPoints() const -> QVector<QVector<QPointF>>;

    auto TwoArrowsData() const -> ArrowsTextPosData;
    auto TwoArrowsTextAboveData() const -> ArrowsTextPosData;
    auto TwoArrowsTextUnderData() const -> ArrowsTextPosData;
    auto TextData() const -> TextPosData;
    auto ThreeDotsData() const -> ThreeDotsPosData;
};

#endif // VFOLDLINE_H
