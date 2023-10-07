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

    connect(ui->lineEditNamePoint, &QLineEdit::textChanged, this,
            [this]()
            {
                CheckPointLabel(this, ui->lineEditNamePoint, ui->labelEditNamePoint, pointName, this->data, flagName);
                CheckState();
            });
    connect(ui->comboBoxCurve1, &QComboBox::currentTextChanged, this, &DialogPointOfIntersectionCurves::CurveChanged);
    connect(ui->comboBoxCurve2, &QComboBox::currentTextChanged, this, &DialogPointOfIntersectionCurves::CurveChanged);
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
    if (prepare == false) // After first choose we ignore all objects
    {
        if (type == SceneObject::Spline || type == SceneObject::Arc || type == SceneObject::ElArc ||
            type == SceneObject::SplinePath)
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
                    if (getCurrentObjectId(ui->comboBoxCurve1) != id)
                    {
                        if (SetObject(id, ui->comboBoxCurve2, QString()))
                        {
                            number = 0;
                            point->SetCurve2Id(id);
                            point->RefreshGeometry();
                            prepare = true;
                            DialogAccepted();
                        }
                    }
                    break;
                default:
                    break;
            }
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
    QRegularExpression rx(NameRegExp());

    const QSharedPointer<VAbstractCurve> curve1 = data->GeometricObject<VAbstractCurve>(GetFirstCurveId());
    QPair<QString, QString> curve1Alias =
        SegmentAliases(curve1->getType(), GetCurve1AliasSuffix1(), GetCurve1AliasSuffix2());

    const QSharedPointer<VAbstractCurve> curve2 = data->GeometricObject<VAbstractCurve>(GetSecondCurveId());
    QPair<QString, QString> curve2Alias =
        SegmentAliases(curve2->getType(), GetCurve2AliasSuffix1(), GetCurve2AliasSuffix2());

    QSet<QString> uniqueAliases;
    int countAliases = 0;

    auto CountUniqueAliases = [&countAliases, &uniqueAliases](const QString &alias)
    {
        if (not alias.isEmpty())
        {
            uniqueAliases.insert(alias);
            ++countAliases;
        }
    };

    CountUniqueAliases(curve1Alias.first);
    CountUniqueAliases(curve1Alias.second);
    CountUniqueAliases(curve2Alias.first);
    CountUniqueAliases(curve2Alias.second);

    auto Validate = [countAliases, uniqueAliases, rx, this](const QString &originalSuffix, const QString &suffix,
                                                            const QString &alias, bool &flagAlias, QLabel *label)
    {
        if (not suffix.isEmpty() &&
            (not rx.match(alias).hasMatch() || (originalSuffix != suffix && not data->IsUnique(alias)) ||
             countAliases != uniqueAliases.size()))
        {
            flagAlias = false;
            ChangeColor(label, errorColor);
        }
        else
        {
            flagAlias = true;
            ChangeColor(label, OkColor(this));
        }
    };

    Validate(originCurve1AliasSuffix1, GetCurve1AliasSuffix1(), curve1Alias.first, flagCurve1Alias1,
             ui->labelCurve1Alias1);
    Validate(originCurve1AliasSuffix2, GetCurve1AliasSuffix2(), curve1Alias.second, flagCurve1Alias2,
             ui->labelCurve1Alias2);
    Validate(originCurve2AliasSuffix1, GetCurve2AliasSuffix1(), curve2Alias.first, flagCurve2Alias1,
             ui->labelCurve2Alias1);
    Validate(originCurve2AliasSuffix2, GetCurve2AliasSuffix2(), curve2Alias.second, flagCurve2Alias2,
             ui->labelCurve2Alias2);

    CheckState();
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
