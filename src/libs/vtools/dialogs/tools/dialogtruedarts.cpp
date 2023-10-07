/************************************************************************
 **
 **  @file   dialogtruedarts.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 6, 2015
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

#include "dialogtruedarts.h"

#include <QColor>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSet>

#include "../../visualization/line/vistooltruedarts.h"
#include "../../visualization/visualization.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../qmuparser/qmudef.h"
#include "../vpatterndb/vcontainer.h"
#include "dialogtool.h"
#include "ui_dialogtruedarts.h"

//---------------------------------------------------------------------------------------------------------------------
DialogTrueDarts::DialogTrueDarts(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent)
  : DialogTool(data, doc, toolId, parent),
    ui(new Ui::DialogTrueDarts),
    d1PointName(),
    d2PointName(),
    ch1(NULL_ID),
    ch2(NULL_ID),
    flagName1(true),
    flagName2(true),
    flagError(false)
{
    ui->setupUi(this);

    ui->lineEditFirstNewDartPoint->setClearButtonEnabled(true);
    ui->lineEditSecondNewDartPoint->setClearButtonEnabled(true);

    const QString name1 = VAbstractValApplication::VApp()->getCurrentDocument()->GenerateLabel(LabelType::NewLabel);
    const QString name2 =
        VAbstractValApplication::VApp()->getCurrentDocument()->GenerateLabel(LabelType::NewLabel, name1);
    ui->lineEditFirstNewDartPoint->setText(name1);
    ui->lineEditSecondNewDartPoint->setText(name2);

    InitOkCancelApply(ui);

    FillComboBoxs(ch1, ch2);

    connect(ui->lineEditFirstNewDartPoint, &QLineEdit::textChanged, this, &DialogTrueDarts::NameDartPoint1Changed);
    connect(ui->lineEditSecondNewDartPoint, &QLineEdit::textChanged, this, &DialogTrueDarts::NameDartPoint2Changed);
    connect(ui->comboBoxFirstBasePoint, &QComboBox::currentTextChanged, this, &DialogTrueDarts::PointNameChanged);
    connect(ui->comboBoxSecondBasePoint, &QComboBox::currentTextChanged, this, &DialogTrueDarts::PointNameChanged);
    connect(ui->comboBoxFirstDartPoint, &QComboBox::currentTextChanged, this, &DialogTrueDarts::PointNameChanged);
    connect(ui->comboBoxSecondDartPoint, &QComboBox::currentTextChanged, this, &DialogTrueDarts::PointNameChanged);
    connect(ui->comboBoxThirdDartPoint, &QComboBox::currentTextChanged, this, &DialogTrueDarts::PointNameChanged);

    vis = new VisToolTrueDarts(data);

    ui->tabWidget->setCurrentIndex(0);
    SetTabStopDistance(ui->plainTextEditToolNotes);
}

//---------------------------------------------------------------------------------------------------------------------
DialogTrueDarts::~DialogTrueDarts()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogTrueDarts::GetFirstNewDartPointName() -> QString
{
    return d1PointName;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogTrueDarts::GetSecondNewDartPointName() -> QString
{
    return d2PointName;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTrueDarts::SetNewDartPointNames(const QString &firstPoint, const QString &secondPoint)
{
    ui->lineEditFirstNewDartPoint->blockSignals(true);
    ui->lineEditSecondNewDartPoint->blockSignals(true);

    d1PointName = firstPoint;
    ui->lineEditFirstNewDartPoint->setText(d1PointName);

    d2PointName = secondPoint;
    ui->lineEditSecondNewDartPoint->setText(d2PointName);

    ui->lineEditSecondNewDartPoint->blockSignals(false);
    ui->lineEditFirstNewDartPoint->blockSignals(false);

    CheckName(ui->lineEditFirstNewDartPoint, ui->labelFirstNewDartPoint, d1PointName, d2PointName,
              ui->lineEditSecondNewDartPoint, flagName1);
    CheckName(ui->lineEditSecondNewDartPoint, ui->labelSecondNewDartPoint, d1PointName, d2PointName,
              ui->lineEditFirstNewDartPoint, flagName2);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogTrueDarts::GetFirstBasePointId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxFirstBasePoint);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTrueDarts::SetFirstBasePointId(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxFirstBasePoint, value, FillComboBox::NoChildren, ch1, ch2);

    VisToolTrueDarts *points = qobject_cast<VisToolTrueDarts *>(vis);
    SCASSERT(points != nullptr)
    points->SetBaseLineP1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogTrueDarts::GetSecondBasePointId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxSecondBasePoint);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTrueDarts::SetSecondBasePointId(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxSecondBasePoint, value, FillComboBox::NoChildren, ch1, ch2);

    VisToolTrueDarts *points = qobject_cast<VisToolTrueDarts *>(vis);
    SCASSERT(points != nullptr)
    points->SetBaseLineP2Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogTrueDarts::GetFirstDartPointId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxFirstDartPoint);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTrueDarts::SetFirstDartPointId(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxFirstDartPoint, value, FillComboBox::NoChildren, ch1, ch2);

    VisToolTrueDarts *points = qobject_cast<VisToolTrueDarts *>(vis);
    SCASSERT(points != nullptr)
    points->SetD1PointId(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogTrueDarts::GetSecondDartPointId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxSecondDartPoint);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTrueDarts::SetSecondDartPointId(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxSecondDartPoint, value, FillComboBox::NoChildren, ch1, ch2);

    VisToolTrueDarts *points = qobject_cast<VisToolTrueDarts *>(vis);
    SCASSERT(points != nullptr)
    points->SetD2PointId(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogTrueDarts::GetThirdDartPointId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxThirdDartPoint);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTrueDarts::SetThirdDartPointId(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxThirdDartPoint, value, FillComboBox::NoChildren, ch1, ch2);

    VisToolTrueDarts *points = qobject_cast<VisToolTrueDarts *>(vis);
    SCASSERT(points != nullptr)
    points->SetD3PointId(value);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTrueDarts::SetChildrenId(const quint32 &ch1, const quint32 &ch2)
{
    this->ch1 = ch1;
    this->ch2 = ch2;
    FillComboBoxs(ch1, ch2);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTrueDarts::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare == false) // After first choose we ignore all objects
    {
        if (type == SceneObject::Point)
        {
            VisToolTrueDarts *points = qobject_cast<VisToolTrueDarts *>(vis);
            SCASSERT(points != nullptr)

            switch (number)
            {
                case 0:
                    if (SetObject(id, ui->comboBoxFirstBasePoint, tr("Select the second base point")))
                    {
                        number++;
                        points->VisualMode(id);
                    }
                    break;
                case 1:
                    if (getCurrentObjectId(ui->comboBoxFirstBasePoint) != id)
                    {
                        if (SetObject(id, ui->comboBoxSecondBasePoint, tr("Select the first dart point")))
                        {
                            number++;
                            points->SetBaseLineP2Id(id);
                            points->RefreshGeometry();
                        }
                    }
                    break;
                case 2:
                {
                    QSet<quint32> set;
                    set.insert(getCurrentObjectId(ui->comboBoxFirstBasePoint));
                    set.insert(getCurrentObjectId(ui->comboBoxSecondBasePoint));
                    set.insert(id);

                    if (set.size() == 3)
                    {
                        if (SetObject(id, ui->comboBoxFirstDartPoint, tr("Select the second dart point")))
                        {
                            number++;
                            points->SetD1PointId(id);
                            points->RefreshGeometry();
                        }
                    }
                    break;
                }
                case 3:
                {
                    QSet<quint32> set;
                    set.insert(getCurrentObjectId(ui->comboBoxFirstBasePoint));
                    set.insert(getCurrentObjectId(ui->comboBoxSecondBasePoint));
                    set.insert(getCurrentObjectId(ui->comboBoxFirstDartPoint));
                    set.insert(id);

                    if (set.size() == 4)
                    {
                        if (SetObject(id, ui->comboBoxSecondDartPoint, tr("Select the third dart point")))
                        {
                            number++;
                            points->SetD2PointId(id);
                            points->RefreshGeometry();
                        }
                    }
                    break;
                }
                case 4:
                {
                    QSet<quint32> set;
                    set.insert(getCurrentObjectId(ui->comboBoxFirstBasePoint));
                    set.insert(getCurrentObjectId(ui->comboBoxSecondBasePoint));
                    set.insert(getCurrentObjectId(ui->comboBoxFirstDartPoint));
                    set.insert(getCurrentObjectId(ui->comboBoxSecondDartPoint));
                    set.insert(id);

                    if (set.size() == 5)
                    {
                        if (SetObject(id, ui->comboBoxThirdDartPoint, QString()))
                        {
                            points->SetD3PointId(id);
                            points->RefreshGeometry();
                            prepare = true;
                            DialogAccepted();
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTrueDarts::PointNameChanged()
{
    QSet<quint32> set;
    set.insert(getCurrentObjectId(ui->comboBoxFirstBasePoint));
    set.insert(getCurrentObjectId(ui->comboBoxSecondBasePoint));
    set.insert(getCurrentObjectId(ui->comboBoxFirstDartPoint));
    set.insert(getCurrentObjectId(ui->comboBoxSecondDartPoint));
    set.insert(getCurrentObjectId(ui->comboBoxThirdDartPoint));

    QColor color;
    if (set.size() != 5)
    {
        flagError = false;
        color = errorColor;
    }
    else
    {
        flagError = true;
        color = OkColor(this);
    }
    ChangeColor(ui->labelFirstBasePoint, color);
    ChangeColor(ui->labelSecondBasePoint, color);
    ChangeColor(ui->labelFirstDartPoint, color);
    ChangeColor(ui->labelSecondDartPoint, color);
    ChangeColor(ui->labelThirdDartPoint, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTrueDarts::NameDartPoint1Changed()
{
    NameChanged(ui->labelFirstNewDartPoint, d1PointName, d2PointName, ui->lineEditSecondNewDartPoint, flagName1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTrueDarts::NameDartPoint2Changed()
{
    NameChanged(ui->labelSecondNewDartPoint, d1PointName, d2PointName, ui->lineEditFirstNewDartPoint, flagName2);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTrueDarts::ShowVisualization()
{
    AddVisualization<VisToolTrueDarts>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTrueDarts::SaveData()
{
    d1PointName = ui->lineEditFirstNewDartPoint->text();
    d2PointName = ui->lineEditSecondNewDartPoint->text();

    VisToolTrueDarts *points = qobject_cast<VisToolTrueDarts *>(vis);
    SCASSERT(points != nullptr)

    points->SetBaseLineP1Id(GetFirstBasePointId());
    points->SetBaseLineP2Id(GetSecondBasePointId());
    points->SetD1PointId(GetFirstDartPointId());
    points->SetD2PointId(GetSecondDartPointId());
    points->SetD3PointId(GetThirdDartPointId());
    points->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTrueDarts::NameChanged(QLabel *labelEditNamePoint, const QString &pointD1Name, const QString &pointD2Name,
                                  QLineEdit *secondPointName, bool &flagName)
{
    SCASSERT(labelEditNamePoint != nullptr)
    SCASSERT(secondPointName != nullptr)
    QLineEdit *edit = qobject_cast<QLineEdit *>(sender());
    if (edit)
    {
        CheckName(edit, labelEditNamePoint, pointD1Name, pointD2Name, secondPointName, flagName);
    }
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTrueDarts::FillComboBoxs(const quint32 &ch1, const quint32 &ch2)
{
    FillComboBoxPoints(ui->comboBoxFirstBasePoint, FillComboBox::NoChildren, ch1, ch2);
    FillComboBoxPoints(ui->comboBoxSecondBasePoint, FillComboBox::NoChildren, ch1, ch2);
    FillComboBoxPoints(ui->comboBoxFirstDartPoint, FillComboBox::NoChildren, ch1, ch2);
    FillComboBoxPoints(ui->comboBoxSecondDartPoint, FillComboBox::NoChildren, ch1, ch2);
    FillComboBoxPoints(ui->comboBoxThirdDartPoint, FillComboBox::NoChildren, ch1, ch2);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTrueDarts::CheckName(QLineEdit *edit, QLabel *labelEditNamePoint, const QString &pointD1Name,
                                const QString &pointD2Name, QLineEdit *secondPointName, bool &flagName)
{
    SCASSERT(labelEditNamePoint != nullptr)
    SCASSERT(secondPointName != nullptr)
    SCASSERT(edit != nullptr)

    const QString name = edit->text();
    const QString secondName = secondPointName->text();
    QRegularExpression rx(NameRegExp());
    if (name.isEmpty() || secondName == name ||
        (pointD1Name != name && pointD2Name != name && data->IsUnique(name) == false) ||
        rx.match(name).hasMatch() == false)
    {
        flagName = false;
        ChangeColor(labelEditNamePoint, errorColor);
    }
    else
    {
        flagName = true;
        ChangeColor(labelEditNamePoint, OkColor(this));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTrueDarts::SetNotes(const QString &notes)
{
    ui->plainTextEditToolNotes->setPlainText(notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogTrueDarts::GetNotes() const -> QString
{
    return ui->plainTextEditToolNotes->toPlainText();
}
