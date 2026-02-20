/************************************************************************
 **
 **  @file   vfoldline.cpp
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
#include "vfoldline.h"
#include "../vformat/vsinglelineoutlinechar.h"
#include "../vgeometry/vgeometrydef.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/def.h"
#include "../vmisc/exception/vexception.h"
#include "../vmisc/svgfont/svgdef.h"
#include "../vmisc/svgfont/vsvgfont.h"
#include "../vmisc/svgfont/vsvgfontdatabase.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vmisc/vtranslator.h"
#include "../vwidgets/vpiecegrainline.h"

#include <QGraphicsSimpleTextItem>
#include <QLineF>
#include <QPainterPath>
#include <QtDebug>
#include <QtMath>
#include <algorithm>

namespace
{
constexpr qreal defLabelMargin = CmToPixel(0.5);
constexpr qreal minArrowsHeight = CmToPixel(0.5);
constexpr qreal defArrowsHeight = CmToPixel(1.);
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VFoldLine::VFoldLine(const QLineF &line, FoldLineType type)
  : m_foldLine(line),
    m_type(type)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VFoldLine::SetHeight(qreal newHeight)
{
    m_height = qAbs(newHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void VFoldLine::SetWidth(qreal newWidth)
{
    m_width = qAbs(newWidth);
}

//---------------------------------------------------------------------------------------------------------------------
void VFoldLine::SetCenterPosition(qreal newCenter)
{
    m_center = qBound(0., qAbs(newCenter), 1.);
}

//---------------------------------------------------------------------------------------------------------------------
void VFoldLine::SetLabel(const QString &newLabel)
{
    m_label = newLabel;
}

//---------------------------------------------------------------------------------------------------------------------
void VFoldLine::SetLabelSvgFontSize(unsigned int newFontSize)
{
    m_svgFontSize = qMax(newFontSize, static_cast<unsigned int>(VCommonSettings::MinPieceLabelFontPointSize()));
}

//---------------------------------------------------------------------------------------------------------------------
void VFoldLine::SetLabelFontItalic(bool newItalic)
{
    m_italic = newItalic;
}

//---------------------------------------------------------------------------------------------------------------------
void VFoldLine::SetLabelFontBold(bool newBold)
{
    m_bold = newBold;
}

//---------------------------------------------------------------------------------------------------------------------
void VFoldLine::SetLabelAlignment(int alignment)
{
    m_alignment = alignment;
}

//---------------------------------------------------------------------------------------------------------------------
void VFoldLine::SetMatrix(const QTransform &matrix)
{
    m_matrix = matrix;
}

//---------------------------------------------------------------------------------------------------------------------
void VFoldLine::SetVerticallyFlipped(bool value)
{
    m_verticallyFlipped = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VFoldLine::SetHorizontallyFlipped(bool value)
{
    m_horizontallyFlipped = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VFoldLine::SetXScale(qreal xs)
{
    m_xScale = xs;
}

//---------------------------------------------------------------------------------------------------------------------
void VFoldLine::SetYScale(qreal ys)
{
    m_yScale = ys;
}

//---------------------------------------------------------------------------------------------------------------------
void VFoldLine::SetOutlineFont(const QFont &font)
{
    m_outlineFont = font;
}

//---------------------------------------------------------------------------------------------------------------------
void VFoldLine::SetSvgFont(const QString &font)
{
    m_svgFont = font;
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::FoldLinePath() const -> QVector<QPainterPath>
{
    if (m_foldLine.isNull())
    {
        return {};
    }

    switch (m_type)
    {
        case FoldLineType::TwoArrows:
            return FoldLineTwoArrowsPath();
        case FoldLineType::TwoArrowsTextAbove:
            return FoldLineTwoArrowsTextAbovePath();
        case FoldLineType::TwoArrowsTextUnder:
            return FoldLineTwoArrowsTextUnderPath();
        case FoldLineType::Text:
            return FoldLineTextPath();
        case FoldLineType::ThreeDots:
            return FoldLineThreeDotsPath();
        case FoldLineType::ThreeX:
            return FoldLineThreeXPath();
        case FoldLineType::LAST_ONE_DO_NOT_USE:
            Q_UNREACHABLE();
            break;
        case FoldLineType::None:
        default:
            return {};
    };
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::FoldLineMarkPoints() const -> QVector<QVector<QPointF>>
{
    if (m_foldLine.isNull())
    {
        return {};
    }

    switch (m_type)
    {
        case FoldLineType::TwoArrows:
        {
            ArrowsTextPosData const data = TwoArrowsData();
            return TwoArrowsPoints(data.arrowsWidth, data.arrowsHeight);
        }
        case FoldLineType::TwoArrowsTextAbove:
        {
            ArrowsTextPosData const data = TwoArrowsTextAboveData();
            return TwoArrowsPoints(data.arrowsWidth, data.arrowsHeight);
        }
        case FoldLineType::TwoArrowsTextUnder:
        {
            ArrowsTextPosData const data = TwoArrowsTextUnderData();
            return TwoArrowsPoints(data.arrowsWidth, data.arrowsHeight);
        }
        case FoldLineType::ThreeDots:
            return ThreeDotsPoints();
        case FoldLineType::ThreeX:
            return ThreeXPoints();
        case FoldLineType::LAST_ONE_DO_NOT_USE:
            Q_UNREACHABLE();
            break;
        case FoldLineType::Text:
        case FoldLineType::None:
        default:
            return {};
    };
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::LabelPosition(bool &ok) const -> FoldLabelPosData
{
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wnoexcept")

    FoldLabelPosData posData;
    std::unique_ptr<TextPosData> data;

    QT_WARNING_POP
    if (m_type == FoldLineType::Text)
    {
        data = std::make_unique<TextPosData>(TextData());
    }
    else if (m_type == FoldLineType::TwoArrowsTextAbove)
    {
        data = std::make_unique<ArrowsTextPosData>(TwoArrowsTextAboveData());
    }
    else
    {
        data = std::make_unique<ArrowsTextPosData>(TwoArrowsTextUnderData());
    }

    if (data->base.isNull())
    {
        ok = false;
        return {};
    }

    qreal const height = data->labelHeight + (qFuzzyIsNull(m_height) ? defLabelMargin : m_height);
    qreal const margin = qMax(0., height - data->labelHeight);

    QPointF const center = TrueCenter(data->base, data->labelWidth);

    QLineF baseLine(center, data->base.p1());
    baseLine.setLength(data->labelWidth / 2);
    Swap(baseLine);
    baseLine.setLength(baseLine.length() * 2);
    baseLine = SimpleParallelLine(baseLine.p1(), baseLine.p2(), -(margin + data->labelHeight));

    posData.font = LabelOutlineFont();
    QFontMetrics const fm(posData.font);
    posData.label = fm.elidedText(FoldLineLabel(), Qt::ElideRight, qFloor(data->labelWidth));

    if (m_alignment & Qt::AlignHCenter) // NOLINT(readability-implicit-bool-conversion)
    {
        qreal const shift = (data->labelWidth - fm.horizontalAdvance(posData.label)) / 2;
        baseLine.setLength(baseLine.length() - shift);
    }
    else if (m_alignment & Qt::AlignRight) // NOLINT(readability-implicit-bool-conversion)
    {
        qreal const shift = data->labelWidth - fm.horizontalAdvance(posData.label);
        baseLine.setLength(baseLine.length() - shift);
    }

    posData.pos = baseLine.p2();
    posData.angle = QLineF(center, data->base.p1()).angle();
    ok = true;

    return posData;
}

//---------------------------------------------------------------------------------------------------------------------
void VFoldLine::UpdateFoldLineLabel(QGraphicsSimpleTextItem *item) const
{
    if (item == nullptr)
    {
        return;
    }

    if (m_foldLine.isNull())
    {
        item->setVisible(false);
        return;
    }

    if (m_type == FoldLineType::TwoArrows || m_type == FoldLineType::ThreeDots || m_type == FoldLineType::ThreeX ||
        m_type == FoldLineType::None)
    {
        item->setVisible(false);
        return;
    }

    item->setVisible(true);

    bool ok = false;
    FoldLabelPosData const labelPos = LabelPosition(ok);

    if (not ok)
    {
        item->setVisible(false);
        return;
    }

    item->setFont(labelPos.font);
    item->setText(labelPos.label);

    QRectF const rect = item->boundingRect();

    QTransform matrix;
    if (m_verticallyFlipped && !m_horizontallyFlipped)
    {
        QFontMetrics const fm(labelPos.font);

        matrix.scale(-1, 1);
        matrix.translate(-rect.width(), -rect.height() - fm.ascent());

        matrix.translate(-labelPos.pos.x(), labelPos.pos.y());
        matrix.rotate(labelPos.angle);
        matrix.translate(labelPos.pos.x(), -labelPos.pos.y());
        matrix.translate(-labelPos.pos.x(), labelPos.pos.y());
    }
    else if (!m_verticallyFlipped && m_horizontallyFlipped)
    {
        matrix.scale(1, -1);
        matrix.translate(0, -rect.height());

        matrix.translate(labelPos.pos.x(), -labelPos.pos.y());
        matrix.rotate(labelPos.angle + 180);
        matrix.translate(-labelPos.pos.x(), labelPos.pos.y());
        matrix.translate(-rect.width(), -rect.height());
        matrix.translate(labelPos.pos.x(), -labelPos.pos.y());
    }
    else
    {
        matrix.translate(labelPos.pos.x(), labelPos.pos.y());
        matrix.rotate(-labelPos.angle);
        matrix.translate(-labelPos.pos.x(), -labelPos.pos.y());
        matrix.translate(labelPos.pos.x(), labelPos.pos.y());
    }

    matrix *= m_matrix;

    item->setTransform(matrix);
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::ThreeDotsRadius() const -> qreal
{
    return ThreeDotsData().radius * qMin(m_xScale, m_yScale);
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::DefaultFoldLineLabel() -> QString
{
    QSharedPointer<VTranslator> const phTr = VAbstractApplication::VApp()->GetPlaceholderTranslator();
    return phTr->translate("FoldLine", "Cut On Fold");
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::FoldLineTwoArrowsPath() const -> QVector<QPainterPath>
{
    if (m_foldLine.isNull())
    {
        return {};
    }

    ArrowsTextPosData const data = TwoArrowsData();
    return {TwoArrowsPath(data.arrowsWidth, data.arrowsHeight)};
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::FoldLineTwoArrowsTextAbovePath() const -> QVector<QPainterPath>
{
    if (m_foldLine.isNull())
    {
        return {};
    }

    ArrowsTextPosData const data = TwoArrowsTextAboveData();

    QVector<QPainterPath> shape;
    shape.append(TwoArrowsPath(data.arrowsWidth, data.arrowsHeight));

    if (const VCommonSettings *settings = VAbstractApplication::VApp()->Settings(); settings->GetSingleLineFonts())
    {
        shape.append(SVGFontLabel(data.base, data.labelWidth, data.labelHeight));
    }
    else
    {
        shape.append(OutlineFontLabel(data.base, data.labelWidth, data.labelHeight));
    }

    return shape;
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::FoldLineTwoArrowsTextUnderPath() const -> QVector<QPainterPath>
{
    if (m_foldLine.isNull())
    {
        return {};
    }

    ArrowsTextPosData const data = TwoArrowsTextUnderData();

    QVector<QPainterPath> shape;
    shape.append(TwoArrowsPath(data.arrowsWidth, data.arrowsHeight));

    if (const VCommonSettings *settings = VAbstractApplication::VApp()->Settings(); settings->GetSingleLineFonts())
    {
        shape.append(SVGFontLabel(data.base, data.labelWidth, data.labelHeight));
    }
    else
    {
        shape.append(OutlineFontLabel(data.base, data.labelWidth, data.labelHeight));
    }

    return shape;
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::FoldLineTextPath() const -> QVector<QPainterPath>
{
    if (m_foldLine.isNull())
    {
        return {};
    }

    TextPosData const data = TextData();

    if (const VCommonSettings *settings = VAbstractApplication::VApp()->Settings(); settings->GetSingleLineFonts())
    {
        return {SVGFontLabel(data.base, data.labelWidth, data.labelHeight)};
    }

    return {OutlineFontLabel(data.base, data.labelWidth, data.labelHeight)};
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::FoldLineThreeDotsPath() const -> QVector<QPainterPath>
{
    QVector<QVector<QPointF>> const shape = ThreeDotsPoints();

    if (shape.isEmpty() || shape.constFirst().size() != 3)
    {
        return {};
    }

    ThreeDotsPosData const data = ThreeDotsData();
    QPainterPath dots;

    auto DrawCircle = [&dots, &data](const QPointF &center)
    {
        qreal const diameter = 2 * data.radius;
        qreal const x = center.x() - data.radius;
        qreal const y = center.y() - data.radius;

        dots.addEllipse(x, y, diameter, diameter);
    };

    const QVector<QPointF> &centers = shape.constFirst();

    DrawCircle(centers.at(0));
    DrawCircle(centers.at(1));
    DrawCircle(centers.at(2));

    return {dots};
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::FoldLineThreeXPath() const -> QVector<QPainterPath>
{
    QVector<QVector<QPointF>> const fullShape = ThreeXPoints();
    if (fullShape.isEmpty())
    {
        return {};
    }

    const QVector<QPointF> &shape = fullShape.constFirst();

    QPainterPath x3;

    for (int i = 0; i < shape.size() - 1; i += 2)
    {
        x3.moveTo(shape.at(i));
        x3.lineTo(shape.at(i + 1));
    }

    return {x3};
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::TrueCenter(const QLineF &base, qreal width) const -> QPointF
{
    if (base.isNull())
    {
        return {};
    }

    width = qMin(width, base.length());

    if (VFuzzyComparePossibleNulls(base.length(), width) || qFuzzyIsNull(width))
    {
        return base.center();
    }

    QLineF seg1 = base;
    seg1.setLength(width / 2.);

    qreal const seg1T = seg1.length() / base.length();

    if (m_center >= 0 && m_center < seg1T)
    {
        return seg1.p2();
    }

    QLineF seg2(base.p2(), base.p1());
    seg2.setLength(width / 2.);

    qreal const seg2T = seg2.length() / base.length();

    if (m_center >= (1 - seg2T) && m_center <= 1)
    {
        return seg2.p2();
    }

    return base.pointAt(m_center);
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::FoldLineLabel() const -> QString
{
    if (!m_label.isEmpty())
    {
        return m_label;
    }

    return DefaultFoldLineLabel();
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::OutlineFontLabel(const QLineF &base, qreal width, qreal textHeight) const -> QPainterPath
{
    if (base.isNull())
    {
        return {};
    }

    qreal const height = textHeight + (qFuzzyIsNull(m_height) ? defLabelMargin : m_height);
    qreal const margin = qMax(0., height - textHeight);

    QPointF const center = TrueCenter(base, width);

    QLineF baseLine(center, base.p1());
    baseLine.setAngle(baseLine.angle() + 90);
    baseLine.setLength(margin);
    Swap(baseLine);
    baseLine.setAngle(baseLine.angle() - 90 - QLineF(base.p2(), base.p1()).angle());
    baseLine.setLength(width / 2);
    QPointF const rotationCenter = baseLine.p1();
    Swap(baseLine);
    baseLine.setLength(width);
    Swap(baseLine);

    QFont const font = LabelOutlineFont();
    QFontMetrics const fm(font);
    QString const label = fm.elidedText(FoldLineLabel(), Qt::ElideRight, qFloor(width));

    if (m_alignment & Qt::AlignHCenter) // NOLINT(readability-implicit-bool-conversion)
    {
        qreal const shift = (width - fm.horizontalAdvance(label)) / 2;
        baseLine.setLength(baseLine.length() - shift);
    }
    else if (m_alignment & Qt::AlignRight) // NOLINT(readability-implicit-bool-conversion)
    {
        qreal const shift = width - fm.horizontalAdvance(label);
        baseLine.setLength(baseLine.length() - shift);
    }

    QPainterPath labelPath;

    if (const VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
        settings->GetSingleStrokeOutlineFont())
    {
        VSingleLineOutlineChar const corrector(font);
        if (!corrector.IsPopulated())
        {
            corrector.LoadCorrections(settings->GetPathFontCorrections());
        }

        int w = 0;
        for (auto c : std::as_const(label))
        {
            labelPath.addPath(corrector.DrawChar(w, static_cast<qreal>(fm.ascent()), c));
            w += fm.horizontalAdvance(c);
        }

        QTransform matrix;

        if (m_verticallyFlipped && !m_horizontallyFlipped)
        {
            matrix.scale(-1, 1);
            matrix.translate(-labelPath.boundingRect().width(), -labelPath.boundingRect().height() - fm.ascent());

            matrix.translate(-rotationCenter.x(), rotationCenter.y());
            matrix.rotate(QLineF(center, base.p1()).angle());
            matrix.translate(rotationCenter.x(), -rotationCenter.y());
            matrix.translate(-baseLine.p2().x(), baseLine.p2().y() - textHeight);
        }
        else if (!m_verticallyFlipped && m_horizontallyFlipped)
        {
            matrix.scale(1, -1);
            matrix.translate(0, -fm.height());

            matrix.translate(rotationCenter.x(), -rotationCenter.y());
            matrix.rotate(QLineF(center, base.p1()).angle() + 180);
            matrix.translate(-rotationCenter.x(), rotationCenter.y());
            matrix.translate(0, -labelPath.boundingRect().height() - textHeight * 2);
            matrix.translate(baseLine.p2().x(), -baseLine.p2().y() + textHeight);
        }
        else
        {
            matrix.translate(rotationCenter.x(), rotationCenter.y());
            matrix.rotate(-QLineF(center, base.p1()).angle());
            matrix.translate(-rotationCenter.x(), -rotationCenter.y());
            matrix.translate(baseLine.p2().x(), baseLine.p2().y() - textHeight);
        }

        matrix *= m_matrix;
        labelPath = matrix.map(labelPath);
    }
    else
    {
        Swap(baseLine);
        baseLine.setAngle(baseLine.angle() + 90);
        baseLine.setLength(fm.descent() + CmToPixel(0.2));
        labelPath.addText(QPointF(), font, label);

        QTransform matrix;

        if (m_verticallyFlipped && !m_horizontallyFlipped)
        {
            matrix.scale(-1, 1);
            matrix.translate(-labelPath.boundingRect().width(), -labelPath.boundingRect().height() - fm.ascent());

            matrix.translate(-rotationCenter.x(), rotationCenter.y());
            matrix.rotate(QLineF(center, base.p1()).angle());
            matrix.translate(rotationCenter.x(), -rotationCenter.y());
            matrix.translate(-baseLine.p2().x(), baseLine.p2().y());
        }
        else if (!m_verticallyFlipped && m_horizontallyFlipped)
        {
            matrix.scale(1, -1);
            matrix.translate(0, -fm.height());

            matrix.translate(rotationCenter.x(), -rotationCenter.y());
            matrix.rotate(QLineF(center, base.p1()).angle() + 180);
            matrix.translate(-rotationCenter.x(), rotationCenter.y());
            matrix.translate(0, -labelPath.boundingRect().height() - fm.height() - CmToPixel(0.1));
            matrix.translate(baseLine.p2().x(), -baseLine.p2().y());
        }
        else
        {
            matrix.translate(rotationCenter.x(), rotationCenter.y());
            matrix.rotate(-QLineF(center, base.p1()).angle());
            matrix.translate(-rotationCenter.x(), -rotationCenter.y());
            matrix.translate(baseLine.p2().x(), baseLine.p2().y());
        }

        matrix *= m_matrix;
        labelPath = matrix.map(labelPath);
    }

    return labelPath;
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::SVGFontLabel(const QLineF &base, qreal width, qreal textHeight) const -> QPainterPath
{
    if (base.isNull())
    {
        return {};
    }

    VSvgFontEngine engine;
    if (!LabelSVGFontEngine(engine))
    {
        return OutlineFontLabel(base, width, textHeight);
    }

    qreal const height = textHeight + (qFuzzyIsNull(m_height) ? defLabelMargin : m_height);
    qreal const margin = qMax(0., height - textHeight);

    QPointF const center = TrueCenter(base, width);

    QLineF baseLine(center, base.p1());
    baseLine.setAngle(baseLine.angle() + 90);
    baseLine.setLength(margin);
    Swap(baseLine);
    baseLine.setAngle(baseLine.angle() - 90 - QLineF(base.p2(), base.p1()).angle());
    baseLine.setLength(width / 2);
    QPointF const rotationCenter = baseLine.p1();
    Swap(baseLine);
    baseLine.setLength(width);
    Swap(baseLine);

    QString const label = engine.ElidedText(FoldLineLabel(), SVGTextElideMode::ElideRight, qFloor(width));

    if (m_alignment & Qt::AlignHCenter) // NOLINT(readability-implicit-bool-conversion)
    {
        qreal const shift = (width - engine.TextWidth(label)) / 2;
        baseLine.setLength(baseLine.length() - shift);
    }
    else if (m_alignment & Qt::AlignRight) // NOLINT(readability-implicit-bool-conversion)
    {
        qreal const shift = width - engine.TextWidth(label);
        baseLine.setLength(baseLine.length() - shift);
    }

    QPainterPath labelPath = engine.DrawPath(QPointF(), label);

    QTransform matrix;

    if (m_verticallyFlipped && !m_horizontallyFlipped)
    {
        matrix.scale(-1, 1);
        matrix.translate(-labelPath.boundingRect().width(), -labelPath.boundingRect().height());

        matrix.translate(-rotationCenter.x(), rotationCenter.y());
        matrix.rotate(QLineF(center, base.p1()).angle());
        matrix.translate(rotationCenter.x(), -rotationCenter.y());
        matrix.translate(-baseLine.p2().x(), baseLine.p2().y() - textHeight);
    }
    else if (!m_verticallyFlipped && m_horizontallyFlipped)
    {
        matrix.scale(1, -1);
        matrix.translate(0, -labelPath.boundingRect().height());

        matrix.translate(rotationCenter.x(), -rotationCenter.y());
        matrix.rotate(QLineF(center, base.p1()).angle() + 180);
        matrix.translate(-rotationCenter.x(), rotationCenter.y());
        matrix.translate(0, -labelPath.boundingRect().height() - textHeight * 2);
        matrix.translate(baseLine.p2().x(), -baseLine.p2().y() + textHeight);
    }
    else
    {
        matrix.translate(rotationCenter.x(), rotationCenter.y());
        matrix.rotate(-QLineF(center, base.p1()).angle());
        matrix.translate(-rotationCenter.x(), -rotationCenter.y());
        matrix.translate(baseLine.p2().x(), baseLine.p2().y() - textHeight);
    }

    matrix *= m_matrix;
    labelPath = matrix.map(labelPath);

    return labelPath;
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::LabelTextHeight() const -> qreal
{
    if (const VCommonSettings *settings = VAbstractApplication::VApp()->Settings(); settings->GetSingleLineFonts())
    {
        if (VSvgFontEngine engine; LabelSVGFontEngine(engine))
        {
            return engine.FontHeight();
        }
    }

    return QFontMetrics(LabelOutlineFont()).height();
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::LabelSVGFontEngine(VSvgFontEngine &engine) const -> bool
{
    VSvgFontDatabase *db = VAbstractApplication::VApp()->SVGFontDatabase();
    VSvgFontEngine const svgEngine =
        db->FontEngine(m_svgFont, SVGFontStyle::Normal, SVGFontWeight::Normal, static_cast<int>(m_svgFontSize));
    VSvgFont svgFont = svgEngine.Font();
    if (!svgFont.IsValid())
    {
        QString const errorMsg = tr("Invalid SVG font '%1'. Fallback to outline font.").arg(svgFont.Name());
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractApplication::warningMessageSignature + errorMsg;
        return false;
    }

    svgFont.SetBold(m_bold);
    svgFont.SetItalic(m_italic);

    engine = db->FontEngine(svgFont);
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::LabelOutlineFont() const -> QFont
{
    QFont font = m_outlineFont;
    font.setItalic(m_italic);
    font.setBold(m_bold);
    return font;
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::TwoArrowsPath(qreal width, qreal height) const -> QPainterPath
{
    QVector<QVector<QPointF>> const shape = TwoArrowsPoints(width, height);

    if (shape.size() != 3)
    {
        return {};
    }

    QPainterPath path;
    path.addPolygon(QPolygonF(shape.constFirst()));

    if (shape.at(1).size() == 2)
    {
        path.moveTo(shape.at(1).constFirst());
        path.lineTo(shape.at(1).constLast());
    }

    path.addPolygon(QPolygonF(shape.constLast()));
    path.setFillRule(Qt::OddEvenFill);

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::TwoArrowsPoints(qreal width, qreal height) const -> QVector<QVector<QPointF>>
{
    if (m_foldLine.isNull())
    {
        return {};
    }

    QPointF const center = TrueCenter(m_foldLine, width);

    QLineF base1(center, m_foldLine.p1());
    base1.setLength(width / 2.);
    Swap(base1);
    base1.setAngle(base1.angle() - 90);
    base1.setLength(height);

    QLineF base2(center, m_foldLine.p2());
    base2.setLength(width / 2.);
    Swap(base2);
    base2.setAngle(base2.angle() + 90);
    base2.setLength(height);

    VPieceGrainline const arrow1(base1, GrainlineArrowDirection::oneWayDown);
    VPieceGrainline const arrow2(base2, GrainlineArrowDirection::oneWayDown);

    QVector<QPointF> arrow1Shape = arrow1.Shape().constFirst();
    QVector<QPointF> arrow2Shape = arrow2.Shape().constFirst();
    QLineF line(arrow1Shape.constLast(), arrow2Shape.constLast());
    std::reverse(arrow2Shape.begin(), arrow2Shape.end());

    std::transform(arrow1Shape.begin(), arrow1Shape.end(), arrow1Shape.begin(),
                   [this](const QPointF &point) { return m_matrix.map(point); });
    line = m_matrix.map(line);
    std::transform(arrow2Shape.begin(), arrow2Shape.end(), arrow2Shape.begin(),
                   [this](const QPointF &point) { return m_matrix.map(point); });

    return {arrow1Shape, {line.p1(), line.p2()}, arrow2Shape};
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::ThreeDotsPoints() const -> QVector<QVector<QPointF>>
{
    if (m_foldLine.isNull())
    {
        return {};
    }

    ThreeDotsPosData const data = ThreeDotsData();

    QPointF const center = TrueCenter(m_foldLine, data.width);

    QLineF r1Line(center, m_foldLine.p1());
    r1Line.setLength(data.radius * 2);
    Swap(r1Line);
    r1Line.setAngle(r1Line.angle() - 90);
    r1Line.setLength(data.radius + data.margin);

    QLineF r2Line(center, m_foldLine.p2());
    r2Line.setLength(data.radius * 2);
    Swap(r2Line);
    r2Line.setAngle(r2Line.angle() + 90);
    r2Line.setLength(data.radius + data.margin);

    QLineF r3Line(center, m_foldLine.p1());
    r3Line.setAngle(r3Line.angle() + 90);
    r3Line.setLength(data.radius * 4 + data.margin);

    QPointF const c1 = m_matrix.map(r1Line.p2());
    QPointF const c2 = m_matrix.map(r2Line.p2());
    QPointF const c3 = m_matrix.map(r3Line.p2());

    return {{c1, c2, c3}};
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::ThreeXPoints() const -> QVector<QVector<QPointF>>
{
    qreal const width = qFuzzyIsNull(m_width) ? m_foldLine.length() * 0.9 : qMin(m_width, m_foldLine.length());
    constexpr qreal defXHeight = CmToPixel(1.5);
    constexpr qreal defXWidth = CmToPixel(1.1);
    qreal xHeight = defXHeight;
    qreal xWidth = defXWidth;

    if (defXHeight * 3 >= width * 0.8)
    {
        xHeight = width * 0.8 / 3.;
        xWidth = defXWidth * (xHeight / defXHeight);
    }

    qreal const height = defXWidth + (qFuzzyIsNull(m_height) ? CmToPixel(0.5) : m_height);
    qreal const margin = qMax(0., height - defXWidth);

    QPointF const center = TrueCenter(m_foldLine, width);

    QLineF x1Line(center, m_foldLine.p1());
    x1Line.setLength(width / 2);
    Swap(x1Line);
    x1Line.setLength(xHeight / 2);
    Swap(x1Line);
    x1Line.setAngle(x1Line.angle() + 90);
    x1Line.setLength(xWidth / 2 + margin);
    Swap(x1Line);
    x1Line.setAngle(x1Line.angle() + 90);

    QLineF x2Line(center, m_foldLine.p1());
    x2Line.setAngle(x2Line.angle() + 90);
    x2Line.setLength(xWidth / 2 + margin);
    Swap(x2Line);
    x2Line.setAngle(x2Line.angle() + 90);

    QLineF x3Line(center, m_foldLine.p2());
    x3Line.setLength(width / 2);
    Swap(x3Line);
    x3Line.setLength(xHeight / 2);
    Swap(x3Line);
    x3Line.setAngle(x3Line.angle() - 90);
    x3Line.setLength(xWidth / 2 + margin);
    Swap(x3Line);
    x3Line.setAngle(x3Line.angle() - 90);

    QVector<QPointF> shape;
    shape.reserve(12);

    auto XShape = [&shape, xHeight, xWidth](const QLineF &baseLine)
    {
        qreal const height = xHeight / 2;
        qreal const width = xWidth / 2;
        qreal const c = qSqrt(height * height + width * width);
        qreal const angle = qRadiansToDegrees(qAcos(height / c));

        QLineF line = baseLine;
        line.setLength(c);
        line.setAngle(line.angle() + angle);
        shape.append(line.p2());

        line.setAngle(line.angle() + 180);
        shape.append(line.p2());

        line = baseLine;
        line.setLength(c);
        line.setAngle(line.angle() + 180 - angle);
        shape.append(line.p2());

        line.setAngle(line.angle() + 180);
        shape.append(line.p2());
    };

    XShape(x1Line);
    XShape(x2Line);
    XShape(x3Line);

    std::transform(shape.begin(), shape.end(), shape.begin(),
                   [this](const QPointF &point) { return m_matrix.map(point); });

    if (m_verticallyFlipped || m_horizontallyFlipped)
    {
        std::reverse(shape.begin(), shape.end());
    }

    return {shape};
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::TwoArrowsData() const -> ArrowsTextPosData
{
    ArrowsTextPosData data{};
    data.arrowsWidth = qFuzzyIsNull(m_width) ? m_foldLine.length() * 0.9 : qMin(m_width, m_foldLine.length());
    data.arrowsHeight = qFuzzyIsNull(m_height) ? defArrowsHeight : m_height;
    return data;
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::TwoArrowsTextAboveData() const -> ArrowsTextPosData
{
    const qreal defLabelHeight = LabelTextHeight();

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wnoexcept")

    ArrowsTextPosData data;

    QT_WARNING_POP

    data.arrowsWidth = qFuzzyIsNull(m_width) ? m_foldLine.length() * 0.9 : qMin(m_width, m_foldLine.length());
    data.labelWidth = data.arrowsWidth;
    data.arrowsHeight = qFuzzyIsNull(m_height) ? defArrowsHeight
                                               : qMax(m_height - (defLabelHeight + CmToPixel(0.2)), minArrowsHeight);
    data.labelHeight = qFuzzyIsNull(m_height) ? defLabelHeight : (defLabelHeight + CmToPixel(0.2)) - m_height;
    data.base = SimpleParallelLine(m_foldLine.p1(), m_foldLine.p2(), -data.arrowsHeight);
    return data;
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::TwoArrowsTextUnderData() const -> ArrowsTextPosData
{
    const qreal defLabelHeight = LabelTextHeight();

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wnoexcept")

    ArrowsTextPosData data;

    QT_WARNING_POP

    data.arrowsWidth = qFuzzyIsNull(m_width) ? m_foldLine.length() * 0.9 : qMin(m_width, m_foldLine.length());
    data.labelWidth = data.arrowsWidth * 0.9;
    data.labelHeight = qFuzzyIsNull(m_height) ? defLabelHeight : (defLabelHeight + CmToPixel(0.2)) - m_height;
    data.arrowsHeight = defLabelHeight + (qFuzzyIsNull(m_height) ? defLabelMargin : m_height) + CmToPixel(0.2);
    data.arrowsHeight = qFuzzyIsNull(m_height) ? qMax(defLabelHeight, defArrowsHeight) : data.arrowsHeight;
    data.base = m_foldLine;
    return data;
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::TextData() const -> TextPosData
{
    const qreal defLabelHeight = LabelTextHeight();

    return {.labelWidth = qFuzzyIsNull(m_width) ? m_foldLine.length() * 0.9 : qMin(m_width, m_foldLine.length()),
            .labelHeight = qFuzzyIsNull(m_height) ? defLabelHeight : (defLabelHeight + CmToPixel(0.2)) - m_height,
            .base = m_foldLine};
}

//---------------------------------------------------------------------------------------------------------------------
auto VFoldLine::ThreeDotsData() const -> ThreeDotsPosData
{
    ThreeDotsPosData data;
    data.width = qFuzzyIsNull(m_width) ? CmToPixel(2.) : qMin(m_width, m_foldLine.length());
    data.defHeight = data.width * qSin(qDegreesToRadians(60.0));
    data.height = data.defHeight + (qFuzzyIsNull(m_height) ? CmToPixel(0.5) : m_height);
    data.radius = data.width / 3 / 2;
    data.margin = qMax(0., data.height - data.defHeight);
    return data;
}
