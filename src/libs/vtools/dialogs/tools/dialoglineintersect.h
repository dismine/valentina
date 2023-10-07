/************************************************************************
 **
 **  @file   dialoglineintersect.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
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

#ifndef DIALOGLINEINTERSECT_H
#define DIALOGLINEINTERSECT_H

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui
{
class DialogLineIntersect;
}

/**
 * @brief The DialogLineIntersect class dialog for ToolLineIntersect. Help create point and edit option.
 */
class DialogLineIntersect : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    DialogLineIntersect(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent = nullptr);
    ~DialogLineIntersect() override;

    auto GetP1Line1() const -> quint32;
    void SetP1Line1(quint32 value);

    auto GetP2Line1() const -> quint32;
    void SetP2Line1(quint32 value);

    auto GetP1Line2() const -> quint32;
    void SetP1Line2(quint32 value);

    auto GetP2Line2() const -> quint32;
    void SetP2Line2(quint32 value);

    auto GetPointName() const -> QString;
    void SetPointName(const QString &value);

    void SetNotes(const QString &notes);
    auto GetNotes() const -> QString;

public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;
    void PointChanged();
    void PointNameChanged() override;

protected:
    void ShowVisualization() override;
    /**
     * @brief SaveData Put dialog data in local variables
     */
    void SaveData() override;
    auto IsValid() const -> bool final;

private:
    Q_DISABLE_COPY_MOVE(DialogLineIntersect) // NOLINT

    /** @brief ui keeps information about user interface */
    Ui::DialogLineIntersect *ui;

    QString pointName;

    bool flagError;

    /** @brief flagPoint keep state of point */
    bool flagPoint;

    bool flagName;

    /** @brief number number of handled objects */
    qint32 number{0};

    auto CheckIntersecion() -> bool;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogLineIntersect::IsValid() const -> bool
{
    return flagName && flagError && flagPoint;
}

#endif // DIALOGLINEINTERSECT_H
