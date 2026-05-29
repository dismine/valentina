/************************************************************************
 **
 **  @file   dialogflippingbyaxis.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 9, 2016
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

#include "dialogflippingbyaxis.h"

#include <QAbstractButton>
#include <QColor>
#include <QComboBox>
#include <QCompleter>
#include <QDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPointF>
#include <QPointer>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSharedPointer>
#include <QStringList>
#include <QToolButton>

#include "../../visualization/line/operation/vistoolflippingbyaxis.h"
#include "../../visualization/visualization.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../qmuparser/qmudef.h"
#include "../vmisc/vvalentinasettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vlineedit.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "../vpropertyexplorer/qtcolorpicker.h"
#include "ui_dialogflippingbyaxis.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
DialogFlippingByAxis::DialogFlippingByAxis(const VContainer *data,
                                           VAbstractPattern *doc,
                                           quint32 toolId,
                                           QWidget *parent)
  : DialogOperationTool(data, doc, toolId, parent),
    ui(new Ui::DialogFlippingByAxis)
{
    ui->setupUi(this);

    InitOkCancelApply(ui);

    FillComboBoxObjectTypes(ui->comboBoxObjectType);

    MakeFilterableComboBox(ui->comboBoxNewObject);

    FillComboBoxPoints(ui->comboBoxOriginPoint);
    FillComboBoxAxisType(ui->comboBoxAxisType);
    FillComboBoxTypeLine(ui->comboBoxPenStyle,
                         OperationLineStylesPics(ui->comboBoxPenStyle->palette().color(QPalette::Base),
                                                 ui->comboBoxPenStyle->palette().color(QPalette::Text)),
                         TypeLineDefault);
    InitOperationColorPicker(ui->pushButtonColor,
                             VAbstractValApplication::VApp()->ValentinaSettings()->GetUserToolColors());
    ui->pushButtonColor->setUseNativeDialog(!VAbstractApplication::VApp()->Settings()->IsDontUseNativeDialog());

    ui->comboBoxOriginPoint->setCurrentIndex(-1);

    connect(ui->lineEditVisibilityGroup, &QLineEdit::textChanged, this,
            &DialogOperationTool::GroupNameChanged);
    connect(ui->comboBoxOriginPoint, &QComboBox::currentTextChanged, this, &DialogFlippingByAxis::PointChanged);

    connect(ui->listWidget, &QListWidget::currentRowChanged, this, &DialogOperationTool::ShowSourceDetails);
    connect(ui->listWidget, &QListWidget::currentRowChanged, this, &DialogOperationTool::CurrentObjectChanged);
    connect(ui->lineEditName, &QLineEdit::textEdited, this, &DialogOperationTool::NameChanged);
    connect(ui->comboBoxPenStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogOperationTool::PenStyleChanged);
    connect(ui->pushButtonColor, &VPE::QtColorPicker::colorChanged, this, &DialogOperationTool::ColorChanged);
    connect(ui->toolButtonBulkRename, &QToolButton::clicked, this, &DialogOperationTool::BulkRename);

    connect(ui->comboBoxObjectType,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DialogOperationTool::ObjectTypeChanged);
    connect(ui->comboBoxNewObject,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DialogOperationTool::NewObjectChanged);
    connect(ui->toolButtonNewObject, &QPushButton::clicked, this, &DialogOperationTool::AddNewObject);
    connect(ui->toolButtonRemoveObject, &QPushButton::clicked, this, &DialogOperationTool::RemoveObject);

    ObjectTypeChanged(ui->comboBoxObjectType->currentIndex());

    vis = new VisToolFlippingByAxis(&this->data);

    ui->tabWidget->setCurrentIndex(0);
    SetTabStopDistance(ui->plainTextEditToolNotes);
}

//---------------------------------------------------------------------------------------------------------------------
DialogFlippingByAxis::~DialogFlippingByAxis()
{
    VAbstractValApplication::VApp()->ValentinaSettings()->SetUserToolColors(ui->pushButtonColor->CustomColors());
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByAxis::SourceListWidget() const -> QListWidget *
{
    return ui->listWidget;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByAxis::NameLineEdit() const -> QLineEdit *
{
    return ui->lineEditName;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByAxis::LabelName() const -> QLabel *
{
    return ui->labelName;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByAxis::LabelStatus() const -> QLabel *
{
    return ui->labelStatus;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByAxis::LabelGroupName() const -> QLabel *
{
    return ui->labelGroupName;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByAxis::PenStyleComboBox() const -> QComboBox *
{
    return ui->comboBoxPenStyle;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByAxis::ColorButton() const -> VPE::QtColorPicker *
{
    return ui->pushButtonColor;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByAxis::ObjectTypeComboBox() const -> QComboBox *
{
    return ui->comboBoxObjectType;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByAxis::NewObjectComboBox() const -> QComboBox *
{
    return ui->comboBoxNewObject;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByAxis::AddObjectButton() const -> QAbstractButton *
{
    return ui->toolButtonNewObject;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByAxis::RemoveObjectButton() const -> QAbstractButton *
{
    return ui->toolButtonRemoveObject;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByAxis::NotesPlainTextEdit() const -> QPlainTextEdit *
{
    return ui->plainTextEditToolNotes;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByAxis::GroupTagsLineEdit() const -> VCompleterLineEdit *
{
    return ui->lineEditGroupTags;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByAxis::VisibilityGroupLineEdit() const -> QLineEdit *
{
    return ui->lineEditVisibilityGroup;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByAxis::VisibilityGroupBox() const -> QGroupBox *
{
    return ui->groupBoxVisibilityGroup;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByAxis::GetOriginPointId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxOriginPoint);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByAxis::SetOriginPointId(quint32 value)
{
    ChangeCurrentData(ui->comboBoxOriginPoint, value);
    auto *operation = qobject_cast<VisToolFlippingByAxis *>(vis);
    SCASSERT(operation != nullptr)
    operation->SetOriginPointId(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByAxis::GetAxisType() const -> AxisType
{
    return getCurrentCrossPoint<AxisType>(ui->comboBoxAxisType);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByAxis::SetAxisType(AxisType type)
{
    auto index = ui->comboBoxAxisType->findData(static_cast<int>(type));
    if (index != -1)
    {
        ui->comboBoxAxisType->setCurrentIndex(index);

        auto *operation = qobject_cast<VisToolFlippingByAxis *>(vis);
        SCASSERT(operation != nullptr)
        operation->SetAxisType(type);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByAxis::ShowDialog(bool click)
{
    if (stage1 && not click)
    {
        if (m_sourceObjects.isEmpty())
        {
            return;
        }

        stage1 = false;

        auto *scene = qobject_cast<VMainGraphicsScene *>(VAbstractValApplication::VApp()->getCurrentScene());
        SCASSERT(scene != nullptr)
        scene->clearSelection();

        auto *operation = qobject_cast<VisToolFlippingByAxis *>(vis);
        SCASSERT(operation != nullptr)
        operation->StartAction();

        scene->ToggleArcSegmentVisibility(false);
        scene->ToggleElArcSegmentVisibility(false);
        scene->ToggleSplineSegmentVisibility(false);
        scene->ToggleSplinePathSegmentVisibility(false);

        scene->ToggleArcHover(false);
        scene->ToggleElArcHover(false);
        scene->ToggleSplineHover(false);
        scene->ToggleSplinePathHover(false);

        VAbstractValApplication::VApp()->getSceneView()->AllowRubberBand(false);

        FillDefSourceNames(m_sourceObjects, &data, "fa"_L1);
        FillSourceList();

        emit ToolTip(tr("Select origin point"));
    }
    else if (not stage1 && prepare && click)
    {
        CheckState();
        setModal(true);
        emit ToolTip(QString());
        show();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByAxis::ChosenObject(quint32 id, const SceneObject &type)
{
    if (stage1)
    {
        if (auto obj = std::find_if(m_sourceObjects.begin(),
                                    m_sourceObjects.end(),
                                    [id](const SourceItem &sItem) -> bool { return sItem.id == id; });
            obj == m_sourceObjects.end())
        {
            m_sourceObjects.push_back(SourceItem{id});
        }
        else
        {
            m_sourceObjects.erase(obj);
        }

        auto *operation = qobject_cast<VisToolFlippingByAxis *>(vis);
        SCASSERT(operation != nullptr)
        operation->SetObjects(SourceToObjects(m_sourceObjects));
        if (!VAbstractValApplication::VApp()->getCurrentScene()->items().contains(operation))
        {
            operation->VisualMode(NULL_ID);
        }
        else
        {
            operation->RefreshGeometry();
        }
    }
    else if (!prepare && type == SceneObject::Point) // After first choose we ignore all objects
    {
        auto obj = std::find_if(m_sourceObjects.begin(),
                                m_sourceObjects.end(),
                                [id](const SourceItem &sItem) -> bool { return sItem.id == id; });

        if (obj != m_sourceObjects.end())
        {
            emit ToolTip(tr("Select origin point that is not part of the list of objects"));
            return;
        }

        if (SetObject(id, ui->comboBoxOriginPoint, QString()))
        {
            auto *operation = qobject_cast<VisToolFlippingByAxis *>(vis);
            SCASSERT(operation != nullptr)
            operation->SetOriginPointId(id);
            operation->RefreshGeometry();

            prepare = true;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByAxis::OnSourceObjectsSet()
{
    auto *operation = qobject_cast<VisToolFlippingByAxis *>(vis);
    SCASSERT(operation != nullptr)
    operation->SetObjects(SourceToObjects(m_sourceObjects));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByAxis::ShowVisualization()
{
    AddVisualization<VisToolFlippingByAxis>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByAxis::SaveData()
{
    m_sourceObjects = SaveSourceObjects();

    if (!vis.isNull())
    {
        auto *operation = qobject_cast<VisToolFlippingByAxis *>(vis);
        SCASSERT(operation != nullptr)

        operation->SetObjects(SourceToObjects(m_sourceObjects));
        operation->SetOriginPointId(GetOriginPointId());
        operation->SetAxisType(GetAxisType());
        operation->RefreshGeometry();
    }

    QStringList groupTags = ui->lineEditGroupTags->text().split(',');
    for (auto &tag : groupTags)
    {
        tag = tag.trimmed();
        if (not m_groupTags.contains(tag))
        {
            m_groupTags.append(tag);
        }
    }

    ui->lineEditGroupTags->SetCompletion(m_groupTags);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByAxis::PointChanged()
{
    QColor color;
    quint32 const id = getCurrentObjectId(ui->comboBoxOriginPoint);
    auto obj = std::find_if(m_sourceObjects.begin(),
                            m_sourceObjects.end(),
                            [id](const SourceItem &sItem) -> bool { return sItem.id == id; });
    if (obj != m_sourceObjects.end())
    {
        flagError = false;
        ui->labelStatus->setText(tr("Invalid point"));
        color = errorColor;
    }
    else
    {
        flagError = true;
        color = OkColor(this);
    }
    ChangeColor(ui->labelOriginPoint, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByAxis::FillComboBoxAxisType(QComboBox *box)
{
    SCASSERT(box != nullptr)

    box->addItem(tr("Vertical axis"), QVariant(static_cast<int>(AxisType::VerticalAxis)));
    box->addItem(tr("Horizontal axis"), QVariant(static_cast<int>(AxisType::HorizontalAxis)));
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByAxis::IsValid() const -> bool
{
    bool const ready = flagError && flagName && flagGroupName;

    if (ready)
    {
        ui->labelStatus->setText(tr("Ready"));
    }

    return ready;
}
