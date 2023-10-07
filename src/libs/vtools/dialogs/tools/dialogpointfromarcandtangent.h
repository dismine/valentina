/************************************************************************
 **
 **  @file   dialogpointfromarcandtangent.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   5 6, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#ifndef DIALOGPOINTFROMARCANDTANGENT_H
#define DIALOGPOINTFROMARCANDTANGENT_H

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui
{
class DialogPointFromArcAndTangent;
}

class DialogPointFromArcAndTangent : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    DialogPointFromArcAndTangent(const VContainer *data, VAbstractPattern *doc, quint32 toolId,
                                 QWidget *parent = nullptr);
    ~DialogPointFromArcAndTangent() override;

    auto GetPointName() const -> QString;
    void SetPointName(const QString &value);

    auto GetArcId() const -> quint32;
    void SetArcId(quint32 value);

    auto GetTangentPointId() const -> quint32;
    void SetTangentPointId(quint32 value);

    auto GetCrossCirclesPoint() const -> CrossCirclesPoint;
    void SetCrossCirclesPoint(CrossCirclesPoint p);

    void SetNotes(const QString &notes);
    auto GetNotes() const -> QString;

public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;

protected:
    void ShowVisualization() override;
    /**
     * @brief SaveData Put dialog data in local variables
     */
    void SaveData() override;
    auto IsValid() const -> bool final;

private:
    Q_DISABLE_COPY_MOVE(DialogPointFromArcAndTangent) // NOLINT

    Ui::DialogPointFromArcAndTangent *ui;

    QString pointName;

    bool flagName;

    /** @brief number number of handled objects */
    qint32 number{0};
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogPointFromArcAndTangent::IsValid() const -> bool
{
    return flagName;
}

#endif // DIALOGPOINTFROMARCANDTANGENT_H
