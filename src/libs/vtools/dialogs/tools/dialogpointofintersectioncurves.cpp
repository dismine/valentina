/************************************************************************
 **
 **  @file   dialogpointofintersectioncurves.cpp
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

#include "dialogpointofintersectioncurves.h"

#include <QColor>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>

#include "../../visualization/path/vistoolpointofintersectioncurves.h"
#include "../../visualization/visualization.h"
#include "../dialogtoolbox.h"
#include "../qmuparser/qmudef.h"
#include "../vpatterndb/vcontainer.h"
#include "ui_dialogpointofintersectioncurves.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
DialogPointOfIntersectionCurves::DialogPointOfIntersectionCurves(const VContainer *data, VAbstractPattern *doc,
                                                                 quint32 toolId, QWidget *parent)
  : DialogTool(data, doc, toolId, parent),
    ui(new Ui::DialogPointOfIntersectionCurves),
    pointName(),
    flagName(false),
    flagError(false)
{
    ui->setupUi(this);

    ui->lineEditNamePoint->setClearButtonEnabled(true);

    ui->lineEditNamePoint->setText(
        VAbstractValApplication::VApp()->getCurrentDocument()->GenerateLabel(LabelType::NewLabel));

    InitOkCancelApply(ui);

    FillComboBoxCurves(ui->comboBoxCurve1);
    FillComboBoxCurves(ui->comboBoxCurve2);
    FillComboBoxVCrossCurvesPoint(ui->comboBoxVCorrection);
    FillComboBoxHCrossCurvesPoint(ui->comboBoxHCorrection);

    ui->lineEditCurve1Name1->setText(GenerateDefCurve1LeftSubName());
    ui->lineEditCurve1Name2->setText(GenerateDefCurve1RightSubName());
    ui->lineEditCurve2Name1->setText(GenerateDefCurve2LeftSubName());
    ui->lineEditCurve2Name2->setText(GenerateDefCurve2RightSubName());

    connect(ui->lineEditNamePoint,
            &QLineEdit::textChanged,
            this,
            [this]() -> void
            {
                CheckPointLabel(this, ui->lineEditNamePoint, ui->labelEditNamePoint, pointName, this->data, flagName);
                CheckState();
            });
    connect(ui->comboBoxCurve1, &QComboBox::currentTextChanged, this, &DialogPointOfIntersectionCurves::CurveChanged);
    connect(ui->comboBoxCurve2, &QComboBox::currentTextChanged, this, &DialogPointOfIntersectionCurves::CurveChanged);

    connect(ui->lineEditCurve1Name1, &QLineEdit::textEdited, this, &DialogPointOfIntersectionCurves::ValidateName);
    connect(ui->lineEditCurve1Name2, &QLineEdit::textEdited, this, &DialogPointOfIntersectionCurves::ValidateName);
    connect(ui->lineEditCurve2Name1, &QLineEdit::textEdited, this, &DialogPointOfIntersectionCurves::ValidateName);
    connect(ui->lineEditCurve2Name2, &QLineEdit::textEdited, this, &DialogPointOfIntersectionCurves::ValidateName);

    connect(ui->lineEditCurve1Alias1, &QLineEdit::textEdited, this, &DialogPointOfIntersectionCurves::ValidateAlias);
    connect(ui->lineEditCurve1Alias2, &QLineEdit::textEdited, this, &DialogPointOfIntersectionCurves::ValidateAlias);
    connect(ui->lineEditCurve2Alias1, &QLineEdit::textEdited, this, &DialogPointOfIntersectionCurves::ValidateAlias);
    connect(ui->lineEditCurve2Alias2, &QLineEdit::textEdited, this, &DialogPointOfIntersectionCurves::ValidateAlias);

    vis = new VisToolPointOfIntersectionCurves(data);

    ui->tabWidget->setCurrentIndex(0);
    SetTabStopDistance(ui->plainTextEditToolNotes);
}

//---------------------------------------------------------------------------------------------------------------------
DialogPointOfIntersectionCurves::~DialogPointOfIntersectionCurves()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionCurves::GetPointName() const -> QString
{
    return pointName;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::SetPointName(const QString &value)
{
    pointName = value;
    ui->lineEditNamePoint->setText(pointName);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionCurves::GetFirstCurveId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxCurve1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::SetFirstCurveId(quint32 value)
{
    setCurrentCurveId(ui->comboBoxCurve1, value);

    auto *point = qobject_cast<VisToolPointOfIntersectionCurves *>(vis);
    SCASSERT(point != nullptr)
    point->SetCurve1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionCurves::GetSecondCurveId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxCurve2);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::SetSecondCurveId(quint32 value)
{
    setCurrentCurveId(ui->comboBoxCurve2, value);

    auto *point = qobject_cast<VisToolPointOfIntersectionCurves *>(vis);
    SCASSERT(point != nullptr)
    point->SetCurve2Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionCurves::GetVCrossPoint() const -> VCrossCurvesPoint
{
    return getCurrentCrossPoint<VCrossCurvesPoint>(ui->comboBoxVCorrection);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::SetVCrossPoint(VCrossCurvesPoint vP)
{
    auto index = ui->comboBoxVCorrection->findData(static_cast<int>(vP));
    if (index != -1)
    {
        ui->comboBoxVCorrection->setCurrentIndex(index);

        auto *point = qobject_cast<VisToolPointOfIntersectionCurves *>(vis);
        SCASSERT(point != nullptr)
        point->SetVCrossPoint(vP);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionCurves::GetHCrossPoint() const -> HCrossCurvesPoint
{
    return getCurrentCrossPoint<HCrossCurvesPoint>(ui->comboBoxHCorrection);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::SetHCrossPoint(HCrossCurvesPoint hP)
{
    auto index = ui->comboBoxHCorrection->findData(static_cast<int>(hP));
    if (index != -1)
    {
        ui->comboBoxHCorrection->setCurrentIndex(index);

        auto *point = qobject_cast<VisToolPointOfIntersectionCurves *>(vis);
        SCASSERT(point != nullptr)
        point->SetHCrossPoint(hP);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare == false && (type == SceneObject::Spline || type == SceneObject::Arc || type == SceneObject::ElArc ||
                             type == SceneObject::SplinePath)) // After first choose we ignore all objects
    {
        auto *point = qobject_cast<VisToolPointOfIntersectionCurves *>(vis);
        SCASSERT(point != nullptr)

        switch (number)
        {
            case 0:
                if (SetObject(id, ui->comboBoxCurve1, tr("Select second curve")))
                {
                    number++;
                    point->VisualMode(id);
                }
                break;
            case 1:
                if (getCurrentObjectId(ui->comboBoxCurve1) != id && SetObject(id, ui->comboBoxCurve2, QString()))
                {
                    number = 0;
                    point->SetCurve2Id(id);
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
void DialogPointOfIntersectionCurves::ShowVisualization()
{
    AddVisualization<VisToolPointOfIntersectionCurves>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::SaveData()
{
    pointName = ui->lineEditNamePoint->text();

    auto *point = qobject_cast<VisToolPointOfIntersectionCurves *>(vis);
    SCASSERT(point != nullptr)

    point->SetCurve1Id(GetFirstCurveId());
    point->SetCurve2Id(GetSecondCurveId());
    point->SetVCrossPoint(GetVCrossPoint());
    point->SetHCrossPoint(GetHCrossPoint());
    point->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::CurveChanged()
{
    QColor color;
    if (getCurrentObjectId(ui->comboBoxCurve1) == getCurrentObjectId(ui->comboBoxCurve2))
    {
        flagError = false;
        color = errorColor;
    }
    else
    {
        flagError = true;
        color = OkColor(this);
    }
    ChangeColor(ui->labelCurve1, color);
    ChangeColor(ui->labelCurve2, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::ValidateAlias()
{
    const QSharedPointer<VAbstractCurve> curve1 = data->GeometricObject<VAbstractCurve>(GetFirstCurveId());
    auto const [curve1AliasFirst, curve1AliasSecond] =
        SegmentAliases(curve1->getType(), GetCurve1AliasSuffix1(), GetCurve1AliasSuffix2());
    auto const [curve1Name1, curve1Name2] = SegmentNames(curve1->getType(), GetCurve1Name1(), GetCurve1Name2());

    const QSharedPointer<VAbstractCurve> curve2 = data->GeometricObject<VAbstractCurve>(GetSecondCurveId());
    auto const [curve2AliasFirst, curve2AliasSecond] =
        SegmentAliases(curve2->getType(), GetCurve2AliasSuffix1(), GetCurve2AliasSuffix2());
    auto const [curve2Name1, curve2Name2] = SegmentNames(curve2->getType(), GetCurve2Name1(), GetCurve2Name2());

    QRegularExpression const rx(NameRegExp());

    // Helper lambda to validate a single alias
    auto ValidateAlias = [&](const QString &alias,
                             const QString &suffix,
                             const QString &originSuffix,
                             const QSet<QString> &conflictSet) -> bool
    {
        if (suffix.isEmpty())
        {
            return true;
        }

        if (!rx.match(alias).hasMatch())
        {
            return false;
        }

        if (originSuffix != suffix && !data->IsUnique(alias))
        {
            return false;
        }

        if (conflictSet.contains(alias))
        {
            return false;
        }

        return true;
    };

    // Build conflict sets (excluding the alias being validated)
    const QSet<QString> conflictsForCurve1Alias1{curve1AliasSecond,
                                                 curve1Name1,
                                                 curve1Name2,
                                                 curve2AliasFirst,
                                                 curve2AliasSecond,
                                                 curve2Name1,
                                                 curve2Name2};
    const QSet<QString> conflictsForCurve1Alias2{curve1AliasFirst,
                                                 curve1Name1,
                                                 curve1Name2,
                                                 curve2AliasFirst,
                                                 curve2AliasSecond,
                                                 curve2Name1,
                                                 curve2Name2};
    const QSet<QString> conflictsForCurve2Alias1{curve1AliasFirst,
                                                 curve1AliasSecond,
                                                 curve1Name1,
                                                 curve1Name2,
                                                 curve2AliasSecond,
                                                 curve2Name1,
                                                 curve2Name2};
    const QSet<QString> conflictsForCurve2Alias2{curve1AliasFirst,
                                                 curve1AliasSecond,
                                                 curve1Name1,
                                                 curve1Name2,
                                                 curve2AliasFirst,
                                                 curve2Name1,
                                                 curve2Name2};

    // Validate all aliases
    flagCurve1Alias1 = ValidateAlias(curve1AliasFirst,
                                     GetCurve1AliasSuffix1(),
                                     originCurve1AliasSuffix1,
                                     conflictsForCurve1Alias1);
    flagCurve1Alias2 = ValidateAlias(curve1AliasSecond,
                                     GetCurve1AliasSuffix2(),
                                     originCurve1AliasSuffix2,
                                     conflictsForCurve1Alias2);
    flagCurve2Alias1 = ValidateAlias(curve2AliasFirst,
                                     GetCurve2AliasSuffix1(),
                                     originCurve2AliasSuffix1,
                                     conflictsForCurve2Alias1);
    flagCurve2Alias2 = ValidateAlias(curve2AliasSecond,
                                     GetCurve2AliasSuffix2(),
                                     originCurve2AliasSuffix2,
                                     conflictsForCurve2Alias2);

    // Update UI colors
    ChangeColor(ui->labelCurve1Alias1, flagCurve1Alias1 ? OkColor(this) : errorColor);
    ChangeColor(ui->labelCurve1Alias2, flagCurve1Alias2 ? OkColor(this) : errorColor);
    ChangeColor(ui->labelCurve2Alias1, flagCurve2Alias1 ? OkColor(this) : errorColor);
    ChangeColor(ui->labelCurve2Alias2, flagCurve2Alias2 ? OkColor(this) : errorColor);

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::ValidateName()
{
    const QSharedPointer<VAbstractCurve> curve1 = data->GeometricObject<VAbstractCurve>(GetFirstCurveId());
    auto const [curve1AliasFirst, curve1AliasSecond] = SegmentAliases(curve1->getType(),
                                                                      GetCurve1AliasSuffix1(),
                                                                      GetCurve1AliasSuffix2());
    auto const [curve1Name1, curve1Name2] = SegmentNames(curve1->getType(), GetCurve1Name1(), GetCurve1Name2());

    const QSharedPointer<VAbstractCurve> curve2 = data->GeometricObject<VAbstractCurve>(GetSecondCurveId());
    auto const [curve2AliasFirst, curve2AliasSecond] = SegmentAliases(curve2->getType(),
                                                                      GetCurve2AliasSuffix1(),
                                                                      GetCurve2AliasSuffix2());
    auto const [curve2Name1, curve2Name2] = SegmentNames(curve2->getType(), GetCurve2Name1(), GetCurve2Name2());

    QRegularExpression const rx(NameRegExp());

    // Helper lambda to validate a single name
    auto ValidateName = [&](const QString &name,
                            const QString &inputName,
                            const QString &originName,
                            const QSet<QString> &conflictSet) -> bool
    {
        if (inputName.isEmpty())
        {
            return false;
        }

        if (!rx.match(name).hasMatch())
        {
            return false;
        }

        if (originName != inputName && !data->IsUnique(name))
        {
            return false;
        }

        if (conflictSet.contains(name))
        {
            return false;
        }

        return true;
    };

    // Build conflict sets (excluding the name being validated)
    const QSet<QString> conflictsForCurve1Name1{curve1AliasFirst,
                                                curve1AliasSecond,
                                                curve1Name2,
                                                curve2AliasFirst,
                                                curve2AliasSecond,
                                                curve2Name1,
                                                curve2Name2};
    const QSet<QString> conflictsForCurve1Name2{curve1AliasFirst,
                                                curve1AliasSecond,
                                                curve1Name1,
                                                curve2AliasFirst,
                                                curve2AliasSecond,
                                                curve2Name1,
                                                curve2Name2};
    const QSet<QString> conflictsForCurve2Name1{curve1AliasFirst,
                                                curve1AliasSecond,
                                                curve1Name1,
                                                curve1Name2,
                                                curve2AliasFirst,
                                                curve2AliasSecond,
                                                curve2Name2};
    const QSet<QString> conflictsForCurve2Name2{curve1AliasFirst,
                                                curve1AliasSecond,
                                                curve1Name1,
                                                curve1Name2,
                                                curve2AliasFirst,
                                                curve2AliasSecond,
                                                curve2Name1};

    // Validate all names
    flagCurve1Name1 = ValidateName(curve1Name1, GetCurve1Name1(), originCurve1Name1, conflictsForCurve1Name1);
    flagCurve1Name2 = ValidateName(curve1Name2, GetCurve1Name2(), originCurve1Name2, conflictsForCurve1Name2);
    flagCurve2Name1 = ValidateName(curve2Name1, GetCurve2Name1(), originCurve2Name1, conflictsForCurve2Name1);
    flagCurve2Name2 = ValidateName(curve2Name2, GetCurve2Name2(), originCurve2Name2, conflictsForCurve2Name2);

    // Update UI colors
    ChangeColor(ui->labelCurve1Name1, flagCurve1Name1 ? OkColor(this) : errorColor);
    ChangeColor(ui->labelCurve1Name2, flagCurve1Name2 ? OkColor(this) : errorColor);
    ChangeColor(ui->labelCurve2Name1, flagCurve2Name1 ? OkColor(this) : errorColor);
    ChangeColor(ui->labelCurve2Name2, flagCurve2Name2 ? OkColor(this) : errorColor);

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionCurves::GenerateDefCurve1LeftSubName() const -> QString
{
    return GenerateDefSubCurveName(data, GetFirstCurveId(), "__1ls"_L1, "L1SubCurve"_L1);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionCurves::GenerateDefCurve1RightSubName() const -> QString
{
    return GenerateDefSubCurveName(data, GetFirstCurveId(), "__1rs"_L1, "R1SubCurve"_L1);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionCurves::GenerateDefCurve2LeftSubName() const -> QString
{
    return GenerateDefSubCurveName(data, GetSecondCurveId(), "__2ls"_L1, "L2SubCurve"_L1);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionCurves::GenerateDefCurve2RightSubName() const -> QString
{
    return GenerateDefSubCurveName(data, GetSecondCurveId(), "__2rs"_L1, "R2SubCurve"_L1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::SetNotes(const QString &notes)
{
    ui->plainTextEditToolNotes->setPlainText(notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionCurves::GetNotes() const -> QString
{
    return ui->plainTextEditToolNotes->toPlainText();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::SetCurve1Name1(const QString &name)
{
    originCurve1Name1 = name;
    ui->lineEditCurve1Name1->setText(originCurve1Name1);
    ValidateAlias();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionCurves::GetCurve1Name1() const -> QString
{
    return ui->lineEditCurve1Name1->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::SetCurve1Name2(const QString &name)
{
    originCurve1Name2 = name;
    ui->lineEditCurve1Name2->setText(originCurve1Name2);
    ValidateAlias();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionCurves::GetCurve1Name2() const -> QString
{
    return ui->lineEditCurve1Name2->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::SetCurve2Name1(const QString &name)
{
    originCurve2Name1 = name;
    ui->lineEditCurve2Name1->setText(originCurve2Name1);
    ValidateAlias();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionCurves::GetCurve2Name1() const -> QString
{
    return ui->lineEditCurve2Name1->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::SetCurve2Name2(const QString &name)
{
    originCurve2Name2 = name;
    ui->lineEditCurve2Name2->setText(originCurve2Name2);
    ValidateAlias();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionCurves::GetCurve2Name2() const -> QString
{
    return ui->lineEditCurve2Name2->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::SetCurve1AliasSuffix1(const QString &alias)
{
    originCurve1AliasSuffix1 = alias;
    ui->lineEditCurve1Alias1->setText(originCurve1AliasSuffix1);
    ValidateAlias();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionCurves::GetCurve1AliasSuffix1() const -> QString
{
    return ui->lineEditCurve1Alias1->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::SetCurve1AliasSuffix2(const QString &alias)
{
    originCurve1AliasSuffix2 = alias;
    ui->lineEditCurve1Alias2->setText(originCurve1AliasSuffix2);
    ValidateAlias();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionCurves::GetCurve1AliasSuffix2() const -> QString
{
    return ui->lineEditCurve1Alias2->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::SetCurve2AliasSuffix1(const QString &alias)
{
    originCurve2AliasSuffix1 = alias;
    ui->lineEditCurve2Alias1->setText(originCurve2AliasSuffix1);
    ValidateAlias();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionCurves::GetCurve2AliasSuffix1() const -> QString
{
    return ui->lineEditCurve2Alias1->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::SetCurve2AliasSuffix2(const QString &alias)
{
    originCurve2AliasSuffix2 = alias;
    ui->lineEditCurve2Alias2->setText(originCurve2AliasSuffix2);
    ValidateAlias();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionCurves::GetCurve2AliasSuffix2() const -> QString
{
    return ui->lineEditCurve2Alias2->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::CheckDependencyTreeComplete()
{
    const bool ready = m_doc->IsPatternGraphComplete();
    ui->lineEditNamePoint->setEnabled(ready);
    ui->lineEditCurve1Name1->setEnabled(ready);
    ui->lineEditCurve1Name2->setEnabled(ready);
    ui->lineEditCurve2Name1->setEnabled(ready);
    ui->lineEditCurve2Name2->setEnabled(ready);
    ui->lineEditCurve1Alias1->setEnabled(ready);
    ui->lineEditCurve1Alias2->setEnabled(ready);
    ui->lineEditCurve2Alias1->setEnabled(ready);
    ui->lineEditCurve2Alias2->setEnabled(ready);
}
