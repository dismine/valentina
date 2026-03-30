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

class DialogPointOfIntersectionArcs final : public DialogTool
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

    void SetArc1Name1(const QString &name);
    auto GetArc1Name1() const -> QString;

    void SetArc1Name2(const QString &name);
    auto GetArc1Name2() const -> QString;

    void SetArc2Name1(const QString &name);
    auto GetArc2Name1() const -> QString;

    void SetArc2Name2(const QString &name);
    auto GetArc2Name2() const -> QString;

    void SetArc1AliasSuffix1(const QString &alias);
    auto GetArc1AliasSuffix1() const -> QString;

    void SetArc1AliasSuffix2(const QString &alias);
    auto GetArc1AliasSuffix2() const -> QString;

    void SetArc2AliasSuffix1(const QString &alias);
    auto GetArc2AliasSuffix1() const -> QString;

    void SetArc2AliasSuffix2(const QString &alias);
    auto GetArc2AliasSuffix2() const -> QString;

    void CheckDependencyTreeComplete() override;

public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;
    void ArcChanged();

protected:
    void ShowVisualization() override;
    /**
     * @brief SaveData Put dialog data in local variables
     */
    void SaveData() override;
    auto IsValid() const -> bool override;

private slots:
    void ValidateAlias();
    void ValidateName();

private:
    Q_DISABLE_COPY_MOVE(DialogPointOfIntersectionArcs) // NOLINT

    Ui::DialogPointOfIntersectionArcs *ui;

    QString pointName;

    bool flagName;
    bool flagError;

    bool flagArc1Name1{true};
    bool flagArc1Name2{true};
    bool flagArc2Name1{true};
    bool flagArc2Name2{true};

    bool flagArc1Alias1{true};
    bool flagArc1Alias2{true};
    bool flagArc2Alias1{true};
    bool flagArc2Alias2{true};

    QString originArc1AliasSuffix1{};
    QString originArc1AliasSuffix2{};
    QString originArc2AliasSuffix1{};
    QString originArc2AliasSuffix2{};

    QString originArc1Name1{};
    QString originArc1Name2{};
    QString originArc2Name1{};
    QString originArc2Name2{};

    /** @brief number number of handled objects */
    qint32 number{0};

    auto GenerateDefArc1LeftSubName() const -> QString;
    auto GenerateDefArc1RightSubName() const -> QString;
    auto GenerateDefArc2LeftSubName() const -> QString;
    auto GenerateDefArc2RightSubName() const -> QString;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogPointOfIntersectionArcs::IsValid() const -> bool
{
    return flagName && flagError && flagArc1Alias1 && flagArc1Alias2 && flagArc2Alias1 && flagArc2Alias2
           && flagArc1Name1 && flagArc1Name2 && flagArc2Name1 && flagArc2Name2;
}

#endif // DIALOGPOINTOFINTERSECTIONARCS_H
