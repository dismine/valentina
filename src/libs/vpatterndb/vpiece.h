/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 11, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#ifndef VPIECE_H
#define VPIECE_H

#include <QtGlobal>
#include <QSharedDataPointer>

#include "../vlayout/vabstractpiece.h"

class VPieceData;
class VPieceNode;
class VPiecePath;
class VPatternLabelData;
class VPieceLabelData;
class VGrainlineData;
class VContainer;
class QPainterPath;
class VPointF;
class VPassmark;

class VPiece : public VAbstractPiece
{
    Q_DECLARE_TR_FUNCTIONS(VPiece) // NOLINT
public:
    VPiece();
    VPiece(const VPiece &piece);

    virtual ~VPiece();

    VPiece &operator=(const VPiece &piece);
#ifdef Q_COMPILER_RVALUE_REFS
    VPiece(VPiece &&piece) Q_DECL_NOTHROW;
    VPiece &operator=(VPiece &&piece) Q_DECL_NOTHROW;
#endif

    VPiecePath GetPath() const;
    VPiecePath &GetPath();
    void       SetPath(const VPiecePath &path);

    QVector<VLayoutPoint>  MainPathPoints(const VContainer *data) const;
    QVector<VLayoutPoint>  UniteMainPathPoints(const VContainer *data) const;
    QVector<VPointF>       MainPathNodePoints(const VContainer *data, bool showExcluded = false) const;
    QVector<VLayoutPoint>  SeamAllowancePoints(const VContainer *data) const;
    QVector<QPointF>       CuttingPathPoints(const VContainer *data) const;
    QVector<QLineF>        PassmarksLines(const VContainer *data) const;

    QVector<VPassmark>     Passmarks(const VContainer *data) const;

    QVector<QPainterPath> CurvesPainterPath(const VContainer *data) const;

    QPainterPath        MainPathPath(const VContainer *data) const;
    static QPainterPath MainPathPath(const QVector<QPointF> &points);

    QPainterPath SeamAllowancePath(const VContainer *data) const;
    template <class T>
    QPainterPath SeamAllowancePath(const QVector<T> &points) const;
    QPainterPath PassmarksPath(const VContainer *data) const;
    QPainterPath PlaceLabelPath(const VContainer *data) const;

    bool IsSeamAllowanceValid(const VContainer *data) const;

    bool IsInLayout() const;
    void SetInLayout(bool inLayout);

    bool IsUnited() const;
    void SetUnited(bool united);

    auto GetShortName() const -> QString;
    void SetShortName(const QString &value);

    QString GetFormulaSAWidth() const;
    void    SetFormulaSAWidth(const QString &formula, qreal value);

    QVector<quint32> GetInternalPaths() const;
    QVector<quint32> &GetInternalPaths();
    void             SetInternalPaths(const QVector<quint32> &iPaths);

    QVector<CustomSARecord> GetCustomSARecords() const;
    QVector<CustomSARecord> &GetCustomSARecords();
    void                    SetCustomSARecords(const QVector<CustomSARecord> &records);

    QVector<quint32> GetPins() const;
    QVector<quint32> &GetPins();
    void             SetPins(const QVector<quint32> &pins);

    QVector<quint32> GetPlaceLabels() const;
    QVector<quint32> &GetPlaceLabels();
    void             SetPlaceLabels(const QVector<quint32> &labels);

    QList<quint32> Dependencies() const;
    QVector<quint32> MissingNodes(const VPiece &det) const;
    QVector<quint32> MissingCSAPath(const VPiece &det) const;
    QVector<quint32> MissingInternalPaths(const VPiece &det) const;
    QVector<quint32> MissingPins(const VPiece &det) const;
    QVector<quint32> MissingPlaceLabels(const VPiece &det) const;

    void                   SetPatternPieceData(const VPieceLabelData &data);
    VPieceLabelData&       GetPatternPieceData();
    const VPieceLabelData& GetPatternPieceData() const;

