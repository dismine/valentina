﻿/************************************************************************
 **
 **  @file   dialoglineintersect.cpp
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

#include "dialoglineintersect.h"

#include <QColor>
#include <QComboBox>
#include <QLabel>
#include <QLine>
#include <QLineEdit>
#include <QLineF>
#include <QPointF>
#include <QPointer>
#include <QPushButton>
#include <QSet>
#include <QSharedPointer>

#include "../../visualization/line/vistoollineintersect.h"
#include "../../visualization/visualization.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/compatibility.h"
#include "../vpatterndb/vcontainer.h"
#include "dialogtool.h"
#include "ui_dialoglineintersect.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogLineIntersect create dialog
 * @param data container with data
 * @param parent parent widget
 */
DialogLineIntersect::DialogLineIntersect(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent)
  : DialogTool(data, doc, toolId, parent),
    ui(new Ui::DialogLineIntersect),
    pointName(),
    flagError(true),
    flagPoint(true),
    flagName(true)
{
    ui->setupUi(this);

    ui->lineEditNamePoint->setClearButtonEnabled(true);

    number = 0;
    InitOkCancelApply(ui);
    ui->lineEditNamePoint->setText(
        VAbstractValApplication::VApp()->getCurrentDocument()->GenerateLabel(LabelType::NewLabel));

    FillComboBoxPoints(ui->comboBoxP1Line1);
    FillComboBoxPoints(ui->comboBoxP2Line1);
    FillComboBoxPoints(ui->comboBoxP1Line2);
    FillComboBoxPoints(ui->comboBoxP2Line2);

    connect(ui->lineEditNamePoint, &QLineEdit::textChanged, this,
            [this]()
            {
                CheckPointLabel(this, ui->lineEditNamePoint, ui->labelEditNamePoint, pointName, this->data, flagName);
                CheckState();
            });
    connect(ui->comboBoxP1Line1, &QComboBox::currentTextChanged, this, &DialogLineIntersect::PointNameChanged);
    connect(ui->comboBoxP2Line1, &QComboBox::currentTextChanged, this, &DialogLineIntersect::PointNameChanged);
    connect(ui->comboBoxP1Line2, &QComboBox::currentTextChanged, this, &DialogLineIntersect::PointNameChanged);
    connect(ui->comboBoxP2Line2, &QComboBox::currentTextChanged, this, &DialogLineIntersect::PointNameChanged);

    vis = new VisToolLineIntersect(data);

    ui->tabWidget->setCurrentIndex(0);
    SetTabStopDistance(ui->plainTextEditToolNotes);
}

//---------------------------------------------------------------------------------------------------------------------
DialogLineIntersect::~DialogLineIntersect()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChoosedObject gets id and type of selected object. Save right data and ignore wrong.
 * @param id id of point or detail
 * @param type type of object
 */
