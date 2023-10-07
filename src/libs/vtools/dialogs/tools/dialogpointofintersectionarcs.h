/************************************************************************
 **
 **  @file   dialogpointofintersectionarcs.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   25 5, 2015
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

#ifndef DIALOGPOINTOFINTERSECTIONARCS_H
#define DIALOGPOINTOFINTERSECTIONARCS_H

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui
{
class DialogPointOfIntersectionArcs;
}

class DialogPointOfIntersectionArcs : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    DialogPointOfIntersectionArcs(const VContainer *data, VAbstractPattern *doc, quint32 toolId,
                                  QWidget *parent = nullptr);
    ~DialogPointOfIntersectionArcs() override;

    auto GetPointName() const -> QString;
    void SetPointName(const QString &value);

    auto GetFirstArcId() const -> quint32;
    void SetFirstArcId(quint32 value);

    auto GetSecondArcId() const -> quint32;
    void SetSecondArcId(quint32 value);

    auto GetCrossArcPoint() const -> CrossCirclesPoint;
    void SetCrossArcPoint(CrossCirclesPoint p);

    void SetNotes(const QString &notes);
    auto GetNotes() const -> QString;

public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;
    void ArcChanged();

protected:
    void ShowVisualization() override;
    /**
     * @brief SaveData Put dialog data in local variables
     */
    void SaveData() override;
    auto IsValid() const -> bool final;

private:
    Q_DISABLE_COPY_MOVE(DialogPointOfIntersectionArcs) // NOLINT

    Ui::DialogPointOfIntersectionArcs *ui;

    QString pointName;

    bool flagName;
    bool flagError;

    /** @brief number number of handled objects */
    qint32 number{0};
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogPointOfIntersectionArcs::IsValid() const -> bool
{
    return flagName && flagError;
}

#endif // DIALOGPOINTOFINTERSECTIONARCS_H
