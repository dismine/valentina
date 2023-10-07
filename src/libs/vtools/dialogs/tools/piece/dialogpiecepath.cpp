/************************************************************************
 **
 **  @file   dialogpiecepath.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 11, 2016
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

#include "dialogpiecepath.h"
#include "../../../tools/vtoolseamallowance.h"
#include "../../../visualization/path/vistoolpiecepath.h"
#include "../../support/dialogeditwrongformula.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vmodifierkey.h"
#include "../vpatterndb/variables/vincrement.h"
#include "../vpatterndb/vpiecenode.h"
#include "ui_dialogpiecepath.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#include "../vmisc/backport/qoverload.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 7, 0)

#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
#include "../vmisc/backport/qscopeguard.h"
#else
#include <QScopeGuard>
#endif

#include <QMenu>
#include <QTimer>

namespace
{
enum class ContextMenuOption : int
{
    NoSelection,
    Reverse,
    NonePassmark,
    OneLine,
    TwoLines,
    ThreeLines,
    TMark,
    ExternalVMark,
    InternalVMark,
    UMark,
    BoxMark,
    CheckMark,
    Uniqueness,
    TurnPoint,
    Excluded,
    Delete,
    LAST_ONE_DO_NOT_USE
};

auto CuttingPath(quint32 id, const VContainer *data) -> QVector<QPointF>
{
    QVector<QPointF> path;
    const quint32 pieceId = data->GetPieceForPiecePath(id);
    if (pieceId > NULL_ID)
    {
        path = data->GetPiece(pieceId).CuttingPathPoints(data);
    }

    return path;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
DialogPiecePath::DialogPiecePath(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent)
  : DialogTool(data, doc, toolId, parent),
    ui(new Ui::DialogPiecePath),
    m_timerWidth(new QTimer(this)),
    m_timerWidthBefore(new QTimer(this)),
    m_timerWidthAfter(new QTimer(this)),
    m_timerVisible(new QTimer(this)),
    m_timerPassmarkLength(new QTimer(this)),
    m_timerPassmarkWidth(new QTimer(this)),
    m_timerPassmarkAngle(new QTimer(this))
{
    ui->setupUi(this);
    InitOkCancel(ui);

    InitIcons();

    InitPathTab();
    InitSeamAllowanceTab();
    InitPassmarksTab();
    InitControlTab();

    EvalVisible();

    m_flagError = PathIsValid();

    vis = new VisToolPiecePath(data);

    ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabSeamAllowance));
    ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabPassmarks));

    connect(ui->comboBoxPiece, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this]() { ValidObjects(PathIsValid()); });
}

//---------------------------------------------------------------------------------------------------------------------
DialogPiecePath::~DialogPiecePath()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::EnbleShowMode(bool disable)
{
    m_showMode = disable;
    ui->comboBoxType->setDisabled(m_showMode);
    ui->comboBoxPiece->setDisabled(m_showMode);
    RefreshPathList(GetPiecePath());
    ValidObjects(PathIsValid());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::HideVisibilityTrigger()
{
    ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabControl));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::ChosenObject(quint32 id, const SceneObject &type)
{
    if (not prepare)
    {
        const bool reverse = QGuiApplication::keyboardModifiers() == Qt::ShiftModifier;
        if (id != GetLastId())
        {
            switch (type)
            {
                case SceneObject::Arc:
                    NewItem(VPieceNode(id, Tool::NodeArc, reverse));
                    break;
                case SceneObject::ElArc:
                    NewItem(VPieceNode(id, Tool::NodeElArc, reverse));
                    break;
                case SceneObject::Point:
                    NewItem(VPieceNode(id, Tool::NodePoint));
                    break;
                case SceneObject::Spline:
                    NewItem(VPieceNode(id, Tool::NodeSpline, reverse));
                    break;
                case SceneObject::SplinePath:
                    NewItem(VPieceNode(id, Tool::NodeSplinePath, reverse));
                    break;
                case (SceneObject::Line):
                case (SceneObject::Detail):
                case (SceneObject::Unknown):
                default:
                    qDebug() << "Got wrong scene object. Ignore.";
                    break;
            }
        }
        else
        {
            if (ui->listWidget->count() > 1)
            {
                delete GetItemById(id);
            }
        }

        ValidObjects(PathIsValid());

        if (not m_showMode)
        {
            auto *visPath = qobject_cast<VisToolPiecePath *>(vis);
            SCASSERT(visPath != nullptr);
            const VPiecePath p = CreatePath();
            visPath->SetPath(p);
            visPath->SetCuttingPath(CuttingPath(toolId, data));

            if (p.CountNodes() == 1)
            {
                emit ToolTip(tr("Select main path objects, <b>%1</b> - reverse direction curve, "
                                "<b>%2</b> - finish creation")
                                 .arg(VModifierKey::Shift(), VModifierKey::EnterKey()));

                if (not VAbstractValApplication::VApp()->getCurrentScene()->items().contains(visPath))
                {
                    visPath->VisualMode(NULL_ID);
                }
                else
                {
                    visPath->RefreshGeometry();
                }
            }
            else
            {
                visPath->RefreshGeometry();
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::ShowDialog(bool click)
{
    if (not click)
    {
        if (CreatePath().CountNodes() > 0)
        {
            emit ToolTip(QString());
            prepare = true;

            if (not m_showMode)
            {
                auto *visPath = qobject_cast<VisToolPiecePath *>(vis);
                SCASSERT(visPath != nullptr);
                visPath->SetMode(Mode::Show);
                visPath->RefreshGeometry();
            }
            setModal(true);
            show();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::CheckState()
{
    SCASSERT(bOk != nullptr);
    if (GetType() == PiecePathType::InternalPath)
    {
        m_flagFormula = true;
        m_flagFormulaBefore = true;
        m_flagFormulaAfter = true;
        m_flagFormulaPassmarkLength = true;
    }
    else
    {
        m_flagFormulaVisible = true; // Works only for internal paths
        if (not m_showMode)
        {
            m_flagFormula = true;
            m_flagFormulaBefore = true;
            m_flagFormulaAfter = true;
            m_flagFormulaPassmarkLength = true;
        }
    }

    bOk->setEnabled(IsValid());

    const int tabSeamAllowanceIndex = ui->tabWidget->indexOf(ui->tabSeamAllowance);
    if (m_flagFormula && m_flagFormulaBefore && m_flagFormulaAfter)
    {
        ui->tabWidget->setTabIcon(tabSeamAllowanceIndex, QIcon());
    }
    else
    {
        const QIcon icon = QIcon::fromTheme(QStringLiteral("dialog-warning"));
        ui->tabWidget->setTabIcon(tabSeamAllowanceIndex, icon);
    }

    ui->comboBoxNodes->setEnabled(m_flagFormulaBefore && m_flagFormulaAfter);

    const int tabControlIndex = ui->tabWidget->indexOf(ui->tabControl);
    if (m_flagFormulaVisible)
    {
        ui->tabWidget->setTabIcon(tabControlIndex, QIcon());
    }
    else
    {
        const QIcon icon = QIcon::fromTheme(QStringLiteral("dialog-warning"));
        ui->tabWidget->setTabIcon(tabControlIndex, icon);
    }

    if (ui->comboBoxPassmarks->count() == 0)
    {
        m_flagFormulaPassmarkLength = true;
    }

    const int tabPassmarksIndex = ui->tabWidget->indexOf(ui->tabPassmarks);
    if (m_flagFormulaPassmarkLength && m_flagFormulaPassmarkWidth && m_flagFormulaPassmarkAngle)
    {
        ui->tabWidget->setTabIcon(tabPassmarksIndex, QIcon());
    }
    else
    {
        const QIcon icon = QIcon::fromTheme(QStringLiteral("dialog-warning"));
        ui->tabWidget->setTabIcon(tabPassmarksIndex, icon);
    }

    ui->comboBoxPassmarks->setEnabled(m_flagFormulaPassmarkLength && m_flagFormulaPassmarkWidth &&
                                      m_flagFormulaPassmarkAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::ShowVisualization()
{
    AddVisualization<VisToolPiecePath>();

    if (m_showMode)
    {
        auto *tool = qobject_cast<VToolSeamAllowance *>(VAbstractPattern::getTool(GetPieceId()));
        SCASSERT(tool != nullptr);
        auto *visPath = qobject_cast<VisToolPiecePath *>(vis);
        SCASSERT(visPath != nullptr);
        visPath->setParentItem(tool);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditFormulaWidth->blockSignals(true);
    ui->plainTextEditFormulaWidthBefore->blockSignals(true);
    ui->plainTextEditFormulaWidthAfter->blockSignals(true);
    ui->plainTextEditFormulaVisible->blockSignals(true);
    ui->plainTextEditPassmarkLength->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }

    if (event->type() == QEvent::PaletteChange)
    {
        InitIcons();
        InitDialogButtonBoxIcons(ui->buttonBox);
    }

    // remember to call base class implementation
    DialogTool::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::ShowContextMenu(const QPoint &pos)
{
    const int row = ui->listWidget->currentRow();
    if (ui->listWidget->count() == 0 || row == -1 || row >= ui->listWidget->count())
    {
        return;
    }

    QListWidgetItem *rowItem = ui->listWidget->item(row);
    SCASSERT(rowItem != nullptr);
    auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

    QMenu menu;
    QHash<int, QAction *> contextMenu = InitContextMenu(&menu, rowNode);

    QAction *selectedAction = menu.exec(ui->listWidget->viewport()->mapToGlobal(pos));
    auto selectedOption = static_cast<ContextMenuOption>(
        contextMenu.key(selectedAction, static_cast<int>(ContextMenuOption::NoSelection)));

    auto SelectPassmarkLineType = [this, &rowNode, rowItem](PassmarkLineType type)
    {
        rowNode.SetPassmark(true);
        rowNode.SetPassmarkLineType(type);
        rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        rowItem->setText(GetNodeName(data, rowNode, IsShowNotch()));
    };

    Q_STATIC_ASSERT_X(static_cast<int>(ContextMenuOption::LAST_ONE_DO_NOT_USE) == 16, "Not all options were handled.");

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wswitch-default")
    switch (selectedOption)
    {
        case ContextMenuOption::NoSelection:
            return;
        case ContextMenuOption::Reverse:
            rowNode.SetReverse(not rowNode.GetReverse());
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
            rowItem->setText(GetNodeName(data, rowNode, IsShowNotch()));
            break;
        case ContextMenuOption::NonePassmark:
            rowNode.SetPassmark(false);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
            rowItem->setText(GetNodeName(data, rowNode, IsShowNotch()));
            break;
        case ContextMenuOption::OneLine:
            SelectPassmarkLineType(PassmarkLineType::OneLine);
            break;
        case ContextMenuOption::TwoLines:
            SelectPassmarkLineType(PassmarkLineType::TwoLines);
            break;
        case ContextMenuOption::ThreeLines:
            SelectPassmarkLineType(PassmarkLineType::ThreeLines);
            break;
        case ContextMenuOption::TMark:
            SelectPassmarkLineType(PassmarkLineType::TMark);
            break;
        case ContextMenuOption::ExternalVMark:
            SelectPassmarkLineType(PassmarkLineType::ExternalVMark);
            break;
        case ContextMenuOption::InternalVMark:
            SelectPassmarkLineType(PassmarkLineType::InternalVMark);
            break;
        case ContextMenuOption::UMark:
            SelectPassmarkLineType(PassmarkLineType::UMark);
            break;
        case ContextMenuOption::BoxMark:
            SelectPassmarkLineType(PassmarkLineType::BoxMark);
            break;
        case ContextMenuOption::CheckMark:
            SelectPassmarkLineType(PassmarkLineType::CheckMark);
            break;
        case ContextMenuOption::Uniqueness:
            rowNode.SetCheckUniqueness(not rowNode.IsCheckUniqueness());
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
            rowItem->setText(GetNodeName(data, rowNode, IsShowNotch()));
            break;
        case ContextMenuOption::TurnPoint:
            rowNode.SetTurnPoint(not rowNode.IsTurnPoint());
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
            rowItem->setText(GetNodeName(data, rowNode, IsShowNotch()));
            break;
        case ContextMenuOption::Excluded:
            rowNode.SetExcluded(not rowNode.IsExcluded());
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
            rowItem->setText(GetNodeName(data, rowNode, true));
            rowItem->setFont(NodeFont(rowItem->font(), rowNode.IsExcluded()));
            break;
        case ContextMenuOption::Delete:
            delete ui->listWidget->item(row);
            break;
        case ContextMenuOption::LAST_ONE_DO_NOT_USE:
            Q_UNREACHABLE();
            break;
    };
    QT_WARNING_POP

    ValidObjects(PathIsValid());
    ListChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::ListChanged()
{
    if (not m_showMode)
    {
        auto *visPath = qobject_cast<VisToolPiecePath *>(vis);
        SCASSERT(visPath != nullptr);
        visPath->SetPath(CreatePath());
        visPath->SetCuttingPath(CuttingPath(toolId, data));
        visPath->RefreshGeometry();
    }

    InitPassmarksList();
    InitNodesList();
    SetMoveControls();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::NameChanged()
{
    if (ui->lineEditName->text().isEmpty())
    {
        m_flagName = false;
        ChangeColor(ui->labelName, errorColor);
    }
    else
    {
        m_flagName = true;
        ChangeColor(ui->labelName, OkColor(this));
    }
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::NodeChanged(int index)
{
    ui->plainTextEditFormulaWidthBefore->setDisabled(true);
    ui->toolButtonExprBefore->setDisabled(true);
    ui->pushButtonDefBefore->setDisabled(true);

    ui->plainTextEditFormulaWidthAfter->setDisabled(true);
    ui->toolButtonExprAfter->setDisabled(true);
    ui->pushButtonDefAfter->setDisabled(true);

    ui->comboBoxAngle->setDisabled(true);

    ui->comboBoxAngle->blockSignals(true);

    if (index != -1)
    {
        const VPiecePath path = CreatePath();
        const int nodeIndex = path.indexOfNode(ui->comboBoxNodes->currentData().toUInt());
        if (nodeIndex != -1)
        {
            const VPieceNode &node = path.at(nodeIndex);

            // Seam alowance before
            ui->plainTextEditFormulaWidthBefore->setEnabled(true);
            ui->toolButtonExprBefore->setEnabled(true);

            QString w1Formula = node.GetFormulaSABefore();
            if (w1Formula != currentSeamAllowance)
            {
                ui->pushButtonDefBefore->setEnabled(true);
            }
            if (w1Formula.length() > 80) // increase height if needed.
            {
                this->DeployWidthBeforeFormulaTextEdit();
            }
            w1Formula = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
                w1Formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
            ui->plainTextEditFormulaWidthBefore->setPlainText(w1Formula);
            MoveCursorToEnd(ui->plainTextEditFormulaWidthBefore);

            // Seam alowance after
            ui->plainTextEditFormulaWidthAfter->setEnabled(true);
            ui->toolButtonExprAfter->setEnabled(true);

            QString w2Formula = node.GetFormulaSAAfter();
            if (w2Formula != currentSeamAllowance)
            {
                ui->pushButtonDefAfter->setEnabled(true);
            }
            if (w2Formula.length() > 80) // increase height if needed.
            {
                this->DeployWidthAfterFormulaTextEdit();
            }
            w2Formula = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
                w2Formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
            ui->plainTextEditFormulaWidthAfter->setPlainText(w2Formula);
            MoveCursorToEnd(ui->plainTextEditFormulaWidthAfter);

            // Angle type
            ui->comboBoxAngle->setEnabled(true);
            const int index = ui->comboBoxAngle->findData(static_cast<unsigned char>(node.GetAngleType()));
            if (index != -1)
            {
                ui->comboBoxAngle->setCurrentIndex(index);
            }
        }
    }
    else
    {
        ui->plainTextEditFormulaWidthBefore->setPlainText(QString());
        ui->plainTextEditFormulaWidthAfter->setPlainText(QString());
        ui->comboBoxAngle->setCurrentIndex(-1);
    }

    ui->comboBoxAngle->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::PassmarkChanged(int index)
{
    ui->groupBoxMarkType->setDisabled(true);
    ui->groupBoxAngleType->setDisabled(true);
    ui->groupBoxManualLength->setDisabled(true);
    ui->groupBoxManualWidth->setDisabled(true);
    ui->groupBoxManualAngle->setDisabled(true);

    ui->labelEditPassmarkLength->setDisabled(true);
    ui->labelEditPassmarkWidth->setDisabled(true);
    ui->labelEditPassmarkAngle->setDisabled(true);

    ui->checkBoxClockwiseOpening->setDisabled(true);
    ui->checkBoxShowSecondPassmark->setDisabled(true);

    ui->checkBoxClockwiseOpening->blockSignals(true);
    ui->checkBoxShowSecondPassmark->blockSignals(true);

    ui->groupBoxManualLength->blockSignals(true);
    ui->groupBoxManualWidth->blockSignals(true);
    ui->groupBoxManualAngle->blockSignals(true);
    ui->groupBoxMarkType->blockSignals(true);
    ui->groupBoxAngleType->blockSignals(true);

    ui->checkBoxClockwiseOpening->setChecked(false);

    ui->groupBoxManualLength->setChecked(false);
    ui->groupBoxManualWidth->setChecked(false);
    ui->groupBoxManualAngle->setChecked(false);

    auto EnableSignals = qScopeGuard(
        [this]
        {
            ui->checkBoxClockwiseOpening->blockSignals(false);
            ui->checkBoxShowSecondPassmark->blockSignals(false);
            ui->groupBoxManualLength->blockSignals(false);
            ui->groupBoxManualWidth->blockSignals(false);
            ui->groupBoxManualAngle->blockSignals(false);
            ui->groupBoxMarkType->blockSignals(false);
            ui->groupBoxAngleType->blockSignals(false);
        });

    if (index == -1)
    {
        return;
    }

    const VPiecePath path = CreatePath();
    const int nodeIndex = path.indexOfNode(ui->comboBoxPassmarks->currentData().toUInt());
    if (nodeIndex == -1)
    {
        return;
    }

    const VPieceNode &node = path.at(nodeIndex);

    InitPassmarkLengthFormula(node);
    InitPassmarkWidthFormula(node);
    InitPassmarkAngleFormula(node);
    InitPassmarkShapeType(node);
    InitPassmarkAngleType(node);

    if (node.GetPassmarkLineType() == PassmarkLineType::CheckMark)
    {
        ui->checkBoxClockwiseOpening->setEnabled(true);
        ui->checkBoxClockwiseOpening->setChecked(node.IsPassmarkClockwiseOpening());
    }

    // Show the second option
    ui->checkBoxShowSecondPassmark->setEnabled(true);
    ui->checkBoxShowSecondPassmark->setChecked(node.IsShowSecondPassmark());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::ReturnDefBefore()
{
    const QString def = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        currentSeamAllowance, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    ui->plainTextEditFormulaWidthBefore->setPlainText(def);
    if (auto *button = qobject_cast<QPushButton *>(sender()))
    {
        button->setEnabled(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::ReturnDefAfter()
{
    const QString def = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        currentSeamAllowance, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    ui->plainTextEditFormulaWidthAfter->setPlainText(def);
    if (auto *button = qobject_cast<QPushButton *>(sender()))
    {
        button->setEnabled(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::PassmarkLineTypeChanged(int id)
{
    const int i = ui->comboBoxPassmarks->currentIndex();
    if (i != -1)
    {
        QListWidgetItem *rowItem = GetItemById(ui->comboBoxPassmarks->currentData().toUInt());
        if (rowItem)
        {
            auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

            PassmarkLineType lineType = PassmarkLineType::OneLine;
            if (id == ui->buttonGroupMarkType->id(ui->radioButtonOneLine))
            {
                lineType = PassmarkLineType::OneLine;
            }
            else if (id == ui->buttonGroupMarkType->id(ui->radioButtonTwoLines))
            {
                lineType = PassmarkLineType::TwoLines;
            }
            else if (id == ui->buttonGroupMarkType->id(ui->radioButtonThreeLines))
            {
                lineType = PassmarkLineType::ThreeLines;
            }
            else if (id == ui->buttonGroupMarkType->id(ui->radioButtonTMark))
            {
                lineType = PassmarkLineType::TMark;
            }
            else if (id == ui->buttonGroupMarkType->id(ui->radioButtonVMark))
            {
                lineType = PassmarkLineType::ExternalVMark;
            }
            else if (id == ui->buttonGroupMarkType->id(ui->radioButtonVMark2))
            {
                lineType = PassmarkLineType::InternalVMark;
            }
            else if (id == ui->buttonGroupMarkType->id(ui->radioButtonUMark))
            {
                lineType = PassmarkLineType::UMark;
            }
            else if (id == ui->buttonGroupMarkType->id(ui->radioButtonBoxMark))
            {
                lineType = PassmarkLineType::BoxMark;
            }
            else if (id == ui->buttonGroupMarkType->id(ui->radioButtonCheckMark))
            {
                lineType = PassmarkLineType::CheckMark;
            }

            rowNode.SetPassmarkLineType(lineType);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
            rowItem->setText(GetNodeName(data, rowNode, IsShowNotch()));

            ListChanged();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::PassmarkAngleTypeChanged(int id)
{
    const int i = ui->comboBoxPassmarks->currentIndex();
    if (i != -1)
    {
        QListWidgetItem *rowItem = GetItemById(ui->comboBoxPassmarks->currentData().toUInt());
        if (rowItem)
        {
            auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

            PassmarkAngleType angleType = PassmarkAngleType::Straightforward;
            if (id == ui->buttonGroupAngleType->id(ui->radioButtonStraightforward))
            {
                angleType = PassmarkAngleType::Straightforward;
            }
            else if (id == ui->buttonGroupAngleType->id(ui->radioButtonBisector))
            {
                angleType = PassmarkAngleType::Bisector;
            }
            else if (id == ui->buttonGroupAngleType->id(ui->radioButtonIntersection))
            {
                angleType = PassmarkAngleType::Intersection;
            }
            else if (id == ui->buttonGroupAngleType->id(ui->radioButtonIntersectionOnlyLeft))
            {
                angleType = PassmarkAngleType::IntersectionOnlyLeft;
            }
            else if (id == ui->buttonGroupAngleType->id(ui->radioButtonIntersectionOnlyRight))
            {
                angleType = PassmarkAngleType::IntersectionOnlyRight;
            }
            else if (id == ui->buttonGroupAngleType->id(ui->radioButtonIntersection2))
            {
                angleType = PassmarkAngleType::Intersection2;
            }
            else if (id == ui->buttonGroupAngleType->id(ui->radioButtonIntersection2OnlyLeft))
            {
                angleType = PassmarkAngleType::Intersection2OnlyLeft;
            }
            else if (id == ui->buttonGroupAngleType->id(ui->radioButtonIntersection2OnlyRight))
            {
                angleType = PassmarkAngleType::Intersection2OnlyRight;
            }

            rowNode.SetPassmarkAngleType(angleType);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
            rowItem->setText(GetNodeName(data, rowNode, IsShowNotch()));

            ListChanged();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::PassmarkShowSecondChanged(int state)
{
    const int i = ui->comboBoxPassmarks->currentIndex();
    if (i != -1)
    {
        QListWidgetItem *rowItem = GetItemById(ui->comboBoxPassmarks->currentData().toUInt());
        if (rowItem)
        {
            auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetShowSecondPassmark(state);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

            ListChanged();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::PassmarkClockwiseOrientationChanged(int state)
{
    const int i = ui->comboBoxPassmarks->currentIndex();
    if (i != -1)
    {
        QListWidgetItem *rowItem = GetItemById(ui->comboBoxPassmarks->currentData().toUInt());
        if (rowItem)
        {
            auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetPassmarkClockwiseOpening(state);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

            ListChanged();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::EvalWidth()
{
    FormulaData formulaData;
    formulaData.formula = ui->plainTextEditFormulaWidth->toPlainText();
    formulaData.variables = data->DataVariables();
    formulaData.labelEditFormula = ui->labelEditWidth;
    formulaData.labelResult = ui->labelResultWidth;
    formulaData.postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
    formulaData.checkZero = false;
    formulaData.checkLessThanZero = true;

    m_saWidth = Eval(formulaData, m_flagFormula);

    if (m_saWidth >= 0)
    {
        auto *locData = const_cast<VContainer *>(data);

        auto *currentSA = new VIncrement(locData, currentSeamAllowance);
        currentSA->SetFormula(m_saWidth, QString().setNum(m_saWidth), true);
        currentSA->SetDescription(tr("Current seam allowance"));

        locData->AddVariable(currentSA);

        EvalWidthBefore();
        EvalWidthAfter();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::EvalWidthBefore()
{
    if (ui->comboBoxNodes->count() > 0)
    {
        FormulaData formulaData;
        formulaData.formula = ui->plainTextEditFormulaWidthBefore->toPlainText();
        formulaData.variables = data->DataVariables();
        formulaData.labelEditFormula = ui->labelEditBefore;
        formulaData.labelResult = ui->labelResultBefore;
        formulaData.postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
        formulaData.checkZero = false;
        formulaData.checkLessThanZero = true;

        const QString formula = GetFormulaSAWidthBefore();
        if (formula != currentSeamAllowance)
        {
            ui->pushButtonDefBefore->setEnabled(true);
        }

        Eval(formulaData, m_flagFormulaBefore);

        if (m_flagFormulaBefore)
        {
            UpdateNodeSABefore(formula);
        }
    }
    else
    {
        ChangeColor(ui->labelEditBefore, OkColor(this));
        ui->labelResultBefore->setText(tr("<Empty>"));
        m_flagFormulaBefore = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::EvalWidthAfter()
{
    if (ui->comboBoxNodes->count() > 0)
    {
        FormulaData formulaData;
        formulaData.formula = ui->plainTextEditFormulaWidthAfter->toPlainText();
        formulaData.variables = data->DataVariables();
        formulaData.labelEditFormula = ui->labelEditAfter;
        formulaData.labelResult = ui->labelResultAfter;
        formulaData.postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
        formulaData.checkZero = false;
        formulaData.checkLessThanZero = true;

        const QString formula = GetFormulaSAWidthAfter();
        if (formula != currentSeamAllowance)
        {
            ui->pushButtonDefAfter->setEnabled(true);
        }

        Eval(formulaData, m_flagFormulaAfter);

        if (m_flagFormulaAfter)
        {
            UpdateNodeSAAfter(formula);
        }
    }
    else
    {
        ChangeColor(ui->labelEditAfter, OkColor(this));
        ui->labelResultAfter->setText(tr("<Empty>"));
        m_flagFormulaAfter = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::EvalVisible()
{
    FormulaData formulaData;
    formulaData.formula = ui->plainTextEditFormulaVisible->toPlainText();
    formulaData.variables = data->DataVariables();
    formulaData.labelEditFormula = ui->labelEditVisible;
    formulaData.labelResult = ui->labelResultVisible;
    formulaData.postfix = QString();
    formulaData.checkZero = false;
    formulaData.checkLessThanZero = true;

    Eval(formulaData, m_flagFormulaVisible);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::EvalPassmarkLength()
{
    if (ui->groupBoxManualLength->isChecked())
    {
        if (ui->comboBoxPassmarks->count() > 0)
        {
            FormulaData formulaData;
            formulaData.formula = ui->plainTextEditPassmarkLength->toPlainText();
            formulaData.variables = data->DataVariables();
            formulaData.labelEditFormula = ui->labelEditPassmarkLength;
            formulaData.labelResult = ui->labelResultPassmarkLength;
            formulaData.postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
            formulaData.checkZero = true;
            formulaData.checkLessThanZero = true;

            Eval(formulaData, m_flagFormulaPassmarkLength);

            UpdateNodePassmarkLength(
                VTranslateVars::TryFormulaFromUser(ui->plainTextEditPassmarkLength->toPlainText(),
                                                   VAbstractApplication::VApp()->Settings()->GetOsSeparator()));
        }
        else
        {
            ChangeColor(ui->labelEditPassmarkLength, OkColor(this));
            ui->labelResultPassmarkLength->setText(tr("<Empty>"));
            m_flagFormulaPassmarkLength = true;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::EvalPassmarkWidth()
{
    if (ui->groupBoxManualWidth->isChecked())
    {
        if (ui->comboBoxPassmarks->count() > 0)
        {
            FormulaData formulaData;
            formulaData.formula = ui->plainTextEditPassmarkWidth->toPlainText();
            formulaData.variables = data->DataVariables();
            formulaData.labelEditFormula = ui->labelEditPassmarkWidth;
            formulaData.labelResult = ui->labelResultPassmarkWidth;
            formulaData.postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
            formulaData.checkZero = true;
            formulaData.checkLessThanZero = false;

            Eval(formulaData, m_flagFormulaPassmarkWidth);

            UpdateNodePassmarkWidth(
                VTranslateVars::TryFormulaFromUser(ui->plainTextEditPassmarkWidth->toPlainText(),
                                                   VAbstractApplication::VApp()->Settings()->GetOsSeparator()));
        }
        else
        {
            ChangeColor(ui->labelEditPassmarkWidth, OkColor(this));
            ui->labelResultPassmarkWidth->setText(tr("<Empty>"));
            m_flagFormulaPassmarkWidth = true;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::EvalPassmarkAngle()
{
    if (ui->groupBoxManualWidth->isChecked())
    {
        if (ui->comboBoxPassmarks->count() > 0)
        {
            FormulaData formulaData;
            formulaData.formula = ui->plainTextEditPassmarkAngle->toPlainText();
            formulaData.variables = data->DataVariables();
            formulaData.labelEditFormula = ui->labelEditPassmarkAngle;
            formulaData.labelResult = ui->labelResultPassmarkAngle;
            formulaData.postfix = degreeSymbol;
            formulaData.checkZero = false;
            formulaData.checkLessThanZero = false;

            Eval(formulaData, m_flagFormulaPassmarkAngle);

            UpdateNodePassmarkAngle(
                VTranslateVars::TryFormulaFromUser(ui->plainTextEditPassmarkAngle->toPlainText(),
                                                   VAbstractApplication::VApp()->Settings()->GetOsSeparator()));
        }
        else
        {
            ChangeColor(ui->labelEditPassmarkAngle, OkColor(this));
            ui->labelResultPassmarkAngle->setText(tr("<Empty>"));
            m_flagFormulaPassmarkAngle = true;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::FXWidth()
{
    QScopedPointer<DialogEditWrongFormula> dialog(new DialogEditWrongFormula(data, toolId, this));
    dialog->setWindowTitle(tr("Edit seam allowance width"));
    dialog->SetFormula(GetFormulaSAWidth());
    dialog->setCheckLessThanZero(true);
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetFormulaSAWidth(dialog->GetFormula());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::FXWidthBefore()
{
    QScopedPointer<DialogEditWrongFormula> dialog(new DialogEditWrongFormula(data, toolId, this));
    dialog->setWindowTitle(tr("Edit seam allowance width before"));
    dialog->SetFormula(GetFormulaSAWidthBefore());
    dialog->setCheckLessThanZero(true);
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetCurrentSABefore(dialog->GetFormula());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::FXWidthAfter()
{
    QScopedPointer<DialogEditWrongFormula> dialog(new DialogEditWrongFormula(data, toolId, this));
    dialog->setWindowTitle(tr("Edit seam allowance width after"));
    dialog->SetFormula(GetFormulaSAWidthAfter());
    dialog->setCheckLessThanZero(true);
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetCurrentSAAfter(dialog->GetFormula());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::FXVisible()
{
    QScopedPointer<DialogEditWrongFormula> dialog(new DialogEditWrongFormula(data, toolId, this));
    dialog->setWindowTitle(tr("Control visibility"));
    dialog->SetFormula(GetFormulaVisible());
    if (dialog->exec() == QDialog::Accepted)
    {
        SetFormulaVisible(dialog->GetFormula());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::FXPassmarkLength()
{
    QScopedPointer<DialogEditWrongFormula> dialog(new DialogEditWrongFormula(data, toolId, this));
    dialog->setWindowTitle(tr("Edit passmark length"));
    dialog->SetFormula(GetFormulaPassmarkLength());
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetFormulaPassmarkLength(dialog->GetFormula());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::FXPassmarkWidth()
{
    QScopedPointer<DialogEditWrongFormula> dialog(new DialogEditWrongFormula(data, toolId, this));
    dialog->setWindowTitle(tr("Edit passmark width"));
    dialog->SetFormula(GetFormulaPassmarkWidth());
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetFormulaPassmarkWidth(dialog->GetFormula());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::FXPassmarkAngle()
{
    QScopedPointer<DialogEditWrongFormula> dialog(new DialogEditWrongFormula(data, toolId, this));
    dialog->setWindowTitle(tr("Edit passmark angle"));
    dialog->SetFormula(GetFormulaPassmarkAngle());
    dialog->setPostfix(degreeSymbol);
    if (dialog->exec() == QDialog::Accepted)
    {
        SetFormulaPassmarkAngle(dialog->GetFormula());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::DeployWidthFormulaTextEdit()
{
    DeployFormula(this, ui->plainTextEditFormulaWidth, ui->pushButtonGrowWidth, m_formulaBaseWidth);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::DeployWidthBeforeFormulaTextEdit()
{
    DeployFormula(this, ui->plainTextEditFormulaWidthBefore, ui->pushButtonGrowWidthBefore, m_formulaBaseWidthBefore);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::DeployWidthAfterFormulaTextEdit()
{
    DeployFormula(this, ui->plainTextEditFormulaWidthAfter, ui->pushButtonGrowWidthAfter, m_formulaBaseWidthAfter);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::DeployVisibleFormulaTextEdit()
{
    DeployFormula(this, ui->plainTextEditFormulaVisible, ui->pushButtonGrowVisible, m_formulaBaseVisible);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::DeployPassmarkLength()
{
    DeployFormula(this, ui->plainTextEditPassmarkLength, ui->pushButtonGrowPassmarkLength, m_formulaBasePassmarkLength);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::DeployPassmarkWidth()
{
    DeployFormula(this, ui->plainTextEditPassmarkWidth, ui->pushButtonGrowPassmarkWidth, m_formulaBasePassmarkWidth);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::DeployPassmarkAngle()
{
    DeployFormula(this, ui->plainTextEditPassmarkAngle, ui->pushButtonGrowPassmarkAngle, m_formulaBasePassmarkAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::SetMoveControls()
{
    ui->toolButtonTop->setEnabled(false);
    ui->toolButtonUp->setEnabled(false);
    ui->toolButtonDown->setEnabled(false);
    ui->toolButtonBottom->setEnabled(false);

    if (ui->listWidget->count() >= 2)
    {
        if (ui->listWidget->currentRow() == 0)
        {
            ui->toolButtonDown->setEnabled(true);
            ui->toolButtonBottom->setEnabled(true);
        }
        else if (ui->listWidget->currentRow() == ui->listWidget->count() - 1)
        {
            ui->toolButtonTop->setEnabled(true);
            ui->toolButtonUp->setEnabled(true);
        }
        else
        {
            ui->toolButtonTop->setEnabled(true);
            ui->toolButtonUp->setEnabled(true);
            ui->toolButtonDown->setEnabled(true);
            ui->toolButtonBottom->setEnabled(true);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::InitPathTab()
{
    ui->lineEditName->setClearButtonEnabled(true);

    FillComboBoxTypeLine(ui->comboBoxPenType, CurvePenStylesPics(ui->comboBoxPenType->palette().color(QPalette::Base),
                                                                 ui->comboBoxPenType->palette().color(QPalette::Text)));

    connect(ui->lineEditName, &QLineEdit::textChanged, this, &DialogPiecePath::NameChanged);

    InitPathTypes();
    connect(ui->comboBoxType, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this]()
            {
                const bool isInternalPath = GetType() == PiecePathType::InternalPath;
                ui->comboBoxPenType->setEnabled(isInternalPath);
                ui->checkBoxCut->setEnabled(isInternalPath);
                ui->tabControl->setEnabled(isInternalPath);
                ui->checkBoxFirstPointToCuttingContour->setEnabled(isInternalPath);
                ui->checkBoxLastPointToCuttingContour->setEnabled(isInternalPath);
                ValidObjects(PathIsValid());
            });

    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listWidget, &QListWidget::customContextMenuRequested, this, &DialogPiecePath::ShowContextMenu);
    connect(ui->listWidget->model(), &QAbstractItemModel::rowsMoved, this, &DialogPiecePath::ListChanged);
    connect(ui->listWidget, &QListWidget::itemSelectionChanged, this, &DialogPiecePath::SetMoveControls);

    connect(ui->listWidget->model(), &QAbstractItemModel::rowsMoved, this, [this]() { ValidObjects(PathIsValid()); });

    connect(ui->toolButtonTop, &QToolButton::clicked, this,
            [this]()
            {
                MoveListRowTop(ui->listWidget);
                ValidObjects(PathIsValid());
            });
    connect(ui->toolButtonUp, &QToolButton::clicked, this,
            [this]()
            {
                MoveListRowUp(ui->listWidget);
                ValidObjects(PathIsValid());
            });
    connect(ui->toolButtonDown, &QToolButton::clicked, this,
            [this]()
            {
                MoveListRowDown(ui->listWidget);
                ValidObjects(PathIsValid());
            });
    connect(ui->toolButtonBottom, &QToolButton::clicked, this,
            [this]()
            {
                MoveListRowBottom(ui->listWidget);
                ValidObjects(PathIsValid());
            });
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::InitSeamAllowanceTab()
{
    this->m_formulaBaseWidth = ui->plainTextEditFormulaWidth->height();
    this->m_formulaBaseWidthBefore = ui->plainTextEditFormulaWidthBefore->height();
    this->m_formulaBaseWidthAfter = ui->plainTextEditFormulaWidthAfter->height();

    ui->plainTextEditFormulaWidth->installEventFilter(this);
    ui->plainTextEditFormulaWidthBefore->installEventFilter(this);
    ui->plainTextEditFormulaWidthAfter->installEventFilter(this);

    m_timerWidth->setSingleShot(true);
    m_timerWidthBefore->setSingleShot(true);
    m_timerWidthAfter->setSingleShot(true);

    connect(m_timerWidth, &QTimer::timeout, this, &DialogPiecePath::EvalWidth);
    connect(m_timerWidthBefore, &QTimer::timeout, this, &DialogPiecePath::EvalWidthBefore);
    connect(m_timerWidthAfter, &QTimer::timeout, this, &DialogPiecePath::EvalWidthAfter);

    // Default value for seam allowence is 1 cm. But pattern have different units, so just set 1 in dialog not enough.
    m_saWidth = UnitConvertor(1, Unit::Cm, VAbstractValApplication::VApp()->patternUnits());
    ui->plainTextEditFormulaWidth->setPlainText(VAbstractApplication::VApp()->LocaleToString(m_saWidth));

    InitNodesList();
    connect(ui->comboBoxNodes, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogPiecePath::NodeChanged);

    connect(ui->pushButtonDefBefore, &QPushButton::clicked, this, &DialogPiecePath::ReturnDefBefore);
    connect(ui->pushButtonDefAfter, &QPushButton::clicked, this, &DialogPiecePath::ReturnDefAfter);

    InitNodeAngles(ui->comboBoxAngle);
    connect(ui->comboBoxAngle, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogPiecePath::NodeAngleChanged);

    connect(ui->toolButtonExprWidth, &QPushButton::clicked, this, &DialogPiecePath::FXWidth);
    connect(ui->toolButtonExprBefore, &QPushButton::clicked, this, &DialogPiecePath::FXWidthBefore);
    connect(ui->toolButtonExprAfter, &QPushButton::clicked, this, &DialogPiecePath::FXWidthAfter);

    connect(ui->plainTextEditFormulaWidth, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerWidth->start(formulaTimerTimeout); });

    connect(ui->plainTextEditFormulaWidthBefore, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerWidthBefore->start(formulaTimerTimeout); });

    connect(ui->plainTextEditFormulaWidthAfter, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerWidthAfter->start(formulaTimerTimeout); });

    connect(ui->pushButtonGrowWidth, &QPushButton::clicked, this, &DialogPiecePath::DeployWidthFormulaTextEdit);
    connect(ui->pushButtonGrowWidthBefore, &QPushButton::clicked, this,
            &DialogPiecePath::DeployWidthBeforeFormulaTextEdit);
    connect(ui->pushButtonGrowWidthAfter, &QPushButton::clicked, this,
            &DialogPiecePath::DeployWidthAfterFormulaTextEdit);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::InitPassmarksTab()
{
    // Length formula
    this->m_formulaBasePassmarkLength = ui->plainTextEditPassmarkLength->height();
    ui->plainTextEditPassmarkLength->installEventFilter(this);
    m_timerPassmarkLength->setSingleShot(true);

    connect(m_timerPassmarkLength, &QTimer::timeout, this, &DialogPiecePath::EvalPassmarkLength);
    connect(ui->groupBoxManualLength, &QGroupBox::toggled, this, &DialogPiecePath::EnabledManualPassmarkLength);
    connect(ui->toolButtonExprLength, &QPushButton::clicked, this, &DialogPiecePath::FXPassmarkLength);
    connect(ui->plainTextEditPassmarkLength, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerPassmarkLength->start(formulaTimerTimeout); });
    connect(ui->pushButtonGrowPassmarkLength, &QPushButton::clicked, this, &DialogPiecePath::DeployPassmarkLength);

    // Width formula
    this->m_formulaBasePassmarkWidth = ui->plainTextEditPassmarkWidth->height();
    ui->plainTextEditPassmarkWidth->installEventFilter(this);
    m_timerPassmarkWidth->setSingleShot(true);

    connect(m_timerPassmarkWidth, &QTimer::timeout, this, &DialogPiecePath::EvalPassmarkWidth);
    connect(ui->groupBoxManualWidth, &QGroupBox::toggled, this, &DialogPiecePath::EnabledManualPassmarkWidth);
    connect(ui->toolButtonExprWidth, &QPushButton::clicked, this, &DialogPiecePath::FXPassmarkWidth);
    connect(ui->plainTextEditPassmarkWidth, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerPassmarkWidth->start(formulaTimerTimeout); });
    connect(ui->pushButtonGrowPassmarkWidth, &QPushButton::clicked, this, &DialogPiecePath::DeployPassmarkWidth);

    // Angle formula
    this->m_formulaBasePassmarkAngle = ui->plainTextEditPassmarkAngle->height();
    ui->plainTextEditPassmarkAngle->installEventFilter(this);
    m_timerPassmarkAngle->setSingleShot(true);

    connect(m_timerPassmarkAngle, &QTimer::timeout, this, &DialogPiecePath::EvalPassmarkAngle);
    connect(ui->groupBoxManualAngle, &QGroupBox::toggled, this, &DialogPiecePath::EnabledManualPassmarkAngle);
    connect(ui->toolButtonExprAngle, &QPushButton::clicked, this, &DialogPiecePath::FXPassmarkAngle);
    connect(ui->plainTextEditPassmarkAngle, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerPassmarkAngle->start(formulaTimerTimeout); });
    connect(ui->pushButtonGrowPassmarkAngle, &QPushButton::clicked, this, &DialogPiecePath::DeployPassmarkAngle);

    // notch list
    InitPassmarksList();
    connect(ui->comboBoxPassmarks, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogPiecePath::PassmarkChanged);

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(ui->buttonGroupMarkType, QOverload<int>::of(&QButtonGroup::buttonClicked), this,
            &DialogPiecePath::PassmarkLineTypeChanged);
    connect(ui->buttonGroupAngleType, QOverload<int>::of(&QButtonGroup::buttonClicked), this,
            &DialogPiecePath::PassmarkAngleTypeChanged);
#else
    connect(ui->buttonGroupMarkType, &QButtonGroup::idClicked, this, &DialogPiecePath::PassmarkLineTypeChanged);
    connect(ui->buttonGroupAngleType, &QButtonGroup::idClicked, this, &DialogPiecePath::PassmarkAngleTypeChanged);
#endif

    connect(ui->checkBoxShowSecondPassmark, &QCheckBox::stateChanged, this,
            &DialogPiecePath::PassmarkShowSecondChanged);
    connect(ui->checkBoxClockwiseOpening, &QCheckBox::stateChanged, this,
            &DialogPiecePath::PassmarkClockwiseOrientationChanged);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::InitControlTab()
{
    this->m_formulaBaseVisible = ui->plainTextEditFormulaVisible->height();

    ui->plainTextEditFormulaVisible->installEventFilter(this);

    m_timerVisible->setSingleShot(true);

    connect(m_timerVisible, &QTimer::timeout, this, &DialogPiecePath::EvalVisible);
    connect(ui->toolButtonExprVisible, &QPushButton::clicked, this, &DialogPiecePath::FXVisible);
    connect(ui->plainTextEditFormulaVisible, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerVisible->start(formulaTimerTimeout); });
    connect(ui->pushButtonGrowVisible, &QPushButton::clicked, this, &DialogPiecePath::DeployVisibleFormulaTextEdit);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::InitPathTypes()
{
    ui->comboBoxType->addItem(tr("Internal path"), static_cast<int>(PiecePathType::InternalPath));
    ui->comboBoxType->addItem(tr("Custom seam allowance"), static_cast<int>(PiecePathType::CustomSeamAllowance));

    ui->comboBoxPenType->setEnabled(GetType() == PiecePathType::InternalPath);
    ui->checkBoxCut->setEnabled(GetType() == PiecePathType::InternalPath);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::InitNodesList()
{
    const quint32 id = ui->comboBoxNodes->currentData().toUInt();

    ui->comboBoxNodes->blockSignals(true);
    ui->comboBoxNodes->clear();

    const VPiecePath path = CreatePath();

    for (int i = 0; i < path.CountNodes(); ++i)
    {
        const VPieceNode &node = path.at(i);
        if (node.GetTypeTool() == Tool::NodePoint)
        {
            const QString name = GetNodeName(data, node);

            ui->comboBoxNodes->addItem(name, node.GetId());
        }
    }
    ui->comboBoxNodes->blockSignals(false);

    const int index = ui->comboBoxNodes->findData(id);
    if (index != -1)
    {
        ui->comboBoxNodes->setCurrentIndex(index);
        NodeChanged(index); // Need in case combox index was not changed
    }
    else
    {
        ui->comboBoxNodes->count() > 0 ? NodeChanged(0) : NodeChanged(-1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::InitPassmarksList()
{
    const quint32 id = ui->comboBoxPassmarks->currentData().toUInt();

    ui->comboBoxPassmarks->blockSignals(true);
    ui->comboBoxPassmarks->clear();

    const QVector<VPieceNode> nodes = GetListInternals<VPieceNode>(ui->listWidget);

    for (const auto &node : nodes)
    {
        if (node.GetTypeTool() == Tool::NodePoint && node.IsPassmark())
        {
            const QString name = GetNodeName(data, node);

            ui->comboBoxPassmarks->addItem(name, node.GetId());
        }
    }
    ui->comboBoxPassmarks->blockSignals(false);

    const int index = ui->comboBoxPassmarks->findData(id);
    if (index != -1)
    {
        ui->comboBoxPassmarks->setCurrentIndex(index);
        PassmarkChanged(index); // Need in case combox index was not changed
    }
    else
    {
        ui->comboBoxPassmarks->count() > 0 ? PassmarkChanged(0) : PassmarkChanged(-1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::NodeAngleChanged(int index)
{
    const int i = ui->comboBoxNodes->currentIndex();
    if (i != -1 && index != -1)
    {
        QListWidgetItem *rowItem = GetItemById(ui->comboBoxNodes->currentData().toUInt());
        if (rowItem)
        {
            const PieceNodeAngle angle = static_cast<PieceNodeAngle>(ui->comboBoxAngle->currentData().toUInt());
            auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetAngleType(angle);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

            ListChanged();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPiecePath::GetPiecePath() const -> VPiecePath
{
    return CreatePath();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::SetPiecePath(const VPiecePath &path)
{
    SetType(path.GetType()); // Set first to know path type

    ui->listWidget->clear();
    for (int i = 0; i < path.CountNodes(); ++i)
    {
        NewItem(path.at(i));
    }

    ui->lineEditName->setText(path.GetName());

    auto *visPath = qobject_cast<VisToolPiecePath *>(vis);
    SCASSERT(visPath != nullptr);
    visPath->SetPath(path);
    visPath->SetCuttingPath(CuttingPath(toolId, data));

    SetPenType(path.GetPenType());
    SetCutPath(path.IsCutPath());
    ui->checkBoxFirstPointToCuttingContour->setChecked(path.IsFirstToCuttingContour());
    ui->checkBoxLastPointToCuttingContour->setChecked(path.IsLastToCuttingContour());

    if (path.GetType() == PiecePathType::InternalPath)
    {
        SetFormulaVisible(path.GetVisibilityTrigger());
    }
    else
    {
        ui->plainTextEditFormulaVisible->setPlainText(QChar('1'));
    }

    ValidObjects(PathIsValid());

    ListChanged();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPiecePath::GetType() const -> PiecePathType
{
    return static_cast<PiecePathType>(ui->comboBoxType->currentData().toInt());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::SetType(PiecePathType type)
{
    const qint32 index = ui->comboBoxType->findData(static_cast<int>(type));
    if (index != -1)
    {
        ui->comboBoxType->setCurrentIndex(index);
    }

    ui->comboBoxPenType->setEnabled(type == PiecePathType::InternalPath);
    ui->checkBoxCut->setEnabled(type == PiecePathType::InternalPath);
    ui->checkBoxFirstPointToCuttingContour->setEnabled(type == PiecePathType::InternalPath);
    ui->checkBoxLastPointToCuttingContour->setEnabled(type == PiecePathType::InternalPath);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPiecePath::GetPenType() const -> Qt::PenStyle
{
    return LineStyleToPenStyle(GetComboBoxCurrentData(ui->comboBoxPenType, TypeLineLine));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::SetPenType(const Qt::PenStyle &type)
{
    ChangeCurrentData(ui->comboBoxPenType, PenStyleToLineStyle(type));
    vis->SetLineStyle(type);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPiecePath::IsCutPath() const -> bool
{
    return ui->checkBoxCut->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::SetCutPath(bool value)
{
    ui->checkBoxCut->setChecked(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPiecePath::GetItemById(quint32 id) -> QListWidgetItem *
{
    for (qint32 i = ui->listWidget->count() - 1; i >= 0; --i)
    {
        QListWidgetItem *item = ui->listWidget->item(i);
        const auto node = qvariant_cast<VPieceNode>(item->data(Qt::UserRole));

        if (node.GetId() == id)
        {
            return item;
        }
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPiecePath::GetLastId() const -> quint32
{
    const int count = ui->listWidget->count();
    if (count > 0)
    {
        QListWidgetItem *item = ui->listWidget->item(count - 1);
        const auto node = qvariant_cast<VPieceNode>(item->data(Qt::UserRole));
        return node.GetId();
    }

    return NULL_ID;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::SetCurrentSABefore(const QString &formula)
{
    UpdateNodeSABefore(formula);
    ListChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::SetCurrentSAAfter(const QString &formula)
{
    UpdateNodeSAAfter(formula);
    ListChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::UpdateNodeSABefore(const QString &formula)
{
    const int index = ui->comboBoxNodes->currentIndex();
    if (index != -1)
    {
        QListWidgetItem *rowItem = GetItemById(ui->comboBoxNodes->currentData().toUInt());
        if (rowItem)
        {
            auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetFormulaSABefore(formula);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::UpdateNodeSAAfter(const QString &formula)
{
    const int index = ui->comboBoxNodes->currentIndex();
    if (index != -1)
    {
        QListWidgetItem *rowItem = GetItemById(ui->comboBoxNodes->currentData().toUInt());
        if (rowItem)
        {
            auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetFormulaSAAfter(formula);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::UpdateNodePassmarkLength(const QString &formula)
{
    const int index = ui->comboBoxPassmarks->currentIndex();
    if (index != -1)
    {
        QListWidgetItem *rowItem = GetItemById(ui->comboBoxPassmarks->currentData().toUInt());
        if (rowItem)
        {
            auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetFormulaPassmarkLength(formula);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::UpdateNodePassmarkWidth(const QString &formula)
{
    const int index = ui->comboBoxPassmarks->currentIndex();
    if (index != -1)
    {
        QListWidgetItem *rowItem = GetItemById(ui->comboBoxPassmarks->currentData().toUInt());
        if (rowItem)
        {
            auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetFormulaPassmarkWidth(formula);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::UpdateNodePassmarkAngle(const QString &formula)
{
    const int index = ui->comboBoxPassmarks->currentIndex();
    if (index != -1)
    {
        QListWidgetItem *rowItem = GetItemById(ui->comboBoxPassmarks->currentData().toUInt());
        if (rowItem)
        {
            auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetFormulaPassmarkAngle(formula);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::EnabledManualPassmarkLength()
{
    const int index = ui->comboBoxPassmarks->currentIndex();
    if (index != -1)
    {
        QListWidgetItem *rowItem = GetItemById(ui->comboBoxPassmarks->currentData().toUInt());
        if (rowItem)
        {
            auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetManualPassmarkLength(ui->groupBoxManualLength->isChecked());
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

            ui->toolButtonExprLength->setEnabled(ui->groupBoxManualLength->isChecked());
            ui->plainTextEditPassmarkLength->setEnabled(ui->groupBoxManualLength->isChecked());
            ui->pushButtonGrowPassmarkLength->setEnabled(ui->groupBoxManualLength->isChecked());
            ui->labelEditPassmarkLength->setEnabled(ui->groupBoxManualLength->isChecked());
            ui->label_8->setEnabled(ui->groupBoxManualLength->isChecked());

            EvalPassmarkLength();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::EnabledManualPassmarkWidth()
{
    const int index = ui->comboBoxPassmarks->currentIndex();
    if (index != -1)
    {
        QListWidgetItem *rowItem = GetItemById(ui->comboBoxPassmarks->currentData().toUInt());
        if (rowItem)
        {
            auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetManualPassmarkWidth(ui->groupBoxManualWidth->isChecked());
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

            ui->toolButtonExprWidth->setEnabled(ui->groupBoxManualWidth->isChecked());
            ui->plainTextEditPassmarkWidth->setEnabled(ui->groupBoxManualWidth->isChecked());
            ui->pushButtonGrowPassmarkWidth->setEnabled(ui->groupBoxManualWidth->isChecked());
            ui->labelEditPassmarkWidth->setEnabled(ui->groupBoxManualWidth->isChecked());
            ui->label_4->setEnabled(ui->groupBoxManualWidth->isChecked());

            EvalPassmarkWidth();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::EnabledManualPassmarkAngle()
{
    const int index = ui->comboBoxPassmarks->currentIndex();
    if (index != -1)
    {
        QListWidgetItem *rowItem = GetItemById(ui->comboBoxPassmarks->currentData().toUInt());
        if (rowItem)
        {
            auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetManualPassmarkAngle(ui->groupBoxManualAngle->isChecked());
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

            ui->toolButtonExprAngle->setEnabled(ui->groupBoxManualAngle->isChecked());
            ui->plainTextEditPassmarkAngle->setEnabled(ui->groupBoxManualAngle->isChecked());
            ui->pushButtonGrowPassmarkAngle->setEnabled(ui->groupBoxManualAngle->isChecked());
            ui->labelEditPassmarkAngle->setEnabled(ui->groupBoxManualAngle->isChecked());
            ui->label_7->setEnabled(ui->groupBoxManualAngle->isChecked());

            EvalPassmarkAngle();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::SetFormulaSAWidth(const QString &formula)
{
    if (formula.isEmpty())
    {
        return;
    }

    const QString width = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (width.length() > 80)
    {
        this->DeployWidthFormulaTextEdit();
    }
    ui->plainTextEditFormulaWidth->setPlainText(width);

    auto *path = qobject_cast<VisToolPiecePath *>(vis);
    SCASSERT(path != nullptr)
    path->SetPath(CreatePath());
    path->SetCuttingPath(CuttingPath(toolId, data));

    if (ui->tabWidget->indexOf(ui->tabSeamAllowance) == -1)
    {
        ui->tabWidget->addTab(ui->tabSeamAllowance, tr("Seam allowance"));
    }

    if (ui->tabWidget->indexOf(ui->tabPassmarks) == -1)
    {
        ui->tabWidget->addTab(ui->tabPassmarks, tr("Passmarks"));
    }

    MoveCursorToEnd(ui->plainTextEditFormulaWidth);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPiecePath::GetPieceId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxPiece);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::SetPieceId(quint32 id)
{
    if (ui->comboBoxPiece->count() <= 0)
    {
        ui->comboBoxPiece->addItem(data->GetPiece(id).GetName(), id);
    }
    else
    {
        const qint32 index = ui->comboBoxPiece->findData(id);
        if (index != -1)
        {
            ui->comboBoxPiece->setCurrentIndex(index);
        }
        else
        {
            ui->comboBoxPiece->setCurrentIndex(0);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPiecePath::GetFormulaSAWidth() const -> QString
{
    QString width = ui->plainTextEditFormulaWidth->toPlainText();
    return VTranslateVars::TryFormulaFromUser(width, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::SetPiecesList(const QVector<quint32> &list)
{
    FillComboBoxPiecesList(ui->comboBoxPiece, list);

    if (list.isEmpty())
    {
        qWarning() << tr("The list of pieces is empty. Please, first create at least one piece for current pattern "
                         "piece.");
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPiecePath::CreatePath() const -> VPiecePath
{
    VPiecePath path;
    for (qint32 i = 0; i < ui->listWidget->count(); ++i)
    {
        QListWidgetItem *item = ui->listWidget->item(i);
        path.Append(qvariant_cast<VPieceNode>(item->data(Qt::UserRole)));
    }

    const bool isInternalPath = (GetType() == PiecePathType::InternalPath);
    path.SetType(GetType());
    path.SetName(ui->lineEditName->text());
    path.SetPenType(isInternalPath ? GetPenType() : Qt::SolidLine);
    path.SetCutPath(isInternalPath ? IsCutPath() : false);
    path.SetFirstToCuttingContour(isInternalPath ? ui->checkBoxFirstPointToCuttingContour->isChecked() : false);
    path.SetLastToCuttingContour(isInternalPath ? ui->checkBoxLastPointToCuttingContour->isChecked() : false);
    path.SetVisibilityTrigger(isInternalPath ? GetFormulaVisible() : QChar('1'));

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPiecePath::PathIsValid() const -> bool
{
    if (CreatePath().PathPoints(data).count() < 2)
    {
        ui->helpLabel->setText(DialogWarningIcon() + tr("You need more points!"));
        return false;
    }

    QString error;
    if (GetType() == PiecePathType::CustomSeamAllowance && FirstPointEqualLast(ui->listWidget, data, error))
    {
        ui->helpLabel->setText(QStringLiteral("%1%2 %3").arg(
            DialogWarningIcon(), tr("First point of <b>custom seam allowance</b> cannot be equal to the last point!"),
            error));
        return false;
    }

    error.clear();
    if (DoublePoints(ui->listWidget, data, error))
    {
        ui->helpLabel->setText(
            QStringLiteral("%1%2 %3").arg(DialogWarningIcon(), tr("You have double points!"), error));
        return false;
    }

    error.clear();
    if (DoubleCurves(ui->listWidget, data, error))
    {
        ui->helpLabel->setText(
            QStringLiteral("%1%2 %3").arg(DialogWarningIcon(), tr("The same curve repeats twice!"), error));
        return false;
    }

    if (GetType() == PiecePathType::CustomSeamAllowance && not EachPointLabelIsUnique(ui->listWidget))
    {
        ui->helpLabel->setText(DialogWarningIcon() +
                               tr("Each point in the <b>custom seam allowance</b> path must be unique!"));
        return false;
    }

    if (not m_showMode && ui->comboBoxPiece->count() <= 0)
    {
        ui->helpLabel->setText(DialogWarningIcon() + tr("List of details is empty!"));
        return false;
    }

    if (not m_showMode && ui->comboBoxPiece->currentIndex() == -1)
    {
        ui->helpLabel->setText(DialogWarningIcon() + tr("Please, select a detail to insert into!"));
        return false;
    }

    error.clear();
    if (InvalidSegment(ui->listWidget, data, error))
    {
        ui->helpLabel->setText(QStringLiteral("%1%2 %3").arg(DialogWarningIcon(), tr("Invalid segment!"), error));
        return false;
    }

    ui->helpLabel->setText(tr("Ready!"));
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::ValidObjects(bool value)
{
    m_flagError = value;
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::NewItem(const VPieceNode &node)
{
    NewNodeItem(ui->listWidget, node, IsShowNotch());
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPiecePath::GetFormulaSAWidthBefore() const -> QString
{
    QString width = ui->plainTextEditFormulaWidthBefore->toPlainText();
    return VTranslateVars::TryFormulaFromUser(width, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPiecePath::GetFormulaSAWidthAfter() const -> QString
{
    QString width = ui->plainTextEditFormulaWidthAfter->toPlainText();
    return VTranslateVars::TryFormulaFromUser(width, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPiecePath::GetFormulaVisible() const -> QString
{
    QString formula = ui->plainTextEditFormulaVisible->toPlainText();
    return VTranslateVars::TryFormulaFromUser(formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::SetFormulaVisible(const QString &formula)
{
    const QString f = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (f.length() > 80)
    {
        this->DeployVisibleFormulaTextEdit();
    }
    ui->plainTextEditFormulaVisible->setPlainText(f);
    MoveCursorToEnd(ui->plainTextEditFormulaVisible);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPiecePath::GetFormulaPassmarkLength() const -> QString
{
    QString formula = ui->plainTextEditPassmarkLength->toPlainText();
    return VTranslateVars::TryFormulaFromUser(formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::SetFormulaPassmarkLength(const QString &formula)
{
    const QString f = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (f.length() > 80)
    {
        this->DeployPassmarkLength();
    }
    ui->plainTextEditPassmarkLength->setPlainText(f);
    MoveCursorToEnd(ui->plainTextEditPassmarkLength);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPiecePath::GetFormulaPassmarkWidth() const -> QString
{
    QString formula = ui->plainTextEditPassmarkWidth->toPlainText();
    return VTranslateVars::TryFormulaFromUser(formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::SetFormulaPassmarkWidth(const QString &formula)
{
    const QString f = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (f.length() > 80)
    {
        this->DeployPassmarkWidth();
    }
    ui->plainTextEditPassmarkWidth->setPlainText(f);
    MoveCursorToEnd(ui->plainTextEditPassmarkWidth);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPiecePath::GetFormulaPassmarkAngle() const -> QString
{
    QString formula = ui->plainTextEditPassmarkWidth->toPlainText();
    return VTranslateVars::TryFormulaFromUser(formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::SetFormulaPassmarkAngle(const QString &formula)
{
    const QString f = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (f.length() > 80)
    {
        this->DeployPassmarkAngle();
    }
    ui->plainTextEditPassmarkAngle->setPlainText(f);
    MoveCursorToEnd(ui->plainTextEditPassmarkAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::RefreshPathList(const VPiecePath &path)
{
    ui->listWidget->blockSignals(true);
    ui->listWidget->clear();
    for (int i = 0; i < path.CountNodes(); ++i)
    {
        NewItem(path.at(i));
    }
    ui->listWidget->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::InitPassmarkLengthFormula(const VPieceNode &node)
{
    // notch depth
    ui->groupBoxManualLength->setEnabled(true);

    if (node.IsManualPassmarkLength())
    {
        ui->groupBoxManualLength->setChecked(true);

        ui->toolButtonExprLength->setEnabled(ui->groupBoxManualLength->isChecked());
        ui->plainTextEditPassmarkLength->setEnabled(ui->groupBoxManualLength->isChecked());
        ui->pushButtonGrowPassmarkLength->setEnabled(ui->groupBoxManualLength->isChecked());
        ui->labelEditPassmarkLength->setEnabled(ui->groupBoxManualLength->isChecked());
        ui->label_8->setEnabled(ui->groupBoxManualLength->isChecked());

        QString passmarkLength = node.GetFormulaPassmarkLength();
        passmarkLength = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
            passmarkLength, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
        if (passmarkLength.length() > 80) // increase height if needed.
        {
            this->DeployPassmarkLength();
        }

        if (passmarkLength.isEmpty())
        {
            qreal length = UnitConvertor(1, Unit::Cm, VAbstractValApplication::VApp()->patternUnits());
            ui->plainTextEditPassmarkLength->setPlainText(VAbstractApplication::VApp()->LocaleToString(length));
        }
        else
        {
            ui->plainTextEditPassmarkLength->setPlainText(passmarkLength);
        }
    }
    else
    {
        qreal length = UnitConvertor(1, Unit::Cm, VAbstractValApplication::VApp()->patternUnits());
        ui->plainTextEditPassmarkLength->setPlainText(VAbstractApplication::VApp()->LocaleToString(length));
    }

    MoveCursorToEnd(ui->plainTextEditPassmarkLength);
    ChangeColor(ui->labelEditPassmarkLength, OkColor(this));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::InitPassmarkWidthFormula(const VPieceNode &node)
{
    // notch width
    if (node.GetPassmarkLineType() != PassmarkLineType::OneLine)
    {
        ui->groupBoxManualWidth->setEnabled(true);

        if (node.IsManualPassmarkWidth())
        {
            ui->groupBoxManualWidth->setChecked(true);

            ui->toolButtonExprWidth->setEnabled(ui->groupBoxManualWidth->isChecked());
            ui->plainTextEditPassmarkWidth->setEnabled(ui->groupBoxManualWidth->isChecked());
            ui->pushButtonGrowPassmarkWidth->setEnabled(ui->groupBoxManualWidth->isChecked());
            ui->labelEditPassmarkWidth->setEnabled(ui->groupBoxManualWidth->isChecked());
            ui->label_4->setEnabled(ui->groupBoxManualWidth->isChecked());

            QString passmarkWidth = node.GetFormulaPassmarkWidth();
            passmarkWidth = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
                passmarkWidth, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
            if (passmarkWidth.length() > 80) // increase height if needed.
            {
                this->DeployPassmarkWidth();
            }

            if (passmarkWidth.isEmpty())
            {
                qreal width = UnitConvertor(0.85, Unit::Cm, VAbstractValApplication::VApp()->patternUnits());
                ui->plainTextEditPassmarkWidth->setPlainText(VAbstractApplication::VApp()->LocaleToString(width));
            }
            else
            {
                ui->plainTextEditPassmarkWidth->setPlainText(passmarkWidth);
            }
        }
        else
        {
            qreal width = UnitConvertor(0.85, Unit::Cm, VAbstractValApplication::VApp()->patternUnits());
            ui->plainTextEditPassmarkWidth->setPlainText(VAbstractApplication::VApp()->LocaleToString(width));
        }

        MoveCursorToEnd(ui->plainTextEditPassmarkWidth);
    }
    else
    {
        qreal width = UnitConvertor(0.85, Unit::Cm, VAbstractValApplication::VApp()->patternUnits());
        ui->plainTextEditPassmarkWidth->setPlainText(VAbstractApplication::VApp()->LocaleToString(width));
    }
    ChangeColor(ui->labelEditPassmarkWidth, OkColor(this));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::InitPassmarkAngleFormula(const VPieceNode &node)
{
    // notch angle
    if (node.GetPassmarkAngleType() == PassmarkAngleType::Straightforward)
    {
        ui->groupBoxManualAngle->setEnabled(true);

        if (node.IsManualPassmarkAngle())
        {
            ui->groupBoxManualAngle->setChecked(true);

            ui->toolButtonExprAngle->setEnabled(ui->groupBoxManualAngle->isChecked());
            ui->plainTextEditPassmarkAngle->setEnabled(ui->groupBoxManualAngle->isChecked());
            ui->pushButtonGrowPassmarkAngle->setEnabled(ui->groupBoxManualAngle->isChecked());
            ui->labelEditPassmarkAngle->setEnabled(ui->groupBoxManualAngle->isChecked());
            ui->label_7->setEnabled(ui->groupBoxManualAngle->isChecked());

            QString passmarkAngle = node.GetFormulaPassmarkLength();
            passmarkAngle = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
                passmarkAngle, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
            if (passmarkAngle.length() > 80) // increase height if needed.
            {
                this->DeployPassmarkAngle();
            }

            ui->plainTextEditPassmarkAngle->setPlainText(passmarkAngle.isEmpty() ? QString::number(0) : passmarkAngle);
        }
        else
        {
            ui->plainTextEditPassmarkAngle->setPlainText(QString::number(0));
        }

        MoveCursorToEnd(ui->plainTextEditPassmarkAngle);
    }
    else
    {
        ui->plainTextEditPassmarkAngle->setPlainText(QString::number(0));
    }
    ChangeColor(ui->labelEditPassmarkAngle, OkColor(this));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::InitPassmarkShapeType(const VPieceNode &node)
{
    // Line type
    ui->groupBoxMarkType->setEnabled(true);

    switch (node.GetPassmarkLineType())
    {
        case PassmarkLineType::OneLine:
            ui->radioButtonOneLine->setChecked(true);
            break;
        case PassmarkLineType::TwoLines:
            ui->radioButtonTwoLines->setChecked(true);
            break;
        case PassmarkLineType::ThreeLines:
            ui->radioButtonThreeLines->setChecked(true);
            break;
        case PassmarkLineType::TMark:
            ui->radioButtonTMark->setChecked(true);
            break;
        case PassmarkLineType::ExternalVMark:
            ui->radioButtonVMark->setChecked(true);
            break;
        case PassmarkLineType::InternalVMark:
            ui->radioButtonVMark2->setChecked(true);
            break;
        case PassmarkLineType::UMark:
            ui->radioButtonUMark->setChecked(true);
            break;
        case PassmarkLineType::BoxMark:
            ui->radioButtonBoxMark->setChecked(true);
            break;
        case PassmarkLineType::CheckMark:
            ui->radioButtonCheckMark->setChecked(true);
            break;
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::InitPassmarkAngleType(const VPieceNode &node)
{
    // Angle type
    ui->groupBoxAngleType->setEnabled(true);

    switch (node.GetPassmarkAngleType())
    {
        case PassmarkAngleType::Straightforward:
            ui->radioButtonStraightforward->setChecked(true);
            break;
        case PassmarkAngleType::Bisector:
            ui->radioButtonBisector->setChecked(true);
            break;
        case PassmarkAngleType::Intersection:
            ui->radioButtonIntersection->setChecked(true);
            break;
        case PassmarkAngleType::IntersectionOnlyLeft:
            ui->radioButtonIntersectionOnlyLeft->setChecked(true);
            break;
        case PassmarkAngleType::IntersectionOnlyRight:
            ui->radioButtonIntersectionOnlyRight->setChecked(true);
            break;
        case PassmarkAngleType::Intersection2:
            ui->radioButtonIntersection2->setChecked(true);
            break;
        case PassmarkAngleType::Intersection2OnlyLeft:
            ui->radioButtonIntersection2OnlyLeft->setChecked(true);
            break;
        case PassmarkAngleType::Intersection2OnlyRight:
            ui->radioButtonIntersection2OnlyRight->setChecked(true);
            break;
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPiecePath::InitIcons()
{
    const QString resource = QStringLiteral("icon");

    const QString fxIcon = QStringLiteral("24x24/fx.png");
    ui->toolButtonExprWidth->setIcon(VTheme::GetIconResource(resource, fxIcon));
    ui->toolButtonExprBefore->setIcon(VTheme::GetIconResource(resource, fxIcon));
    ui->toolButtonExprAfter->setIcon(VTheme::GetIconResource(resource, fxIcon));
    ui->toolButtonExprLength->setIcon(VTheme::GetIconResource(resource, fxIcon));
    ui->toolButtonExprWidth_2->setIcon(VTheme::GetIconResource(resource, fxIcon));
    ui->toolButtonExprAngle->setIcon(VTheme::GetIconResource(resource, fxIcon));
    ui->toolButtonExprVisible->setIcon(VTheme::GetIconResource(resource, fxIcon));

    const QString equalIcon = QStringLiteral("24x24/equal.png");
    ui->label_2->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    ui->label_6->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    ui->label_10->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    ui->label_5->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPiecePath::InitContextMenu(QMenu *menu, const VPieceNode &rowNode) -> QHash<int, QAction *>
{
    SCASSERT(menu != nullptr)

    QHash<int, QAction *> contextMenu;

    if (rowNode.GetTypeTool() != Tool::NodePoint)
    {
        QAction *actionReverse = menu->addAction(tr("Reverse"));
        actionReverse->setCheckable(true);
        actionReverse->setChecked(rowNode.GetReverse());
    }
    else
    {
        if (m_showMode && GetType() == PiecePathType::CustomSeamAllowance &&
            ui->tabWidget->indexOf(ui->tabPassmarks) != -1)
        {
            QMenu *passmarkSubmenu = menu->addMenu(tr("Passmark"));

            QAction *actionNonePassmark = passmarkSubmenu->addAction(tr("None"));
            actionNonePassmark->setCheckable(true);
            actionNonePassmark->setChecked(!rowNode.IsPassmark());
            contextMenu.insert(static_cast<int>(ContextMenuOption::NonePassmark), actionNonePassmark);

            Q_STATIC_ASSERT_X(static_cast<int>(PassmarkLineType::LAST_ONE_DO_NOT_USE) == 9,
                              "Not all types were handled.");

            auto InitPassmarkLineTypeAction = [passmarkSubmenu, rowNode](const QString &name, PassmarkLineType lineType)
            {
                QAction *action = passmarkSubmenu->addAction(name);
                action->setCheckable(true);
                action->setChecked(rowNode.IsPassmark() && lineType == rowNode.GetPassmarkLineType());
                return action;
            };

            contextMenu.insert(static_cast<int>(ContextMenuOption::OneLine),
                               InitPassmarkLineTypeAction(tr("One line"), PassmarkLineType::OneLine));
            contextMenu.insert(static_cast<int>(ContextMenuOption::TwoLines),
                               InitPassmarkLineTypeAction(tr("Two lines"), PassmarkLineType::TwoLines));
            contextMenu.insert(static_cast<int>(ContextMenuOption::ThreeLines),
                               InitPassmarkLineTypeAction(tr("Three lines"), PassmarkLineType::ThreeLines));
            contextMenu.insert(static_cast<int>(ContextMenuOption::TMark),
                               InitPassmarkLineTypeAction(tr("T mark"), PassmarkLineType::TMark));
            contextMenu.insert(static_cast<int>(ContextMenuOption::ExternalVMark),
                               InitPassmarkLineTypeAction(tr("External V mark"), PassmarkLineType::ExternalVMark));
            contextMenu.insert(static_cast<int>(ContextMenuOption::InternalVMark),
                               InitPassmarkLineTypeAction(tr("Internal V mark"), PassmarkLineType::InternalVMark));
            contextMenu.insert(static_cast<int>(ContextMenuOption::UMark),
                               InitPassmarkLineTypeAction(tr("U mark"), PassmarkLineType::UMark));
            contextMenu.insert(static_cast<int>(ContextMenuOption::BoxMark),
                               InitPassmarkLineTypeAction(tr("Box mark"), PassmarkLineType::BoxMark));
            contextMenu.insert(static_cast<int>(ContextMenuOption::CheckMark),
                               InitPassmarkLineTypeAction(tr("Check mark"), PassmarkLineType::CheckMark));
        }

        QAction *actionUniqueness = menu->addAction(tr("Check uniqueness"));
        actionUniqueness->setCheckable(true);
        actionUniqueness->setChecked(rowNode.IsCheckUniqueness());
        contextMenu.insert(static_cast<int>(ContextMenuOption::Uniqueness), actionUniqueness);

        QAction *actionTurnPoint = menu->addAction(tr("Turn point"));
        actionTurnPoint->setCheckable(true);
        actionTurnPoint->setChecked(rowNode.IsTurnPoint());
        contextMenu.insert(static_cast<int>(ContextMenuOption::TurnPoint), actionTurnPoint);
    }

    QAction *actionExcluded = menu->addAction(tr("Excluded"));
    actionExcluded->setCheckable(true);
    actionExcluded->setChecked(rowNode.IsExcluded());
    contextMenu.insert(static_cast<int>(ContextMenuOption::Excluded), actionExcluded);

    QAction *actionDelete = menu->addAction(QIcon::fromTheme(QStringLiteral("edit-delete")), tr("Delete"));
    contextMenu.insert(static_cast<int>(ContextMenuOption::Delete), actionDelete);

    return contextMenu;
}