void DialogLineIntersect::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare == false && type == SceneObject::Point) // After first choose we ignore all objects
    {
        auto *line = qobject_cast<VisToolLineIntersect *>(vis);
        SCASSERT(line != nullptr)

        switch (number)
        {
            case 0:
                if (SetObject(id, ui->comboBoxP1Line1, tr("Select second point of first line")))
                {
                    number++;
                    line->VisualMode(id);
                }
                break;
            case 1:
                if (getCurrentObjectId(ui->comboBoxP1Line1) != id &&
                    SetObject(id, ui->comboBoxP2Line1, tr("Select first point of second line")))
                {
                    number++;
                    line->SetLine1P2Id(id);
                    line->RefreshGeometry();
                }
                break;
            case 2:
                if (SetObject(id, ui->comboBoxP1Line2, tr("Select second point of second line")))
                {
                    number++;
                    line->SetLine2P1Id(id);
                    line->RefreshGeometry();
                }
                break;
            case 3:
            {
                QSet<quint32> set;
                set.insert(getCurrentObjectId(ui->comboBoxP1Line1));
                set.insert(getCurrentObjectId(ui->comboBoxP2Line1));
                set.insert(getCurrentObjectId(ui->comboBoxP1Line2));
                set.insert(id);

                if (set.size() >= 3 && SetObject(id, ui->comboBoxP2Line2, QString()))
                {
                    line->SetLine2P2Id(id);
                    line->RefreshGeometry();
                    prepare = true;
                    flagPoint = CheckIntersecion();
                    CheckState();
                    if (flagPoint)
                    {
                        DialogAccepted();
                    }
                    else
                    {
                        this->setModal(true);
                        this->show();
                        connect(ui->comboBoxP1Line1, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                                &DialogLineIntersect::PointChanged);
                        connect(ui->comboBoxP2Line1, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                                &DialogLineIntersect::PointChanged);
                        connect(ui->comboBoxP1Line2, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                                &DialogLineIntersect::PointChanged);
                        connect(ui->comboBoxP2Line2, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                                &DialogLineIntersect::PointChanged);
                    }
                }
            }
            break;
            default:
                break;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersect::SaveData()
{
    pointName = ui->lineEditNamePoint->text();

    auto *line = qobject_cast<VisToolLineIntersect *>(vis);
    SCASSERT(line != nullptr)

    line->SetLine1P1Id(GetP1Line1());
    line->SetLine1P2Id(GetP2Line1());
    line->SetLine2P1Id(GetP1Line2());
    line->SetLine2P2Id(GetP2Line2());
    line->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief P1Line1Changed changed first point of first line
 */
void DialogLineIntersect::PointChanged()
{
    flagPoint = CheckIntersecion();
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersect::PointNameChanged()
{
    QColor color;

    try
    {
        QSet<quint32> set;
        const quint32 p1Line1Id = getCurrentObjectId(ui->comboBoxP1Line1);
        const quint32 p2Line1Id = getCurrentObjectId(ui->comboBoxP2Line1);
        const quint32 p1Line2Id = getCurrentObjectId(ui->comboBoxP1Line2);
        const quint32 p2Line2Id = getCurrentObjectId(ui->comboBoxP2Line2);

        set.insert(p1Line1Id);
        set.insert(p2Line1Id);
        set.insert(p1Line2Id);
        set.insert(p2Line2Id);

        const QSharedPointer<VPointF> p1Line1 = data->GeometricObject<VPointF>(p1Line1Id);
        const QSharedPointer<VPointF> p2Line1 = data->GeometricObject<VPointF>(p2Line1Id);
        const QSharedPointer<VPointF> p1Line2 = data->GeometricObject<VPointF>(p1Line2Id);
        const QSharedPointer<VPointF> p2Line2 = data->GeometricObject<VPointF>(p2Line2Id);

        QLineF const line1(static_cast<QPointF>(*p1Line1), static_cast<QPointF>(*p2Line1));
        QLineF const line2(static_cast<QPointF>(*p1Line2), static_cast<QPointF>(*p2Line2));
        QPointF fPoint;
        QLineF::IntersectType const intersect = line1.intersects(line2, &fPoint);

        flagError = not(set.size() < 3 || intersect == QLineF::NoIntersection);
        color = flagError ? OkColor(this) : errorColor;
    }
    catch (const VExceptionBadId &)
    {
        flagError = false;
        color = errorColor;
    }

    ChangeColor(ui->labelP1Line1, color);
    ChangeColor(ui->labelP2Line1, color);
    ChangeColor(ui->labelP1Line2, color);
    ChangeColor(ui->labelP2Line2, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersect::ShowVisualization()
{
    AddVisualization<VisToolLineIntersect>();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief CheckIntersecion check intersection of points
 * @return true - line have intersection, false = don't have
 */
auto DialogLineIntersect::CheckIntersecion() -> bool
{
    try
    {
        const QSharedPointer<VPointF> p1L1 = data->GeometricObject<VPointF>(GetP1Line1());
        const QSharedPointer<VPointF> p2L1 = data->GeometricObject<VPointF>(GetP2Line1());
        const QSharedPointer<VPointF> p1L2 = data->GeometricObject<VPointF>(GetP1Line2());
        const QSharedPointer<VPointF> p2L2 = data->GeometricObject<VPointF>(GetP2Line2());

        QLineF const line1(static_cast<QPointF>(*p1L1), static_cast<QPointF>(*p2L1));
        QLineF const line2(static_cast<QPointF>(*p1L2), static_cast<QPointF>(*p2L2));
        QPointF fPoint;
        QLineF::IntersectType const intersect = line1.intersects(line2, &fPoint);
        return intersect == QLineF::UnboundedIntersection || intersect == QLineF::BoundedIntersection;
    }
    catch (const VExceptionBadId &)
    {
        return false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetP2Line2 set id second point of second line
 * @param value id
 */
void DialogLineIntersect::SetP2Line2(quint32 value)
{
    setCurrentPointId(ui->comboBoxP2Line2, value);

    auto *line = qobject_cast<VisToolLineIntersect *>(vis);
    SCASSERT(line != nullptr)
    line->SetLine2P2Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogLineIntersect::GetPointName() const -> QString
{
    return pointName;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetP1Line2 set id first point of second line
 * @param value id
 */
void DialogLineIntersect::SetP1Line2(quint32 value)
{
    setCurrentPointId(ui->comboBoxP1Line2, value);

    auto *line = qobject_cast<VisToolLineIntersect *>(vis);
    SCASSERT(line != nullptr)
    line->SetLine2P1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetP2Line1 set id second point of first line
 * @param value id
 */
void DialogLineIntersect::SetP2Line1(quint32 value)
{
    setCurrentPointId(ui->comboBoxP2Line1, value);

    auto *line = qobject_cast<VisToolLineIntersect *>(vis);
    SCASSERT(line != nullptr)
    line->SetLine1P2Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetP1Line1 set id first point of first line
 * @param value id
 */
void DialogLineIntersect::SetP1Line1(quint32 value)
{
    setCurrentPointId(ui->comboBoxP1Line1, value);

    auto *line = qobject_cast<VisToolLineIntersect *>(vis);
    SCASSERT(line != nullptr)
    line->SetLine1P1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetPointName set name of point
 * @param value name of point
 */
void DialogLineIntersect::SetPointName(const QString &value)
{
    pointName = value;
    ui->lineEditNamePoint->setText(pointName);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetP1Line1 return id first point of first line
 * @return id
 */
auto DialogLineIntersect::GetP1Line1() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxP1Line1);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetP2Line1 return id second point of first line
 * @return id
 */
auto DialogLineIntersect::GetP2Line1() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxP2Line1);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetP1Line2 return id first point of second line
 * @return id
 */
auto DialogLineIntersect::GetP1Line2() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxP1Line2);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetP2Line2 return id second point of second line
 * @return id
 */
auto DialogLineIntersect::GetP2Line2() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxP2Line2);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersect::SetNotes(const QString &notes)
{
    ui->plainTextEditToolNotes->setPlainText(notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogLineIntersect::GetNotes() const -> QString
{
    return ui->plainTextEditToolNotes->toPlainText();
}
