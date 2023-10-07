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
#include "dialogtool.h"
#include "ui_dialogpointofintersectionarcs.h"

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

    ui->lineEditNamePoint->setClearButtonEnabled(true);

    ui->lineEditNamePoint->setText(
        VAbstractValApplication::VApp()->getCurrentDocument()->GenerateLabel(LabelType::NewLabel));

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

    VisToolPointOfIntersectionArcs *point = qobject_cast<VisToolPointOfIntersectionArcs *>(vis);
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

    VisToolPointOfIntersectionArcs *point = qobject_cast<VisToolPointOfIntersectionArcs *>(vis);
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

        VisToolPointOfIntersectionArcs *point = qobject_cast<VisToolPointOfIntersectionArcs *>(vis);
        SCASSERT(point != nullptr)
        point->SetCrossPoint(p);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare == false) // After first choose we ignore all objects
    {
        if (type == SceneObject::Arc)
        {
            VisToolPointOfIntersectionArcs *point = qobject_cast<VisToolPointOfIntersectionArcs *>(vis);
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
                    if (getCurrentObjectId(ui->comboBoxArc1) != id)
                    {
                        if (SetObject(id, ui->comboBoxArc2, QString()))
                        {
                            number = 0;
                            point->SetArc2Id(id);
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

    VisToolPointOfIntersectionArcs *point = qobject_cast<VisToolPointOfIntersectionArcs *>(vis);
    SCASSERT(point != nullptr)

    point->SetArc1Id(GetFirstArcId());
    point->SetArc2Id(GetSecondArcId());
    point->SetCrossPoint(GetCrossArcPoint());
    point->RefreshGeometry();
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
