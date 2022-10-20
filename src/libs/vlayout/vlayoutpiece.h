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

#include <qcompilerdetection.h>
#include <QDate>
#include <QLineF>
#include <QMatrix>
#include <QPointF>
#include <QRectF>
#include <QSharedDataPointer>
#include <QString>
#include <QTypeInfo>
#include <QVector>
#include <QtGlobal>
#include <QCoreApplication>

#include "vabstractpiece.h"
#include "../vmisc/typedef.h"
#include "../vpatterndb/floatItemData/floatitemdef.h"

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

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")
QT_WARNING_DISABLE_GCC("-Wsuggest-final-methods")

class VLayoutPiece :public VAbstractPiece
{
    Q_DECLARE_TR_FUNCTIONS(VLayoutPiece) // NOLINT
public:
    VLayoutPiece();
    VLayoutPiece(const VLayoutPiece &detail);

    ~VLayoutPiece() override;

    auto operator=(const VLayoutPiece &detail) -> VLayoutPiece &;
#ifdef Q_COMPILER_RVALUE_REFS
    VLayoutPiece(VLayoutPiece &&detail) Q_DECL_NOTHROW;
    auto operator=(VLayoutPiece &&detail) Q_DECL_NOTHROW -> VLayoutPiece &;
#endif

    static auto Create(const VPiece &piece, vidtype id, const VContainer *pattern) -> VLayoutPiece;

    auto GetUniqueID() const -> QString override;

    auto GetMappedContourPoints() const -> QVector<VLayoutPoint>;
    auto GetContourPoints() const -> QVector<VLayoutPoint>;
    void SetCountourPoints(const QVector<VLayoutPoint> &points, bool hideMainPath = false);

    auto GetMappedSeamAllowancePoints() const -> QVector<VLayoutPoint>;
    auto GetSeamAllowancePoints() const -> QVector<VLayoutPoint>;
    void SetSeamAllowancePoints(const QVector<VLayoutPoint> &points, bool seamAllowance = true,
                                bool seamAllowanceBuiltIn = false);

    auto GetMappedLayoutAllowancePoints() const -> QVector<QPointF>;
    auto GetLayoutAllowancePoints() const -> QVector<QPointF>;
    void SetLayoutAllowancePoints();

    auto GetMappedExternalContourPoints() const -> QVector<VLayoutPoint>;
    auto GetExternalContourPoints() const -> QVector<VLayoutPoint>;

    auto GetMappedPassmarks() const -> QVector<VLayoutPassmark>;
    auto GetPassmarks() const -> QVector<VLayoutPassmark>;
    void SetPassmarks(const QVector<VLayoutPassmark> &passmarks);

    auto GetPlaceLabels() const -> QVector<VLayoutPlaceLabel>;
    void SetPlaceLabels(const QVector<VLayoutPlaceLabel> &labels);

    auto MappedInternalPathsForCut(bool cut) const -> QVector<QVector<VLayoutPoint> >;
    auto GetInternalPaths() const -> QVector<VLayoutPiecePath>;
    void SetInternalPaths(const QVector<VLayoutPiecePath> &internalPaths);

    auto GetPieceTextPosition() const -> QPointF;
    auto GetPieceText() const -> QStringList;
    void SetPieceText(const QString &qsName, const VPieceLabelData& data, const QFont& font, const VContainer *pattern);

    auto GetPatternTextPosition() const -> QPointF;
    auto GetPatternText() const -> QStringList;
    void SetPatternInfo(VAbstractPattern *pDoc, const VPatternLabelData& geom, const QFont& font,
                        const VContainer *pattern);

    void SetGrainline(const VGrainlineData& geom, const VContainer *pattern);
    auto GetMappedGrainline() const -> QVector<QPointF>;
    auto GetGrainline() const -> QVector<QPointF>;
    auto IsGrainlineEnabled() const -> bool;
    auto GrainlineAngle() const -> qreal;
    auto GrainlineArrowType() const -> GrainlineArrowDirection;

    auto GetMatrix() const -> QTransform;
    void SetMatrix(const QTransform &matrix);

