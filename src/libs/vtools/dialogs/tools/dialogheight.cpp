/************************************************************************
 **
 **  @file   dialogheight.cpp
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

#include "dialogheight.h"

#include <QColor>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QLineF>
#include <QPoint>
#include <QPointF>
#include <QPointer>
#include <QSet>
#include <QSharedPointer>
#include <new>

#include "../../visualization/line/visline.h"
#include "../../visualization/line/vistoolheight.h"
#include "../../visualization/visualization.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "dialogtool.h"
#include "ui_dialogheight.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogHeight create dialog
 * @param data container with data
 * @param parent parent widget
 */
DialogHeight::DialogHeight(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent)
  : DialogTool(data, doc, toolId, parent),
    ui(new Ui::DialogHeight)
{
    ui->setupUi(this);

    ui->lineEditNamePoint->setClearButtonEnabled(true);

    ui->lineEditNamePoint->setText(
        VAbstractValApplication::VApp()->getCurrentDocument()->GenerateLabel(LabelType::NewLabel));
    InitOkCancelApply(ui);

    FillComboBoxPoints(ui->comboBoxBasePoint);
    FillComboBoxPoints(ui->comboBoxP1Line);
    FillComboBoxPoints(ui->comboBoxP2Line);
    FillComboBoxTypeLine(ui->comboBoxLineType, LineStylesPics(ui->comboBoxLineType->palette().color(QPalette::Base),
                                                              ui->comboBoxLineType->palette().color(QPalette::Text)));
    FillComboBoxLineColors(ui->comboBoxLineColor);

    connect(ui->lineEditNamePoint, &QLineEdit::textChanged, this,
            [this]()
            {
                CheckPointLabel(this, ui->lineEditNamePoint, ui->labelEditNamePoint, m_pointName, this->data,
                                m_flagName);
                CheckState();
            });
    connect(ui->comboBoxBasePoint, &QComboBox::currentTextChanged, this, &DialogHeight::PointNameChanged);
    connect(ui->comboBoxP1Line, &QComboBox::currentTextChanged, this, &DialogHeight::PointNameChanged);
    connect(ui->comboBoxP2Line, &QComboBox::currentTextChanged, this, &DialogHeight::PointNameChanged);

    vis = new VisToolHeight(data);

    ui->tabWidget->setCurrentIndex(0);
    SetTabStopDistance(ui->plainTextEditToolNotes);
}

