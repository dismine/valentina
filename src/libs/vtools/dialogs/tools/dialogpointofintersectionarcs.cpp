/************************************************************************
 **
 **  @file   dialogpointofintersectionarcs.cpp
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

#include "dialogpointofintersectionarcs.h"

#include <QColor>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>

#include "../../visualization/line/vistoolpointofintersectionarcs.h"
#include "../../visualization/visualization.h"
#include "../vgeometry/varc.h"
#include "../vpatterndb/vcontainer.h"
#include "dialogtool.h"
#include "ui_dialogpointofintersectionarcs.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
DialogPointOfIntersectionArcs::DialogPointOfIntersectionArcs(const VContainer *data, VAbstractPattern *doc,
                                                             quint32 toolId, QWidget *parent)
  : DialogTool(data, doc, toolId, parent),
    ui(new Ui::DialogPointOfIntersectionArcs),
    pointName(),
    flagName(true),
    flagError(true)
{
    ui->setupUi(this);

    MakeFilterableComboBox(ui->comboBoxArc1);
    MakeFilterableComboBox(ui->comboBoxArc2);

    ui->lineEditNamePoint->setClearButtonEnabled(true);

    SetPointName(VAbstractValApplication::VApp()->getCurrentDocument()->GenerateLabel(LabelType::NewLabel));

    InitOkCancelApply(ui);

    FillComboBoxArcs(ui->comboBoxArc1);
    FillComboBoxArcs(ui->comboBoxArc2);
    FillComboBoxCrossCirclesPoints(ui->comboBoxResult);

    connect(ui->lineEditNamePoint, &QLineEdit::textChanged, this,
            [this]()
            {
                CheckPointLabel(this, ui->lineEditNamePoint, ui->labelEditNamePoint, pointName, this->data, flagName);
                CheckState();
            });
    connect(ui->comboBoxArc1, &QComboBox::currentTextChanged, this, &DialogPointOfIntersectionArcs::ArcChanged);
    connect(ui->comboBoxArc1, &QComboBox::currentTextChanged, this, &DialogPointOfIntersectionArcs::ArcChanged);

    connect(ui->lineEditArc1Name1, &QLineEdit::textEdited, this, &DialogPointOfIntersectionArcs::ValidateName);
    connect(ui->lineEditArc1Name2, &QLineEdit::textEdited, this, &DialogPointOfIntersectionArcs::ValidateName);
    connect(ui->lineEditArc2Name1, &QLineEdit::textEdited, this, &DialogPointOfIntersectionArcs::ValidateName);
    connect(ui->lineEditArc2Name2, &QLineEdit::textEdited, this, &DialogPointOfIntersectionArcs::ValidateName);

    connect(ui->lineEditArc1Alias1, &QLineEdit::textEdited, this, &DialogPointOfIntersectionArcs::ValidateAlias);
    connect(ui->lineEditArc1Alias2, &QLineEdit::textEdited, this, &DialogPointOfIntersectionArcs::ValidateAlias);
    connect(ui->lineEditArc2Alias1, &QLineEdit::textEdited, this, &DialogPointOfIntersectionArcs::ValidateAlias);
    connect(ui->lineEditArc2Alias2, &QLineEdit::textEdited, this, &DialogPointOfIntersectionArcs::ValidateAlias);

    vis = new VisToolPointOfIntersectionArcs(data);

    ui->tabWidget->setCurrentIndex(0);
    SetTabStopDistance(ui->plainTextEditToolNotes);
}

//---------------------------------------------------------------------------------------------------------------------
DialogPointOfIntersectionArcs::~DialogPointOfIntersectionArcs()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionArcs::GetPointName() const -> QString
{
    return pointName;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::SetPointName(const QString &value)
{
    pointName = value;
    ui->lineEditNamePoint->setText(pointName);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionArcs::GetFirstArcId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxArc1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::SetFirstArcId(quint32 value)
{
    setCurrentArcId(ui->comboBoxArc1, value);

    auto *point = qobject_cast<VisToolPointOfIntersectionArcs *>(vis);
    SCASSERT(point != nullptr)
    point->SetArc1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionArcs::GetSecondArcId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxArc2);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::SetSecondArcId(quint32 value)
{
    setCurrentArcId(ui->comboBoxArc2, value);

    auto *point = qobject_cast<VisToolPointOfIntersectionArcs *>(vis);
    SCASSERT(point != nullptr)
    point->SetArc2Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionArcs::GetCrossArcPoint() const -> CrossCirclesPoint
{
    return getCurrentCrossPoint<CrossCirclesPoint>(ui->comboBoxResult);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::SetCrossArcPoint(CrossCirclesPoint p)
{
    const qint32 index = ui->comboBoxResult->findData(static_cast<int>(p));
    if (index != -1)
    {
        ui->comboBoxResult->setCurrentIndex(index);

        auto *point = qobject_cast<VisToolPointOfIntersectionArcs *>(vis);
        SCASSERT(point != nullptr)
        point->SetCrossPoint(p);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare == false && type == SceneObject::Arc) // After first choose we ignore all objects
    {
        auto *point = qobject_cast<VisToolPointOfIntersectionArcs *>(vis);
        SCASSERT(point != nullptr)

        switch (number)
        {
            case 0:
                if (SetObject(id, ui->comboBoxArc1, tr("Select second an arc")))
                {
                    number++;
                    point->VisualMode(id);
                }
                break;
            case 1:
                if (getCurrentObjectId(ui->comboBoxArc1) != id && SetObject(id, ui->comboBoxArc2, QString()))
                {
                    ui->lineEditArc1Name1->setText(GenerateDefArc1LeftSubName());
                    ui->lineEditArc1Name2->setText(GenerateDefArc1RightSubName());
                    ui->lineEditArc2Name1->setText(GenerateDefArc2LeftSubName());
                    ui->lineEditArc2Name2->setText(GenerateDefArc2RightSubName());

                    number = 0;
                    point->SetArc2Id(id);
                    point->RefreshGeometry();
                    prepare = true;
                    DialogAccepted();
                }
                break;
            default:
                break;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::ArcChanged()
{
    QColor color;
    if (getCurrentObjectId(ui->comboBoxArc1) == getCurrentObjectId(ui->comboBoxArc2))
    {
        flagError = false;
        color = errorColor;
    }
    else
    {
        flagError = true;
        color = OkColor(this);
    }
    ChangeColor(ui->labelArc1, color);
    ChangeColor(ui->labelArc2, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::ShowVisualization()
{
    AddVisualization<VisToolPointOfIntersectionArcs>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::SaveData()
{
    pointName = ui->lineEditNamePoint->text();

    auto *point = qobject_cast<VisToolPointOfIntersectionArcs *>(vis);
    SCASSERT(point != nullptr)

    point->SetArc1Id(GetFirstArcId());
    point->SetArc2Id(GetSecondArcId());
    point->SetCrossPoint(GetCrossArcPoint());
    point->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::ValidateAlias()
{
    const QSharedPointer<VArc> arc1 = data->GeometricObject<VArc>(GetFirstArcId());
    auto const [arc1AliasFirst, arc1AliasSecond] = SegmentAliases(arc1->getType(),
                                                                  GetArc1AliasSuffix1(),
                                                                  GetArc1AliasSuffix2());
    auto const [arc1Name1, arc1Name2] = SegmentNames(arc1->getType(), GetArc1Name1(), GetArc1Name2());

    const QSharedPointer<VArc> arc2 = data->GeometricObject<VArc>(GetSecondArcId());
    auto const [arc2AliasFirst, arc2AliasSecond] = SegmentAliases(arc2->getType(),
                                                                  GetArc2AliasSuffix1(),
                                                                  GetArc2AliasSuffix2());
    auto const [arc2Name1, arc2Name2] = SegmentNames(arc2->getType(), GetArc2Name1(), GetArc2Name2());

    QRegularExpression const rx(NameRegExp());

    auto IsAliasFormatValid = [&](const QString &alias, const QString &suffix) -> bool
    { return suffix.isEmpty() || rx.match(alias).hasMatch(); };

    auto IsAliasUnique = [&](const QString &alias,
                             const QString &suffix,
                             const QString &originSuffix,
                             const QSet<QString> &conflictSet) -> bool
    {
        if (originSuffix != suffix && !data->IsUnique(alias))
        {
            return false;
        }
        return !conflictSet.contains(alias);
    };

    auto ValidateSingleAlias = [&](const QString &alias,
                                   const QString &suffix,
                                   const QString &originSuffix,
                                   const QSet<QString> &conflictSet) -> bool
    { return IsAliasFormatValid(alias, suffix) && IsAliasUnique(alias, suffix, originSuffix, conflictSet); };

    // Build conflict sets (excluding the alias being validated)
    const QSet<QString> conflictsForArc1Alias1{arc1AliasSecond,
                                               arc1Name1,
                                               arc1Name2,
                                               arc2AliasFirst,
                                               arc2AliasSecond,
                                               arc2Name1,
                                               arc2Name2};
    const QSet<QString> conflictsForArc1Alias2{arc1AliasFirst,
                                               arc1Name1,
                                               arc1Name2,
                                               arc2AliasFirst,
                                               arc2AliasSecond,
                                               arc2Name1,
                                               arc2Name2};
    const QSet<QString> conflictsForArc2Alias1{arc1AliasFirst,
                                               arc1AliasSecond,
                                               arc1Name1,
                                               arc1Name2,
                                               arc2AliasSecond,
                                               arc2Name1,
                                               arc2Name2};
    const QSet<QString> conflictsForArc2Alias2{arc1AliasFirst,
                                               arc1AliasSecond,
                                               arc1Name1,
                                               arc1Name2,
                                               arc2AliasFirst,
                                               arc2Name1,
                                               arc2Name2};

    // Validate all aliases
    flagArc1Alias1 = ValidateSingleAlias(arc1AliasFirst,
                                         GetArc1AliasSuffix1(),
                                         originArc1AliasSuffix1,
                                         conflictsForArc1Alias1);
    flagArc1Alias2 = ValidateSingleAlias(arc1AliasSecond,
                                         GetArc1AliasSuffix2(),
                                         originArc1AliasSuffix2,
                                         conflictsForArc1Alias2);
    flagArc2Alias1 = ValidateSingleAlias(arc2AliasFirst,
                                         GetArc2AliasSuffix1(),
                                         originArc2AliasSuffix1,
                                         conflictsForArc2Alias1);
    flagArc2Alias2 = ValidateSingleAlias(arc2AliasSecond,
                                         GetArc2AliasSuffix2(),
                                         originArc2AliasSuffix2,
                                         conflictsForArc2Alias2);

    // Update UI colors
    ChangeColor(ui->labelArc1Alias1, flagArc1Alias1 ? OkColor(this) : errorColor);
    ChangeColor(ui->labelArc1Alias2, flagArc1Alias2 ? OkColor(this) : errorColor);
    ChangeColor(ui->labelArc2Alias1, flagArc2Alias1 ? OkColor(this) : errorColor);
    ChangeColor(ui->labelArc2Alias2, flagArc2Alias2 ? OkColor(this) : errorColor);

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::ValidateName()
{
    const QSharedPointer<VArc> arc1 = data->GeometricObject<VArc>(GetFirstArcId());
    auto const [arc1AliasFirst, arc1AliasSecond] = SegmentAliases(arc1->getType(),
                                                                  GetArc1AliasSuffix1(),
                                                                  GetArc1AliasSuffix2());
    auto const [arc1Name1, arc1Name2] = SegmentNames(arc1->getType(), GetArc1Name1(), GetArc1Name2());

    const QSharedPointer<VArc> arc2 = data->GeometricObject<VArc>(GetSecondArcId());
    auto const [arc2AliasFirst, arc2AliasSecond] = SegmentAliases(arc2->getType(),
                                                                  GetArc2AliasSuffix1(),
                                                                  GetArc2AliasSuffix2());
    auto const [arc2Name1, arc2Name2] = SegmentNames(arc2->getType(), GetArc2Name1(), GetArc2Name2());

    QRegularExpression const rx(NameRegExp());

    auto IsNameFormatValid = [&](const QString &name, const QString &inputName) -> bool
    { return !inputName.isEmpty() && rx.match(name).hasMatch(); };

    auto IsNameUnique = [&](const QString &name,
                            const QString &inputName,
                            const QString &originName,
                            const QSet<QString> &conflictSet) -> bool
    {
        if (originName != inputName && !data->IsUnique(name))
        {
            return false;
        }
        return !conflictSet.contains(name);
    };

    auto ValidateSingleName = [&](const QString &name,
                                  const QString &inputName,
                                  const QString &originName,
                                  const QSet<QString> &conflictSet) -> bool
    { return IsNameFormatValid(name, inputName) && IsNameUnique(name, inputName, originName, conflictSet); };

    // Build conflict sets (excluding the name being validated)
    const QSet<QString> conflictsForArc1Name1{arc1AliasFirst,
                                              arc1AliasSecond,
                                              arc1Name2,
                                              arc2AliasFirst,
                                              arc2AliasSecond,
                                              arc2Name1,
                                              arc2Name2};
    const QSet<QString> conflictsForArc1Name2{arc1AliasFirst,
                                              arc1AliasSecond,
                                              arc1Name1,
                                              arc2AliasFirst,
                                              arc2AliasSecond,
                                              arc2Name1,
                                              arc2Name2};
    const QSet<QString> conflictsForArc2Name1{arc1AliasFirst,
                                              arc1AliasSecond,
                                              arc1Name1,
                                              arc1Name2,
                                              arc2AliasFirst,
                                              arc2AliasSecond,
                                              arc2Name2};
    const QSet<QString> conflictsForArc2Name2{arc1AliasFirst,
                                              arc1AliasSecond,
                                              arc1Name1,
                                              arc1Name2,
                                              arc2AliasFirst,
                                              arc2AliasSecond,
                                              arc2Name1};

    // Validate all names
    flagArc1Name1 = ValidateSingleName(arc1Name1, GetArc1Name1(), originArc1Name1, conflictsForArc1Name1);
    flagArc1Name2 = ValidateSingleName(arc1Name2, GetArc1Name2(), originArc1Name2, conflictsForArc1Name2);
    flagArc2Name1 = ValidateSingleName(arc2Name1, GetArc2Name1(), originArc2Name1, conflictsForArc2Name1);
    flagArc2Name2 = ValidateSingleName(arc2Name2, GetArc2Name2(), originArc2Name2, conflictsForArc2Name2);

    // Update UI colors
    ChangeColor(ui->labelArc1Name1, flagArc1Name1 ? OkColor(this) : errorColor);
    ChangeColor(ui->labelArc1Name2, flagArc1Name2 ? OkColor(this) : errorColor);
    ChangeColor(ui->labelArc2Name1, flagArc2Name1 ? OkColor(this) : errorColor);
    ChangeColor(ui->labelArc2Name2, flagArc2Name2 ? OkColor(this) : errorColor);

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionArcs::GenerateDefArc1LeftSubName() const -> QString
{
    return GenerateDefSubCurveName(data, GetFirstArcId(), "__1ls"_L1, "L1SubArc"_L1);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionArcs::GenerateDefArc1RightSubName() const -> QString
{
    return GenerateDefSubCurveName(data, GetFirstArcId(), "__1rs"_L1, "R1SubArc"_L1);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionArcs::GenerateDefArc2LeftSubName() const -> QString
{
    return GenerateDefSubCurveName(data, GetSecondArcId(), "__2ls"_L1, "L2SubArc"_L1);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionArcs::GenerateDefArc2RightSubName() const -> QString
{
    return GenerateDefSubCurveName(data, GetSecondArcId(), "__2rs"_L1, "R2SubArc"_L1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::SetNotes(const QString &notes)
{
    ui->plainTextEditToolNotes->setPlainText(notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionArcs::GetNotes() const -> QString
{
    return ui->plainTextEditToolNotes->toPlainText();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::SetArc1Name1(const QString &name)
{
    originArc1Name1 = name;
    ui->lineEditArc1Name1->setText(originArc1Name1);
    ValidateName();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionArcs::GetArc1Name1() const -> QString
{
    return ui->lineEditArc1Name1->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::SetArc1Name2(const QString &name)
{
    originArc1Name2 = name;
    ui->lineEditArc1Name2->setText(originArc1Name2);
    ValidateName();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionArcs::GetArc1Name2() const -> QString
{
    return ui->lineEditArc1Name2->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::SetArc2Name1(const QString &name)
{
    originArc2Name1 = name;
    ui->lineEditArc2Name1->setText(originArc2Name1);
    ValidateName();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionArcs::GetArc2Name1() const -> QString
{
    return ui->lineEditArc2Name1->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::SetArc2Name2(const QString &name)
{
    originArc2Name2 = name;
    ui->lineEditArc2Name2->setText(originArc2Name2);
    ValidateName();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionArcs::GetArc2Name2() const -> QString
{
    return ui->lineEditArc2Name2->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::SetArc1AliasSuffix1(const QString &alias)
{
    originArc1AliasSuffix1 = alias;
    ui->lineEditArc1Alias1->setText(originArc1AliasSuffix1);
    ValidateAlias();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionArcs::GetArc1AliasSuffix1() const -> QString
{
    return ui->lineEditArc1Alias1->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::SetArc1AliasSuffix2(const QString &alias)
{
    originArc1AliasSuffix2 = alias;
    ui->lineEditArc1Alias2->setText(originArc1AliasSuffix2);
    ValidateAlias();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionArcs::GetArc1AliasSuffix2() const -> QString
{
    return ui->lineEditArc1Alias2->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::SetArc2AliasSuffix1(const QString &alias)
{
    originArc2AliasSuffix1 = alias;
    ui->lineEditArc2Alias1->setText(originArc2AliasSuffix1);
    ValidateAlias();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionArcs::GetArc2AliasSuffix1() const -> QString
{
    return ui->lineEditArc2Alias1->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::SetArc2AliasSuffix2(const QString &alias)
{
    originArc2AliasSuffix2 = alias;
    ui->lineEditArc2Alias2->setText(originArc2AliasSuffix2);
    ValidateAlias();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionArcs::GetArc2AliasSuffix2() const -> QString
{
    return ui->lineEditArc2Alias2->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::CheckDependencyTreeComplete()
{
    const bool ready = m_doc->IsPatternGraphComplete();
    ui->lineEditNamePoint->setEnabled(ready);
    ui->lineEditArc1Name1->setEnabled(ready);
    ui->lineEditArc1Name2->setEnabled(ready);
    ui->lineEditArc2Name1->setEnabled(ready);
    ui->lineEditArc2Name2->setEnabled(ready);
    ui->lineEditArc1Alias1->setEnabled(ready);
    ui->lineEditArc1Alias2->setEnabled(ready);
    ui->lineEditArc2Alias1->setEnabled(ready);
    ui->lineEditArc2Alias2->setEnabled(ready);
}
