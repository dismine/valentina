/************************************************************************
 **
 **  @file   dialogpointofintersectioncurves.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   23 1, 2016
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

#ifndef DIALOGPOINTOFINTERSECTIONCURVES_H
#define DIALOGPOINTOFINTERSECTIONCURVES_H

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui
{
class DialogPointOfIntersectionCurves;
}

class DialogPointOfIntersectionCurves final : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    explicit DialogPointOfIntersectionCurves(const VContainer *data, VAbstractPattern *doc, quint32 toolId,
                                             QWidget *parent = nullptr);
    ~DialogPointOfIntersectionCurves() override;

    auto GetPointName() const -> QString;
    void SetPointName(const QString &value);

    auto GetFirstCurveId() const -> quint32;
    void SetFirstCurveId(quint32 value);

    auto GetSecondCurveId() const -> quint32;
    void SetSecondCurveId(quint32 value);

    auto GetVCrossPoint() const -> VCrossCurvesPoint;
    void SetVCrossPoint(VCrossCurvesPoint vP);

    auto GetHCrossPoint() const -> HCrossCurvesPoint;
    void SetHCrossPoint(HCrossCurvesPoint hP);

    void SetNotes(const QString &notes);
    auto GetNotes() const -> QString;

    void SetCurve1Name1(const QString &name);
    auto GetCurve1Name1() const -> QString;

    void SetCurve1Name2(const QString &name);
    auto GetCurve1Name2() const -> QString;

    void SetCurve2Name1(const QString &name);
    auto GetCurve2Name1() const -> QString;

    void SetCurve2Name2(const QString &name);
    auto GetCurve2Name2() const -> QString;

    void SetCurve1AliasSuffix1(const QString &alias);
    auto GetCurve1AliasSuffix1() const -> QString;

    void SetCurve1AliasSuffix2(const QString &alias);
    auto GetCurve1AliasSuffix2() const -> QString;

    void SetCurve2AliasSuffix1(const QString &alias);
    auto GetCurve2AliasSuffix1() const -> QString;

    void SetCurve2AliasSuffix2(const QString &alias);
    auto GetCurve2AliasSuffix2() const -> QString;

    void CheckDependencyTreeComplete() override;

public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;

protected:
    void ShowVisualization() override;

    /**
     * @brief SaveData Put dialog data in local variables
     */
    void SaveData() override;
    auto IsValid() const -> bool override;

private slots:
    void CurveChanged();
    void ValidateAlias();
    void ValidateName();

private:
    Q_DISABLE_COPY_MOVE(DialogPointOfIntersectionCurves) // NOLINT

    Ui::DialogPointOfIntersectionCurves *ui;

    QString pointName;

    bool flagName;
    bool flagError;

    bool flagCurve1Name1{true};
    bool flagCurve1Name2{true};
    bool flagCurve2Name1{true};
    bool flagCurve2Name2{true};

    bool flagCurve1Alias1{true};
    bool flagCurve1Alias2{true};
    bool flagCurve2Alias1{true};
    bool flagCurve2Alias2{true};

    QString originCurve1AliasSuffix1{};
    QString originCurve1AliasSuffix2{};
    QString originCurve2AliasSuffix1{};
    QString originCurve2AliasSuffix2{};

    QString originCurve1Name1{};
    QString originCurve1Name2{};
    QString originCurve2Name1{};
    QString originCurve2Name2{};

    /** @brief number number of handled objects */
    qint32 number{0};

    auto GenerateDefCurve1LeftSubName() const -> QString;
    auto GenerateDefCurve1RightSubName() const -> QString;
    auto GenerateDefCurve2LeftSubName() const -> QString;
    auto GenerateDefCurve2RightSubName() const -> QString;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogPointOfIntersectionCurves::IsValid() const -> bool
{
    return flagName && flagError && flagCurve1Alias1 && flagCurve1Alias2 && flagCurve2Alias1 && flagCurve2Alias2
           && flagCurve1Name1 && flagCurve1Name2 && flagCurve2Name1 && flagCurve2Name2;
}

#endif // DIALOGPOINTOFINTERSECTIONCURVES_H