//---------------------------------------------------------------------------------------------------------------------
DialogHeight::~DialogHeight()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogHeight::GetPointName() const -> QString
{
    return m_pointName;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetPointName set name of point
 * @param value name
 */
void DialogHeight::SetPointName(const QString &value)
{
    m_pointName = value;
    ui->lineEditNamePoint->setText(m_pointName);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetTypeLine set type of line
 * @param value type
 */
void DialogHeight::SetTypeLine(const QString &value)
{
    ChangeCurrentData(ui->comboBoxLineType, value);
    vis->SetLineStyle(LineStyleToPenStyle(value));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetBasePointId set id base point of height
 * @param value id
 */
void DialogHeight::SetBasePointId(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxBasePoint, value);

    auto *line = qobject_cast<VisToolHeight *>(vis);
    SCASSERT(line != nullptr)
    line->SetBasePointId(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetP1LineId set id first point of line
 * @param value id
 */
void DialogHeight::SetP1LineId(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxP1Line, value);

    auto *line = qobject_cast<VisToolHeight *>(vis);
    SCASSERT(line != nullptr)
    line->SetLineP1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetP2LineId set id second point of line
 * @param value id
 */
void DialogHeight::SetP2LineId(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxP2Line, value);

    auto *line = qobject_cast<VisToolHeight *>(vis);
    SCASSERT(line != nullptr)
    line->SetLineP2Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogHeight::GetLineColor() const -> QString
{
    return GetComboBoxCurrentData(ui->comboBoxLineColor, ColorBlack);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogHeight::SetLineColor(const QString &value)
{
    ChangeCurrentData(ui->comboBoxLineColor, value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChoosedObject gets id and type of selected object. Save right data and ignore wrong.
 * @param id id of point or detail
 * @param type type of object
 */
void DialogHeight::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare) // After first choose we ignore all objects
    {
        return;
    }

    if (type == SceneObject::Point)
    {
        auto *line = qobject_cast<VisToolHeight *>(vis);
        SCASSERT(line != nullptr)

        switch (m_number)
        {
            case (0):
                if (SetObject(id, ui->comboBoxBasePoint, tr("Select first point of line")))
                {
                    m_number++;
                    line->VisualMode(id);
                }
                break;
            case (1):
                if (getCurrentObjectId(ui->comboBoxBasePoint) != id)
                {
                    if (SetObject(id, ui->comboBoxP1Line, tr("Select second point of line")))
                    {
                        m_number++;
                        line->SetLineP1Id(id);
                        line->RefreshGeometry();
                    }
                }
                break;
            case (2):
            {
                QSet<quint32> set;
                set.insert(getCurrentObjectId(ui->comboBoxBasePoint));
                set.insert(getCurrentObjectId(ui->comboBoxP1Line));
                set.insert(id);

                if (set.size() == 3)
                {
                    if (SetObject(id, ui->comboBoxP2Line, QString()))
                    {
                        line->SetLineP2Id(id);
                        line->RefreshGeometry();
                        prepare = true;
                        DialogAccepted();
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
void DialogHeight::SaveData()
{
    m_pointName = ui->lineEditNamePoint->text();

    auto *line = qobject_cast<VisToolHeight *>(vis);
    SCASSERT(line != nullptr)

    line->SetBasePointId(GetBasePointId());
    line->SetLineP1Id(GetP1LineId());
    line->SetLineP2Id(GetP2LineId());
    line->SetLineStyle(LineStyleToPenStyle(GetTypeLine()));
    line->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogHeight::PointNameChanged()
{
    QSet<quint32> set;
    const quint32 basePointId = getCurrentObjectId(ui->comboBoxBasePoint);
    const quint32 p1LineId = getCurrentObjectId(ui->comboBoxP1Line);
    const quint32 p2LineId = getCurrentObjectId(ui->comboBoxP2Line);

    set.insert(basePointId);
    set.insert(p1LineId);
    set.insert(p2LineId);

    QColor color;

    try
    {
        const QPointF basePoint = static_cast<QPointF>(*data->GeometricObject<VPointF>(basePointId));
        const QPointF p1Line = static_cast<QPointF>(*data->GeometricObject<VPointF>(p1LineId));
        const QPointF p2Line = static_cast<QPointF>(*data->GeometricObject<VPointF>(p2LineId));

        if (set.size() != 3 || VGObject::ClosestPoint(QLineF(p1Line, p2Line), basePoint) == QPointF())
        {
            m_flagError = false;
            color = errorColor;
        }
        else
        {
            m_flagError = true;
            color = OkColor(this);
        }
    }
    catch (const VExceptionBadId &)
    {
        m_flagError = false;
        color = errorColor;
    }

    ChangeColor(ui->labelBasePoint, color);
    ChangeColor(ui->labelFirstLinePoint, color);
    ChangeColor(ui->labelSecondLinePoint, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogHeight::ShowVisualization()
{
    AddVisualization<VisLine>();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetTypeLine return type of line
 * @return type
 */
auto DialogHeight::GetTypeLine() const -> QString
{
    return GetComboBoxCurrentData(ui->comboBoxLineType, TypeLineLine);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetBasePointId return id base point of height
 * @return id
 */
auto DialogHeight::GetBasePointId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxBasePoint);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetP1LineId return id first point of line
 * @return id id
 */
auto DialogHeight::GetP1LineId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxP1Line);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetP2LineId return id second point of line
 * @return id
 */
auto DialogHeight::GetP2LineId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxP2Line);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogHeight::SetNotes(const QString &notes)
{
    ui->plainTextEditToolNotes->setPlainText(notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogHeight::GetNotes() const -> QString
{
    return ui->plainTextEditToolNotes->toPlainText();
}
