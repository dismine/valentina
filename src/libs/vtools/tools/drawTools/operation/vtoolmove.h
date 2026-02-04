/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   1 10, 2016
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

#ifndef VTOOLMOVING_H
#define VTOOLMOVING_H

#include <QMap>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QVector>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "vabstractoperation.h"

template <class T> class QSharedPointer;
class VFormula;

struct VToolMoveInitData : VAbstractOperationInitData
{
    QString formulaAngle{};
    QString formulaRotationAngle{};
    QString formulaLength{};
    quint32 rotationOrigin{NULL_ID};
};

class VToolMove : public VAbstractOperation
{
    Q_OBJECT // NOLINT

public:
    ~VToolMove() override = default;

    void SetDialog() override;

    static auto Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolMove *;
    static auto Create(VToolMoveInitData &initData) -> VToolMove *;

    static const QString ToolType;

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Tool::Move)
    };

    auto GetFormulaAngle() const -> VFormula;
    void SetFormulaAngle(const VFormula &value);

    auto GetFormulaRotationAngle() const -> VFormula;
    void SetFormulaRotationAngle(const VFormula &value);

    auto GetFormulaLength() const -> VFormula;
    void SetFormulaLength(const VFormula &value);

    auto OriginPointName() const -> QString;

    void ShowVisualization(bool show) override;

protected slots:
    void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id = NULL_ID) override;

protected:
    void SetVisualization() override;
    void SaveDialog(QDomElement &domElement) override;
    void ReadToolAttributes(const QDomElement &domElement) override;
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    auto MakeToolTip() const -> QString override;
    void ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement) override;

private:
    Q_DISABLE_COPY_MOVE(VToolMove) // NOLINT
    QString formulaAngle;
    QString formulaRotationAngle;
    QString formulaLength;
    quint32 origPointId;

    explicit VToolMove(const VToolMoveInitData &initData, QGraphicsItem *parent = nullptr);
};

#endif // VTOOLMOVING_H