    auto GetLayoutWidth() const -> qreal;
    void SetLayoutWidth(qreal value);

    auto GetQuantity() const -> quint16;
    void SetQuantity(quint16 value);

    auto GetId() const -> vidtype;
    void SetId(vidtype id);

    auto IsMirror() const -> bool;
    void SetMirror(bool value);

    void SetGradationId(const QString &id);
    auto GetGradationId() const -> QString;

    auto GetXScale() const -> qreal;
    void SetXScale(qreal xs);

    auto GetYScale() const -> qreal;
    void SetYScale(qreal ys);

    void Translate(const QPointF &p);
    void Translate(qreal dx, qreal dy);
    void Scale(qreal sx, qreal sy);
    void Rotate(const QPointF &originPoint, qreal degrees);
    void Mirror(const QLineF &edge);
    void Mirror();

    auto DetailEdgesCount() const -> int;
    auto LayoutEdgesCount() const -> int;

    auto LayoutEdge(int i) const -> QLineF;
    auto LayoutEdgeByPoint(const QPointF &p1) const -> int;

    auto MappedDetailBoundingRect() const -> QRectF;
    auto DetailBoundingRect() const -> QRectF;
    auto MappedLayoutBoundingRect() const -> QRectF;
    auto Diagonal() const -> qreal;

    static auto BoundingRect(QVector<QPointF> points) -> QRectF;

    auto isNull() const -> bool;
    auto Square() const -> qint64;

    auto MappedContourPath() const -> QPainterPath;
    auto ContourPath() const -> QPainterPath;
    auto MappedLayoutAllowancePath() const -> QPainterPath;

    void DrawMiniature(QPainter &painter) const;

    Q_REQUIRED_RESULT auto GetItem(bool textAsPaths) const -> QGraphicsItem *;

    auto IsLayoutAllowanceValid() const -> bool;

    auto BiggestEdge() const -> qreal;

    friend auto operator<< (QDataStream& dataStream, const VLayoutPiece& piece) -> QDataStream&;
    friend auto operator>> (QDataStream& dataStream, VLayoutPiece& piece) -> QDataStream&;

    auto MapPlaceLabelShape(PlaceLabelImg shape) const -> PlaceLabelImg;

protected:
    void SetGrainlineEnabled(bool enabled);
    void SetGrainlineAngle(qreal angle);
    void SetGrainlineArrowType(GrainlineArrowDirection type);
    void SetGrainlinePoints(const QVector<QPointF> &points);

    auto GetPieceLabelRect() const -> QVector<QPointF>;
    void SetPieceLabelRect(const QVector<QPointF> &rect);

    auto GetPieceLabelData() const ->VTextManager;
    void SetPieceLabelData(const VTextManager &data);

    auto GetPatternLabelRect() const -> QVector<QPointF>;
    void SetPatternLabelRect(const QVector<QPointF> &rect);

    auto GetPatternLabelData() const ->VTextManager;
    void SetPatternLabelData(const VTextManager &data);

private:
    QSharedDataPointer<VLayoutPieceData> d;

    auto DetailPath() const -> QVector<VLayoutPoint>;

    Q_REQUIRED_RESULT auto GetMainItem() const -> QGraphicsPathItem *;
    Q_REQUIRED_RESULT auto GetMainPathItem() const -> QGraphicsPathItem *;

    void CreateLabelStrings(QGraphicsItem *parent, const QVector<QPointF> &labelShape, const VTextManager &tm,
                            bool textAsPaths) const;
    void CreateGrainlineItem(QGraphicsItem *parent) const;

    template <class T>
    auto Map(QVector<T> points) const -> QVector<T>;

    auto Edge(const QVector<QPointF> &path, int i) const -> QLineF;
    auto EdgeByPoint(const QVector<QPointF> &path, const QPointF &p1) const -> int;
};

QT_WARNING_POP

Q_DECLARE_TYPEINFO(VLayoutPiece, Q_MOVABLE_TYPE); // NOLINT

#endif // VLAYOUTDETAIL_H
