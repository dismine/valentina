/************************************************************************
 **
 **  @file   vlayoutdetail.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   2 1, 2015
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

#ifndef VLAYOUTDETAIL_H
#define VLAYOUTDETAIL_H

#include <QCoreApplication>
#include <QDate>
#include <QLineF>
#include <QPointF>
#include <QRectF>
#include <QSharedDataPointer>
#include <QString>
#include <QTypeInfo>
#include <QVector>
#include <QtGlobal>

#include "../vmisc/typedef.h"
#include "../vwidgets/vpiecegrainline.h"
#include "qpainterpath.h"
#include "vabstractpiece.h"

#include <optional>
using EdgeIndex = std::optional<int>;

class VLayoutPieceData;
class VLayoutPiecePath;
class QGraphicsItem;
class QGraphicsPathItem;
class VTextManager;
class VPiece;
class VPieceLabelData;
class VAbstractPattern;
class VPatternLabelData;
class VLayoutPoint;
class VFoldLine;

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")
QT_WARNING_DISABLE_GCC("-Wsuggest-final-methods")

class VLayoutPiece : public VAbstractPiece
{
    Q_DECLARE_TR_FUNCTIONS(VLayoutPiece) // NOLINT

public:
    VLayoutPiece();
    VLayoutPiece(const VLayoutPiece &detail);

    ~VLayoutPiece() override;

    auto operator=(const VLayoutPiece &detail) -> VLayoutPiece &;

    VLayoutPiece(VLayoutPiece &&detail) noexcept;
    auto operator=(VLayoutPiece &&detail) noexcept -> VLayoutPiece &;

    static auto Create(const VPiece &piece, vidtype id, const VContainer *pattern) -> VLayoutPiece;
    static auto ConvertPassmarks(const VPiece &piece, const VContainer *pattern) -> QVector<VLayoutPassmark>;

    auto GetUniqueID() const -> QString override;

    auto GetMappedContourPoints() const -> QVector<VLayoutPoint>;
    auto GetMappedFullContourPoints() const -> QVector<VLayoutPoint>;
    auto GetFullContourPoints() const -> QVector<VLayoutPoint>;
    auto GetContourPoints() const -> QVector<VLayoutPoint>;
    void SetContourPoints(const QVector<VLayoutPoint> &points, bool hideMainPath = false);

    auto GetMappedSeamAllowancePoints() const -> QVector<VLayoutPoint>;
    auto GetMappedFullSeamAllowancePoints() const -> QVector<VLayoutPoint>;
    auto GetFullSeamAllowancePoints() const -> QVector<VLayoutPoint>;
    auto GetSeamAllowancePoints() const -> QVector<VLayoutPoint>;
    void SetSeamAllowancePoints(const QVector<VLayoutPoint> &points, bool seamAllowance = true,
                                bool seamAllowanceBuiltIn = false);

    auto GetMappedLayoutAllowancePoints() const -> QVector<QPointF>;
    auto GetLayoutAllowancePoints() const -> QVector<QPointF>;
    void SetLayoutAllowancePoints(bool togetherWithNotches);

    auto GetMappedExternalContourPoints() const -> QVector<VLayoutPoint>;
    auto GetExternalContourPoints() const -> QVector<VLayoutPoint>;

    auto GetMappedPassmarks() const -> QVector<VLayoutPassmark>;
    auto GetPassmarks() const -> QVector<VLayoutPassmark>;
    void SetPassmarks(const QVector<VLayoutPassmark> &passmarks);

    auto GetPlaceLabels() const -> QVector<VLayoutPlaceLabel>;
    void SetPlaceLabels(const QVector<VLayoutPlaceLabel> &labels);

    auto MappedInternalPathsForCut(bool cut) const -> QVector<VLayoutPiecePath>;
    auto GetInternalPaths() const -> QVector<VLayoutPiecePath>;
    void SetInternalPaths(const QVector<VLayoutPiecePath> &internalPaths);

    auto GetPieceText() const -> QStringList;
    void SetPieceText(const VAbstractPattern *pDoc, const QString &qsName, const VPieceLabelData &data,
                      const QFont &font, const QString &SVGFontFamily, const VContainer *pattern);

    auto GetPatternText() const -> QStringList;
    void SetPatternInfo(const VAbstractPattern *pDoc, const VPatternLabelData &geom, const QFont &font,
                        const QString &SVGFontFamily, const VContainer *pattern);

    void SetGrainline(const VGrainlineData &geom, const VContainer *pattern);
    auto GetGrainline() const -> VPieceGrainline;
    auto GetGrainline() -> VPieceGrainline &;
    auto GetMappedGrainlineShape() const -> GrainlineShape;
    auto GetGrainlineShape() const -> GrainlineShape;
    auto GetMappedGrainlineMainLine() const -> QLineF;
    auto GetGrainlineMainLine() const -> QLineF;
    auto IsGrainlineEnabled() const -> bool;
    auto IsGrainlineVisible() const -> bool;

    auto GetMatrix() const -> QTransform;
    void SetMatrix(const QTransform &matrix);

    auto GetLayoutWidth() const -> qreal;
    void SetLayoutWidth(qreal value);

    auto GetQuantity() const -> quint16;
    void SetQuantity(quint16 value);

    auto GetId() const -> vidtype;
    void SetId(vidtype id);

    auto IsVerticallyFlipped() const -> bool;
    void SetVerticallyFlipped(bool value);

    auto IsHorizontallyFlipped() const -> bool;
    void SetHorizontallyFlipped(bool value);

    void SetGradationId(const QString &id);
    auto GetGradationId() const -> QString;

    auto GetXScale() const -> qreal;
    void SetXScale(qreal xs);

    auto GetYScale() const -> qreal;
    void SetYScale(qreal ys);

    auto GetPieceLabelRect() const -> QVector<QPointF>;
    void SetPieceLabelRect(const QVector<QPointF> &rect);

    auto GetPieceLabelData() const -> VTextManager;
    void SetPieceLabelData(const VTextManager &data);

    auto GetPatternLabelRect() const -> QVector<QPointF>;
    void SetPatternLabelRect(const QVector<QPointF> &rect);

    auto GetPatternLabelData() const -> VTextManager;
    void SetPatternLabelData(const VTextManager &data);

    auto GetMappedSeamMirrorLine() const -> QLineF;
    auto GetSeamMirrorLine() const -> QLineF;
    void SetSeamMirrorLine(const QLineF &line);

    auto GetMappedSeamAllowanceMirrorLine() const -> QLineF;
    auto GetSeamAllowanceMirrorLine() const -> QLineF;
    void SetSeamAllowanceMirrorLine(const QLineF &line);

    auto GetFoldLineHeight() const -> qreal;
    void SetFoldLineHeight(qreal height);

    auto GetFoldLineWidth() const -> qreal;
    void SetFoldLineWidth(qreal width);

    auto GetFoldLineCenterPosition() const -> qreal;
    void SetFoldLineCenterPosition(qreal center);

    auto GetFoldLineOutlineFont() const -> QFont;
    void SetFoldLineOutlineFont(const QFont &font);

    auto GetFoldLineSVGFontFamily() const -> QString;
    void SetFoldLineSVGFontFamily(const QString &font);

    void Translate(const QPointF &p);
    void Translate(qreal dx, qreal dy);
    void Scale(qreal sx, qreal sy);
    void Rotate(const QPointF &originPoint, qreal degrees);
    void Mirror(const QLineF &edge);
    void Mirror();

    auto DetailEdgesCount() const -> vsizetype;
    auto LayoutEdgesCount() const -> vsizetype;

    auto LayoutEdge(int i) const -> QLineF;
    auto LayoutEdgeByPoint(const QPointF &p1) const -> EdgeIndex;

    auto MappedDetailBoundingRect() const -> QRectF;
    auto DetailBoundingRect() const -> QRectF;
    auto MappedLayoutBoundingRect() const -> QRectF;
    auto Diagonal() const -> qreal;

    auto FoldLine() const -> VFoldLine;

    static auto BoundingRect(QVector<QPointF> points) -> QRectF;

    static auto GrainlinePath(const GrainlineShape &shape) -> QPainterPath;

    auto isNull() const -> bool;
    auto Square() const -> qint64;

    auto MappedContourPath(bool togetherWithNotches, bool showLayoutAllowance) const -> QPainterPath;
    auto ContourPath(bool togetherWithNotches, bool showLayoutAllowance) const -> QPainterPath;
    auto MappedLayoutAllowancePath() const -> QPainterPath;

    void DrawMiniature(QPainter &painter, bool togetherWithNotches) const;

    Q_REQUIRED_RESULT auto GetItem(bool textAsPaths, bool togetherWithNotches, bool showLayoutAllowance) const
        -> QGraphicsItem *;

    auto IsLayoutAllowanceValid(bool togetherWithNotches) const -> bool;

    auto BiggestEdge() const -> qreal;

    friend auto operator<<(QDataStream &dataStream, const VLayoutPiece &piece) -> QDataStream &;
    friend auto operator>>(QDataStream &dataStream, VLayoutPiece &piece) -> QDataStream &;

    auto MapPlaceLabelShape(PlaceLabelImg shape) const -> PlaceLabelImg;

    static auto MapPassmark(VLayoutPassmark passmark, const QTransform &matrix, bool mirror) -> VLayoutPassmark;

    auto LineMatrix(const QPointF &topLeft, qreal angle, const QPointF &linePos, qreal maxLineWidth) const
        -> QTransform;

protected:
    void SetGrainline(const VPieceGrainline &grainline);

private:
    QSharedDataPointer<VLayoutPieceData> d;

    auto DetailPath() const -> QVector<VLayoutPoint>;

    Q_REQUIRED_RESULT auto GetMainItem(bool togetherWithNotches, bool showLayoutAllowance) const -> QGraphicsPathItem *;
    Q_REQUIRED_RESULT auto GetMainPathItem() const -> QGraphicsPathItem *;

    void LabelStringsSVGFont(QGraphicsItem *parent, const QVector<QPointF> &labelShape, const VTextManager &tm,
                             bool textAsPaths) const;
    void LabelStringsOutlineFont(QGraphicsItem *parent, const QVector<QPointF> &labelShape, const VTextManager &tm,
                                 bool textAsPaths) const;

    void CreateLabelStrings(QGraphicsItem *parent, const QVector<QPointF> &labelShape, const VTextManager &tm,
                            bool textAsPaths) const;
    void CreateGrainlineItem(QGraphicsItem *parent) const;
    void CreateFoldLineItem(QGraphicsItem *parent, bool textAsPaths) const;

    template <class T> auto Map(QVector<T> points) const -> QVector<T>;
    auto Map(const GrainlineShape &shape) const -> GrainlineShape;

    auto Edge(const QVector<QPointF> &path, int i) const -> QLineF;
    auto EdgeByPoint(const QVector<QPointF> &path, const QPointF &p1) const -> EdgeIndex;

    auto CorrectSeamAllowanceMirrorLine() const -> QLineF;
};

QT_WARNING_POP

Q_DECLARE_TYPEINFO(VLayoutPiece, Q_MOVABLE_TYPE); // NOLINT

#endif // VLAYOUTDETAIL_H
