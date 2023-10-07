/************************************************************************
 **
 **  @file   vhpglengine.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   7 7, 2020
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
#include "vhpglengine.h"

#include "../vformat/vsinglelineoutlinechar.h"
#include "../vgeometry/vlayoutplacelabel.h"
#include "../vlayout/vlayoutpiece.h"
#include "../vlayout/vlayoutpiecepath.h"
#include "../vlayout/vlayoutpoint.h"
#include "../vlayout/vtextmanager.h"
#include "../vmisc/def.h"
#include "../vmisc/defglobal.h"
#include "../vmisc/svgfont/vsvgfont.h"
#include "../vmisc/svgfont/vsvgfontdatabase.h"
#include "../vmisc/svgfont/vsvgfontengine.h"
#include "../vmisc/vabstractapplication.h"
#include "qmath.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include "../vmisc/backport/text.h"
#endif

#include <QFile>
#include <QLine>
#include <QtMath>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
const qreal accuracyPointOnLine{0.99};

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

// mnemonics
Q_GLOBAL_STATIC_WITH_ARGS(const QString, mIN, ("IN"_L1)) // NOLINT initialize set instruction
Q_GLOBAL_STATIC_WITH_ARGS(const QString, mPU, ("PU"_L1)) // NOLINT pen up
Q_GLOBAL_STATIC_WITH_ARGS(const QString, mPD, ("PD"_L1)) // NOLINT pen down
Q_GLOBAL_STATIC_WITH_ARGS(const QString, mLT, ("LT"_L1)) // NOLINT line type
Q_GLOBAL_STATIC_WITH_ARGS(const QString, mSP, ("SP"_L1)) // NOLINT select pen
Q_GLOBAL_STATIC_WITH_ARGS(const QString, mPG, ("PG"_L1)) // NOLINT page feed
Q_GLOBAL_STATIC_WITH_ARGS(const QString, mPA, ("PA"_L1)) // NOLINT plot absolute
Q_GLOBAL_STATIC_WITH_ARGS(const QString, mPW, ("PW"_L1)) // NOLINT pen width
Q_GLOBAL_STATIC_WITH_ARGS(const QString, mLA, ("LA"_L1)) // NOLINT line attributes

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_RELAXED_CONSTEXPR inline auto ConvertPixels(qreal pix) -> qreal
{
    // Default plating measurement in the HP-GL(HP-GL/2) graphics mode is 1/1016"(0.025mm).
    // 40 plotter units = 1 mm
    return FromPixel(pix, Unit::Mm) * 40.;
}

//---------------------------------------------------------------------------------------------------------------------
auto RemoveDublicates(QVector<QPoint> points) -> QVector<QPoint>
{
    if (points.size() < 3)
    {
        return points;
    }

    for (int i = 0; i < points.size() - 1; ++i)
    {
        if (points.at(i) == points.at(i + 1))
        {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            points.erase(points.cbegin() + i + 1);
#else
            points.erase(points.begin() + i + 1);
#endif
            --i;
        }
    }

    return points;
}

//---------------------------------------------------------------------------------------------------------------------
auto OptimizePath(QVector<QPoint> path) -> QVector<QPoint>
{
    if (path.size() < 3)
    {
        return path;
    }

    path = RemoveDublicates(path);

    if (path.size() < 3)
    {
        return path;
    }

    vsizetype prev = -1;
    const bool closedPath = (path.first() == path.last());
    const vsizetype startIndex = closedPath ? 0 : 1;
    const vsizetype endIndex = closedPath ? path.size() : path.size() - 1;

    QVector<QPoint> cleared;
    cleared.reserve(path.size());

    if (!closedPath)
    {
        cleared.append(path.first());
    }

    // Remove point on line
    for (vsizetype i = startIndex; i < endIndex; ++i)
    {
        if (prev == -1)
        {
            prev = (i == 0) ? path.size() - 1 : i - 1;
        }

        const vsizetype next = (i == path.size() - 1) ? 0 : i + 1;

        const QPoint &iPoint = path.at(i);
        const QPoint &prevPoint = path.at(prev);
        const QPoint &nextPoint = path.at(next);

        // If RemoveDublicates does not remove these points it is a valid case.
        // Case where last point equal first point
        if (((i == 0 || i == path.size() - 1) && (iPoint == prevPoint || iPoint == nextPoint)) ||
            not VGObject::IsPointOnLineviaPDP(iPoint, prevPoint, nextPoint, accuracyPointOnLine))
        {
            cleared.append(iPoint);
            prev = -1;
        }
    }

    if (!closedPath)
    {
        cleared.append(path.last());
    }

    cleared = RemoveDublicates(cleared);

    return cleared;
}

//---------------------------------------------------------------------------------------------------------------------
template <class T> inline auto CastToPoint(const QVector<T> &points) -> QVector<QPoint>
{
    QVector<QPoint> casted;
    casted.reserve(points.size());

    for (const auto &p : points)
    {
        casted.append(p.toPoint());
    }

    return casted;
}

//---------------------------------------------------------------------------------------------------------------------
auto LineFont(const TextLine &tl, const VSvgFont &base) -> VSvgFont
{
    VSvgFont fnt = base;
    fnt.SetPointSize(base.PointSize() + tl.m_iFontSize);
    fnt.SetBold(tl.m_bold);
    fnt.SetItalic(tl.m_italic);
    return fnt;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto LineFont(const TextLine &tl, const QFont &base) -> QFont
{
    QFont fnt = base;
    fnt.setPointSize(qMax(base.pointSize() + tl.m_iFontSize, 1));
    fnt.setBold(tl.m_bold);
    fnt.setItalic(tl.m_italic);
    return fnt;
}

//---------------------------------------------------------------------------------------------------------------------
auto LineAlign(const TextLine &tl, const QString &text, const VSvgFontEngine &engine, qreal width, qreal penWidth)
    -> qreal
{
    const int lineWidth = qRound(engine.TextWidth(text, penWidth));

    qreal dX = 0;
    if (tl.m_eAlign == 0 || (tl.m_eAlign & Qt::AlignLeft) > 0)
    {
        dX = 0;
    }
    else if ((tl.m_eAlign & Qt::AlignHCenter) > 0)
    {
        dX = (width - lineWidth) / 2;
    }
    else if ((tl.m_eAlign & Qt::AlignRight) > 0)
    {
        dX = width - lineWidth;
    }

    return dX;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto LineAlign(const TextLine &tl, const QString &text, const QFontMetrics &fm, qreal width) -> qreal
{
    const int lineWidth = TextWidth(fm, text);

    qreal dX = 0;
    if (tl.m_eAlign == 0 || (tl.m_eAlign & Qt::AlignLeft) > 0)
    {
        dX = 0;
    }
    else if ((tl.m_eAlign & Qt::AlignHCenter) > 0)
    {
        dX = (width - lineWidth) / 2;
    }
    else if ((tl.m_eAlign & Qt::AlignRight) > 0)
    {
        dX = width - lineWidth;
    }

    return dX;
}

//---------------------------------------------------------------------------------------------------------------------
auto LineMatrix(const VLayoutPiece &piece, const QPointF &topLeft, qreal angle, const QPointF &linePos,
                int maxLineWidth) -> QTransform
{
    QTransform labelMatrix;
    labelMatrix.translate(topLeft.x(), topLeft.y());

    if (piece.IsMirror())
    {
        labelMatrix.scale(-1, 1);
        labelMatrix.rotate(-angle);
        labelMatrix.translate(-maxLineWidth, 0);
    }
    else
    {
        labelMatrix.rotate(angle);
    }

    labelMatrix.translate(linePos.x(), linePos.y()); // Each string has own position
    labelMatrix *= piece.GetMatrix();

    return labelMatrix;
}

//---------------------------------------------------------------------------------------------------------------------
auto OptimizePattern(QVector<int> pattern) -> QVector<int>
{
    // Extend the pattern if it has an odd number of elements
    if (pattern.size() % 2 == 1)
    {
        const vsizetype originalSize = pattern.size();
        pattern.reserve(originalSize * 2);

        for (int i = 0; i < originalSize; ++i)
        {
            pattern.append(pattern.at(i));
        }
    }

    return pattern;
}

//---------------------------------------------------------------------------------------------------------------------
auto CurrentPatternDistance(qreal patternDistance, bool dashMode, const QVector<int> &pattern, int patternIndex)
    -> qreal
{
    if (qFuzzyIsNull(patternDistance))
    {
        patternDistance = dashMode ? qAbs(pattern[patternIndex % pattern.size()])
                                   : qAbs(pattern[(patternIndex + 1) % pattern.size()]);
    }

    return patternDistance;
}

//---------------------------------------------------------------------------------------------------------------------
auto NextPattern(int patternIndex, const QVector<int> &pattern) -> int
{
    return (patternIndex + 2) % static_cast<int>(pattern.size());
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VHPGLEngine::VHPGLEngine()
  : m_penWidthPx(qCeil(ToPixel(0.025, Unit::Mm)))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VHPGLEngine::SortDetails(const QVector<VLayoutPiece> &details) -> QList<VLayoutPiece>
{
    QList<VLayoutPiece> sorted;

    for (const auto &detail : details)
    {
        if (detail.GetPriority() == 0 || sorted.isEmpty())
        {
            sorted.append(detail);
        }
        else
        {
            bool found = false;
            for (int i = 0; i < sorted.size(); ++i)
            {
                if (detail.GetPriority() < sorted.at(i).GetPriority() || sorted.at(i).GetPriority() == 0)
                {
                    sorted.insert(i, detail);
                    found = true;
                    break;
                }
            }

            if (not found)
            {
                sorted.append(detail);
            }
        }
    }

    return sorted;
}

//---------------------------------------------------------------------------------------------------------------------
auto VHPGLEngine::GenerateHPGL(const QVector<VLayoutPiece> &details) -> bool
{
    if (details.isEmpty())
    {
        qCritical() << "VHPGLEngine::GenerateHPGL(), details list is empty";
        return false;
    }

    if (not m_size.isValid())
    {
        qCritical() << "VHPGLEngine::GenerateHPGL(), size is not valid";
        return false;
    }

    m_currentPos = QPoint(-1, -1); // Fake position

    QFile data(m_fileName);
    if (data.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream out(&data);
        out.setRealNumberPrecision(0);

        GenerateHPGLHeader(out);
        ExportDetails(out, SortDetails(details));
        GenerateHPGLFooter(out);

        data.close();
    }
    else
    {
        qCritical() << "VHPGLEngine::GenerateHPGL(), cannot open file " << m_fileName << ". Reason " << data.error()
                    << ".";
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VHPGLEngine::GenerateHPGL2(const QVector<VLayoutPiece> &details) -> bool
{
    if (details.isEmpty())
    {
        qCritical() << "VHPGLEngine::ExportToHPGL2(), details list is empty";
        return false;
    }

    if (not m_size.isValid())
    {
        qCritical() << "VHPGLEngine::ExportToHPGL2(), size is not valid";
        return false;
    }

    m_ver2 = true;
    m_currentPos = QPoint(-1, -1); // Fake position

    QFile data(m_fileName);
    if (data.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream out(&data);
        out.setRealNumberPrecision(6);

        GenerateHPGLHeader(out);
        ExportDetails(out, SortDetails(details));
        GenerateHPGLFooter(out);

        data.close();
    }
    else
    {
        qCritical() << "VHPGLEngine::ExportToHPGL2(), cannot open file " << m_fileName << ". Reason " << data.error()
                    << ".";
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLEngine::HPComand(QTextStream &out, const QString &mnemonic, const QString &parameters) const
{
    out << qPrintable(mnemonic + parameters) << ';';
    if (m_inserNewLine)
    {
        out << Qt::endl;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLEngine::GenerateHPGLHeader(QTextStream &out)
{
    HPComand(out, *mIN);
    HPPenUp(out);
    HPComand(out, *mSP, QChar('1')); // select first pen
    HPComand(out, *mLT);             // select line type by default
    HPComand(out, *mPA);
    HPPenUp(out, QPoint()); // move to the origin

    if (m_ver2)
    {
        HPComand(out, *mPW, QString::number(FromPixel(m_penWidthPx, Unit::Mm), 'f', 6));
        HPComand(out, *mLA, QStringLiteral("1,4,2,4"));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLEngine::ExportDetails(QTextStream &out, const QList<VLayoutPiece> &details)
{
    for (auto detail : details)
    {
        detail.Scale(m_xscale, m_yscale);

        PlotSeamAllowance(out, detail);
        PlotMainPath(out, detail);
        PlotInternalPaths(out, detail);
        PlotGrainline(out, detail);
        PlotPlaceLabels(out, detail);
        PlotPassmarks(out, detail);
        PlotLabels(out, detail);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLEngine::GenerateHPGLFooter(QTextStream &out)
{
    HPPenUp(out);                    // pen up
    HPComand(out, *mSP, QChar('0')); // select pen
    HPComand(out, *mPG);             // Page Feed
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLEngine::PlotMainPath(QTextStream &out, const VLayoutPiece &detail)
{
    if (not detail.IsSeamAllowance() ||
        (detail.IsSeamAllowance() && not detail.IsSeamAllowanceBuiltIn() && not detail.IsHideMainPath()))
    {
        QVector<QPoint> points = CastToPoint(ConvertPath(detail.GetMappedContourPoints()));

        if (points.size() > 1 && points.first() != points.last())
        {
            points.append(points.first()); // must be closed
        }

        PlotPath(out, points, Qt::SolidLine);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLEngine::PlotSeamAllowance(QTextStream &out, const VLayoutPiece &detail)
{
    QVector<QPoint> points = detail.IsSeamAllowance() && not detail.IsSeamAllowanceBuiltIn()
                                 ? CastToPoint(ConvertPath(detail.GetMappedSeamAllowancePoints()))
                                 : CastToPoint(ConvertPath(detail.GetMappedContourPoints()));

    if (points.size() > 1 && points.first() != points.last())
    {
        points.append(points.first()); // must be closed
    }

    PlotPath(out, points, Qt::SolidLine);
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLEngine::PlotInternalPaths(QTextStream &out, const VLayoutPiece &detail)
{
    QVector<VLayoutPiecePath> internalPaths = detail.GetInternalPaths();
    for (const auto &path : internalPaths)
    {
        QVector<VLayoutPoint> points = VLayoutPiece::MapVector(path.Points(), detail.GetMatrix(), detail.IsMirror());
        PlotPath(out, CastToPoint(ConvertPath(points)), path.PenStyle());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLEngine::PlotPlaceLabels(QTextStream &out, const VLayoutPiece &detail)
{
    const QVector<VLayoutPlaceLabel> placeLabels = detail.GetPlaceLabels();
    for (const auto &pLabel : placeLabels)
    {
        PlaceLabelImg shape = detail.MapPlaceLabelShape(VAbstractPiece::PlaceLabelShape(pLabel));
        for (auto &subShape : shape)
        {
            PlotPath(out, CastToPoint(ConvertPath(subShape)), Qt::SolidLine);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLEngine::PlotPassmarks(QTextStream &out, const VLayoutPiece &detail)
{
    const QVector<VLayoutPassmark> passmarks = detail.GetMappedPassmarks();
    for (const auto &passmark : passmarks)
    {
        for (const auto &subLine : passmark.lines)
        {
            HPPenUp(out, ConvertPoint(subLine.p1()).toPoint());
            HPPenDown(out, ConvertPoint(subLine.p2()).toPoint());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLEngine::PlotLabels(QTextStream &out, const VLayoutPiece &detail)
{
    PlotLabel(out, detail, detail.GetPieceLabelRect(), detail.GetPieceLabelData());
    PlotLabel(out, detail, detail.GetPatternLabelRect(), detail.GetPatternLabelData());
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLEngine::PlotGrainline(QTextStream &out, const VLayoutPiece &detail)
{
    if (!m_showGrainline)
    {
        return;
    }

    GrainlineShape shape = detail.GetMappedGrainlineShape();
    for (const auto &subShape : shape)
    {
        PlotPath(out, CastToPoint(ConvertPath(subShape)), Qt::SolidLine);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLEngine::PlotLabel(QTextStream &out, const VLayoutPiece &detail, const QVector<QPointF> &labelShape,
                            const VTextManager &tm)
{
    if (m_singleLineFont)
    {
        PlotLabelSVGFont(out, detail, labelShape, tm);
    }
    else
    {
        PlotLabelOutlineFont(out, detail, labelShape, tm);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLEngine::PlotLabelSVGFont(QTextStream &out, const VLayoutPiece &detail, const QVector<QPointF> &labelShape,
                                   const VTextManager &tm)
{
    if (labelShape.count() <= 2)
    {
        return;
    }

    VSvgFontDatabase *db = VAbstractApplication::VApp()->SVGFontDatabase();
    VSvgFontEngine engine =
        db->FontEngine(tm.GetSVGFontFamily(), SVGFontStyle::Normal, SVGFontWeight::Normal, tm.GetSVGFontPointSize());

    VSvgFont svgFont = engine.Font();
    if (!svgFont.IsValid())
    {
        qDebug() << QStringLiteral("Invalid SVG font '%1'. Fallback to outline font.").arg(svgFont.Name());
        PlotLabelOutlineFont(out, detail, labelShape, tm);
        return;
    }

    const qreal dW = QLineF(labelShape.at(0), labelShape.at(1)).length();
    const qreal dH = QLineF(labelShape.at(1), labelShape.at(2)).length();
    const qreal angle = -QLineF(labelShape.at(0), labelShape.at(1)).angle();
    const int maxLineWidth = tm.MaxLineWidthSVGFont(static_cast<int>(dW), m_penWidthPx);

    qreal dY = m_penWidthPx;

    const QVector<TextLine> labelLines = tm.GetLabelSourceLines(qFloor(dW), svgFont, m_penWidthPx);

    for (const auto &tl : labelLines)
    {
        const VSvgFont fnt = LineFont(tl, svgFont);
        engine = db->FontEngine(fnt);

        if (dY + engine.FontHeight() + m_penWidthPx > dH)
        {
            break;
        }

        const QString qsText = tl.m_qsText;
        const qreal dX = LineAlign(tl, qsText, engine, dW, m_penWidthPx);
        // set up the rotation around top-left corner matrix
        const QTransform lineMatrix = LineMatrix(detail, labelShape.at(0), angle, QPointF(dX, dY), maxLineWidth);

        QPainterPath path = lineMatrix.map(engine.DrawPath(QPointF(), qsText));
        PlotPainterPath(out, path, Qt::SolidLine);

        dY += engine.FontHeight() + m_penWidthPx + tm.GetSpacing();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLEngine::PlotLabelOutlineFont(QTextStream &out, const VLayoutPiece &detail, const QVector<QPointF> &labelShape,
                                       const VTextManager &tm)
{
    if (labelShape.count() <= 2)
    {
        return;
    }

    const qreal dW = QLineF(labelShape.at(0), labelShape.at(1)).length();
    const qreal dH = QLineF(labelShape.at(1), labelShape.at(2)).length();
    const qreal angle = -QLineF(labelShape.at(0), labelShape.at(1)).angle();
    const int maxLineWidth = tm.MaxLineWidthOutlineFont(static_cast<int>(dW));

    qreal dY = 0;

    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();

    if (m_singleStrokeOutlineFont)
    {
        dY += m_penWidthPx;
    }

    const QVector<TextLine> labelLines = tm.GetLabelSourceLines(qFloor(dW), tm.GetFont());

    for (const auto &tl : labelLines)
    {
        const QFont fnt = LineFont(tl, tm.GetFont());

        VSingleLineOutlineChar corrector(fnt);
        if (m_singleStrokeOutlineFont && !corrector.IsPopulated())
        {
            corrector.LoadCorrections(settings->GetPathFontCorrections());
        }

        QFontMetrics fm(fnt);

        if (dY + fm.height() > dH)
        {
            break;
        }

        const QString qsText = tl.m_qsText;
        const qreal dX = LineAlign(tl, qsText, fm, dW);
        // set up the rotation around top-left corner matrix
        const QTransform lineMatrix = LineMatrix(detail, labelShape.at(0), angle, QPointF(dX, dY), maxLineWidth);

        QPainterPath path;

        if (m_singleStrokeOutlineFont)
        {
            int w = 0;
            for (auto c : qAsConst(qsText))
            {
                path.addPath(corrector.DrawChar(w, static_cast<qreal>(fm.ascent()), c));
                w += TextWidth(fm, c);
            }
        }
        else
        {
            path.addText(0, static_cast<qreal>(fm.ascent()), fnt, qsText);
        }

        PlotPainterPath(out, lineMatrix.map(path), Qt::SolidLine);

        dY += fm.height() + m_penWidthPx + tm.GetSpacing();
    }
}

//---------------------------------------------------------------------------------------------------------------------

template <class T> auto VHPGLEngine::ConvertPath(const QVector<T> &path) const -> QVector<T>
{
    QVector<T> convertedPath;
    convertedPath.reserve(path.size());

    for (const auto &point : path)
    {
        convertedPath.append(ConvertPoint(point));
    }

    return convertedPath;
}

//---------------------------------------------------------------------------------------------------------------------
template <class T> auto VHPGLEngine::ConvertPoint(T point) const -> T
{
    point.setY(point.y() * -1 + m_size.height());

    return {ConvertPixels(point.x()), ConvertPixels(point.y())};
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLEngine::PlotPath(QTextStream &out, QVector<QPoint> path, Qt::PenStyle penStyle)
{
    if (penStyle == Qt::NoPen)
    {
        return;
    }

    path = OptimizePath(path);

    if (penStyle != Qt::SolidLine && penStyle != Qt::DashLine && penStyle != Qt::DotLine &&
        penStyle != Qt::DashDotLine && penStyle != Qt::DashDotDotLine)
    {
        penStyle = Qt::SolidLine;
    }

    if (penStyle == Qt::SolidLine)
    {
        PlotSolidLinePath(out, path);
    }
    else
    {
        QVector<int> patetrn = PatternForStyle(penStyle);
        PlotPathForStyle(out, path, patetrn);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLEngine::PlotSolidLinePath(QTextStream &out, QVector<QPoint> path)
{
    if (path.size() < 2)
    {
        HPPenUp(out, path.first());
        HPPenDown(out);
        return;
    }

    HPPenUp(out, path.first());

    for (int i = 1; i < path.size(); ++i)
    {
        HPPenDown(out, path.at(i));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLEngine::PlotPathForStyle(QTextStream &out, QVector<QPoint> path, QVector<int> pattern)
{
    if (pattern.isEmpty())
    {
        return;
    }

    if (path.size() < 2)
    {
        HPPenUp(out, path.first());
        HPPenDown(out);
        return;
    }

    pattern = OptimizePattern(pattern);

    bool dashMode = true;
    int patternIndex = 0;
    qreal patternDistance = 0.0;

    HPPenUp(out, path.first());

    for (int i = 1; i < path.size(); ++i)
    {
        QPointF prevPoint = path.at(i - 1);
        QPoint currPoint = path.at(i);

        qreal distance = QLineF(prevPoint, currPoint).length();
        qreal segmentDistance = 0;
        do
        {
            qreal subDistance = QLineF(prevPoint, currPoint).length();
            patternDistance = CurrentPatternDistance(patternDistance, dashMode, pattern, patternIndex);

            if (subDistance < patternDistance)
            {
                if (dashMode)
                {
                    HPPenDown(out, currPoint);
                }
                patternDistance = patternDistance - subDistance;
                break;
            }

            if (VFuzzyComparePossibleNulls(subDistance, patternDistance))
            {
                if (dashMode)
                {
                    HPPenDown(out, currPoint);
                    dashMode = false;
                }
                else
                {
                    HPPenUp(out, currPoint);
                    dashMode = true;
                    patternIndex = NextPattern(patternIndex, pattern);
                }
                patternDistance = 0;
                break;
            }

            QLineF segment(prevPoint, currPoint);
            segment.setLength(patternDistance);
            prevPoint = segment.p2();
            segmentDistance += patternDistance;
            patternDistance = 0;

            if (dashMode)
            {
                HPPenDown(out, prevPoint.toPoint());
                dashMode = false;
            }
            else
            {
                HPPenUp(out, prevPoint.toPoint());
                dashMode = true;
                patternIndex = NextPattern(patternIndex, pattern);
            }
        } while (distance - segmentDistance > 0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLEngine::PlotPainterPath(QTextStream &out, const QPainterPath &path, Qt::PenStyle penStyle)
{
    const QList<QPolygonF> subpaths = path.toSubpathPolygons();

    for (const auto &subpath : subpaths)
    {
        PlotPath(out, CastToPoint(ConvertPath(subpath)), penStyle);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLEngine::HPPenUp(QTextStream &out, QPoint point)
{
    if (m_currentPos != point)
    {
        HPComand(out, *mPU, QStringLiteral("%1,%2").arg(point.x()).arg(point.y()));
        m_currentPos = point;
    }
    else
    {
        HPPenUp(out);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLEngine::HPPenUp(QTextStream &out)
{
    HPComand(out, *mPU);
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLEngine::HPPenDown(QTextStream &out, QPoint point)
{
    if (m_currentPos != point)
    {
        HPComand(out, *mPD, QStringLiteral("%1,%2").arg(point.x()).arg(point.y()));
        m_currentPos = point;
    }
    else
    {
        HPPenDown(out);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLEngine::HPPenDown(QTextStream &out)
{
    HPComand(out, *mPD);
}

//---------------------------------------------------------------------------------------------------------------------
auto VHPGLEngine::PatternForStyle(Qt::PenStyle style) const -> QVector<int>
{
    int penWidth = m_penWidthPx;

    if (penWidth <= 0)
    {
        penWidth = qCeil(ToPixel(0.025, Unit::Mm));
    }

    const int space = qCeil(ToPixel(2, Unit::Mm)) * penWidth * 3;
    const int dot = qCeil(ToPixel(1, Unit::Mm));
    const int dash = qCeil(ToPixel(4, Unit::Mm));

    QVector<int> pattern;
    pattern.reserve(6);

    switch (style)
    {
        case Qt::DashLine:
            pattern << dash << space;
            break;
        case Qt::DotLine:
            pattern << dot << space;
            break;
        case Qt::DashDotLine:
            pattern << dash << space << dot << space;
            break;
        case Qt::DashDotDotLine:
            pattern << dash << space << dot << space << dot << space;
            break;
        default:
            break;
    }
    return pattern;
}
