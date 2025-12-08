/************************************************************************
 **
 **  @file   vtoolparallelcurve.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   29 11, 2025
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2025 Valentina project
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
#ifndef VTOOLPARALLELCURVE_H
#define VTOOLPARALLELCURVE_H

#include "vabstractspline.h"

class VFormula;
template<class T>
class QSharedPointer;

struct VToolParallelCurveInitData : VAbstractSplineInitData
{
    VToolParallelCurveInitData() = default;

    quint32 originCurveId{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)
    QString formulaWidth{'0'};      // NOLINT(misc-non-private-member-variables-in-classes)
    QString suffix{};               // NOLINT(misc-non-private-member-variables-in-classes)
};

class VToolParallelCurve : public VAbstractSpline
{
    Q_OBJECT // NOLINT

public:
    ~VToolParallelCurve() override = default;
    void SetDialog() override;
    static auto Create(const QPointer<DialogTool> &dialog,
                       VMainGraphicsScene *scene,
                       VAbstractPattern *doc,
                       VContainer *data) -> VToolParallelCurve *;
    static auto Create(VToolParallelCurveInitData &initData) -> VToolParallelCurve *;
    static const QString ToolType;
    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Tool::ParallelCurve)
    };

    auto GetFormulaWidth() const -> VFormula;
    void SetFormulaWidth(const VFormula &value);

    auto GetApproximationScale() const -> qreal override;
    void SetApproximationScale(qreal value);

    auto GetSuffix() const -> QString;
    void SetSuffix(QString suffix);

    auto CurveName() const -> QString;

    void ShowVisualization(bool show) override;

protected slots:
    void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id = NULL_ID) override;

protected:
    void RemoveReferens() override;
    void SaveDialog(QDomElement &domElement, QList<quint32> &oldDependencies, QList<quint32> &newDependencies) override;
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    void SetVisualization() override;
    void ReadToolAttributes(const QDomElement &domElement) override;

private:
    Q_DISABLE_COPY_MOVE(VToolParallelCurve) // NOLINT

    QString m_formulaWidth;
    quint32 m_originCurveId;
    QString m_suffix;

    explicit VToolParallelCurve(const VToolParallelCurveInitData &initData, QGraphicsItem *parent = nullptr);
};

#endif // VTOOLPARALLELCURVE_H
