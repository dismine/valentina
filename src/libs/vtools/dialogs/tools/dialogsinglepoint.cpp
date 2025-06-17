/************************************************************************
 **
 **  @file   dialogsinglepoint.cpp
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

#include "dialogsinglepoint.h"

#include <QDoubleSpinBox>
#include <QLineEdit>

#include "../vmisc/def.h"
#include "dialogtool.h"
#include "ui_dialogsinglepoint.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogSinglePoint create dialog
 * @param data container with data
 * @param parent parent widget
 */
DialogSinglePoint::DialogSinglePoint(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent)
  : DialogTool(data, doc, toolId, parent),
    ui(new Ui::DialogSinglePoint),
    point(),
    pointName(),
    flagName(true)
{
    ui->setupUi(this);

    ui->lineEditName->setClearButtonEnabled(true);

    ui->doubleSpinBoxX->setRange(0, VAbstractValApplication::VApp()->fromPixel(maxSceneSize));
    ui->doubleSpinBoxY->setRange(0, VAbstractValApplication::VApp()->fromPixel(maxSceneSize));
    InitOkCancel(ui);

    connect(ui->lineEditName, &QLineEdit::textChanged, this,
            [this]()
            {
                CheckPointLabel(this, ui->lineEditName, ui->labelEditName, pointName, this->data, flagName);
                CheckState();
            });

    ui->tabWidget->setCurrentIndex(0);
    SetTabStopDistance(ui->plainTextEditToolNotes);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief mousePress get mouse position
 * @param scenePos position of cursor
 */
void DialogSinglePoint::mousePress(const QPointF &scenePos)
{
    if (isInitialized == false)
    {
        ui->doubleSpinBoxX->setValue(VAbstractValApplication::VApp()->fromPixel(scenePos.x()));
        ui->doubleSpinBoxY->setValue(VAbstractValApplication::VApp()->fromPixel(scenePos.y()));
        this->show();
    }
    else
    {
        ui->doubleSpinBoxX->setValue(VAbstractValApplication::VApp()->fromPixel(scenePos.x()));
        ui->doubleSpinBoxY->setValue(VAbstractValApplication::VApp()->fromPixel(scenePos.y()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSinglePoint::SaveData()
{
    point = QPointF(VAbstractValApplication::VApp()->toPixel(ui->doubleSpinBoxX->value()),
                    VAbstractValApplication::VApp()->toPixel(ui->doubleSpinBoxY->value()));
    pointName = ui->lineEditName->text();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setData set name and point
 * @param name name of point
 * @param point data for point
 */
void DialogSinglePoint::SetData(const QString &name, const QPointF &point)
{
    pointName = name;
    this->point = point;
    isInitialized = true;
    ui->lineEditName->setText(name);
    ui->doubleSpinBoxX->setValue(VAbstractValApplication::VApp()->fromPixel(point.x()));
    ui->doubleSpinBoxY->setValue(VAbstractValApplication::VApp()->fromPixel(point.y()));
}

//---------------------------------------------------------------------------------------------------------------------
DialogSinglePoint::~DialogSinglePoint()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getPoint return point
 * @return point
 */
auto DialogSinglePoint::GetPoint() const -> QPointF
{
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSinglePoint::GetPointName() const -> QString
{
    return pointName;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSinglePoint::SetNotes(const QString &notes)
{
    ui->plainTextEditToolNotes->setPlainText(notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSinglePoint::GetNotes() const -> QString
{
    return ui->plainTextEditToolNotes->toPlainText();
}