    void                     SetPatternInfo(const VPatternLabelData &info);
    VPatternLabelData&       GetPatternInfo();
    const VPatternLabelData& GetPatternInfo() const;

    void                    SetGrainlineGeometry(const VGrainlineData &data);
    VGrainlineData&         GetGrainlineGeometry();
    const VGrainlineData&   GetGrainlineGeometry() const;

    QVector<VPieceNode> GetUnitedPath(const VContainer *data) const;

    QVector<VLayoutPoint> SeamAllowancePointsWithRotation(const VContainer *data, int makeFirst) const;

    void SetGradationLabel(const QString &label);
    auto GetGradationLabel() const -> QString;

    static void DumpPiece(const VPiece &piece, const VContainer *data, const QString &templateName=QString());

    void TestInternalPaths(const VContainer *data) const;

    static auto ShortNameRegExp() -> QString;
    auto Area(const VContainer *data) const -> qreal;
private:
    QSharedDataPointer<VPieceData> d;

    QVector<CustomSARecord> GetValidRecords() const;
    QVector<CustomSARecord> FilterRecords(QVector<CustomSARecord> records) const;

    QVector<VSAPoint> GetNodeSAPoints(const QVector<VPieceNode> &path, int index, const VContainer *data) const;

    bool GetPassmarkSAPoint(const QVector<VPieceNode> &path, int index, const VContainer *data, VSAPoint &point) const;
    bool GetPassmarkPreviousSAPoints(const QVector<VPieceNode> &path, int index, const VSAPoint &passmarkSAPoint,
                                     const VContainer *data, VSAPoint &point, int passmarkIndex) const;
    bool GetPassmarkNextSAPoints(const QVector<VPieceNode> &path, int index, const VSAPoint &passmarkSAPoint,
                                 const VContainer *data, VSAPoint &point, int passmarkIndex) const;

    bool IsPassmarkVisible(const QVector<VPieceNode> &path, int passmarkIndex) const;

    VPassmark CreatePassmark(const QVector<VPieceNode> &path, int previousIndex, int passmarkIndex, int nextIndex,
                             const VContainer *data) const;

    static int IsCSAStart(const QVector<CustomSARecord> &records, quint32 id);

    QJsonObject MainPathToJson() const;
    QJsonObject DBToJson(const VContainer *data) const;

    qreal GlobalPassmarkLength(const VContainer *data) const;

    void TestInternalPathCuttingPathIntersection(const VContainer *data) const;
    void TestInternalPathsIntersections(const VContainer *data) const;
};

Q_DECLARE_TYPEINFO(VPiece, Q_MOVABLE_TYPE); // NOLINT

//---------------------------------------------------------------------------------------------------------------------
template <class T>
inline QPainterPath VPiece::SeamAllowancePath(const QVector<T> &points) const
{
    QPainterPath ekv;

    // seam allowence
    if (IsSeamAllowance() && not IsSeamAllowanceBuiltIn())
    {
        if (not points.isEmpty())
        {
            ekv.moveTo(points.at(0));
            for (qint32 i = 1; i < points.count(); ++i)
            {
                ekv.lineTo(points.at(i));
            }

#if !defined(V_NO_ASSERT)
            // uncomment for debug
//            QFont font;
//            font.setPixelSize(1);
//            for (qint32 i = 0; i < points.count(); ++i)
//            {
//                ekv.addEllipse(points.at(i).x()-accuracyPointOnLine, points.at(i).y()-accuracyPointOnLine,
//                               accuracyPointOnLine*2., accuracyPointOnLine*2.);
//                ekv.addText(points.at(i).x()-accuracyPointOnLine, points.at(i).y()-accuracyPointOnLine, font,
//                            QString::number(i+1));
//            }
#endif

            ekv.setFillRule(Qt::WindingFill);
        }
    }

    return ekv;
}

#endif // VPIECE_H
