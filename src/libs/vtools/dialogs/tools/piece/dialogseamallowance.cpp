/************************************************************************
 **
 **  @file   dialogseamallowance.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 11, 2016
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

#include "dialogseamallowance.h"
#include "../../../tools/vtoolseamallowance.h"
#include "../../../undocommands/savepiecepathoptions.h"
#include "../../../undocommands/saveplacelabeloptions.h"
#include "../../../visualization/path/vispiecespecialpoints.h"
#include "../../../visualization/path/vistoolpiece.h"
#include "../../dialogtoolbox.h"
#include "../../support/dialogeditlabel.h"
#include "../../support/dialogeditwrongformula.h"
#include "../vgeometry/vplacelabelitem.h"
#include "../vmisc/def.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/typedef.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vmisc/vmodifierkey.h"
#include "../vmisc/vvalentinasettings.h"
#include "../vpatterndb/calculator.h"
#include "../vpatterndb/floatItemData/vgrainlinedata.h"
#include "../vpatterndb/floatItemData/vpiecelabeldata.h"
#include "../vpatterndb/variables/vincrement.h"
#include "../vpatterndb/variables/vmeasurement.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vpatterndb/vpiecepath.h"
#include "../vwidgets/fancytabbar/fancytabbar.h"
#include "../vwidgets/vmousewheelwidgetadjustmentguard.h"
#include "dialogpatternmaterials.h"
#include "dialogpiecepath.h"
#include "dialogplacelabel.h"
#include "ui_dialogseamallowance.h"
#include "ui_tabfoldline.h"
#include "ui_tabgrainline.h"
#include "ui_tablabels.h"
#include "ui_tabpassmarks.h"
#include "ui_tabpaths.h"
#include "ui_tabpins.h"
#include "ui_tabplacelabels.h"

#include <QMenu>
#include <QMessageBox>
#include <QScopeGuard>
#include <QTimer>
#include <QUuid>
#include <QtNumeric>
#include <array>
#include <cstddef>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

enum TabOrder
{
    Paths = 0,
    Pins = 1,
    Labels = 2,
    Grainline = 3,
    Passmarks = 4,
    PlaceLabels = 5,
    FoldLine = 6,
    Count = 7
};

namespace
{
enum class MainPathContextMenuOption : int
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
    DuplicateNode,
    Delete,
    LAST_ONE_DO_NOT_USE
};

//---------------------------------------------------------------------------------------------------------------------
void EnableDefButton(QPushButton *defButton, const QString &formula)
{
    SCASSERT(defButton != nullptr)

    if (formula != currentSeamAllowance)
    {
        defButton->setEnabled(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto GetFormulaFromUser(QPlainTextEdit *textEdit) -> QString
{
    SCASSERT(textEdit != nullptr)
    return VTranslateVars::TryFormulaFromUser(textEdit->toPlainText(),
                                              VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void InitComboBoxFormats(QComboBox *box, const QStringList &items, const QString &currentFormat)
{
    SCASSERT(box != nullptr)

    const QSignalBlocker blocker(box);
    box->addItems(items);
    int const index = box->findText(currentFormat);
    if (index != -1)
    {
        box->setCurrentIndex(index);
    }
    else
    {
        box->setCurrentIndex(0);
    }
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
DialogSeamAllowance::DialogSeamAllowance(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent)
  : DialogTool(data, doc, toolId, parent),
    ui(new Ui::DialogSeamAllowance),
    uiTabPaths(new Ui::TabPaths),
    uiTabLabels(new Ui::TabLabels),
    uiTabGrainline(new Ui::TabGrainline),
    uiTabPins(new Ui::TabPins),
    uiTabPassmarks(new Ui::TabPassmarks),
    uiTabPlaceLabels(new Ui::TabPlaceLabels),
    uiTabFoldLine(new Ui::TabFoldLine),
    m_tabPaths(new QWidget),
    m_tabLabels(new QWidget),
    m_tabGrainline(new QWidget),
    m_tabPins(new QWidget),
    m_tabPassmarks(new QWidget),
    m_tabPlaceLabels(new QWidget),
    m_tabFoldLine(new QWidget),
    m_ftb(new FancyTabBar(FancyTabBar::Left, this)),
    m_timerWidth(new QTimer(this)),
    m_timerWidthBefore(new QTimer(this)),
    m_timerWidthAfter(new QTimer(this)),
    m_timerPassmarkLength(new QTimer(this)),
    m_timerPassmarkWidth(new QTimer(this)),
    m_timerPassmarkAngle(new QTimer(this)),
    m_timerFoldHeight(new QTimer(this)),
    m_timerFoldWidth(new QTimer(this)),
    m_timerFoldCenter(new QTimer(this)),
    m_placeholdersMenu(new QMenu(this))
{
    ui->setupUi(this);

    m_defLabelValue = QString().setNum(UnitConvertor(10, Unit::Cm, *data->GetPatternUnit()));

    InitOkCancelApply(ui);
    InitFancyTabBar();
    EnableApply(applyAllowed);

    InitMainPathTab();
    InitPieceTab();
    InitSeamAllowanceTab();
    InitInternalPathsTab();
    InitPatternPieceDataTab();
    InitLabelsTab();
    InitGrainlineTab();
    InitPinsTab();
    InitPassmarksTab();
    InitPlaceLabelsTab();
    InitFoldLineTab();

    InitIcons();

    // Prevent stealing focus when scrolling
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(uiTabGrainline->comboBoxGrainlineCenterPin);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(uiTabGrainline->comboBoxGrainlineTopPin);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(uiTabGrainline->comboBoxGrainlineBottomPin);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(uiTabGrainline->comboBoxArrow);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(uiTabLabels->comboBoxPieceLabelSize);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(uiTabLabels->comboBoxDLCenterPin);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(uiTabLabels->comboBoxDLTopLeftPin);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(uiTabLabels->comboBoxDLBottomRightPin);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(uiTabLabels->comboBoxPatternLabelSize);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(uiTabLabels->comboBoxPLCenterPin);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(uiTabLabels->comboBoxPLTopLeftPin);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(uiTabLabels->comboBoxPLBottomRightPin);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(uiTabLabels->dateEditCustomerBirthDate);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(uiTabLabels->comboBoxDateFormat);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(uiTabLabels->comboBoxTimeFormat);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(uiTabLabels->spinBoxQuantity);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(uiTabPassmarks->comboBoxPassmarks);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(uiTabPaths->comboBoxMLStartPoint);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(uiTabPaths->comboBoxMLEndPoint);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(uiTabPaths->spinBoxPriority);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(uiTabPaths->comboBoxNodes);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(uiTabPaths->comboBoxAngle);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(uiTabPaths->comboBoxStartPoint);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(uiTabPaths->comboBoxEndPoint);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(uiTabPaths->comboBoxIncludeType);

    ChangeColor(uiTabPaths->labelEditName, OkColor(this));
    flagMainPathIsValid = MainPathIsValid();

    m_ftb->SetCurrentIndex(TabOrder::Paths); // Show always first tab active on start.

    uiTabLabels->groupBoxPatternLabel->setEnabled(not m_doc->GetPatternLabelTemplate().isEmpty());
}

//---------------------------------------------------------------------------------------------------------------------
DialogSeamAllowance::~DialogSeamAllowance()
{
    delete m_visSpecialPoints.data();
    delete m_tabFoldLine;
    delete m_tabPlaceLabels;
    delete m_tabPassmarks;
    delete m_tabPins;
    delete m_tabGrainline;
    delete m_tabLabels;
    delete m_tabPaths;
    delete uiTabPassmarks;
    delete uiTabPins;
    delete uiTabGrainline;
    delete uiTabLabels;
    delete uiTabPaths;
    delete ui;

    for (auto &command : m_undoStack)
    {
        delete command.data();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EnableApply(bool enable)
{
    SCASSERT(bApply != nullptr);
    bApply->setEnabled(enable);
    applyAllowed = enable;

    uiTabPaths->tabSeamAllowance->setEnabled(applyAllowed);
    uiTabPaths->tabInternalPaths->setEnabled(applyAllowed);
    uiTabPaths->groupBoxMirrorLine->setEnabled(applyAllowed);
    m_ftb->SetTabEnabled(TabOrder::Pins, applyAllowed);
    m_ftb->SetTabEnabled(TabOrder::Labels, applyAllowed);
    m_ftb->SetTabEnabled(TabOrder::Grainline, applyAllowed);
    m_ftb->SetTabEnabled(TabOrder::Passmarks, applyAllowed);
    m_ftb->SetTabEnabled(TabOrder::PlaceLabels, applyAllowed);
    m_ftb->SetTabEnabled(TabOrder::FoldLine, applyAllowed);

    if (not applyAllowed && vis.isNull())
    {
        vis = new VisToolPiece(data);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSeamAllowance::GetPiece() const -> VPiece
{
    return CreatePiece();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetPiece(const VPiece &piece)
{
    uiTabPaths->listWidgetMainPath->clear();
    for (int i = 0; i < piece.GetPath().CountNodes(); ++i)
    {
        NewMainPathItem(piece.GetPath().at(i));
    }

    uiTabPaths->checkBoxHideMainPath->setChecked(piece.IsHideMainPath());
    {
        const QSignalBlocker blocker(uiTabPaths->listWidgetCustomSA);
        uiTabPaths->listWidgetCustomSA->clear();
        QVector<CustomSARecord> const records = piece.GetCustomSARecords();
        for (auto record : records)
        {
            if (record.path > NULL_ID)
            {
                const QString name = GetPathName(record.path, record.reverse);

                auto *item = new QListWidgetItem(name);
                item->setFont(NodeFont(item->font()));
                item->setData(Qt::UserRole, QVariant::fromValue(record));
                uiTabPaths->listWidgetCustomSA->addItem(item);
                uiTabPaths->listWidgetCustomSA->setCurrentRow(uiTabPaths->listWidgetCustomSA->count() - 1);
            }
        }
    }

    uiTabPaths->listWidgetInternalPaths->clear();
    const QVector<quint32> paths = piece.GetInternalPaths();
    for (auto path : paths)
    {
        if (path > NULL_ID)
        {
            const QString name = GetPathName(path);

            auto *item = new QListWidgetItem(name);
            item->setFont(NodeFont(item->font()));
            item->setData(Qt::UserRole, QVariant::fromValue(path));
            uiTabPaths->listWidgetInternalPaths->addItem(item);
            uiTabPaths->listWidgetInternalPaths->setCurrentRow(uiTabPaths->listWidgetInternalPaths->count() - 1);
        }
    }

    auto NewSpecialPoint = [this](QListWidget *listWidget, quint32 point)
    {
        if (point > NULL_ID)
        {
            try
            {
                const QSharedPointer<VGObject> p = data->GetGObject(point);

                auto *item = new QListWidgetItem(p->name());
                item->setFont(NodeFont(item->font()));
                item->setData(Qt::UserRole, QVariant::fromValue(point));
                listWidget->addItem(item);
                listWidget->setCurrentRow(uiTabPins->listWidgetPins->count() - 1);
            }
            catch (const VExceptionBadId &e)
            {
                qWarning() << qUtf8Printable(e.ErrorMessage());
            }
        }
    };

    uiTabPins->listWidgetPins->clear();
    const QVector<quint32> pins = piece.GetPins();
    for (auto pin : pins)
    {
        NewSpecialPoint(uiTabPins->listWidgetPins, pin);
    }

    InitAllPinComboboxes();

    uiTabPlaceLabels->listWidgetPlaceLabels->clear();
    const QVector<quint32> labels = piece.GetPlaceLabels();
    for (auto label : labels)
    {
        NewSpecialPoint(uiTabPlaceLabels->listWidgetPlaceLabels, label);
    }

    if (!piece.GetPlaceLabels().empty())
    {
        uiTabPlaceLabels->listWidgetPlaceLabels->setCurrentRow(0);
    }

    {
        const QSignalBlocker blocker(uiTabPaths->comboBoxStartPoint);
        uiTabPaths->comboBoxStartPoint->clear();
    }

    {
        const QSignalBlocker blocker(uiTabPaths->comboBoxEndPoint);
        uiTabPaths->comboBoxEndPoint->clear();
    }

    CustomSAChanged(0);

    uiTabPaths->checkBoxForbidFlipping->setChecked(piece.IsForbidFlipping());
    uiTabPaths->checkBoxForceFlipping->setChecked(piece.IsForceFlipping());
    uiTabPaths->checkBoxSymmetricalCopy->setChecked(piece.IsSymmetricalCopy());
    uiTabPaths->checkBoxFollowGrainline->setChecked(piece.IsFollowGrainline());
    uiTabPaths->checkBoxOnDrawing->setChecked(piece.IsSewLineOnDrawing());
    uiTabPaths->checkBoxSeams->setChecked(piece.IsSeamAllowance());
    uiTabPaths->checkBoxBuiltIn->setChecked(piece.IsSeamAllowanceBuiltIn());
    uiTabPaths->lineEditName->setText(piece.GetName());
    uiTabPaths->lineEditUUID->setText(piece.GetUUID().toString());
    uiTabPaths->lineEditShortName->setText(piece.GetShortName());
    uiTabPaths->lineEditGradationLabel->setText(piece.GetGradationLabel());
    uiTabPaths->spinBoxPriority->setValue(static_cast<int>(piece.GetPriority()));

    InitFold(piece);

    uiTabPaths->checkBoxTrueZeroWidth->setChecked(piece.IsTrueZeroWidth());
    uiTabPaths->plainTextEditFormulaWidth->setPlainText(VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        piece.GetFormulaSAWidth(), VAbstractApplication::VApp()->Settings()->GetOsSeparator()));
    m_saWidth = piece.GetSAWidth();

    const VPieceLabelData &ppData = piece.GetPieceLabelData();
    uiTabLabels->lineEditLetter->setText(ppData.GetLetter());
    uiTabLabels->lineEditAnnotation->setText(ppData.GetAnnotation());
    uiTabLabels->lineEditOrientation->setText(ppData.GetOrientation());
    uiTabLabels->lineEditRotation->setText(ppData.GetRotationWay());
    uiTabLabels->lineEditTilt->setText(ppData.GetTilt());
    uiTabLabels->lineEditFoldPosition->setText(ppData.GetFoldPosition());
    uiTabLabels->spinBoxQuantity->setValue(ppData.GetQuantity());
    uiTabLabels->checkBoxFold->setChecked(ppData.IsOnFold());
    m_templateLines = ppData.GetLabelTemplate();

    {
        const int piceFontSizeIndex = uiTabLabels->comboBoxPieceLabelSize->findData(ppData.GetFontSize());
        uiTabLabels->comboBoxPieceLabelSize->setCurrentIndex(piceFontSizeIndex != -1 ? piceFontSizeIndex : 0);
    }

    uiTabLabels->groupBoxDetailLabel->setEnabled(not m_templateLines.isEmpty());

    int index = uiTabGrainline->comboBoxArrow->findData(static_cast<int>(piece.GetGrainlineGeometry().GetArrowType()));
    if (index == -1)
    {
        index = 0;
    }
    uiTabGrainline->comboBoxArrow->setCurrentIndex(index);

    uiTabLabels->groupBoxDetailLabel->setChecked(ppData.IsEnabled());
    ChangeCurrentData(uiTabLabels->comboBoxDLCenterPin, ppData.CenterPin());
    ChangeCurrentData(uiTabLabels->comboBoxDLTopLeftPin, ppData.TopLeftPin());
    ChangeCurrentData(uiTabLabels->comboBoxDLBottomRightPin, ppData.BottomRightPin());
    SetDLWidth(ppData.GetLabelWidth());
    SetDLHeight(ppData.GetLabelHeight());
    SetDLAngle(ppData.GetRotation());

    const VPatternLabelData &patternInfo = piece.GetPatternLabelData();
    uiTabLabels->groupBoxPatternLabel->setEnabled(not m_doc->GetPatternLabelTemplate().isEmpty());
    uiTabLabels->groupBoxPatternLabel->setChecked(patternInfo.IsEnabled());
    ChangeCurrentData(uiTabLabels->comboBoxPLCenterPin, patternInfo.CenterPin());
    ChangeCurrentData(uiTabLabels->comboBoxPLTopLeftPin, patternInfo.TopLeftPin());
    ChangeCurrentData(uiTabLabels->comboBoxPLBottomRightPin, patternInfo.BottomRightPin());
    SetPLWidth(patternInfo.GetLabelWidth());
    SetPLHeight(patternInfo.GetLabelHeight());
    SetPLAngle(patternInfo.GetRotation());

    {
        const int patternFontSizeIndex = uiTabLabels->comboBoxPatternLabelSize->findData(patternInfo.GetFontSize());
        uiTabLabels->comboBoxPatternLabelSize->setCurrentIndex(patternFontSizeIndex != -1 ? patternFontSizeIndex : 0);
    }

    const VGrainlineData &grainlineGeometry = piece.GetGrainlineGeometry();
    uiTabGrainline->groupBoxGrainline->setChecked(grainlineGeometry.IsEnabled());
    ChangeCurrentData(uiTabGrainline->comboBoxGrainlineCenterPin, grainlineGeometry.CenterPin());
    ChangeCurrentData(uiTabGrainline->comboBoxGrainlineTopPin, grainlineGeometry.TopPin());
    ChangeCurrentData(uiTabGrainline->comboBoxGrainlineBottomPin, grainlineGeometry.BottomPin());
    SetGrainlineAngle(grainlineGeometry.GetRotation());
    SetGrainlineLength(grainlineGeometry.GetLength());
    uiTabGrainline->checkBoxGrainlineVisible->setChecked(grainlineGeometry.IsVisible());

    ValidObjects(MainPathIsValid());
    EnabledGrainline();
    EnabledDetailLabel();
    EnabledPatternLabel();

    ListChanged();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChoosedObject gets id and type of selected object. Save right data and ignore wrong.
 * @param id id of objects (points, arcs, splines, spline paths)
 * @param type type of object
 */
void DialogSeamAllowance::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare)
    {
        return;
    }

    bool reverse = false;
    if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
    {
        reverse = true;
    }

    if (id != GetLastId())
    {
        switch (type)
        {
            case SceneObject::Arc:
                NewMainPathItem(VPieceNode(id, Tool::NodeArc, reverse));
                break;
            case SceneObject::ElArc:
                NewMainPathItem(VPieceNode(id, Tool::NodeElArc, reverse));
                break;
            case SceneObject::Point:
                NewMainPathItem(VPieceNode(id, Tool::NodePoint));
                break;
            case SceneObject::Spline:
                NewMainPathItem(VPieceNode(id, Tool::NodeSpline, reverse));
                break;
            case SceneObject::SplinePath:
                NewMainPathItem(VPieceNode(id, Tool::NodeSplinePath, reverse));
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
        if (uiTabPaths->listWidgetMainPath->count() > 1)
        {
            delete GetItemById(id);
        }
    }

    ValidObjects(MainPathIsValid());

    if (not applyAllowed)
    {
        auto *visPath = qobject_cast<VisToolPiece *>(vis);
        SCASSERT(visPath != nullptr);
        const VPiece p = CreatePiece();
        visPath->SetPiece(p);

        if (p.GetPath().CountNodes() == 1)
        {
            emit ToolTip(tr("Select main path objects clockwise, <b>%1</b> - reverse direction curve, "
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

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ShowDialog(bool click)
{
    if (!click && uiTabPaths->listWidgetMainPath->count() > 0)
    {
        emit ToolTip(QString());
        prepare = true;

        if (not applyAllowed)
        {
            auto *visPath = qobject_cast<VisToolPiece *>(vis);
            SCASSERT(visPath != nullptr);
            visPath->SetMode(Mode::Show);
            visPath->RefreshGeometry();
        }

        // Fix issue #526. Dialog Detail is not on top after selection second object on Mac.
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
        show();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSeamAllowance::eventFilter(QObject* obj, QEvent* event) -> bool
{
    if (obj == uiTabPaths->listWidgetMainPath && event->type() == QEvent::KeyPress)
    {
        if (const auto* keyEvent = static_cast<QKeyEvent*>(event); keyEvent->key() == Qt::Key_Delete)
        {
            if (QListWidgetItem* selectedItem = uiTabPaths->listWidgetMainPath->currentItem();selectedItem)
            {
                delete selectedItem;
            }
            return true; // Event is handled
        }
    }
    return DialogTool::eventFilter(obj, event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SaveData()
{
    SavePatternLabelData();
    SavePatternTemplateData();
    SavePatternMaterialData();

    if (m_doc != nullptr)
    {
        emit m_doc->UpdatePatternLabel();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::CheckTabPathsState()
{
    bool const isValid = flagFormula && flagFormulaBefore && flagFormulaAfter;
    bool const isMainPathValid = flagMainPathIsValid && flagMirrorLineIsValid;
    bool const isNameAndUUIDValid = flagName && flagUUID;

    m_ftb->SetTabText(TabOrder::Paths,
                      (isValid && flagMainPathIsValid && isNameAndUUIDValid) ? tr("Paths") : tr("Paths") + '*'_L1);

    const QIcon warningIcon = FromTheme(VThemeIcon::DialogWarning);

    uiTabPaths->tabWidget->setTabIcon(uiTabPaths->tabWidget->indexOf(uiTabPaths->tabSeamAllowance),
                                      isValid ? QIcon() : warningIcon);
    uiTabPaths->tabWidget->setTabIcon(uiTabPaths->tabWidget->indexOf(uiTabPaths->tabMainPath),
                                      isMainPathValid ? QIcon() : warningIcon);
    uiTabPaths->tabWidget->setTabIcon(uiTabPaths->tabWidget->indexOf(uiTabPaths->tabPiece),
                                      isNameAndUUIDValid ? QIcon() : warningIcon);

    if (flagMainPathIsValid && flagMirrorLineIsValid)
    {
        QString tooltip = tr("Ready!");
        if (!applyAllowed)
        {
            tooltip += "  <b>"_L1 +
                       tr("To open all detail's features complete creating the main path. Please, press OK.") +
                       "</b>"_L1;
        }
        uiTabPaths->helpLabel->setText(tooltip);
    }

    uiTabPaths->comboBoxNodes->setEnabled(flagFormulaBefore && flagFormulaAfter);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::CheckTabPassmarksState()
{
    if (uiTabPassmarks->comboBoxPassmarks->count() == 0)
    {
        flagFormulaPassmarkLength = true;
        flagFormulaPassmarkWidth = true;
        flagFormulaPassmarkAngle = true;
    }

    const bool allFormulasValid = flagFormulaPassmarkLength && flagFormulaPassmarkWidth && flagFormulaPassmarkAngle;

    m_ftb->SetTabText(TabOrder::Passmarks, tr("Passmarks") + (allFormulasValid ? QString() : QChar('*')));

    const QIcon icon = FromTheme(VThemeIcon::DialogWarning);
    uiTabPassmarks->tabWidget->setTabIcon(uiTabPassmarks->tabWidget->indexOf(uiTabPassmarks->tabManualShape),
                                          allFormulasValid ? QIcon() : icon);

    uiTabPassmarks->comboBoxPassmarks->setEnabled(allFormulasValid);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::CheckTabFoldLineState()
{
    if (!uiTabFoldLine->groupBoxManualHeight->isChecked())
    {
        flagFormulaFoldHeight = true;
    }

    if (!uiTabFoldLine->groupBoxManualWidth->isChecked())
    {
        flagFormulaFoldWidth = true;
    }

    if (!uiTabFoldLine->groupBoxManualCenter->isChecked())
    {
        flagFormulaFoldCenter = true;
    }

    const bool allFormulasValid = flagFormulaFoldHeight && flagFormulaFoldWidth && flagFormulaFoldCenter;

    m_ftb->SetTabText(TabOrder::FoldLine, tr("Fold line") + (allFormulasValid ? QString() : QChar('*')));

    const QIcon icon = FromTheme(VThemeIcon::DialogWarning);
    uiTabFoldLine->tabWidget->setTabIcon(uiTabFoldLine->tabWidget->indexOf(uiTabFoldLine->tabShape),
                                         allFormulasValid ? QIcon() : icon);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSeamAllowance::GetMirrorLineStartPoint() const -> quint32
{
    const quint32 startPoint = getCurrentObjectId(uiTabPaths->comboBoxMLStartPoint);
    const quint32 endPoint = getCurrentObjectId(uiTabPaths->comboBoxMLEndPoint);

    if (startPoint == endPoint || startPoint == NULL_ID || endPoint == NULL_ID)
    {
        return startPoint;
    }

    const int index = NodeRowIndex(uiTabPaths->listWidgetMainPath, startPoint);
    if (index == -1)
    {
        return startPoint;
    }

    int nextIndex = index + 1;
    if (nextIndex >= uiTabPaths->listWidgetMainPath->count())
    {
        nextIndex = 0;
    }

    if (const int next = FindNotExcludedNeighborNodeDown(uiTabPaths->listWidgetMainPath, nextIndex);
        next >= 0 && RowNode(uiTabPaths->listWidgetMainPath, next).GetId() == endPoint)
    {
        return startPoint;
    }

    return endPoint;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSeamAllowance::GetMirrorLineEndPoint() const -> quint32
{
    const quint32 startPoint = getCurrentObjectId(uiTabPaths->comboBoxMLStartPoint);
    const quint32 endPoint = getCurrentObjectId(uiTabPaths->comboBoxMLEndPoint);

    if (startPoint == endPoint || startPoint == NULL_ID || endPoint == NULL_ID)
    {
        return endPoint;
    }

    const int index = NodeRowIndex(uiTabPaths->listWidgetMainPath, endPoint);
    if (index == -1)
    {
        return endPoint;
    }

    int prevIndex = index - 1;
    if (prevIndex < 0)
    {
        prevIndex = uiTabPaths->listWidgetMainPath->count() - 1;
    }

    if (const int prev = FindNotExcludedNeighborNodeUp(uiTabPaths->listWidgetMainPath, prevIndex);
        prev >= 0 && RowNode(uiTabPaths->listWidgetMainPath, prev).GetId() == startPoint)
    {
        return endPoint;
    }

    return startPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitFold(const VPiece &piece)
{
    QLineF const mirrorLine = piece.SeamAllowanceMirrorLine(data);
    uiTabPaths->checkBoxShowFullPiece->setEnabled(not mirrorLine.isNull());
    uiTabPaths->checkBoxShowFullPiece->setChecked(true);
    if (not mirrorLine.isNull())
    {
        uiTabPaths->checkBoxShowFullPiece->setChecked(piece.IsShowFullPiece());
    }

    uiTabPaths->checkBoxShowMirrorLine->setChecked(piece.IsShowMirrorLine());

    InitMirrorLine();

    {
        const int index = uiTabPaths->comboBoxMLStartPoint->findData(piece.GetMirrorLineStartPoint());
        if (index != -1)
        {
            uiTabPaths->comboBoxMLStartPoint->setCurrentIndex(index);
        }
    }

    {
        const int index = uiTabPaths->comboBoxMLEndPoint->findData(piece.GetMirrorLineEndPoint());
        if (index != -1)
        {
            uiTabPaths->comboBoxMLEndPoint->setCurrentIndex(index);
        }
    }

    {
        const QSignalBlocker blockerManualHeight(uiTabFoldLine->groupBoxManualHeight);
        const QSignalBlocker blockerManualWidth(uiTabFoldLine->groupBoxManualWidth);
        const QSignalBlocker blockerManualCenter(uiTabFoldLine->groupBoxManualCenter);

        InitFoldHeightFormula(piece);
        InitFoldWidthFormula(piece);
        InitFoldCenterFormula(piece);
    }

    {
        const int index = uiTabFoldLine->comboBoxType->findData(static_cast<unsigned int>(piece.GetFoldLineType()));
        if (index != -1)
        {
            uiTabFoldLine->comboBoxType->setCurrentIndex(index);
        }
    }

    {
        const qint32 indexSize = uiTabFoldLine->comboBoxLabelFontSize->findData(piece.GetFoldLineSvgFontSize());
        if (indexSize != -1)
        {
            uiTabFoldLine->comboBoxLabelFontSize->setCurrentIndex(indexSize);
        }
    }

    uiTabFoldLine->toolButtonItalic->setChecked(piece.IsFoldLineLabelFontItalic());
    uiTabFoldLine->toolButtonBold->setChecked(piece.IsFoldLineLabelFontBold());
    uiTabFoldLine->lineEditLabel->setText(piece.GetFoldLineLabel());

    const int lineAlignment = piece.GetFoldLineLabelAlignment();

    if (lineAlignment == 0 || lineAlignment & Qt::AlignLeft)
    {
        uiTabFoldLine->toolButtonTextLeft->setChecked(true);
        uiTabFoldLine->toolButtonTextCenter->setChecked(false);
        uiTabFoldLine->toolButtonTextRight->setChecked(false);
    }
    else if (lineAlignment & Qt::AlignHCenter)
    {
        uiTabFoldLine->toolButtonTextLeft->setChecked(false);
        uiTabFoldLine->toolButtonTextCenter->setChecked(true);
        uiTabFoldLine->toolButtonTextRight->setChecked(false);
    }
    else if (lineAlignment & Qt::AlignRight)
    {
        uiTabFoldLine->toolButtonTextLeft->setChecked(false);
        uiTabFoldLine->toolButtonTextCenter->setChecked(false);
        uiTabFoldLine->toolButtonTextRight->setChecked(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitFoldHeightFormula(const VPiece &piece)
{
    if (piece.IsManualFoldHeight())
    {
        uiTabFoldLine->groupBoxManualHeight->setChecked(true);

        uiTabFoldLine->toolButtonExprHeight->setEnabled(uiTabFoldLine->groupBoxManualHeight->isChecked());
        uiTabFoldLine->plainTextEditHeight->setEnabled(uiTabFoldLine->groupBoxManualHeight->isChecked());
        uiTabFoldLine->pushButtonGrowHeight->setEnabled(uiTabFoldLine->groupBoxManualHeight->isChecked());
        uiTabFoldLine->labelEditHeight->setEnabled(uiTabFoldLine->groupBoxManualHeight->isChecked());
        uiTabFoldLine->label_3->setEnabled(uiTabFoldLine->groupBoxManualHeight->isChecked());

        QString foldHeight = piece.GetFormulaFoldHeight();
        foldHeight = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
            foldHeight, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
        if (foldHeight.length() > 80) // increase height if needed.
        {
            this->DeployFoldHeight();
        }

        uiTabFoldLine->plainTextEditHeight->setPlainText(foldHeight.isEmpty() ? QChar('0') : foldHeight);
    }
    else
    {
        uiTabFoldLine->plainTextEditHeight->setPlainText(QChar('0'));
    }

    MoveCursorToEnd(uiTabFoldLine->plainTextEditHeight);
    ChangeColor(uiTabFoldLine->labelEditHeight, OkColor(this));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitFoldWidthFormula(const VPiece &piece)
{
    if (piece.IsManualFoldWidth())
    {
        uiTabFoldLine->groupBoxManualWidth->setChecked(true);

        uiTabFoldLine->toolButtonExprWidth->setEnabled(uiTabFoldLine->groupBoxManualWidth->isChecked());
        uiTabFoldLine->plainTextEditWidth->setEnabled(uiTabFoldLine->groupBoxManualWidth->isChecked());
        uiTabFoldLine->pushButtonGrowWidth->setEnabled(uiTabFoldLine->groupBoxManualWidth->isChecked());
        uiTabFoldLine->labelEditWidth->setEnabled(uiTabFoldLine->groupBoxManualWidth->isChecked());
        uiTabFoldLine->label_4->setEnabled(uiTabFoldLine->groupBoxManualWidth->isChecked());

        QString foldWidth = piece.GetFormulaFoldWidth();
        foldWidth = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
            foldWidth, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
        if (foldWidth.length() > 80) // increase height if needed.
        {
            this->DeployPassmarkWidth();
        }

        uiTabFoldLine->plainTextEditWidth->setPlainText(foldWidth.isEmpty() ? QChar('0') : foldWidth);
    }
    else
    {
        uiTabFoldLine->plainTextEditWidth->setPlainText(QChar('0'));
    }

    MoveCursorToEnd(uiTabFoldLine->plainTextEditWidth);
    ChangeColor(uiTabFoldLine->labelEditWidth, OkColor(this));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitFoldCenterFormula(const VPiece &piece)
{
    const QString defCenterFormula = VTranslateVars::TryFormulaToUser(
        QString::number(0.5), VAbstractApplication::VApp()->Settings()->GetOsSeparator());

    if (piece.IsManualFoldCenter())
    {
        uiTabFoldLine->groupBoxManualCenter->setChecked(true);

        uiTabFoldLine->toolButtonExprCenter->setEnabled(uiTabFoldLine->groupBoxManualCenter->isChecked());
        uiTabFoldLine->plainTextEditCenter->setEnabled(uiTabFoldLine->groupBoxManualCenter->isChecked());
        uiTabFoldLine->pushButtonGrowCenter->setEnabled(uiTabFoldLine->groupBoxManualCenter->isChecked());
        uiTabFoldLine->labelEditCenter->setEnabled(uiTabFoldLine->groupBoxManualCenter->isChecked());
        uiTabFoldLine->label_5->setEnabled(uiTabFoldLine->groupBoxManualCenter->isChecked());

        QString foldCenter = piece.GetFormulaFoldCenter();
        foldCenter = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
            foldCenter, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
        if (foldCenter.length() > 80) // increase height if needed.
        {
            this->DeployPassmarkAngle();
        }

        uiTabFoldLine->plainTextEditCenter->setPlainText(foldCenter.isEmpty() ? defCenterFormula : foldCenter);
    }
    else
    {

        uiTabFoldLine->plainTextEditCenter->setPlainText(defCenterFormula);
    }

    MoveCursorToEnd(uiTabFoldLine->plainTextEditCenter);
    ChangeColor(uiTabFoldLine->labelEditCenter, OkColor(this));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::CheckState()
{
    CheckTabPathsState();
    CheckTabPassmarksState();
    CheckTabFoldLineState();

    SCASSERT(bOk != nullptr);
    bOk->setEnabled(IsValid());
    // In case dialog hasn't apply button
    if (bApply != nullptr && applyAllowed)
    {
        bApply->setEnabled(bOk->isEnabled());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::closeEvent(QCloseEvent *event)
{
    uiTabPaths->plainTextEditFormulaWidth->blockSignals(true);
    uiTabPaths->plainTextEditFormulaWidthBefore->blockSignals(true);
    uiTabPaths->plainTextEditFormulaWidthAfter->blockSignals(true);
    uiTabGrainline->lineEditRotFormula->blockSignals(true);
    uiTabGrainline->lineEditLenFormula->blockSignals(true);
    uiTabPassmarks->plainTextEditPassmarkLength->blockSignals(true);
    uiTabPassmarks->plainTextEditPassmarkWidth->blockSignals(true);
    uiTabPassmarks->plainTextEditPassmarkAngle->blockSignals(true);
    uiTabFoldLine->plainTextEditHeight->blockSignals(true);
    uiTabFoldLine->plainTextEditWidth->blockSignals(true);
    uiTabFoldLine->plainTextEditCenter->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event); // clazy:exclude=skipped-base-method NOLINT(bugprone-parent-virtual-call)
    if (event->spontaneous())
    {
        return;
    }

    if (isInitialized)
    {
        return;
    }
    // do your init stuff here

    if (m_doc != nullptr)
    {
        uiTabLabels->lineEditPatternName->setText(m_doc->GetPatternName());
        uiTabLabels->lineEditPatternNumber->setText(m_doc->GetPatternNumber());
        uiTabLabels->lineEditCompanyName->setText(m_doc->GetCompanyName());

        VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
        m_patternMaterials = m_doc->GetPatternMaterials();

        InitComboBoxFormats(uiTabLabels->comboBoxDateFormat,
                            VCommonSettings::PredefinedDateFormats() + settings->GetUserDefinedDateFormats(),
                            m_doc->GetLabelDateFormat());
        InitComboBoxFormats(uiTabLabels->comboBoxTimeFormat,
                            VCommonSettings::PredefinedTimeFormats() + settings->GetUserDefinedTimeFormats(),
                            m_doc->GetLabelTimeFormat());
    }

    const QSize sz = VAbstractApplication::VApp()->Settings()->GetToolSeamAllowanceDialogSize();
    if (not sz.isEmpty())
    {
        resize(sz);
    }

    isInitialized = true; // first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::resizeEvent(QResizeEvent *event)
{
    // remember the size for the next time this dialog is opened, but only
    // if widget was already initialized, which rules out the resize at
    // dialog creating, which would
    if (isInitialized)
    {
        VAbstractApplication::VApp()->Settings()->SetToolSeamAllowanceDialogSize(size());
    }
    DialogTool::resizeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::changeEvent(QEvent *event)
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
void DialogSeamAllowance::SetPatternDoc(VAbstractPattern *doc)
{
    SCASSERT(doc != nullptr)
    m_doc = doc;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::CheckDependencyTreeComplete()
{
    // does nothing
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::NameDetailChanged()
{
    if (const auto *edit = qobject_cast<QLineEdit *>(sender());edit)
    {
        if (edit->text().isEmpty())
        {
            flagName = false;
            ChangeColor(uiTabPaths->labelEditName, errorColor);
        }
        else
        {
            flagName = true;
            ChangeColor(uiTabPaths->labelEditName, OkColor(this));
        }
    }
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DetailUUIDChanged()
{
    if (const auto *edit = qobject_cast<QLineEdit *>(sender());edit)
    {
        static QRegularExpression const re("^$|^{[0-9a-fA-F]{8}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{4}\\-"
                                           "[0-9a-fA-F]{12}}$");
        QRegularExpressionMatch const match = re.match(edit->text());

        if (match.hasMatch())
        {
            flagUUID = true;
            ChangeColor(uiTabPaths->labelEditUUID, OkColor(this));
        }
        else
        {
            flagUUID = false;
            ChangeColor(uiTabPaths->labelEditUUID, errorColor);
        }
    }
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ShowMainPathContextMenu(const QPoint &pos)
{
    const int row = uiTabPaths->listWidgetMainPath->currentRow();
    if (uiTabPaths->listWidgetMainPath->count() == 0 || row == -1 || row >= uiTabPaths->listWidgetMainPath->count())
    {
        return;
    }

    QListWidgetItem *rowItem = uiTabPaths->listWidgetMainPath->item(row);
    SCASSERT(rowItem != nullptr)
    auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

    QMenu menu;
    QHash<int, QAction *> const contextMenu = InitMainPathContextMenu(&menu, rowNode);

    QAction *selectedAction = menu.exec(uiTabPaths->listWidgetMainPath->viewport()->mapToGlobal(pos));
    auto selectedOption = static_cast<MainPathContextMenuOption>(
        contextMenu.key(selectedAction, static_cast<int>(MainPathContextMenuOption::NoSelection)));

    auto SelectPassmarkLineType = [this, &rowNode, rowItem](PassmarkLineType type)
    {
        rowNode.SetPassmark(true);
        rowNode.SetPassmarkLineType(type);
        rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        rowItem->setText(GetNodeName(data, rowNode, true));
    };

    Q_STATIC_ASSERT_X(static_cast<int>(MainPathContextMenuOption::LAST_ONE_DO_NOT_USE) == 17,
                      "Not all options were handled.");

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wswitch-default")
    QT_WARNING_DISABLE_CLANG("-Wswitch-default")

    switch (selectedOption)
    {
        case MainPathContextMenuOption::NoSelection:
            return;
        case MainPathContextMenuOption::Reverse:
            rowNode.SetReverse(not rowNode.GetReverse());
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
            rowItem->setText(GetNodeName(data, rowNode, true));
            break;
        case MainPathContextMenuOption::NonePassmark:
            rowNode.SetPassmark(false);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
            rowItem->setText(GetNodeName(data, rowNode, true));
            break;
        case MainPathContextMenuOption::OneLine:
            SelectPassmarkLineType(PassmarkLineType::OneLine);
            break;
        case MainPathContextMenuOption::TwoLines:
            SelectPassmarkLineType(PassmarkLineType::TwoLines);
            break;
        case MainPathContextMenuOption::ThreeLines:
            SelectPassmarkLineType(PassmarkLineType::ThreeLines);
            break;
        case MainPathContextMenuOption::TMark:
            SelectPassmarkLineType(PassmarkLineType::TMark);
            break;
        case MainPathContextMenuOption::ExternalVMark:
            SelectPassmarkLineType(PassmarkLineType::ExternalVMark);
            break;
        case MainPathContextMenuOption::InternalVMark:
            SelectPassmarkLineType(PassmarkLineType::InternalVMark);
            break;
        case MainPathContextMenuOption::UMark:
            SelectPassmarkLineType(PassmarkLineType::UMark);
            break;
        case MainPathContextMenuOption::BoxMark:
            SelectPassmarkLineType(PassmarkLineType::BoxMark);
            break;
        case MainPathContextMenuOption::CheckMark:
            SelectPassmarkLineType(PassmarkLineType::CheckMark);
            break;
        case MainPathContextMenuOption::Uniqueness:
            rowNode.SetCheckUniqueness(not rowNode.IsCheckUniqueness());
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
            rowItem->setText(GetNodeName(data, rowNode, true));
            break;
        case MainPathContextMenuOption::TurnPoint:
            rowNode.SetTurnPoint(not rowNode.IsTurnPoint());
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
            rowItem->setText(GetNodeName(data, rowNode, true));
            break;
        case MainPathContextMenuOption::Excluded:
            rowNode.SetExcluded(not rowNode.IsExcluded());
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
            rowItem->setText(GetNodeName(data, rowNode, true));
            rowItem->setFont(NodeFont(rowItem->font(), rowNode.IsExcluded()));
            break;
        case MainPathContextMenuOption::DuplicateNode:
        {
            VPieceNode duplicate = rowNode;
            duplicate.SetExcluded(true);

            auto *item = new QListWidgetItem(GetNodeName(data, duplicate, true));
            item->setFont(NodeFont(item->font(), true));
            item->setData(Qt::UserRole, QVariant::fromValue(duplicate));
            uiTabPaths->listWidgetMainPath->addItem(item);
            break;
        }
        case MainPathContextMenuOption::Delete:
            delete uiTabPaths->listWidgetMainPath->item(row);
            break;
        case MainPathContextMenuOption::LAST_ONE_DO_NOT_USE:
            Q_UNREACHABLE();
            break;
    };

    QT_WARNING_POP

    ValidObjects(MainPathIsValid());
    ListChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ShowCustomSAContextMenu(const QPoint &pos)
{
    const int row = uiTabPaths->listWidgetCustomSA->currentRow();
    if (uiTabPaths->listWidgetCustomSA->count() == 0 || row == -1 || row >= uiTabPaths->listWidgetCustomSA->count())
    {
        return;
    }

    QScopedPointer<QMenu> const menu(new QMenu());
    QAction *actionOption = menu->addAction(FromTheme(VThemeIcon::PreferencesOther), tr("Options"));

    QListWidgetItem *rowItem = uiTabPaths->listWidgetCustomSA->item(row);
    SCASSERT(rowItem != nullptr)
    auto record = qvariant_cast<CustomSARecord>(rowItem->data(Qt::UserRole));

    QAction *actionReverse = menu->addAction(tr("Reverse"));
    actionReverse->setCheckable(true);
    actionReverse->setChecked(record.reverse);

    QAction *actionDelete = menu->addAction(FromTheme(VThemeIcon::EditDelete), tr("Delete"));

    QAction *selectedAction = menu->exec(uiTabPaths->listWidgetCustomSA->viewport()->mapToGlobal(pos));
    if (selectedAction == actionDelete)
    {
        delete uiTabPaths->listWidgetCustomSA->item(row);
    }
    else if (selectedAction == actionReverse)
    {
        record.reverse = not record.reverse;
        rowItem->setData(Qt::UserRole, QVariant::fromValue(record));
        rowItem->setText(GetPathName(record.path, record.reverse));
    }
    else if (selectedAction == actionOption)
    {
        auto *dialog = new DialogPiecePath(data, m_doc, record.path, this);
        dialog->SetPiecePath(CurrentPath(record.path));
        dialog->SetPieceId(toolId);
        if (record.includeType == PiecePathIncludeType::AsMainPath)
        {
            dialog->SetFormulaSAWidth(GetFormulaSAWidth());
        }
        dialog->HideVisibilityTrigger();
        dialog->EnbleShowMode(true);
        m_dialog = dialog;
        m_dialog->setModal(true);
        connect(m_dialog.data(), &DialogTool::DialogClosed, this, &DialogSeamAllowance::PathDialogClosed);
        m_dialog->show();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ShowInternalPathsContextMenu(const QPoint &pos)
{
    const int row = uiTabPaths->listWidgetInternalPaths->currentRow();
    if (uiTabPaths->listWidgetInternalPaths->count() == 0 || row == -1 ||
        row >= uiTabPaths->listWidgetInternalPaths->count())
    {
        return;
    }

    QScopedPointer<QMenu> const menu(new QMenu());
    QAction *actionOption = menu->addAction(FromTheme(VThemeIcon::PreferencesOther), tr("Options"));
    QAction *actionDelete = menu->addAction(FromTheme(VThemeIcon::EditDelete), tr("Delete"));

    QAction *selectedAction = menu->exec(uiTabPaths->listWidgetInternalPaths->viewport()->mapToGlobal(pos));
    if (selectedAction == actionDelete)
    {
        delete uiTabPaths->listWidgetInternalPaths->item(row);
    }
    else if (selectedAction == actionOption)
    {
        QListWidgetItem *rowItem = uiTabPaths->listWidgetInternalPaths->item(row);
        SCASSERT(rowItem != nullptr)
        const auto pathId = qvariant_cast<quint32>(rowItem->data(Qt::UserRole));

        auto *dialog = new DialogPiecePath(data, m_doc, pathId, this);
        dialog->SetPiecePath(CurrentPath(pathId));
        dialog->SetPieceId(toolId);
        dialog->EnbleShowMode(true);
        m_dialog = dialog;
        m_dialog->setModal(true);
        connect(m_dialog.data(), &DialogTool::DialogClosed, this, &DialogSeamAllowance::PathDialogClosed);
        m_dialog->show();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ShowPinsContextMenu(const QPoint &pos)
{
    const int row = uiTabPins->listWidgetPins->currentRow();
    if (uiTabPins->listWidgetPins->count() == 0 || row == -1 || row >= uiTabPins->listWidgetPins->count())
    {
        return;
    }

    QScopedPointer<QMenu> const menu(new QMenu());
    QAction *actionDelete = menu->addAction(FromTheme(VThemeIcon::EditDelete), tr("Delete"));

    QAction *selectedAction = menu->exec(uiTabPins->listWidgetPins->viewport()->mapToGlobal(pos));
    if (selectedAction == actionDelete)
    {
        delete uiTabPins->listWidgetPins->item(row);
        FancyTabChanged(m_ftb->CurrentIndex());
        InitAllPinComboboxes();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ShowPlaceLabelsContextMenu(const QPoint &pos)
{
    const int row = uiTabPlaceLabels->listWidgetPlaceLabels->currentRow();
    if (uiTabPlaceLabels->listWidgetPlaceLabels->count() == 0 || row == -1 ||
        row >= uiTabPlaceLabels->listWidgetPlaceLabels->count())
    {
        return;
    }

    QListWidgetItem *rowItem = uiTabPlaceLabels->listWidgetPlaceLabels->item(row);
    SCASSERT(rowItem != nullptr)
    const auto labelId = qvariant_cast<quint32>(rowItem->data(Qt::UserRole));
    VPlaceLabelItem currentLabel = CurrentPlaceLabel(labelId);

    QScopedPointer<QMenu> menu(new QMenu());

    auto InitAction = [&currentLabel, &menu](const QString &text, PlaceLabelType type)
    {
        QAction *action = menu->addAction(text);
        action->setCheckable(true);
        action->setChecked(currentLabel.GetLabelType() == type);
        return action;
    };

    auto SaveType = [this, &currentLabel, labelId](PlaceLabelType type)
    {
        auto newLabel = VPlaceLabelItem(currentLabel);
        newLabel.SetLabelType(type);
        m_newPlaceLabels.insert(labelId, newLabel);

        QPointer<VUndoCommand> const saveCommand = new SavePlaceLabelOptions(
            toolId, currentLabel, newLabel, VAbstractValApplication::VApp()->getCurrentDocument(),
            const_cast<VContainer *>(data), labelId);
        m_undoStack.append(saveCommand);
        UpdateCurrentPlaceLabelRecords();
    };

    QAction *actionOption = menu->addAction(FromTheme(VThemeIcon::PreferencesOther), tr("Options"));
    menu->addSeparator();
    QAction *actionSegment = InitAction(tr("Segment"), PlaceLabelType::Segment);
    QAction *actionRectangle = InitAction(tr("Rectangle"), PlaceLabelType::Rectangle);
    QAction *actionCross = InitAction(tr("Cross"), PlaceLabelType::Cross);
    QAction *actionTshaped = InitAction(tr("T-shaped"), PlaceLabelType::Tshaped);
    QAction *actionDoubletree = InitAction(tr("Doubletree"), PlaceLabelType::Doubletree);
    QAction *actionCorner = InitAction(tr("Corner"), PlaceLabelType::Corner);
    QAction *actionTriangle = InitAction(tr("Triangle"), PlaceLabelType::Triangle);
    QAction *actionHshaped = InitAction(tr("H-shaped"), PlaceLabelType::Hshaped);
    QAction *actionButton = InitAction(tr("Button"), PlaceLabelType::Button);
    QAction *actionCircle = InitAction(tr("Circle"), PlaceLabelType::Circle);
    menu->addSeparator();
    QAction *actionDelete = menu->addAction(FromTheme(VThemeIcon::EditDelete), tr("Delete"));

    QAction *selectedAction = menu->exec(uiTabPlaceLabels->listWidgetPlaceLabels->viewport()->mapToGlobal(pos));
    if (selectedAction == actionDelete)
    {
        delete uiTabPlaceLabels->listWidgetPlaceLabels->item(row);
        FancyTabChanged(m_ftb->CurrentIndex());
    }
    else if (selectedAction == actionOption)
    {
        auto *dialog = new DialogPlaceLabel(data, m_doc, labelId, this);
        dialog->EnbleShowMode(true);
        dialog->SetLabelType(currentLabel.GetLabelType());
        dialog->SetWidth(currentLabel.GetWidthFormula());
        dialog->SetHeight(currentLabel.GetHeightFormula());
        dialog->SetAngle(currentLabel.GetAngleFormula());
        dialog->SetFormulaVisible(currentLabel.GetVisibilityTrigger());
        dialog->SetPieceId(toolId);
        dialog->SetCenterPoint(currentLabel.GetCenterPoint());
        dialog->SetNotMirrored(currentLabel.IsNotMirrored());
        m_dialog = dialog;
        m_dialog->setModal(true);
        connect(m_dialog.data(), &DialogTool::DialogClosed, this, &DialogSeamAllowance::PlaceLabelDialogClosed);
        m_dialog->show();
    }
    else if (selectedAction == actionSegment)
    {
        SaveType(PlaceLabelType::Segment);
    }
    else if (selectedAction == actionRectangle)
    {
        SaveType(PlaceLabelType::Rectangle);
    }
    else if (selectedAction == actionCross)
    {
        SaveType(PlaceLabelType::Cross);
    }
    else if (selectedAction == actionTshaped)
    {
        SaveType(PlaceLabelType::Tshaped);
    }
    else if (selectedAction == actionDoubletree)
    {
        SaveType(PlaceLabelType::Doubletree);
    }
    else if (selectedAction == actionCorner)
    {
        SaveType(PlaceLabelType::Corner);
    }
    else if (selectedAction == actionTriangle)
    {
        SaveType(PlaceLabelType::Triangle);
    }
    else if (selectedAction == actionHshaped)
    {
        SaveType(PlaceLabelType::Hshaped);
    }
    else if (selectedAction == actionButton)
    {
        SaveType(PlaceLabelType::Button);
    }
    else if (selectedAction == actionCircle)
    {
        SaveType(PlaceLabelType::Circle);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ListChanged()
{
    if (not applyAllowed)
    {
        auto *visPath = qobject_cast<VisToolPiece *>(vis);
        SCASSERT(visPath != nullptr);
        visPath->SetPiece(CreatePiece());
        visPath->RefreshGeometry();
    }
    InitNodesList();
    InitPassmarksList();
    InitMirrorLine();
    CustomSAChanged(uiTabPaths->listWidgetCustomSA->currentRow());
    SetMoveControls();
    SetOptionControls();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::NodeChanged(int index)
{
    uiTabPaths->plainTextEditFormulaWidthBefore->setDisabled(true);
    uiTabPaths->toolButtonExprBefore->setDisabled(true);
    uiTabPaths->pushButtonDefBefore->setDisabled(true);

    uiTabPaths->plainTextEditFormulaWidthAfter->setDisabled(true);
    uiTabPaths->toolButtonExprAfter->setDisabled(true);
    uiTabPaths->pushButtonDefAfter->setDisabled(true);

    uiTabPaths->comboBoxAngle->setDisabled(true);

    const QSignalBlocker blocker(uiTabPaths->comboBoxAngle);

    if (index != -1)
    {
        const VPiece piece = CreatePiece();
        const int nodeIndex = piece.GetPath().indexOfNode(uiTabPaths->comboBoxNodes->currentData().toUInt());
        if (nodeIndex != -1)
        {
            const VPiecePath &path = piece.GetPath();
            const VPieceNode &node = path.at(nodeIndex);

            // Seam alowance before
            uiTabPaths->plainTextEditFormulaWidthBefore->setEnabled(true);
            uiTabPaths->toolButtonExprBefore->setEnabled(true);

            QString w1Formula = node.GetFormulaSABefore();
            EnableDefButton(uiTabPaths->pushButtonDefBefore, w1Formula);
            w1Formula = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
                w1Formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
            if (w1Formula.length() > 80) // increase height if needed.
            {
                this->DeployWidthBeforeFormulaTextEdit();
            }
            uiTabPaths->plainTextEditFormulaWidthBefore->setPlainText(w1Formula);
            MoveCursorToEnd(uiTabPaths->plainTextEditFormulaWidthBefore);

            // Seam alowance after
            uiTabPaths->plainTextEditFormulaWidthAfter->setEnabled(true);
            uiTabPaths->toolButtonExprAfter->setEnabled(true);

            QString w2Formula = node.GetFormulaSAAfter();
            EnableDefButton(uiTabPaths->pushButtonDefAfter, w2Formula);
            w2Formula = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
                w2Formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
            if (w2Formula.length() > 80) // increase height if needed.
            {
                this->DeployWidthAfterFormulaTextEdit();
            }
            uiTabPaths->plainTextEditFormulaWidthAfter->setPlainText(w2Formula);
            MoveCursorToEnd(uiTabPaths->plainTextEditFormulaWidthAfter);

            // Angle type
            uiTabPaths->comboBoxAngle->setEnabled(true);
            const int index = uiTabPaths->comboBoxAngle->findData(static_cast<unsigned char>(node.GetAngleType()));
            if (index != -1)
            {
                uiTabPaths->comboBoxAngle->setCurrentIndex(index);
            }
        }
    }
    else
    {
        uiTabPaths->plainTextEditFormulaWidthBefore->setPlainText(currentSeamAllowance);
        uiTabPaths->plainTextEditFormulaWidthAfter->setPlainText(currentSeamAllowance);
        uiTabPaths->comboBoxAngle->setCurrentIndex(-1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::PassmarkChanged(int index)
{
    uiTabPassmarks->groupBoxMarkType->setDisabled(true);
    uiTabPassmarks->groupBoxAngleType->setDisabled(true);
    uiTabPassmarks->groupBoxManualLength->setDisabled(true);
    uiTabPassmarks->groupBoxManualWidth->setDisabled(true);
    uiTabPassmarks->groupBoxManualAngle->setDisabled(true);

    uiTabPassmarks->labelEditPassmarkLength->setDisabled(true);
    uiTabPassmarks->labelEditPassmarkWidth->setDisabled(true);
    uiTabPassmarks->labelEditPassmarkAngle->setDisabled(true);

    uiTabPassmarks->checkBoxClockwiseOpening->setDisabled(true);
    uiTabPassmarks->checkBoxNotMirrored->setDisabled(true);
    uiTabPassmarks->checkBoxShowSecondPassmark->setDisabled(true);

    const QSignalBlocker blockerNotMirrored(uiTabPassmarks->checkBoxNotMirrored);
    const QSignalBlocker blockerClockwiseOpening(uiTabPassmarks->checkBoxClockwiseOpening);
    const QSignalBlocker blockerShowSecondPassmark(uiTabPassmarks->checkBoxShowSecondPassmark);
    const QSignalBlocker blockerManualLength(uiTabPassmarks->groupBoxManualLength);
    const QSignalBlocker blockerManualWidth(uiTabPassmarks->groupBoxManualWidth);
    const QSignalBlocker blockerManualAngle(uiTabPassmarks->groupBoxManualAngle);
    const QSignalBlocker blockerMarkType(uiTabPassmarks->groupBoxMarkType);
    const QSignalBlocker blockerAngleType(uiTabPassmarks->groupBoxAngleType);

    uiTabPassmarks->checkBoxClockwiseOpening->setChecked(false);
    uiTabPassmarks->checkBoxNotMirrored->setChecked(false);

    uiTabPassmarks->groupBoxManualLength->setChecked(false);
    uiTabPassmarks->groupBoxManualWidth->setChecked(false);
    uiTabPassmarks->groupBoxManualAngle->setChecked(false);

    if (index == -1)
    {
        return;
    }

    const VPiece piece = CreatePiece();
    const int nodeIndex = piece.GetPath().indexOfNode(uiTabPassmarks->comboBoxPassmarks->currentData().toUInt());
    if (nodeIndex == -1)
    {
        return;
    }

    const VPiecePath &path = piece.GetPath();
    const VPieceNode &node = path.at(nodeIndex);

    InitPassmarkLengthFormula(node);
    InitPassmarkWidthFormula(node);
    InitPassmarkAngleFormula(node);
    InitPassmarkShapeType(node);
    InitPassmarkAngleType(node);

    uiTabPassmarks->checkBoxNotMirrored->setEnabled(true);
    uiTabPassmarks->checkBoxNotMirrored->setChecked(node.IsPassmarkNotMirrored());

    if (node.GetPassmarkLineType() == PassmarkLineType::CheckMark)
    {
        uiTabPassmarks->checkBoxClockwiseOpening->setEnabled(true);
        uiTabPassmarks->checkBoxClockwiseOpening->setChecked(node.IsPassmarkClockwiseOpening());
    }

    // Show the second option
    uiTabPassmarks->checkBoxShowSecondPassmark->setEnabled(true);
    uiTabPassmarks->checkBoxShowSecondPassmark->setChecked(node.IsShowSecondPassmark());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::CSAStartPointChanged(int index)
{
    Q_UNUSED(index);

    const int row = uiTabPaths->listWidgetCustomSA->currentRow();
    if (uiTabPaths->listWidgetCustomSA->count() == 0 || row == -1 || row >= uiTabPaths->listWidgetCustomSA->count())
    {
        return;
    }

    QListWidgetItem *rowItem = uiTabPaths->listWidgetCustomSA->item(row);
    SCASSERT(rowItem != nullptr)
    auto record = qvariant_cast<CustomSARecord>(rowItem->data(Qt::UserRole));
    record.startPoint = uiTabPaths->comboBoxStartPoint->currentData().toUInt();
    rowItem->setData(Qt::UserRole, QVariant::fromValue(record));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::CSAEndPointChanged(int index)
{
    Q_UNUSED(index);

    const int row = uiTabPaths->listWidgetCustomSA->currentRow();
    if (uiTabPaths->listWidgetCustomSA->count() == 0 || row == -1 || row >= uiTabPaths->listWidgetCustomSA->count())
    {
        return;
    }

    QListWidgetItem *rowItem = uiTabPaths->listWidgetCustomSA->item(row);
    SCASSERT(rowItem != nullptr)
    auto record = qvariant_cast<CustomSARecord>(rowItem->data(Qt::UserRole));
    record.endPoint = uiTabPaths->comboBoxEndPoint->currentData().toUInt();
    rowItem->setData(Qt::UserRole, QVariant::fromValue(record));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::MirrorLinePointChanged(int index)
{
    Q_UNUSED(index)

    flagMirrorLineIsValid = MirrorLineIsValid();
    uiTabPaths->checkBoxShowFullPiece->setEnabled(flagMirrorLineIsValid);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::CSAIncludeTypeChanged(int index)
{
    Q_UNUSED(index);

    const int row = uiTabPaths->listWidgetCustomSA->currentRow();
    if (uiTabPaths->listWidgetCustomSA->count() == 0 || row == -1 || row >= uiTabPaths->listWidgetCustomSA->count())
    {
        return;
    }

    QListWidgetItem *rowItem = uiTabPaths->listWidgetCustomSA->item(row);
    SCASSERT(rowItem != nullptr)
    auto record = qvariant_cast<CustomSARecord>(rowItem->data(Qt::UserRole));
    record.includeType = static_cast<PiecePathIncludeType>(uiTabPaths->comboBoxIncludeType->currentData().toUInt());
    rowItem->setData(Qt::UserRole, QVariant::fromValue(record));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::NodeAngleChanged(int index)
{
    const int i = uiTabPaths->comboBoxNodes->currentIndex();
    if (i != -1 && index != -1)
    {
        QListWidgetItem *rowItem = GetItemById(uiTabPaths->comboBoxNodes->currentData().toUInt());
        if (rowItem)
        {
            auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetAngleType(static_cast<PieceNodeAngle>(uiTabPaths->comboBoxAngle->currentData().toUInt()));
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

            ListChanged();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ReturnDefBefore()
{
    const QString def = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        currentSeamAllowance, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    uiTabPaths->plainTextEditFormulaWidthBefore->setPlainText(def);
    if (auto *button = qobject_cast<QPushButton *>(sender()))
    {
        button->setEnabled(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ReturnDefAfter()
{
    const QString def = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        currentSeamAllowance, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    uiTabPaths->plainTextEditFormulaWidthAfter->setPlainText(def);
    if (auto *button = qobject_cast<QPushButton *>(sender()))
    {
        button->setEnabled(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::CustomSAChanged(int row)
{
    if (uiTabPaths->listWidgetCustomSA->count() == 0 || row == -1 || row >= uiTabPaths->listWidgetCustomSA->count())
    {
        {
            const QSignalBlocker blocker(uiTabPaths->comboBoxStartPoint);
            uiTabPaths->comboBoxStartPoint->clear();
        }

        {
            const QSignalBlocker blocker(uiTabPaths->comboBoxEndPoint);
            uiTabPaths->comboBoxEndPoint->clear();
        }

        const QSignalBlocker blocker(uiTabPaths->comboBoxIncludeType);
        uiTabPaths->comboBoxIncludeType->clear();
        return;
    }

    const QListWidgetItem *item = uiTabPaths->listWidgetCustomSA->item(row);
    SCASSERT(item != nullptr);
    const auto record = qvariant_cast<CustomSARecord>(item->data(Qt::UserRole));

    {
        const QSignalBlocker blocker(uiTabPaths->comboBoxStartPoint);
        InitCSAPoint(uiTabPaths->comboBoxStartPoint);
        {
            const int index = uiTabPaths->comboBoxStartPoint->findData(record.startPoint);
            if (index != -1)
            {
                uiTabPaths->comboBoxStartPoint->setCurrentIndex(index);
            }
        }
    }

    {
        const QSignalBlocker blocker(uiTabPaths->comboBoxEndPoint);
        InitCSAPoint(uiTabPaths->comboBoxEndPoint);
        {
            const int index = uiTabPaths->comboBoxEndPoint->findData(record.endPoint);
            if (index != -1)
            {
                uiTabPaths->comboBoxEndPoint->setCurrentIndex(index);
            }
        }
    }

    const QSignalBlocker blocker(uiTabPaths->comboBoxIncludeType);
    InitSAIncludeType();
    {
        const int index = uiTabPaths->comboBoxIncludeType->findData(static_cast<unsigned char>(record.includeType));
        if (index != -1)
        {
            uiTabPaths->comboBoxIncludeType->setCurrentIndex(index);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::PathDialogClosed(int result)
{
    if (result == QDialog::Accepted)
    {
        SCASSERT(not m_dialog.isNull());
        auto *dialogTool = qobject_cast<DialogPiecePath *>(m_dialog.data());
        SCASSERT(dialogTool != nullptr);
        try
        {
            VPiecePath const currentPath = CurrentPath(dialogTool->GetToolId());
            VPiecePath const newPath = dialogTool->GetPiecePath();
            m_newPaths.insert(dialogTool->GetToolId(), newPath);

            QPointer<VUndoCommand> const saveCommand = new SavePiecePathOptions(
                toolId, currentPath, newPath, VAbstractValApplication::VApp()->getCurrentDocument(),
                const_cast<VContainer *>(data), dialogTool->GetToolId());
            m_undoStack.append(saveCommand);
            UpdateCurrentCustomSARecord();
            UpdateCurrentInternalPathRecord();
        }
        catch (const VExceptionBadId &e)
        {
            qCritical("%s\n\n%s\n\n%s", qUtf8Printable(tr("Error. Can't save piece path.")),
                      qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        }
    }
    delete m_dialog.data();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::PlaceLabelDialogClosed(int result)
{
    if (result == QDialog::Accepted)
    {
        SCASSERT(not m_dialog.isNull());
        auto *dialogTool = qobject_cast<DialogPlaceLabel *>(m_dialog.data());
        SCASSERT(dialogTool != nullptr);
        try
        {
            VPlaceLabelItem const currentLabel = CurrentPlaceLabel(dialogTool->GetToolId());

            const QHash<QString, QSharedPointer<VInternalVariable>> *vars = data->DataVariables();

            const qreal w = qAbs(Visualization::FindLengthFromUser(dialogTool->GetWidth(), vars, false));
            const qreal h = qAbs(Visualization::FindLengthFromUser(dialogTool->GetHeight(), vars, false));
            const qreal a = Visualization::FindValFromUser(dialogTool->GetAngle(), vars, false);
            const qreal v = Visualization::FindValFromUser(dialogTool->GetFormulaVisible(), vars, false);
            qDebug() << w << h << a << v;

            auto newLabel = VPlaceLabelItem();
            newLabel.setName(currentLabel.name());
            newLabel.setX(currentLabel.x());
            newLabel.setY(currentLabel.y());
            newLabel.setMx(currentLabel.mx());
            newLabel.setMy(currentLabel.my());
            newLabel.SetWidth(w, dialogTool->GetWidth());
            newLabel.SetHeight(h, dialogTool->GetHeight());
            newLabel.SetAngle(a, dialogTool->GetAngle());
            newLabel.SetVisibilityTrigger(v, dialogTool->GetFormulaVisible());
            newLabel.SetLabelType(dialogTool->GetLabelType());
            newLabel.SetCenterPoint(currentLabel.GetCenterPoint());
            newLabel.SetCorrectionAngle(currentLabel.GetCorrectionAngle());
            newLabel.SetNotMirrored(dialogTool->IsNotMirrored());

            m_newPlaceLabels.insert(dialogTool->GetToolId(), newLabel);

            QPointer<VUndoCommand> const saveCommand = new SavePlaceLabelOptions(
                toolId, currentLabel, newLabel, VAbstractValApplication::VApp()->getCurrentDocument(),
                const_cast<VContainer *>(data), dialogTool->GetToolId());
            m_undoStack.append(saveCommand);
            UpdateCurrentPlaceLabelRecords();
        }
        catch (const VExceptionBadId &e)
        {
            qCritical("%s\n\n%s\n\n%s", qUtf8Printable(tr("Error. Can't save piece path.")),
                      qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        }
    }
    delete m_dialog.data();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::FancyTabChanged(int index)
{
    if (index < 0 || index >= TabOrder::Count)
    {
        return;
    }

    m_tabPaths->hide();
    m_tabLabels->hide();
    m_tabGrainline->hide();
    m_tabPins->hide();
    m_tabPassmarks->hide();
    m_tabPlaceLabels->hide();
    m_tabFoldLine->hide();

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wswitch-default")
    QT_WARNING_DISABLE_CLANG("-Wswitch-default")

    switch (index) // NOLINT(hicpp-multiway-paths-covered)
    {
        case TabOrder::Paths:
            m_tabPaths->show();
            break;
        case TabOrder::Pins:
            m_tabPins->show();
            break;
        case TabOrder::Labels:
            m_tabLabels->show();
            break;
        case TabOrder::Grainline:
            m_tabGrainline->show();
            break;
        case TabOrder::Passmarks:
            m_tabPassmarks->show();
            break;
        case TabOrder::PlaceLabels:
            m_tabPlaceLabels->show();
            break;
        case TabOrder::FoldLine:
            m_tabFoldLine->show();
            break;
        case TabOrder::Count:
            Q_UNREACHABLE();
            break;
    }

    QT_WARNING_POP

    if (index == TabOrder::Pins || index == TabOrder::Grainline ||
        (index == TabOrder::Labels &&
         uiTabLabels->tabWidget->currentIndex() == uiTabLabels->tabWidget->indexOf(uiTabLabels->tabLabels)))
    {
        ShowPieceSpecialPointsWithRect(uiTabPins->listWidgetPins, false);
    }
    else if (index == TabOrder::PlaceLabels)
    {
        ShowPieceSpecialPointsWithRect(uiTabPlaceLabels->listWidgetPlaceLabels, true);
    }
    else
    {
        delete m_visSpecialPoints.data();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::TabChanged(int index)
{
    if (index == uiTabLabels->tabWidget->indexOf(uiTabLabels->tabLabels))
    {
        ShowPieceSpecialPointsWithRect(uiTabPins->listWidgetPins, false);
    }
    else
    {
        delete m_visSpecialPoints.data();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::PassmarkLineTypeChanged(int id)
{
    const int i = uiTabPassmarks->comboBoxPassmarks->currentIndex();
    if (i != -1)
    {
        QListWidgetItem *rowItem = GetItemById(uiTabPassmarks->comboBoxPassmarks->currentData().toUInt());
        if (rowItem)
        {
            auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

            PassmarkLineType lineType = PassmarkLineType::OneLine;
            if (id == uiTabPassmarks->buttonGroupLineType->id(uiTabPassmarks->radioButtonOneLine))
            {
                lineType = PassmarkLineType::OneLine;
            }
            else if (id == uiTabPassmarks->buttonGroupLineType->id(uiTabPassmarks->radioButtonTwoLines))
            {
                lineType = PassmarkLineType::TwoLines;
            }
            else if (id == uiTabPassmarks->buttonGroupLineType->id(uiTabPassmarks->radioButtonThreeLines))
            {
                lineType = PassmarkLineType::ThreeLines;
            }
            else if (id == uiTabPassmarks->buttonGroupLineType->id(uiTabPassmarks->radioButtonTMark))
            {
                lineType = PassmarkLineType::TMark;
            }
            else if (id == uiTabPassmarks->buttonGroupLineType->id(uiTabPassmarks->radioButtonVMark))
            {
                lineType = PassmarkLineType::ExternalVMark;
            }
            else if (id == uiTabPassmarks->buttonGroupLineType->id(uiTabPassmarks->radioButtonVMark2))
            {
                lineType = PassmarkLineType::InternalVMark;
            }
            else if (id == uiTabPassmarks->buttonGroupLineType->id(uiTabPassmarks->radioButtonUMark))
            {
                lineType = PassmarkLineType::UMark;
            }
            else if (id == uiTabPassmarks->buttonGroupLineType->id(uiTabPassmarks->radioButtonBoxMark))
            {
                lineType = PassmarkLineType::BoxMark;
            }
            else if (id == uiTabPassmarks->buttonGroupLineType->id(uiTabPassmarks->radioButtonCheckMark))
            {
                lineType = PassmarkLineType::CheckMark;
            }

            rowNode.SetPassmarkLineType(lineType);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
            rowItem->setText(GetNodeName(data, rowNode, true));

            ListChanged();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::PassmarkAngleTypeChanged(int id)
{
    const int i = uiTabPassmarks->comboBoxPassmarks->currentIndex();
    if (i != -1)
    {
        QListWidgetItem *rowItem = GetItemById(uiTabPassmarks->comboBoxPassmarks->currentData().toUInt());
        if (rowItem)
        {
            auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

            PassmarkAngleType angleType = PassmarkAngleType::Straightforward;
            if (id == uiTabPassmarks->buttonGroupAngleType->id(uiTabPassmarks->radioButtonStraightforward))
            {
                angleType = PassmarkAngleType::Straightforward;
            }
            else if (id == uiTabPassmarks->buttonGroupAngleType->id(uiTabPassmarks->radioButtonBisector))
            {
                angleType = PassmarkAngleType::Bisector;
            }
            else if (id == uiTabPassmarks->buttonGroupAngleType->id(uiTabPassmarks->radioButtonIntersection))
            {
                angleType = PassmarkAngleType::Intersection;
            }
            else if (id == uiTabPassmarks->buttonGroupAngleType->id(uiTabPassmarks->radioButtonIntersectionOnlyLeft))
            {
                angleType = PassmarkAngleType::IntersectionOnlyLeft;
            }
            else if (id == uiTabPassmarks->buttonGroupAngleType->id(uiTabPassmarks->radioButtonIntersectionOnlyRight))
            {
                angleType = PassmarkAngleType::IntersectionOnlyRight;
            }
            else if (id == uiTabPassmarks->buttonGroupAngleType->id(uiTabPassmarks->radioButtonIntersection2))
            {
                angleType = PassmarkAngleType::Intersection2;
            }
            else if (id == uiTabPassmarks->buttonGroupAngleType->id(uiTabPassmarks->radioButtonIntersection2OnlyLeft))
            {
                angleType = PassmarkAngleType::Intersection2OnlyLeft;
            }
            else if (id == uiTabPassmarks->buttonGroupAngleType->id(uiTabPassmarks->radioButtonIntersection2OnlyRight))
            {
                angleType = PassmarkAngleType::Intersection2OnlyRight;
            }

            rowNode.SetPassmarkAngleType(angleType);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

            ListChanged();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::PassmarkShowSecondChanged(int state)
{
    const int i = uiTabPassmarks->comboBoxPassmarks->currentIndex();
    if (i != -1)
    {
        QListWidgetItem *rowItem = GetItemById(uiTabPassmarks->comboBoxPassmarks->currentData().toUInt());
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
void DialogSeamAllowance::PassmarkClockwiseOrientationChanged(int state)
{
    const int i = uiTabPassmarks->comboBoxPassmarks->currentIndex();
    if (i != -1)
    {
        QListWidgetItem *rowItem = GetItemById(uiTabPassmarks->comboBoxPassmarks->currentData().toUInt());
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
void DialogSeamAllowance::PassmarkNotMirroredStateChanged(int state)
{
    const int i = uiTabPassmarks->comboBoxPassmarks->currentIndex();
    if (i != -1)
    {
        QListWidgetItem *rowItem = GetItemById(uiTabPassmarks->comboBoxPassmarks->currentData().toUInt());
        if (rowItem)
        {
            auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetPassmarkNotMirrored(state);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

            ListChanged();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::UpdateGrainlineValues()
{
    std::array<const QPlainTextEdit *, 2> apleSender = {uiTabGrainline->lineEditRotFormula,
                                                        uiTabGrainline->lineEditLenFormula};
    std::array<bool, 2> bFormulasOK = {true, true};

    for (std::size_t i = 0; i < 2; ++i)
    {
        QLabel *plbVal;
        QLabel *plbText;
        QString qsUnit;
        if (i == 0)
        {
            plbVal = uiTabGrainline->labelRot;
            plbText = uiTabGrainline->labelEditRot;
            qsUnit = degreeSymbol;
        }
        else
        {
            plbVal = uiTabGrainline->labelLen;
            plbText = uiTabGrainline->labelEditLen;
            qsUnit = QChar(QChar::Space) + UnitsToStr(VAbstractValApplication::VApp()->patternUnits());
        }

        plbVal->setToolTip(tr("Value"));

        QString qsFormula = apleSender.at(i)->toPlainText().simplified();
        QString qsVal;
        try
        {
            qsFormula = VAbstractApplication::VApp()->TrVars()->FormulaFromUser(
                qsFormula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
            Calculator cal;
            qreal const dVal = cal.EvalFormula(data->DataVariables(), qsFormula);
            if (qIsInf(dVal) || qIsNaN(dVal))
            {
                throw qmu::QmuParserError(tr("Infinite/undefined result"));
            }
            if (i == 1 && dVal <= 0.0)
            {
                throw qmu::QmuParserError(tr("Length should be positive"));
            }

            qsVal.setNum(dVal, 'f', 2);
            ChangeColor(plbText, OkColor(this));
        }
        catch (qmu::QmuParserError &e)
        {
            qsVal = tr("Error");
            not flagGPin ? ChangeColor(plbText, errorColor) : ChangeColor(plbText, OkColor(this));
            bFormulasOK.at(i) = false;
            plbVal->setToolTip(tr("Parser error: %1").arg(e.GetMsg()));
        }

        if (bFormulasOK.at(i) && not qsVal.isEmpty())
        {
            qsVal += qsUnit;
        }
        plbVal->setText(qsVal);
    }

    flagGFormulas = bFormulasOK[0] && bFormulasOK[1];
    if (not flagGFormulas && not flagGPin)
    {
        m_ftb->SetTabText(TabOrder::Grainline, tr("Grainline") + '*');
    }
    else
    {
        ResetGrainlineWarning();
    }
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::UpdateDetailLabelValues()
{
    std::array<const QPlainTextEdit *, 3> apleSender = {
        uiTabLabels->lineEditDLWidthFormula, uiTabLabels->lineEditDLHeightFormula, uiTabLabels->lineEditDLAngleFormula};
    std::array<bool, 3> bFormulasOK = {true, true, true};

    for (std::size_t i = 0; i < 3; ++i)
    {
        QLabel *plbVal;
        QLabel *plbText;
        QString qsUnit;
        if (i == 0)
        {
            plbVal = uiTabLabels->labelDLWidth;
            plbText = uiTabLabels->labelEditDLWidth;
            qsUnit = QChar(QChar::Space) + UnitsToStr(VAbstractValApplication::VApp()->patternUnits());
        }
        else if (i == 1)
        {
            plbVal = uiTabLabels->labelDLHeight;
            plbText = uiTabLabels->labelEditDLHeight;
            qsUnit = QChar(QChar::Space) + UnitsToStr(VAbstractValApplication::VApp()->patternUnits());
        }
        else
        {
            plbVal = uiTabLabels->labelDLAngle;
            plbText = uiTabLabels->labelEditDLAngle;
            qsUnit = degreeSymbol;
        }

        plbVal->setToolTip(tr("Value"));

        QString qsFormula = apleSender.at(i)->toPlainText().simplified();
        QString qsVal;
        try
        {

            qsFormula = VAbstractApplication::VApp()->TrVars()->FormulaFromUser(
                qsFormula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
            Calculator cal;
            qreal const dVal = cal.EvalFormula(data->DataVariables(), qsFormula);
            if (qIsInf(dVal) || qIsNaN(dVal))
            {
                throw qmu::QmuParserError(tr("Infinite/undefined result"));
            }
            if ((i == 0 || i == 1) && dVal <= 0.0)
            {
                throw qmu::QmuParserError(tr("Length should be positive"));
            }

            qsVal.setNum(dVal, 'f', 2);
            ChangeColor(plbText, OkColor(this));
        }
        catch (qmu::QmuParserError &e)
        {
            qsVal = tr("Error");
            not flagDPin ? ChangeColor(plbText, errorColor) : ChangeColor(plbText, OkColor(this));
            bFormulasOK.at(i) = false;
            plbVal->setToolTip(tr("Parser error: %1").arg(e.GetMsg()));
        }

        if (bFormulasOK.at(i) && not qsVal.isEmpty())
        {
            qsVal += qsUnit;
        }
        plbVal->setText(qsVal);
    }

    flagDLAngle = bFormulasOK[2];
    flagDLFormulas = bFormulasOK[0] && bFormulasOK[1];
    if (not flagDLAngle || not(flagDLFormulas || flagDPin) || not flagPLAngle || not(flagPLFormulas || flagPPin))
    {
        m_ftb->SetTabText(TabOrder::Labels, tr("Labels") + '*');
        const QIcon icon = FromTheme(VThemeIcon::DialogWarning);
        uiTabLabels->tabWidget->setTabIcon(uiTabLabels->tabWidget->indexOf(uiTabLabels->tabLabels), icon);
    }
    else
    {
        ResetLabelsWarning();
    }
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::UpdatePatternLabelValues()
{
    std::array<const QPlainTextEdit *, 3> apleSender = {
        uiTabLabels->lineEditPLWidthFormula, uiTabLabels->lineEditPLHeightFormula, uiTabLabels->lineEditPLAngleFormula};
    std::array<bool, 3> bFormulasOK = {true, true, true};

    for (std::size_t i = 0; i < 3; ++i)
    {
        QLabel *plbVal;
        QLabel *plbText;
        QString qsUnit;
        if (i == 0)
        {
            plbVal = uiTabLabels->labelPLWidth;
            plbText = uiTabLabels->labelEditPLWidth;
            qsUnit = QChar(QChar::Space) + UnitsToStr(VAbstractValApplication::VApp()->patternUnits());
        }
        else if (i == 1)
        {
            plbVal = uiTabLabels->labelPLHeight;
            plbText = uiTabLabels->labelEditPLHeight;
            qsUnit = QChar(QChar::Space) + UnitsToStr(VAbstractValApplication::VApp()->patternUnits());
        }
        else
        {
            plbVal = uiTabLabels->labelPLAngle;
            plbText = uiTabLabels->labelEditPLAngle;
            qsUnit = degreeSymbol;
        }

        plbVal->setToolTip(tr("Value"));

        QString qsFormula = apleSender.at(i)->toPlainText().simplified();
        QString qsVal;
        try
        {
            qsFormula = VAbstractApplication::VApp()->TrVars()->FormulaFromUser(
                qsFormula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
            Calculator cal;
            qreal const dVal = cal.EvalFormula(data->DataVariables(), qsFormula);
            if (qIsInf(dVal) || qIsNaN(dVal))
            {
                throw qmu::QmuParserError(tr("Infinite/undefined result"));
            }
            if ((i == 0 || i == 1) && dVal <= 0.0)
            {
                throw qmu::QmuParserError(tr("Length should be positive"));
            }

            qsVal.setNum(dVal, 'f', 2);
            ChangeColor(plbText, OkColor(this));
        }
        catch (qmu::QmuParserError &e)
        {
            qsVal = tr("Error");
            not flagPPin ? ChangeColor(plbText, errorColor) : ChangeColor(plbText, OkColor(this));
            bFormulasOK.at(i) = false;
            plbVal->setToolTip(tr("Parser error: %1").arg(e.GetMsg()));
        }

        if (bFormulasOK.at(i) && not qsVal.isEmpty())
        {
            qsVal += qsUnit;
        }
        plbVal->setText(qsVal);
    }

    flagPLAngle = bFormulasOK[2];
    flagPLFormulas = bFormulasOK[0] && bFormulasOK[1];
    if (not flagDLAngle || not(flagDLFormulas || flagDPin) || not flagPLAngle || not(flagPLFormulas || flagPPin))
    {
        m_ftb->SetTabText(TabOrder::Labels, tr("Labels") + '*');
        const QIcon icon = FromTheme(VThemeIcon::DialogWarning);
        uiTabLabels->tabWidget->setTabIcon(uiTabLabels->tabWidget->indexOf(uiTabLabels->tabLabels), icon);
    }
    else
    {
        ResetLabelsWarning();
    }
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EnabledGrainline()
{
    if (uiTabGrainline->groupBoxGrainline->isChecked())
    {
        GrainlinePinPointChanged();
    }
    else
    {
        flagGFormulas = true;
        ResetGrainlineWarning();
        CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EnabledDetailLabel()
{
    if (uiTabLabels->groupBoxDetailLabel->isChecked())
    {
        UpdateDetailLabelValues();
        DetailPinPointChanged();
    }
    else
    {
        flagDLAngle = true;
        flagDLFormulas = true;
        ResetLabelsWarning();
        CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EnabledPatternLabel()
{
    if (uiTabLabels->groupBoxPatternLabel->isChecked())
    {
        UpdatePatternLabelValues();
        PatternPinPointChanged();
    }
    else
    {
        flagPLAngle = true;
        flagPLFormulas = true;
        ResetLabelsWarning();
        CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EnabledManualPassmarkLength()
{
    const int index = uiTabPassmarks->comboBoxPassmarks->currentIndex();
    if (index != -1)
    {
        QListWidgetItem *rowItem = GetItemById(uiTabPassmarks->comboBoxPassmarks->currentData().toUInt());
        if (rowItem)
        {
            auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetManualPassmarkLength(uiTabPassmarks->groupBoxManualLength->isChecked());
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

            uiTabPassmarks->toolButtonExprLength->setEnabled(uiTabPassmarks->groupBoxManualLength->isChecked());
            uiTabPassmarks->plainTextEditPassmarkLength->setEnabled(uiTabPassmarks->groupBoxManualLength->isChecked());
            uiTabPassmarks->pushButtonGrowPassmarkLength->setEnabled(uiTabPassmarks->groupBoxManualLength->isChecked());
            uiTabPassmarks->labelEditPassmarkLength->setEnabled(uiTabPassmarks->groupBoxManualLength->isChecked());
            uiTabPassmarks->label_3->setEnabled(uiTabPassmarks->groupBoxManualLength->isChecked());

            EvalPassmarkLength();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EnabledManualPassmarkWidth()
{
    const int index = uiTabPassmarks->comboBoxPassmarks->currentIndex();
    if (index != -1)
    {
        QListWidgetItem *rowItem = GetItemById(uiTabPassmarks->comboBoxPassmarks->currentData().toUInt());
        if (rowItem)
        {
            auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetManualPassmarkWidth(uiTabPassmarks->groupBoxManualWidth->isChecked());
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

            uiTabPassmarks->toolButtonExprWidth->setEnabled(uiTabPassmarks->groupBoxManualWidth->isChecked());
            uiTabPassmarks->plainTextEditPassmarkWidth->setEnabled(uiTabPassmarks->groupBoxManualWidth->isChecked());
            uiTabPassmarks->pushButtonGrowPassmarkWidth->setEnabled(uiTabPassmarks->groupBoxManualWidth->isChecked());
            uiTabPassmarks->labelEditPassmarkWidth->setEnabled(uiTabPassmarks->groupBoxManualWidth->isChecked());
            uiTabPassmarks->label_4->setEnabled(uiTabPassmarks->groupBoxManualWidth->isChecked());

            EvalPassmarkWidth();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EnabledManualPassmarkAngle()
{
    const int index = uiTabPassmarks->comboBoxPassmarks->currentIndex();
    if (index != -1)
    {
        QListWidgetItem *rowItem = GetItemById(uiTabPassmarks->comboBoxPassmarks->currentData().toUInt());
        if (rowItem)
        {
            auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetManualPassmarkAngle(uiTabPassmarks->groupBoxManualAngle->isChecked());
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

            uiTabPassmarks->toolButtonExprAngle->setEnabled(uiTabPassmarks->groupBoxManualAngle->isChecked());
            uiTabPassmarks->plainTextEditPassmarkAngle->setEnabled(uiTabPassmarks->groupBoxManualAngle->isChecked());
            uiTabPassmarks->pushButtonGrowPassmarkAngle->setEnabled(uiTabPassmarks->groupBoxManualAngle->isChecked());
            uiTabPassmarks->labelEditPassmarkAngle->setEnabled(uiTabPassmarks->groupBoxManualAngle->isChecked());
            uiTabPassmarks->label_5->setEnabled(uiTabPassmarks->groupBoxManualAngle->isChecked());

            EvalPassmarkAngle();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EnabledManualFoldHeight()
{
    uiTabFoldLine->toolButtonExprHeight->setEnabled(uiTabFoldLine->groupBoxManualHeight->isChecked());
    uiTabFoldLine->plainTextEditHeight->setEnabled(uiTabFoldLine->groupBoxManualHeight->isChecked());
    uiTabFoldLine->pushButtonGrowHeight->setEnabled(uiTabFoldLine->groupBoxManualHeight->isChecked());
    uiTabFoldLine->labelEditHeight->setEnabled(uiTabFoldLine->groupBoxManualHeight->isChecked());
    uiTabFoldLine->label_3->setEnabled(uiTabFoldLine->groupBoxManualHeight->isChecked());

    EvalFoldHeight();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EnabledManualFoldWidth()
{
    uiTabFoldLine->toolButtonExprWidth->setEnabled(uiTabFoldLine->groupBoxManualWidth->isChecked());
    uiTabFoldLine->plainTextEditWidth->setEnabled(uiTabFoldLine->groupBoxManualWidth->isChecked());
    uiTabFoldLine->pushButtonGrowWidth->setEnabled(uiTabFoldLine->groupBoxManualWidth->isChecked());
    uiTabFoldLine->labelEditWidth->setEnabled(uiTabFoldLine->groupBoxManualWidth->isChecked());
    uiTabFoldLine->label_4->setEnabled(uiTabFoldLine->groupBoxManualWidth->isChecked());

    EvalFoldWidth();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EnabledManualFoldCenter()
{
    uiTabFoldLine->toolButtonExprCenter->setEnabled(uiTabFoldLine->groupBoxManualCenter->isChecked());
    uiTabFoldLine->plainTextEditCenter->setEnabled(uiTabFoldLine->groupBoxManualCenter->isChecked());
    uiTabFoldLine->pushButtonGrowCenter->setEnabled(uiTabFoldLine->groupBoxManualCenter->isChecked());
    uiTabFoldLine->labelEditCenter->setEnabled(uiTabFoldLine->groupBoxManualCenter->isChecked());
    uiTabFoldLine->label_5->setEnabled(uiTabFoldLine->groupBoxManualCenter->isChecked());

    EvalFoldCenter();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EditGrainlineFormula()
{
    QPlainTextEdit *pleFormula;
    bool bCheckZero;
    QString title;

    if (sender() == uiTabGrainline->pushButtonLen)
    {
        pleFormula = uiTabGrainline->lineEditLenFormula;
        bCheckZero = true;
        title = tr("Edit length");
    }
    else if (sender() == uiTabGrainline->pushButtonRot)
    {
        pleFormula = uiTabGrainline->lineEditRotFormula;
        bCheckZero = false;
        title = tr("Edit angle");
    }
    else
    {
        // should not get here!
        return;
    }

    DialogEditWrongFormula dlg(data, NULL_ID, this);
    dlg.setWindowTitle(title);
    dlg.SetFormula(VTranslateVars::TryFormulaFromUser(pleFormula->toPlainText(),
                                                      VAbstractApplication::VApp()->Settings()->GetOsSeparator()));
    dlg.setCheckZero(bCheckZero);
    if (dlg.exec() == QDialog::Accepted)
    {
        QString const qsFormula = dlg.GetFormula();

        if (sender() == uiTabGrainline->pushButtonLen)
        {
            SetGrainlineLength(qsFormula);
        }
        else if (sender() == uiTabGrainline->pushButtonRot)
        {
            SetGrainlineAngle(qsFormula);
        }
        else
        {
            // should not get here!
            pleFormula->setPlainText(qsFormula);
        }
        UpdateGrainlineValues();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EditDLFormula()
{
    QPlainTextEdit *pleFormula;
    bool bCheckZero;
    QString title;

    if (sender() == uiTabLabels->pushButtonDLHeight)
    {
        pleFormula = uiTabLabels->lineEditDLHeightFormula;
        bCheckZero = true;
        title = tr("Edit height");
    }
    else if (sender() == uiTabLabels->pushButtonDLWidth)
    {
        pleFormula = uiTabLabels->lineEditDLWidthFormula;
        bCheckZero = true;
        title = tr("Edit width");
    }
    else if (sender() == uiTabLabels->pushButtonDLAngle)
    {
        pleFormula = uiTabLabels->lineEditDLAngleFormula;
        bCheckZero = false;
        title = tr("Edit angle");
    }
    else
    {
        // should not get here!
        return;
    }

    DialogEditWrongFormula dlg(data, NULL_ID, this);
    dlg.setWindowTitle(title);
    dlg.SetFormula(VTranslateVars::TryFormulaFromUser(pleFormula->toPlainText(),
                                                      VAbstractApplication::VApp()->Settings()->GetOsSeparator()));
    dlg.setCheckZero(bCheckZero);
    if (dlg.exec() == QDialog::Accepted)
    {
        QString const qsFormula = dlg.GetFormula();
        if (sender() == uiTabLabels->pushButtonDLHeight)
        {
            SetDLHeight(qsFormula);
        }
        else if (sender() == uiTabLabels->pushButtonDLWidth)
        {
            SetDLWidth(qsFormula);
        }
        else if (sender() == uiTabLabels->pushButtonDLAngle)
        {
            SetDLAngle(qsFormula);
        }
        else
        {
            // should not get here!
            pleFormula->setPlainText(qsFormula);
        }
        UpdateDetailLabelValues();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EditPLFormula()
{
    QPlainTextEdit *pleFormula;
    bool bCheckZero;
    QString title;

    if (sender() == uiTabLabels->pushButtonPLHeight)
    {
        pleFormula = uiTabLabels->lineEditPLHeightFormula;
        bCheckZero = true;
        title = tr("Edit height");
    }
    else if (sender() == uiTabLabels->pushButtonPLWidth)
    {
        pleFormula = uiTabLabels->lineEditPLWidthFormula;
        bCheckZero = true;
        title = tr("Edit width");
    }
    else if (sender() == uiTabLabels->pushButtonPLAngle)
    {
        pleFormula = uiTabLabels->lineEditPLAngleFormula;
        bCheckZero = false;
        title = tr("Edit angle");
    }
    else
    {
        // should not get here!
        return;
    }

    DialogEditWrongFormula dlg(data, NULL_ID, this);
    dlg.setWindowTitle(title);
    dlg.SetFormula(VTranslateVars::TryFormulaFromUser(pleFormula->toPlainText(),
                                                      VAbstractApplication::VApp()->Settings()->GetOsSeparator()));
    dlg.setCheckZero(bCheckZero);
    if (dlg.exec() == QDialog::Accepted)
    {
        QString const qsFormula = dlg.GetFormula();
        if (sender() == uiTabLabels->pushButtonPLHeight)
        {
            SetPLHeight(qsFormula);
        }
        else if (sender() == uiTabLabels->pushButtonPLWidth)
        {
            SetPLWidth(qsFormula);
        }
        else if (sender() == uiTabLabels->pushButtonPLAngle)
        {
            SetPLAngle(qsFormula);
        }
        else
        {
            // should not get here!
            pleFormula->setPlainText(qsFormula);
        }
        UpdatePatternLabelValues();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployGrainlineRotation()
{
    DeployFormula(this, uiTabGrainline->lineEditRotFormula, uiTabGrainline->pushButtonShowRot, m_iRotBaseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployGrainlineLength()
{
    DeployFormula(this, uiTabGrainline->lineEditLenFormula, uiTabGrainline->pushButtonShowLen, m_iLenBaseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployDLWidth()
{
    DeployFormula(this, uiTabLabels->lineEditDLWidthFormula, uiTabLabels->pushButtonShowDLWidth, m_DLWidthBaseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployDLHeight()
{
    DeployFormula(this, uiTabLabels->lineEditDLHeightFormula, uiTabLabels->pushButtonShowDLHeight,
                  m_DLHeightBaseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployDLAngle()
{
    DeployFormula(this, uiTabLabels->lineEditDLAngleFormula, uiTabLabels->pushButtonShowDLAngle, m_DLAngleBaseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployPLWidth()
{
    DeployFormula(this, uiTabLabels->lineEditPLWidthFormula, uiTabLabels->pushButtonShowPLWidth, m_PLWidthBaseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployPLHeight()
{
    DeployFormula(this, uiTabLabels->lineEditPLHeightFormula, uiTabLabels->pushButtonShowPLHeight,
                  m_PLHeightBaseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployPLAngle()
{
    DeployFormula(this, uiTabLabels->lineEditPLAngleFormula, uiTabLabels->pushButtonShowPLAngle, m_PLAngleBaseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ResetGrainlineWarning()
{
    if (flagGFormulas || flagGPin)
    {
        m_ftb->SetTabText(TabOrder::Grainline, tr("Grainline"));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ResetLabelsWarning()
{
    if (flagDLAngle && (flagDLFormulas || flagDPin) && flagPLAngle && (flagPLFormulas || flagPPin))
    {
        m_ftb->SetTabText(TabOrder::Labels, tr("Labels"));
        uiTabLabels->tabWidget->setTabIcon(uiTabLabels->tabWidget->indexOf(uiTabLabels->tabLabels), QIcon());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EvalWidth()
{
    // Avoid circular dependency. CurrentSeamAllowance value based on the width field value. On this stage, we must
    // hide from user the existence of CurrentSeamAllowance variable.
    QHash<QString, QSharedPointer<VInternalVariable>> variables = *data->DataVariables();
    variables.remove(currentSeamAllowance);

    m_saWidth = Eval({.formula = uiTabPaths->plainTextEditFormulaWidth->toPlainText(),
                      .variables = &variables,
                      .labelEditFormula = uiTabPaths->labelEditWidth,
                      .labelResult = uiTabPaths->labelResultWidth,
                      .postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true),
                      .checkLessThanZero = true}, flagFormula);

    if (m_saWidth >= 0)
    {
        auto *locData = const_cast<VContainer *>(data); // NOLINT(cppcoreguidelines-pro-type-const-cast)

        auto *currentSA = new VIncrement(locData, currentSeamAllowance);
        currentSA->SetFormula(m_saWidth, QString().setNum(m_saWidth), true);
        currentSA->SetDescription(tr("Current seam allowance"));

        locData->AddVariable(currentSA);

        EvalWidthBefore();
        EvalWidthAfter();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EvalWidthBefore()
{
    if (uiTabPaths->checkBoxSeams->isChecked())
    {
        if (uiTabPaths->comboBoxNodes->count() > 0)
        {
            Eval({.formula = uiTabPaths->plainTextEditFormulaWidthBefore->toPlainText(),
                  .variables = data->DataVariables(),
                  .labelEditFormula = uiTabPaths->labelEditBefore,
                  .labelResult = uiTabPaths->labelResultBefore,
                  .postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true),
                  .checkLessThanZero = true}, flagFormulaBefore);

            const QString formulaSABefore = GetFormulaFromUser(uiTabPaths->plainTextEditFormulaWidthBefore);
            UpdateNodeSABefore(formulaSABefore);
            EnableDefButton(uiTabPaths->pushButtonDefBefore, formulaSABefore);
        }
        else
        {
            ChangeColor(uiTabPaths->labelEditBefore, OkColor(this));
            uiTabPaths->labelResultBefore->setText(tr("<Empty>"));
            flagFormulaBefore = true;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EvalWidthAfter()
{
    if (uiTabPaths->checkBoxSeams->isChecked())
    {
        if (uiTabPaths->comboBoxNodes->count() > 0)
        {
            Eval({.formula = uiTabPaths->plainTextEditFormulaWidthAfter->toPlainText(),
                  .variables = data->DataVariables(),
                  .labelEditFormula = uiTabPaths->labelEditAfter,
                  .labelResult = uiTabPaths->labelResultAfter,
                  .postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true),
                  .checkLessThanZero = true}, flagFormulaAfter);

            const QString formulaSAAfter = GetFormulaFromUser(uiTabPaths->plainTextEditFormulaWidthAfter);
            UpdateNodeSAAfter(formulaSAAfter);
            EnableDefButton(uiTabPaths->pushButtonDefAfter, formulaSAAfter);
        }
        else
        {
            ChangeColor(uiTabPaths->labelEditAfter, OkColor(this));
            uiTabPaths->labelResultAfter->setText(tr("<Empty>"));
            flagFormulaAfter = true;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EvalPassmarkLength()
{
    if (uiTabPassmarks->groupBoxManualLength->isChecked())
    {
        if (uiTabPassmarks->comboBoxPassmarks->count() > 0)
        {
            Eval({.formula = uiTabPassmarks->plainTextEditPassmarkLength->toPlainText(),
                  .variables = data->DataVariables(),
                  .labelEditFormula = uiTabPassmarks->labelEditPassmarkLength,
                  .labelResult = uiTabPassmarks->labelResultPassmarkLength,
                  .postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true),
                  .checkZero = true,
                  .checkLessThanZero = true}, flagFormulaPassmarkLength);

            UpdateNodePassmarkLength(GetFormulaFromUser(uiTabPassmarks->plainTextEditPassmarkLength));
        }
        else
        {
            ChangeColor(uiTabPassmarks->labelEditPassmarkLength, OkColor(this));
            uiTabPassmarks->labelResultPassmarkLength->setText(tr("<Empty>"));
            flagFormulaPassmarkLength = true;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EvalPassmarkWidth()
{
    if (uiTabPassmarks->groupBoxManualWidth->isChecked())
    {
        if (uiTabPassmarks->comboBoxPassmarks->count() > 0)
        {
            Eval({.formula = uiTabPassmarks->plainTextEditPassmarkWidth->toPlainText(),
                  .variables = data->DataVariables(),
                  .labelEditFormula = uiTabPassmarks->labelEditPassmarkWidth,
                  .labelResult = uiTabPassmarks->labelResultPassmarkWidth,
                  .postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true),
                  .checkZero = true}, flagFormulaPassmarkWidth);

            UpdateNodePassmarkWidth(GetFormulaFromUser(uiTabPassmarks->plainTextEditPassmarkWidth));
        }
        else
        {
            ChangeColor(uiTabPassmarks->labelEditPassmarkWidth, OkColor(this));
            uiTabPassmarks->labelResultPassmarkWidth->setText(tr("<Empty>"));
            flagFormulaPassmarkWidth = true;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EvalPassmarkAngle()
{
    if (uiTabPassmarks->groupBoxManualAngle->isChecked())
    {
        if (uiTabPassmarks->comboBoxPassmarks->count() > 0)
        {
            Eval({.formula = uiTabPassmarks->plainTextEditPassmarkAngle->toPlainText(),
                  .variables = data->DataVariables(),
                  .labelEditFormula = uiTabPassmarks->labelEditPassmarkAngle,
                  .labelResult = uiTabPassmarks->labelResultPassmarkAngle,
                  .postfix = degreeSymbol}, flagFormulaPassmarkAngle);

            UpdateNodePassmarkAngle(GetFormulaFromUser(uiTabPassmarks->plainTextEditPassmarkAngle));
        }
        else
        {
            ChangeColor(uiTabPassmarks->labelEditPassmarkAngle, OkColor(this));
            uiTabPassmarks->labelResultPassmarkAngle->setText(tr("<Empty>"));
            flagFormulaPassmarkAngle = true;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EvalFoldHeight()
{
    if (uiTabFoldLine->groupBoxManualHeight->isChecked())
    {
        Eval({.formula = uiTabFoldLine->plainTextEditHeight->toPlainText(),
              .variables = data->DataVariables(),
              .labelEditFormula = uiTabFoldLine->labelEditHeight,
              .labelResult = uiTabFoldLine->labelResultHeight,
              .postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true),
              .checkLessThanZero = true}, flagFormulaFoldHeight);
    }
    else
    {
        CheckState(); // Disable Ok and Apply buttons if something wrong.
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EvalFoldWidth()
{
    if (uiTabFoldLine->groupBoxManualWidth->isChecked())
    {
        Eval({.formula = uiTabFoldLine->plainTextEditWidth->toPlainText(),
              .variables = data->DataVariables(),
              .labelEditFormula = uiTabFoldLine->labelEditWidth,
              .labelResult = uiTabFoldLine->labelResultWidth,
              .postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true),
              .checkLessThanZero = true}, flagFormulaFoldWidth);
    }
    else
    {
        CheckState(); // Disable Ok and Apply buttons if something wrong.
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EvalFoldCenter()
{
    if (uiTabFoldLine->groupBoxManualCenter->isChecked())
    {
        Eval({.formula = uiTabFoldLine->plainTextEditCenter->toPlainText(),
              .variables = data->DataVariables(),
              .labelEditFormula = uiTabFoldLine->labelEditCenter,
              .labelResult = uiTabFoldLine->labelResultCenter,
              .checkLessThanZero = true}, flagFormulaFoldCenter);
    }
    else
    {
        CheckState(); // Disable Ok and Apply buttons if something wrong.
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::FXWidth()
{
    // Avoid circular dependency. CurrentSeamAllowance value based on the width field value. On this stage, we must
    // hide from user the existence of CurrentSeamAllowance variable.
    VContainer localData = *data;
    localData.RemoveVariable(currentSeamAllowance);

    QScopedPointer<DialogEditWrongFormula> const dialog(new DialogEditWrongFormula(&localData, toolId, this));
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
void DialogSeamAllowance::FXWidthBefore()
{
    QScopedPointer<DialogEditWrongFormula> const dialog(new DialogEditWrongFormula(data, toolId, this));
    dialog->setWindowTitle(tr("Edit seam allowance width before"));
    dialog->SetFormula(GetFormulaFromUser(uiTabPaths->plainTextEditFormulaWidthBefore));
    dialog->setCheckLessThanZero(true);
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetCurrentSABefore(dialog->GetFormula());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::FXWidthAfter()
{
    QScopedPointer<DialogEditWrongFormula> const dialog(new DialogEditWrongFormula(data, toolId, this));
    dialog->setWindowTitle(tr("Edit seam allowance width after"));
    dialog->SetFormula(GetFormulaFromUser(uiTabPaths->plainTextEditFormulaWidthAfter));
    dialog->setCheckLessThanZero(true);
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetCurrentSAAfter(dialog->GetFormula());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::FXPassmarkLength()
{
    QScopedPointer<DialogEditWrongFormula> const dialog(new DialogEditWrongFormula(data, toolId, this));
    dialog->setWindowTitle(tr("Edit passmark length"));
    dialog->SetFormula(GetFormulaFromUser(uiTabPassmarks->plainTextEditPassmarkLength));
    dialog->setCheckLessThanZero(true);
    dialog->setCheckZero(true);
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetFormulaPassmarkLength(dialog->GetFormula());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::FXPassmarkWidth()
{
    QScopedPointer<DialogEditWrongFormula> const dialog(new DialogEditWrongFormula(data, toolId, this));
    dialog->setWindowTitle(tr("Edit passmark width"));
    dialog->SetFormula(GetFormulaFromUser(uiTabPassmarks->plainTextEditPassmarkWidth));
    dialog->setCheckZero(true);
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetFormulaPassmarkWidth(dialog->GetFormula());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::FXPassmarkAngle()
{
    QScopedPointer<DialogEditWrongFormula> const dialog(new DialogEditWrongFormula(data, toolId, this));
    dialog->setWindowTitle(tr("Edit passmark angle"));
    dialog->SetFormula(GetFormulaFromUser(uiTabPassmarks->plainTextEditPassmarkAngle));
    dialog->setPostfix(degreeSymbol);
    if (dialog->exec() == QDialog::Accepted)
    {
        SetFormulaPassmarkAngle(dialog->GetFormula());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::FXFoldHeight()
{
    QScopedPointer<DialogEditWrongFormula> const dialog(new DialogEditWrongFormula(data, toolId, this));
    dialog->setWindowTitle(tr("Edit fold line height"));
    dialog->SetFormula(GetFormulaFromUser(uiTabFoldLine->plainTextEditHeight));
    dialog->setCheckLessThanZero(true);
    dialog->setCheckZero(true);
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetFormulaFoldHeight(dialog->GetFormula());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::FXFoldWidth()
{
    QScopedPointer<DialogEditWrongFormula> const dialog(new DialogEditWrongFormula(data, toolId, this));
    dialog->setWindowTitle(tr("Edit fold line width"));
    dialog->SetFormula(GetFormulaFromUser(uiTabFoldLine->plainTextEditWidth));
    dialog->setCheckLessThanZero(true);
    dialog->setCheckZero(true);
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetFormulaFoldWidth(dialog->GetFormula());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::FXFoldCenter()
{
    QScopedPointer<DialogEditWrongFormula> const dialog(new DialogEditWrongFormula(data, toolId, this));
    dialog->setWindowTitle(tr("Edit fold line center position"));
    dialog->SetFormula(GetFormulaFromUser(uiTabFoldLine->plainTextEditCenter));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetFormulaFoldCenter(dialog->GetFormula());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployWidthFormulaTextEdit()
{
    DeployFormula(this, uiTabPaths->plainTextEditFormulaWidth, uiTabPaths->pushButtonGrowWidth, m_formulaBaseWidth);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployWidthBeforeFormulaTextEdit()
{
    DeployFormula(this, uiTabPaths->plainTextEditFormulaWidthBefore, uiTabPaths->pushButtonGrowWidthBefore,
                  m_formulaBaseWidthBefore);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployWidthAfterFormulaTextEdit()
{
    DeployFormula(this, uiTabPaths->plainTextEditFormulaWidthAfter, uiTabPaths->pushButtonGrowWidthAfter,
                  m_formulaBaseWidthAfter);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployPassmarkLength()
{
    DeployFormula(this, uiTabPassmarks->plainTextEditPassmarkLength, uiTabPassmarks->pushButtonGrowPassmarkLength,
                  m_formulaBasePassmarkLength);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployPassmarkWidth()
{
    DeployFormula(this, uiTabPassmarks->plainTextEditPassmarkWidth, uiTabPassmarks->pushButtonGrowPassmarkWidth,
                  m_formulaBasePassmarkWidth);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployPassmarkAngle()
{
    DeployFormula(this, uiTabPassmarks->plainTextEditPassmarkAngle, uiTabPassmarks->pushButtonGrowPassmarkAngle,
                  m_formulaBasePassmarkAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployFoldHeight()
{
    DeployFormula(this, uiTabFoldLine->plainTextEditHeight, uiTabFoldLine->pushButtonGrowHeight,
                  m_formulaBaseFoldHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployFoldWidth()
{
    DeployFormula(this, uiTabFoldLine->plainTextEditWidth, uiTabFoldLine->pushButtonGrowWidth, m_formulaBaseFoldWidth);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployFoldCenter()
{
    DeployFormula(this, uiTabFoldLine->plainTextEditCenter, uiTabFoldLine->pushButtonGrowCenter,
                  m_formulaBaseFoldCenter);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::GrainlinePinPointChanged()
{
    QColor color;
    const quint32 topPinId = getCurrentObjectId(uiTabGrainline->comboBoxGrainlineTopPin);
    const quint32 bottomPinId = getCurrentObjectId(uiTabGrainline->comboBoxGrainlineBottomPin);
    if (topPinId != NULL_ID && bottomPinId != NULL_ID && topPinId != bottomPinId)
    {
        flagGPin = true;
        color = OkColor(this);

        ResetGrainlineWarning();
    }
    else
    {
        flagGPin = false;
        topPinId == NULL_ID &&bottomPinId == NULL_ID ? color = OkColor(this) : color = errorColor;

        if (not flagGFormulas && not flagGPin)
        {
            m_ftb->SetTabText(TabOrder::Grainline, tr("Grainline"));
        }
    }
    EnableGrainlineFormulaControls(not flagGPin);
    UpdateGrainlineValues();
    ChangeColor(uiTabGrainline->labelGrainlineTopPin, color);
    ChangeColor(uiTabGrainline->labelGrainlineBottomPin, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DetailPinPointChanged()
{
    QColor color;
    const quint32 topPinId = getCurrentObjectId(uiTabLabels->comboBoxDLTopLeftPin);
    const quint32 bottomPinId = getCurrentObjectId(uiTabLabels->comboBoxDLBottomRightPin);
    if (topPinId != NULL_ID && bottomPinId != NULL_ID && topPinId != bottomPinId)
    {
        flagDPin = true;
        color = OkColor(this);

        if (flagPPin)
        {
            m_ftb->SetTabText(TabOrder::Labels, tr("Labels"));
            uiTabLabels->tabWidget->setTabIcon(uiTabLabels->tabWidget->indexOf(uiTabLabels->tabLabels), QIcon());
        }
    }
    else
    {
        flagDPin = false;
        topPinId == NULL_ID &&bottomPinId == NULL_ID ? color = OkColor(this) : color = errorColor;

        m_ftb->SetTabText(TabOrder::Labels, tr("Labels") + '*');
        const QIcon icon = FromTheme(VThemeIcon::DialogWarning);
        uiTabLabels->tabWidget->setTabIcon(uiTabLabels->tabWidget->indexOf(uiTabLabels->tabLabels), icon);
    }
    EnableDetailLabelFormulaControls(not flagDPin);
    UpdateDetailLabelValues();
    ChangeColor(uiTabLabels->labelDLTopLeftPin, color);
    ChangeColor(uiTabLabels->labelDLBottomRightPin, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::PatternPinPointChanged()
{
    QColor color;
    const quint32 topPinId = getCurrentObjectId(uiTabLabels->comboBoxPLTopLeftPin);
    const quint32 bottomPinId = getCurrentObjectId(uiTabLabels->comboBoxPLBottomRightPin);
    if (topPinId != NULL_ID && bottomPinId != NULL_ID && topPinId != bottomPinId)
    {
        flagPPin = true;
        color = OkColor(this);

        if (flagDPin)
        {
            m_ftb->SetTabText(TabOrder::Labels, tr("Labels"));
            uiTabLabels->tabWidget->setTabIcon(uiTabLabels->tabWidget->indexOf(uiTabLabels->tabLabels), QIcon());
        }
    }
    else
    {
        flagPPin = false;
        topPinId == NULL_ID &&bottomPinId == NULL_ID ? color = OkColor(this) : color = errorColor;

        m_ftb->SetTabText(TabOrder::Labels, tr("Labels") + '*');
        const QIcon icon = FromTheme(VThemeIcon::DialogWarning);
        uiTabLabels->tabWidget->setTabIcon(uiTabLabels->tabWidget->indexOf(uiTabLabels->tabLabels), icon);
    }
    EnablePatternLabelFormulaControls(not flagPPin);
    UpdatePatternLabelValues();
    ChangeColor(uiTabLabels->labelPLTopLeftPin, color);
    ChangeColor(uiTabLabels->labelPLBottomRightPin, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EditPieceLabel()
{
    DialogEditLabel editor(VAbstractValApplication::VApp()->getCurrentDocument(), data);
    editor.SetTemplate(m_templateLines);
    editor.SetPiece(GetPiece());

    if (QDialog::Accepted == editor.exec())
    {
        m_templateLines = editor.GetTemplate();
        uiTabLabels->groupBoxDetailLabel->setEnabled(not m_templateLines.isEmpty());
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSeamAllowance::CreatePiece() const -> VPiece
{
    VPiece piece = data->DataPieces()->value(toolId); // Get options we do not control with the dialog
    piece.GetPath().SetNodes(GetListInternals<VPieceNode>(uiTabPaths->listWidgetMainPath));
    piece.SetCustomSARecords(GetListInternals<CustomSARecord>(uiTabPaths->listWidgetCustomSA));
    piece.SetInternalPaths(GetListInternals<quint32>(uiTabPaths->listWidgetInternalPaths));
    piece.SetPins(GetListInternals<quint32>(uiTabPins->listWidgetPins));
    piece.SetPlaceLabels(GetListInternals<quint32>(uiTabPlaceLabels->listWidgetPlaceLabels));
    piece.SetForbidFlipping(uiTabPaths->checkBoxForbidFlipping->isChecked());
    piece.SetForceFlipping(uiTabPaths->checkBoxForceFlipping->isChecked());
    piece.SetSymmetricalCopy(uiTabPaths->checkBoxSymmetricalCopy->isChecked());
    piece.SetFollowGrainline(uiTabPaths->checkBoxFollowGrainline->isChecked());
    piece.SetSewLineOnDrawing(uiTabPaths->checkBoxOnDrawing->isChecked());
    piece.SetSeamAllowance(uiTabPaths->checkBoxSeams->isChecked());
    piece.SetSeamAllowanceBuiltIn(uiTabPaths->checkBoxBuiltIn->isChecked());
    piece.SetHideMainPath(uiTabPaths->checkBoxHideMainPath->isChecked());
    piece.SetName(uiTabPaths->lineEditName->text());
    piece.SetUUID(uiTabPaths->lineEditUUID->text());
    piece.SetShortName(uiTabPaths->lineEditShortName->text());
    piece.SetGradationLabel(uiTabPaths->lineEditGradationLabel->text());
    piece.SetPriority(static_cast<uint>(uiTabPaths->spinBoxPriority->value()));
    piece.SetTrueZeroWidth(uiTabPaths->checkBoxTrueZeroWidth->isChecked());
    piece.SetFormulaSAWidth(GetFormulaFromUser(uiTabPaths->plainTextEditFormulaWidth), m_saWidth);
    piece.GetPieceLabelData().SetLetter(uiTabLabels->lineEditLetter->text());
    piece.GetPieceLabelData().SetAnnotation(uiTabLabels->lineEditAnnotation->text());
    piece.GetPieceLabelData().SetOrientation(uiTabLabels->lineEditOrientation->text());
    piece.GetPieceLabelData().SetRotationWay(uiTabLabels->lineEditRotation->text());
    piece.GetPieceLabelData().SetTilt(uiTabLabels->lineEditTilt->text());
    piece.GetPieceLabelData().SetFoldPosition(uiTabLabels->lineEditFoldPosition->text());
    piece.GetPieceLabelData().SetQuantity(static_cast<quint16>(uiTabLabels->spinBoxQuantity->value()));
    piece.GetPieceLabelData().SetOnFold(uiTabLabels->checkBoxFold->isChecked());
    piece.GetPieceLabelData().SetLabelTemplate(m_templateLines);
    piece.GetPieceLabelData().SetRotation(GetFormulaFromUser(uiTabLabels->lineEditDLAngleFormula));
    piece.GetPieceLabelData().SetEnabled(uiTabLabels->groupBoxDetailLabel->isChecked());
    piece.GetPieceLabelData().SetFontSize(uiTabLabels->comboBoxPieceLabelSize->currentData().toInt());
    piece.SetMirrorLineStartPoint(GetMirrorLineStartPoint());
    piece.SetMirrorLineEndPoint(GetMirrorLineEndPoint());
    piece.SetShowMirrorLine(uiTabPaths->checkBoxShowMirrorLine->isChecked());
    piece.SetShowFullPiece(
        !piece.SeamAllowanceMirrorLine(data).isNull() ? uiTabPaths->checkBoxShowFullPiece->isChecked() : true);
    piece.SetManualFoldHeight(uiTabFoldLine->groupBoxManualHeight->isChecked());
    piece.SetManualFoldWidth(uiTabFoldLine->groupBoxManualWidth->isChecked());
    piece.SetManualFoldCenter(uiTabFoldLine->groupBoxManualCenter->isChecked());
    piece.SetFormulaFoldHeight(uiTabFoldLine->groupBoxManualHeight->isChecked()
                                   ? GetFormulaFromUser(uiTabFoldLine->plainTextEditHeight)
                                   : QString());
    piece.SetFormulaFoldWidth(uiTabFoldLine->groupBoxManualWidth->isChecked()
                                  ? GetFormulaFromUser(uiTabFoldLine->plainTextEditWidth)
                                  : QString());
    piece.SetFormulaFoldCenter(uiTabFoldLine->groupBoxManualCenter->isChecked()
                                   ? GetFormulaFromUser(uiTabFoldLine->plainTextEditCenter)
                                   : QString());
    piece.SetFoldLineType(static_cast<FoldLineType>(uiTabFoldLine->comboBoxType->currentData().toUInt()));
    piece.SetFoldLineSvgFontSize(uiTabFoldLine->comboBoxLabelFontSize->currentData().toUInt());
    piece.SetFoldLineLabelFontItalic(uiTabFoldLine->toolButtonItalic->isChecked());
    piece.SetFoldLineLabelFontBold(uiTabFoldLine->toolButtonBold->isChecked());
    piece.SetFoldLineLabel(uiTabFoldLine->lineEditLabel->text());

    if (uiTabFoldLine->toolButtonTextLeft->isChecked())
    {
        piece.SetFoldLineLabelAlignment(Qt::AlignLeft);
    }
    else if (uiTabFoldLine->toolButtonTextCenter->isChecked())
    {
        piece.SetFoldLineLabelAlignment(Qt::AlignHCenter);
    }
    else if (uiTabFoldLine->toolButtonTextRight->isChecked())
    {
        piece.SetFoldLineLabelAlignment(Qt::AlignRight);
    }

    if (not flagDPin)
    {
        piece.GetPieceLabelData().SetLabelWidth(GetFormulaFromUser(uiTabLabels->lineEditDLWidthFormula));
        piece.GetPieceLabelData().SetLabelHeight(GetFormulaFromUser(uiTabLabels->lineEditDLHeightFormula));
        piece.GetPieceLabelData().SetCenterPin(getCurrentObjectId(uiTabLabels->comboBoxDLCenterPin));
        piece.GetPieceLabelData().SetTopLeftPin(NULL_ID);
        piece.GetPieceLabelData().SetBottomRightPin(NULL_ID);
    }
    else
    {
        piece.GetPieceLabelData().SetLabelWidth(m_defLabelValue);
        piece.GetPieceLabelData().SetLabelHeight(m_defLabelValue);
        piece.GetPieceLabelData().SetCenterPin(NULL_ID);
        piece.GetPieceLabelData().SetTopLeftPin(getCurrentObjectId(uiTabLabels->comboBoxDLTopLeftPin));
        piece.GetPieceLabelData().SetBottomRightPin(getCurrentObjectId(uiTabLabels->comboBoxDLBottomRightPin));
    }

    piece.GetPatternLabelData().SetEnabled(uiTabLabels->groupBoxPatternLabel->isChecked());
    piece.GetPatternLabelData().SetRotation(GetFormulaFromUser(uiTabLabels->lineEditPLAngleFormula));
    piece.GetPatternLabelData().SetFontSize(uiTabLabels->comboBoxPatternLabelSize->currentData().toInt());

    if (not flagPPin)
    {
        piece.GetPatternLabelData().SetCenterPin(getCurrentObjectId(uiTabLabels->comboBoxPLCenterPin));
        piece.GetPatternLabelData().SetTopLeftPin(NULL_ID);
        piece.GetPatternLabelData().SetBottomRightPin(NULL_ID);
        piece.GetPatternLabelData().SetLabelWidth(GetFormulaFromUser(uiTabLabels->lineEditPLWidthFormula));
        piece.GetPatternLabelData().SetLabelHeight(GetFormulaFromUser(uiTabLabels->lineEditPLHeightFormula));
    }
    else
    {
        piece.GetPatternLabelData().SetCenterPin(NULL_ID);
        piece.GetPatternLabelData().SetTopLeftPin(getCurrentObjectId(uiTabLabels->comboBoxPLTopLeftPin));
        piece.GetPatternLabelData().SetBottomRightPin(getCurrentObjectId(uiTabLabels->comboBoxPLBottomRightPin));
        piece.GetPatternLabelData().SetLabelWidth(m_defLabelValue);
        piece.GetPatternLabelData().SetLabelHeight(m_defLabelValue);
    }

    piece.GetGrainlineGeometry().SetEnabled(uiTabGrainline->groupBoxGrainline->isChecked());
    piece.GetGrainlineGeometry().SetArrowType(
        static_cast<GrainlineArrowDirection>(uiTabGrainline->comboBoxArrow->currentData().toInt()));
    piece.GetGrainlineGeometry().SetVisible(uiTabGrainline->checkBoxGrainlineVisible->isChecked());

    if (not flagGPin)
    {
        piece.GetGrainlineGeometry().SetRotation(GetFormulaFromUser(uiTabGrainline->lineEditRotFormula));
        piece.GetGrainlineGeometry().SetLength(GetFormulaFromUser(uiTabGrainline->lineEditLenFormula));
        piece.GetGrainlineGeometry().SetCenterPin(getCurrentObjectId(uiTabGrainline->comboBoxGrainlineCenterPin));
        piece.GetGrainlineGeometry().SetTopPin(NULL_ID);
        piece.GetGrainlineGeometry().SetBottomPin(NULL_ID);
    }
    else
    {
        piece.GetGrainlineGeometry().SetRotation(QString::number(90));
        piece.GetGrainlineGeometry().SetLength(m_defLabelValue);
        piece.GetGrainlineGeometry().SetCenterPin(NULL_ID);
        piece.GetGrainlineGeometry().SetTopPin(getCurrentObjectId(uiTabGrainline->comboBoxGrainlineTopPin));
        piece.GetGrainlineGeometry().SetBottomPin(getCurrentObjectId(uiTabGrainline->comboBoxGrainlineBottomPin));
    }

    return piece;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::NewMainPathItem(const VPieceNode &node)
{
    NewNodeItem(uiTabPaths->listWidgetMainPath, node);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSeamAllowance::GetPathName(quint32 path, bool reverse) const -> QString
{
    QString name;

    if (path > NULL_ID)
    {
        name = CurrentPath(path).GetName();

        if (reverse)
        {
            name = QStringLiteral("- ") + name;
        }
    }

    return name;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSeamAllowance::MainPathIsValid() const -> bool
{
    if (CreatePiece().MainPathPoints(data).count() < 3)
    {
        uiTabPaths->helpLabel->setText(DialogWarningIcon(uiTabPaths->helpLabel) + tr("You need more points!"));
        return false;
    }

    if (not MainPathIsClockwise())
    {
        uiTabPaths->helpLabel->setText(DialogWarningIcon(uiTabPaths->helpLabel) +
                                       tr("You have to choose points in a clockwise direction!"));
        return false;
    }

    QString error;
    if (FirstPointEqualLast(uiTabPaths->listWidgetMainPath, data, error))
    {
        uiTabPaths->helpLabel->setText(QStringLiteral("%1%2 %3").arg(
            DialogWarningIcon(uiTabPaths->helpLabel), tr("First point cannot be equal to the last point!"), error));
        return false;
    }

    error.clear();
    if (DoublePoints(uiTabPaths->listWidgetMainPath, data, error))
    {
        uiTabPaths->helpLabel->setText(QStringLiteral("%1%2 %3").arg(DialogWarningIcon(uiTabPaths->helpLabel),
                                                                     tr("You have double points!"), error));
        return false;
    }

    error.clear();
    if (DoubleCurves(uiTabPaths->listWidgetMainPath, data, error))
    {
        uiTabPaths->helpLabel->setText(QStringLiteral("%1%2 %3").arg(DialogWarningIcon(uiTabPaths->helpLabel),
                                                                     tr("The same curve repeats twice!"), error));
        return false;
    }

    if (not EachPointLabelIsUnique(uiTabPaths->listWidgetMainPath))
    {
        uiTabPaths->helpLabel->setText(DialogWarningIcon(uiTabPaths->helpLabel) +
                                       tr("Each point in the path must be unique!"));
        return false;
    }

    error.clear();
    if (InvalidSegment(uiTabPaths->listWidgetMainPath, data, error))
    {
        uiTabPaths->helpLabel->setText(
            QStringLiteral("%1%2 %3").arg(DialogWarningIcon(uiTabPaths->helpLabel), tr("Invalid segment!"), error));
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSeamAllowance::MirrorLineIsValid() const -> bool
{
    ChangeColor(uiTabPaths->labelMLStartPoint, OkColor(this));
    ChangeColor(uiTabPaths->labelMLEndPoint, OkColor(this));

    const quint32 startPoint = getCurrentObjectId(uiTabPaths->comboBoxMLStartPoint);
    const quint32 endPoint = getCurrentObjectId(uiTabPaths->comboBoxMLEndPoint);

    if (startPoint == NULL_ID && endPoint == NULL_ID)
    {
        return true;
    }

    if (startPoint == NULL_ID && endPoint != NULL_ID)
    {
        ChangeColor(uiTabPaths->labelMLStartPoint, errorColor);
        uiTabPaths->helpLabel->setText(DialogWarningIcon(uiTabPaths->helpLabel) +
                                       tr("Invalid mirror line start point!"));
        return false;
    }

    if (startPoint != NULL_ID && endPoint == NULL_ID)
    {
        ChangeColor(uiTabPaths->labelMLEndPoint, errorColor);
        uiTabPaths->helpLabel->setText(DialogWarningIcon(uiTabPaths->helpLabel) + tr("Invalid mirror line end point!"));
        return false;
    }

    if (startPoint == endPoint)
    {
        ChangeColor(uiTabPaths->labelMLStartPoint, errorColor);
        ChangeColor(uiTabPaths->labelMLEndPoint, errorColor);
        uiTabPaths->helpLabel->setText(DialogWarningIcon(uiTabPaths->helpLabel) +
                                       tr("Start and end mirror line points must be unique!"));
        return false;
    }

    QSet<quint32> uniquePoints;
    for (int i = 0; i < uiTabPaths->listWidgetMainPath->count(); ++i)
    {
        const QListWidgetItem *rowItem = uiTabPaths->listWidgetMainPath->item(i);
        SCASSERT(rowItem != nullptr)
        const auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
        if (rowNode.GetTypeTool() == Tool::NodePoint && not rowNode.IsExcluded())
        {
            uniquePoints.insert(rowNode.GetId());
        }
    }

    if (!uniquePoints.contains(startPoint))
    {
        ChangeColor(uiTabPaths->labelMLStartPoint, errorColor);
        uiTabPaths->helpLabel->setText(DialogWarningIcon(uiTabPaths->helpLabel) +
                                       tr("Invalid mirror line start point!"));
        return false;
    }

    if (!uniquePoints.contains(endPoint))
    {
        ChangeColor(uiTabPaths->labelMLEndPoint, errorColor);
        uiTabPaths->helpLabel->setText(DialogWarningIcon(uiTabPaths->helpLabel) + tr("Invalid mirror line end point!"));
        return false;
    }

    if (!MirrorLinePointsNeighbors(uiTabPaths->listWidgetMainPath, startPoint, endPoint))
    {
        ChangeColor(uiTabPaths->labelMLStartPoint, errorColor);
        ChangeColor(uiTabPaths->labelMLEndPoint, errorColor);
        uiTabPaths->helpLabel->setText(DialogWarningIcon(uiTabPaths->helpLabel) +
                                       tr("Mirror points must be neighbors!"));
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ValidObjects(bool value)
{
    flagMainPathIsValid = value;
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSeamAllowance::MainPathIsClockwise() const -> bool
{
    QVector<QPointF> points;
    CastTo(CreatePiece().MainPathPoints(data), points);

    if (points.count() < 3)
    {
        return false;
    }

    const qreal res = VPiece::SumTrapezoids(points);
    return res < 0;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitNodesList()
{
    const quint32 id = uiTabPaths->comboBoxNodes->currentData().toUInt();

    {
        const QSignalBlocker blocker(uiTabPaths->comboBoxNodes);
        uiTabPaths->comboBoxNodes->clear();

        const QVector<VPieceNode> nodes = GetListInternals<VPieceNode>(uiTabPaths->listWidgetMainPath);

        for (const auto &node : nodes)
        {
            if (node.GetTypeTool() == Tool::NodePoint && not node.IsExcluded())
            {
                const QString name = GetNodeName(data, node);

                uiTabPaths->comboBoxNodes->addItem(name, node.GetId());
            }
        }
    }

    const int index = uiTabPaths->comboBoxNodes->findData(id);
    if (index != -1)
    {
        uiTabPaths->comboBoxNodes->setCurrentIndex(index);
        NodeChanged(index); // Need in case combox index was not changed
    }
    else
    {
        uiTabPaths->comboBoxNodes->count() > 0 ? NodeChanged(0) : NodeChanged(-1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitPassmarksList()
{
    const quint32 id = uiTabPassmarks->comboBoxPassmarks->currentData().toUInt();

    {
        const QSignalBlocker blocker(uiTabPassmarks->comboBoxPassmarks);
        uiTabPassmarks->comboBoxPassmarks->clear();

        const QVector<VPieceNode> nodes = GetListInternals<VPieceNode>(uiTabPaths->listWidgetMainPath);

        for (const auto &node : nodes)
        {
            if (node.GetTypeTool() == Tool::NodePoint && node.IsPassmark())
            {
                const QString name = GetNodeName(data, node);

                uiTabPassmarks->comboBoxPassmarks->addItem(name, node.GetId());
            }
        }
    }

    const int index = uiTabPassmarks->comboBoxPassmarks->findData(id);
    if (index != -1)
    {
        uiTabPassmarks->comboBoxPassmarks->setCurrentIndex(index);
        PassmarkChanged(index); // Need in case combox index was not changed
    }
    else
    {
        uiTabPassmarks->comboBoxPassmarks->count() > 0 ? PassmarkChanged(0) : PassmarkChanged(-1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSeamAllowance::GetItemById(quint32 id) -> QListWidgetItem *
{
    for (qint32 i = 0; i < uiTabPaths->listWidgetMainPath->count(); ++i)
    {
        QListWidgetItem *item = uiTabPaths->listWidgetMainPath->item(i);
        const auto node = qvariant_cast<VPieceNode>(item->data(Qt::UserRole));

        if (node.GetId() == id)
        {
            return item;
        }
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSeamAllowance::GetLastId() const -> quint32
{
    const int count = uiTabPaths->listWidgetMainPath->count();
    if (count > 0)
    {
        QListWidgetItem *item = uiTabPaths->listWidgetMainPath->item(count - 1);
        const auto node = qvariant_cast<VPieceNode>(item->data(Qt::UserRole));
        return node.GetId();
    }

    return NULL_ID;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetCurrentSABefore(const QString &formula)
{
    UpdateNodeSABefore(formula);
    ListChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetCurrentSAAfter(const QString &formula)
{
    UpdateNodeSAAfter(formula);
    ListChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::UpdateNodeSABefore(const QString &formula)
{
    const int index = uiTabPaths->comboBoxNodes->currentIndex();
    if (index != -1)
    {
        QListWidgetItem *rowItem = GetItemById(uiTabPaths->comboBoxNodes->currentData().toUInt());
        if (rowItem)
        {
            auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetFormulaSABefore(formula);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::UpdateNodeSAAfter(const QString &formula)
{
    const int index = uiTabPaths->comboBoxNodes->currentIndex();
    if (index != -1)
    {
        QListWidgetItem *rowItem = GetItemById(uiTabPaths->comboBoxNodes->currentData().toUInt());
        if (rowItem)
        {
            auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetFormulaSAAfter(formula);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::UpdateNodePassmarkLength(const QString &formula)
{
    const int index = uiTabPassmarks->comboBoxPassmarks->currentIndex();
    if (index != -1)
    {
        QListWidgetItem *rowItem = GetItemById(uiTabPassmarks->comboBoxPassmarks->currentData().toUInt());
        if (rowItem)
        {
            auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetFormulaPassmarkLength(formula);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::UpdateNodePassmarkWidth(const QString &formula)
{
    const int index = uiTabPassmarks->comboBoxPassmarks->currentIndex();
    if (index != -1)
    {
        QListWidgetItem *rowItem = GetItemById(uiTabPassmarks->comboBoxPassmarks->currentData().toUInt());
        if (rowItem)
        {
            auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetFormulaPassmarkWidth(formula);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::UpdateNodePassmarkAngle(const QString &formula)
{
    const int index = uiTabPassmarks->comboBoxPassmarks->currentIndex();
    if (index != -1)
    {
        QListWidgetItem *rowItem = GetItemById(uiTabPassmarks->comboBoxPassmarks->currentData().toUInt());
        if (rowItem)
        {
            auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetFormulaPassmarkAngle(formula);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitFancyTabBar()
{
    m_ftb->InsertTab(TabOrder::Paths, QIcon("://icon/32x32/paths.png"), tr("Paths"));
    m_ftb->InsertTab(TabOrder::Pins, QIcon("://icon/32x32/pins.png"), tr("Pins"));
    m_ftb->InsertTab(TabOrder::Labels, QIcon("://icon/32x32/labels.png"), tr("Labels"));
    m_ftb->InsertTab(TabOrder::Grainline, QIcon("://icon/32x32/grainline.png"), tr("Grainline"));
    m_ftb->InsertTab(TabOrder::Passmarks, QIcon("://icon/32x32/passmark.png"), tr("Passmarks"));
    m_ftb->InsertTab(TabOrder::PlaceLabels, QIcon("://icon/32x32/button.png"), tr("Place label"));
    m_ftb->InsertTab(TabOrder::FoldLine, QIcon("://icon/32x32/fold.png"), tr("Fold line"));

    ui->horizontalLayout->addWidget(m_ftb, 0, Qt::AlignLeft);

    m_ftb->SetTabEnabled(TabOrder::Paths, true);

    m_tabPaths->hide();
    uiTabPaths->setupUi(m_tabPaths);
    ui->horizontalLayout->addWidget(m_tabPaths, 1);

    m_tabLabels->hide();
    uiTabLabels->setupUi(m_tabLabels);
    ui->horizontalLayout->addWidget(m_tabLabels, 1);

    m_tabGrainline->hide();
    uiTabGrainline->setupUi(m_tabGrainline);
    ui->horizontalLayout->addWidget(m_tabGrainline, 1);

    m_tabPins->hide();
    uiTabPins->setupUi(m_tabPins);
    ui->horizontalLayout->addWidget(m_tabPins, 1);

    m_tabPassmarks->hide();
    uiTabPassmarks->setupUi(m_tabPassmarks);
    ui->horizontalLayout->addWidget(m_tabPassmarks, 1);

    m_tabPlaceLabels->hide();
    uiTabPlaceLabels->setupUi(m_tabPlaceLabels);
    ui->horizontalLayout->addWidget(m_tabPlaceLabels, 1);

    m_tabFoldLine->hide();
    uiTabFoldLine->setupUi(m_tabFoldLine);
    ui->horizontalLayout->addWidget(m_tabFoldLine, 1);

    connect(m_ftb, &FancyTabBar::CurrentChanged, this, &DialogSeamAllowance::FancyTabChanged);
    connect(uiTabLabels->tabWidget, &QTabWidget::currentChanged, this, &DialogSeamAllowance::TabChanged);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitMainPathTab()
{
    uiTabPaths->checkBoxHideMainPath->setChecked(VAbstractApplication::VApp()->Settings()->IsHideMainPath());

    uiTabPaths->listWidgetMainPath->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(uiTabPaths->listWidgetMainPath, &QListWidget::customContextMenuRequested, this,
            &DialogSeamAllowance::ShowMainPathContextMenu);
    connect(uiTabPaths->listWidgetMainPath->model(), &QAbstractItemModel::rowsMoved, this,
            &DialogSeamAllowance::ListChanged);
    connect(uiTabPaths->listWidgetMainPath, &QListWidget::itemSelectionChanged, this,
            &DialogSeamAllowance::SetMoveControls);
    connect(uiTabPaths->listWidgetMainPath, &QListWidget::itemSelectionChanged, this,
            &DialogSeamAllowance::SetOptionControls);

    connect(uiTabPaths->listWidgetMainPath->model(), &QAbstractItemModel::rowsMoved, this,
            [this]() { ValidObjects(MainPathIsValid()); });

    connect(uiTabPaths->toolButtonTop, &QToolButton::clicked, this,
            [this]()
            {
                MoveListRowTop(uiTabPaths->listWidgetMainPath);
                ValidObjects(MainPathIsValid());
            });
    connect(uiTabPaths->toolButtonUp, &QToolButton::clicked, this,
            [this]()
            {
                MoveListRowUp(uiTabPaths->listWidgetMainPath);
                ValidObjects(MainPathIsValid());
            });
    connect(uiTabPaths->toolButtonDown, &QToolButton::clicked, this,
            [this]()
            {
                MoveListRowDown(uiTabPaths->listWidgetMainPath);
                ValidObjects(MainPathIsValid());
            });
    connect(uiTabPaths->toolButtonBottom, &QToolButton::clicked, this,
            [this]()
            {
                MoveListRowBottom(uiTabPaths->listWidgetMainPath);
                ValidObjects(MainPathIsValid());
            });

    connect(uiTabPaths->comboBoxMLStartPoint, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogSeamAllowance::MirrorLinePointChanged);
    connect(uiTabPaths->comboBoxMLEndPoint, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogSeamAllowance::MirrorLinePointChanged);

    connect(uiTabPaths->toolButtonReverse, &QToolButton::toggled, this,
            [this]()
            {
                const int row = uiTabPaths->listWidgetMainPath->currentRow();
                if (row < 0)
                {
                    uiTabPaths->toolButtonReverse->setEnabled(false);
                    return;
                }

                QListWidgetItem *rowItem = uiTabPaths->listWidgetMainPath->item(row);
                SCASSERT(rowItem != nullptr)
                auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

                rowNode.SetReverse(not rowNode.GetReverse());
                rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
                rowItem->setText(GetNodeName(data, rowNode, true));
            });

    connect(uiTabPaths->toolButtonExcluded, &QToolButton::toggled, this,
            [this]()
            {
                const int row = uiTabPaths->listWidgetMainPath->currentRow();
                if (row < 0)
                {
                    uiTabPaths->toolButtonExcluded->setEnabled(false);
                    return;
                }

                QListWidgetItem *rowItem = uiTabPaths->listWidgetMainPath->item(row);
                SCASSERT(rowItem != nullptr)
                auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

                rowNode.SetExcluded(not rowNode.IsExcluded());
                rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
                rowItem->setText(GetNodeName(data, rowNode, true));
                rowItem->setFont(NodeFont(rowItem->font(), rowNode.IsExcluded()));
            });

    connect(uiTabPaths->toolButtonTurnPoint, &QToolButton::toggled, this,
            [this]()
            {
                const int row = uiTabPaths->listWidgetMainPath->currentRow();
                if (row < 0)
                {
                    uiTabPaths->toolButtonTurnPoint->setEnabled(false);
                    return;
                }

                QListWidgetItem *rowItem = uiTabPaths->listWidgetMainPath->item(row);
                SCASSERT(rowItem != nullptr)
                auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

                rowNode.SetTurnPoint(not rowNode.IsTurnPoint());
                rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
                rowItem->setText(GetNodeName(data, rowNode, true));
            });

    connect(uiTabPaths->toolButtonCheckUniqness, &QToolButton::toggled, this,
            [this]()
            {
                const int row = uiTabPaths->listWidgetMainPath->currentRow();
                if (row < 0)
                {
                    uiTabPaths->toolButtonCheckUniqness->setEnabled(false);
                    return;
                }

                QListWidgetItem *rowItem = uiTabPaths->listWidgetMainPath->item(row);
                SCASSERT(rowItem != nullptr)
                auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

                rowNode.SetCheckUniqueness(not rowNode.IsCheckUniqueness());
                rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
                rowItem->setText(GetNodeName(data, rowNode, true));
            });

    connect(uiTabPaths->toolButtonPassmark, &QToolButton::toggled, this,
            [this](bool checked)
            {
                const int row = uiTabPaths->listWidgetMainPath->currentRow();
                if (row < 0)
                {
                    uiTabPaths->toolButtonPassmark->setEnabled(false);
                    return;
                }

                QListWidgetItem *rowItem = uiTabPaths->listWidgetMainPath->item(row);
                SCASSERT(rowItem != nullptr)
                auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

                rowNode.SetPassmark(checked);
                rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
                rowItem->setText(GetNodeName(data, rowNode, true));
            });

    connect(uiTabPaths->toolButtonDuplicateNode, &QToolButton::toggled, this,
            [this]()
            {
                const int row = uiTabPaths->listWidgetMainPath->currentRow();
                if (row < 0)
                {
                    uiTabPaths->toolButtonDuplicateNode->setEnabled(false);
                    return;
                }

                const QListWidgetItem *rowItem = uiTabPaths->listWidgetMainPath->item(row);
                SCASSERT(rowItem != nullptr)
                auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

                VPieceNode duplicate = rowNode;
                duplicate.SetExcluded(true);

                auto *item = new QListWidgetItem(GetNodeName(data, duplicate, true));
                item->setFont(NodeFont(item->font(), true));
                item->setData(Qt::UserRole, QVariant::fromValue(duplicate));
                uiTabPaths->listWidgetMainPath->addItem(item);
            });

    connect(uiTabPaths->toolButtonDelete, &QToolButton::clicked, this,
            [this]()
            {
                const int row = uiTabPaths->listWidgetMainPath->currentRow();
                if (row < 0)
                {
                    uiTabPaths->toolButtonDelete->setEnabled(false);
                    return;
                }

                delete uiTabPaths->listWidgetMainPath->item(row);
            });

    uiTabPaths->listWidgetMainPath->installEventFilter(this);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitPieceTab()
{
    connect(uiTabPaths->lineEditName, &QLineEdit::textChanged, this, &DialogSeamAllowance::NameDetailChanged);

    uiTabPaths->lineEditShortName->setValidator(
        new QRegularExpressionValidator(QRegularExpression(VPiece::ShortNameRegExp()), this));

    uiTabPaths->lineEditName->setClearButtonEnabled(true);
    uiTabPaths->lineEditName->setText(GetDefaultPieceName());

    connect(uiTabPaths->checkBoxForbidFlipping, CHECKBOX_STATE_CHANGED, this,
            [this](int state)
            {
                if (state == Qt::Checked)
                {
                    uiTabPaths->checkBoxForceFlipping->setChecked(false);
                }
            });

    connect(uiTabPaths->checkBoxForceFlipping, CHECKBOX_STATE_CHANGED, this,
            [this](int state)
            {
                if (state == Qt::Checked)
                {
                    uiTabPaths->checkBoxForbidFlipping->setChecked(false);
                }
            });

    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    uiTabPaths->checkBoxForbidFlipping->setChecked(settings->GetForbidWorkpieceFlipping());
    uiTabPaths->checkBoxForceFlipping->setChecked(settings->GetForceWorkpieceFlipping());
    uiTabPaths->checkBoxOnDrawing->setChecked(settings->GetSewLineOnDrawing());

    connect(uiTabPaths->lineEditUUID, &QLineEdit::textChanged, this, &DialogSeamAllowance::DetailUUIDChanged);

    InitGradationPlaceholders();
    InitGradationPlaceholdersMenu();

    m_placeholdersMenu->setStyleSheet(QStringLiteral("QMenu { menu-scrollable: 1; }"));
    uiTabPaths->pushButtonGradationPlaceholder->setMenu(m_placeholdersMenu);
    uiTabPaths->pushButtonGradationPlaceholder->setProperty("hasMenu", true);
    uiTabPaths->pushButtonGradationPlaceholder->style()->unpolish(uiTabPaths->pushButtonGradationPlaceholder);
    uiTabPaths->pushButtonGradationPlaceholder->style()->polish(uiTabPaths->pushButtonGradationPlaceholder);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitSeamAllowanceTab()
{
    this->m_formulaBaseWidth = uiTabPaths->plainTextEditFormulaWidth->height();
    this->m_formulaBaseWidthBefore = uiTabPaths->plainTextEditFormulaWidthBefore->height();
    this->m_formulaBaseWidthAfter = uiTabPaths->plainTextEditFormulaWidthAfter->height();

    uiTabPaths->plainTextEditFormulaWidth->installEventFilter(this);
    uiTabPaths->plainTextEditFormulaWidthBefore->installEventFilter(this);
    uiTabPaths->plainTextEditFormulaWidthAfter->installEventFilter(this);

    m_timerWidth->setSingleShot(true);
    connect(m_timerWidth, &QTimer::timeout, this, &DialogSeamAllowance::EvalWidth);

    m_timerWidthBefore->setSingleShot(true);
    connect(m_timerWidthBefore, &QTimer::timeout, this, &DialogSeamAllowance::EvalWidthBefore);

    m_timerWidthAfter->setSingleShot(true);
    connect(m_timerWidthAfter, &QTimer::timeout, this, &DialogSeamAllowance::EvalWidthAfter);

    connect(uiTabPaths->checkBoxSeams, &QCheckBox::toggled, this,
            [this](bool enable)
            {
                uiTabPaths->checkBoxBuiltIn->setEnabled(enable);

                if (not enable)
                {
                    uiTabPaths->groupBoxAutomatic->setEnabled(enable);
                    uiTabPaths->groupBoxCustom->setEnabled(enable);
                }
                else
                {
                    emit uiTabPaths->checkBoxBuiltIn->toggled(uiTabPaths->checkBoxBuiltIn->isChecked());
                }
            });

    connect(uiTabPaths->checkBoxBuiltIn, &QCheckBox::toggled, this,
            [this](bool enable)
            {
                uiTabPaths->groupBoxAutomatic->setEnabled(not enable);
                uiTabPaths->groupBoxCustom->setEnabled(not enable);

                if (not enable)
                {
                    InitNodesList();
                }
            });

    // init the default seam allowance, convert the value if app unit is different than pattern unit
    m_saWidth = UnitConvertor(VAbstractApplication::VApp()->Settings()->GetDefaultSeamAllowance(),
                              StrToUnits(VAbstractApplication::VApp()->Settings()->GetUnit()),
                              VAbstractValApplication::VApp()->patternUnits());

    uiTabPaths->plainTextEditFormulaWidth->setPlainText(VAbstractApplication::VApp()->LocaleToString(m_saWidth));

    InitNodesList();
    connect(uiTabPaths->comboBoxNodes, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogSeamAllowance::NodeChanged);

    connect(uiTabPaths->pushButtonDefBefore, &QPushButton::clicked, this, &DialogSeamAllowance::ReturnDefBefore);
    connect(uiTabPaths->pushButtonDefAfter, &QPushButton::clicked, this, &DialogSeamAllowance::ReturnDefAfter);

    InitNodeAngles(uiTabPaths->comboBoxAngle);
    connect(uiTabPaths->comboBoxAngle, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogSeamAllowance::NodeAngleChanged);

    uiTabPaths->listWidgetCustomSA->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(uiTabPaths->listWidgetCustomSA, &QListWidget::customContextMenuRequested, this,
            &DialogSeamAllowance::ShowCustomSAContextMenu);
    connect(uiTabPaths->listWidgetCustomSA, &QListWidget::currentRowChanged, this,
            &DialogSeamAllowance::CustomSAChanged);
    connect(uiTabPaths->comboBoxStartPoint, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogSeamAllowance::CSAStartPointChanged);
    connect(uiTabPaths->comboBoxEndPoint, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogSeamAllowance::CSAEndPointChanged);
    connect(uiTabPaths->comboBoxIncludeType, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogSeamAllowance::CSAIncludeTypeChanged);

    connect(uiTabPaths->toolButtonExprWidth, &QPushButton::clicked, this, &DialogSeamAllowance::FXWidth);
    connect(uiTabPaths->toolButtonExprBefore, &QPushButton::clicked, this, &DialogSeamAllowance::FXWidthBefore);
    connect(uiTabPaths->toolButtonExprAfter, &QPushButton::clicked, this, &DialogSeamAllowance::FXWidthAfter);

    connect(uiTabPaths->plainTextEditFormulaWidth, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerWidth->start(formulaTimerTimeout); });

    connect(uiTabPaths->plainTextEditFormulaWidthBefore, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerWidthBefore->start(formulaTimerTimeout); });

    connect(uiTabPaths->plainTextEditFormulaWidthAfter, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerWidthAfter->start(formulaTimerTimeout); });

    connect(uiTabPaths->pushButtonGrowWidth, &QPushButton::clicked, this,
            &DialogSeamAllowance::DeployWidthFormulaTextEdit);
    connect(uiTabPaths->pushButtonGrowWidthBefore, &QPushButton::clicked, this,
            &DialogSeamAllowance::DeployWidthBeforeFormulaTextEdit);
    connect(uiTabPaths->pushButtonGrowWidthAfter, &QPushButton::clicked, this,
            &DialogSeamAllowance::DeployWidthAfterFormulaTextEdit);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitCSAPoint(QComboBox *box)
{
    SCASSERT(box != nullptr);
    box->clear();
    box->addItem(tr("Empty"), NULL_ID);

    const QVector<VPieceNode> nodes = GetListInternals<VPieceNode>(uiTabPaths->listWidgetMainPath);

    for (const auto &node : nodes)
    {
        if (node.GetTypeTool() == Tool::NodePoint && not node.IsExcluded())
        {
            const QString name = GetNodeName(data, node);
            box->addItem(name, node.GetId());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitPinPoint(QComboBox *box)
{
    SCASSERT(box != nullptr);

    quint32 currentId = NULL_ID;
    if (box->count() > 0)
    {
        currentId = box->currentData().toUInt();
    }

    box->clear();
    box->addItem('<'_L1 + tr("no pin") + '>'_L1, NULL_ID);

    const QVector<quint32> pins = GetListInternals<quint32>(uiTabPins->listWidgetPins);

    for (auto pin : pins)
    {
        box->addItem(data->GetGObject(pin)->name(), pin);
    }

    const int index = box->findData(currentId);
    if (index != -1)
    {
        box->setCurrentIndex(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitMirrorLine()
{
    const QVariant idStartPoint = uiTabPaths->comboBoxMLStartPoint->currentData();
    const QVariant idEndPoint = uiTabPaths->comboBoxMLEndPoint->currentData();

    InitMirrorLinePoint(uiTabPaths->comboBoxMLStartPoint);
    InitMirrorLinePoint(uiTabPaths->comboBoxMLEndPoint);

    uiTabPaths->comboBoxMLStartPoint->setCurrentIndex(uiTabPaths->comboBoxMLStartPoint->findData(idStartPoint));
    uiTabPaths->comboBoxMLEndPoint->setCurrentIndex(uiTabPaths->comboBoxMLEndPoint->findData(idEndPoint));

    flagMirrorLineIsValid = MirrorLineIsValid();
    uiTabPaths->checkBoxShowFullPiece->setEnabled(flagMirrorLineIsValid);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitMirrorLinePoint(QComboBox *box)
{
    SCASSERT(box != nullptr);
    const QSignalBlocker blocker(box);
    box->clear();

    box->addItem(tr("None"), QUuid());

    const QVector<VPieceNode> nodes = GetListInternals<VPieceNode>(uiTabPaths->listWidgetMainPath);

    for (const auto &node : nodes)
    {
        if (node.GetTypeTool() == Tool::NodePoint && not node.IsExcluded())
        {
            const QString name = GetNodeName(data, node);
            box->addItem(name, node.GetId());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitSAIncludeType()
{
    uiTabPaths->comboBoxIncludeType->clear();

    uiTabPaths->comboBoxIncludeType->addItem(tr("main path"),
                                             static_cast<unsigned char>(PiecePathIncludeType::AsMainPath));
    uiTabPaths->comboBoxIncludeType->addItem(tr("custom seam allowance"),
                                             static_cast<unsigned char>(PiecePathIncludeType::AsCustomSA));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitInternalPathsTab()
{
    uiTabPaths->listWidgetInternalPaths->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(uiTabPaths->listWidgetInternalPaths, &QListWidget::customContextMenuRequested, this,
            &DialogSeamAllowance::ShowInternalPathsContextMenu);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitPatternPieceDataTab()
{
    uiTabLabels->lineEditLetter->setClearButtonEnabled(true);
    uiTabLabels->lineEditAnnotation->setClearButtonEnabled(true);
    uiTabLabels->lineEditOrientation->setClearButtonEnabled(true);
    uiTabLabels->lineEditRotation->setClearButtonEnabled(true);
    uiTabLabels->lineEditTilt->setClearButtonEnabled(true);
    uiTabLabels->lineEditFoldPosition->setClearButtonEnabled(true);

    connect(uiTabLabels->pushButtonEditPieceLabel, &QPushButton::clicked, this, &DialogSeamAllowance::EditPieceLabel);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitLabelsTab()
{
    InitLabelFontSize(uiTabLabels->comboBoxPatternLabelSize);
    InitLabelFontSize(uiTabLabels->comboBoxPieceLabelSize);

    uiTabLabels->lineEditDLWidthFormula->setPlainText(m_defLabelValue);
    uiTabLabels->lineEditDLHeightFormula->setPlainText(m_defLabelValue);
    uiTabLabels->lineEditPLWidthFormula->setPlainText(m_defLabelValue);
    uiTabLabels->lineEditPLHeightFormula->setPlainText(m_defLabelValue);

    m_DLWidthBaseHeight = uiTabLabels->lineEditDLWidthFormula->height();
    m_DLHeightBaseHeight = uiTabLabels->lineEditDLHeightFormula->height();
    m_DLAngleBaseHeight = uiTabLabels->lineEditDLAngleFormula->height();

    connect(uiTabLabels->groupBoxDetailLabel, &QGroupBox::toggled, this, &DialogSeamAllowance::EnabledDetailLabel);
    InitPinPoint(uiTabLabels->comboBoxDLCenterPin);
    InitPinPoint(uiTabLabels->comboBoxDLTopLeftPin);
    InitPinPoint(uiTabLabels->comboBoxDLBottomRightPin);

    connect(uiTabLabels->comboBoxDLTopLeftPin, &QComboBox::currentTextChanged, this,
            &DialogSeamAllowance::DetailPinPointChanged);
    connect(uiTabLabels->comboBoxDLBottomRightPin, &QComboBox::currentTextChanged, this,
            &DialogSeamAllowance::DetailPinPointChanged);

    connect(uiTabLabels->pushButtonDLWidth, &QPushButton::clicked, this, &DialogSeamAllowance::EditDLFormula);
    connect(uiTabLabels->pushButtonDLHeight, &QPushButton::clicked, this, &DialogSeamAllowance::EditDLFormula);
    connect(uiTabLabels->pushButtonDLAngle, &QPushButton::clicked, this, &DialogSeamAllowance::EditDLFormula);

    connect(uiTabLabels->lineEditDLWidthFormula, &QPlainTextEdit::textChanged, this,
            &DialogSeamAllowance::UpdateDetailLabelValues);
    connect(uiTabLabels->lineEditDLHeightFormula, &QPlainTextEdit::textChanged, this,
            &DialogSeamAllowance::UpdateDetailLabelValues);
    connect(uiTabLabels->lineEditDLAngleFormula, &QPlainTextEdit::textChanged, this,
            &DialogSeamAllowance::UpdateDetailLabelValues);

    connect(uiTabLabels->pushButtonShowDLWidth, &QPushButton::clicked, this, &DialogSeamAllowance::DeployDLWidth);
    connect(uiTabLabels->pushButtonShowDLHeight, &QPushButton::clicked, this, &DialogSeamAllowance::DeployDLHeight);
    connect(uiTabLabels->pushButtonShowDLAngle, &QPushButton::clicked, this, &DialogSeamAllowance::DeployDLAngle);

    EnabledDetailLabel();

    m_PLWidthBaseHeight = uiTabLabels->lineEditPLWidthFormula->height();
    m_PLHeightBaseHeight = uiTabLabels->lineEditPLHeightFormula->height();
    m_PLAngleBaseHeight = uiTabLabels->lineEditPLAngleFormula->height();

    connect(uiTabLabels->groupBoxPatternLabel, &QGroupBox::toggled, this, &DialogSeamAllowance::EnabledPatternLabel);
    InitPinPoint(uiTabLabels->comboBoxPLCenterPin);
    InitPinPoint(uiTabLabels->comboBoxPLTopLeftPin);
    InitPinPoint(uiTabLabels->comboBoxPLBottomRightPin);

    connect(uiTabLabels->comboBoxPLTopLeftPin, &QComboBox::currentTextChanged, this,
            &DialogSeamAllowance::PatternPinPointChanged);
    connect(uiTabLabels->comboBoxPLBottomRightPin, &QComboBox::currentTextChanged, this,
            &DialogSeamAllowance::PatternPinPointChanged);

    connect(uiTabLabels->pushButtonPLWidth, &QPushButton::clicked, this, &DialogSeamAllowance::EditPLFormula);
    connect(uiTabLabels->pushButtonPLHeight, &QPushButton::clicked, this, &DialogSeamAllowance::EditPLFormula);
    connect(uiTabLabels->pushButtonPLAngle, &QPushButton::clicked, this, &DialogSeamAllowance::EditPLFormula);

    connect(uiTabLabels->lineEditPLWidthFormula, &QPlainTextEdit::textChanged, this,
            &DialogSeamAllowance::UpdatePatternLabelValues);
    connect(uiTabLabels->lineEditPLHeightFormula, &QPlainTextEdit::textChanged, this,
            &DialogSeamAllowance::UpdatePatternLabelValues);
    connect(uiTabLabels->lineEditPLAngleFormula, &QPlainTextEdit::textChanged, this,
            &DialogSeamAllowance::UpdatePatternLabelValues);

    connect(uiTabLabels->pushButtonShowPLWidth, &QPushButton::clicked, this, &DialogSeamAllowance::DeployPLWidth);
    connect(uiTabLabels->pushButtonShowPLHeight, &QPushButton::clicked, this, &DialogSeamAllowance::DeployPLHeight);
    connect(uiTabLabels->pushButtonShowPLAngle, &QPushButton::clicked, this, &DialogSeamAllowance::DeployPLAngle);

    EnabledPatternLabel();

    // Pattern label data
    uiTabLabels->lineEditCustomerEmail->setClearButtonEnabled(true);

    if (VAbstractValApplication::VApp()->GetMeasurementsType() == MeasurementsType::Individual)
    {
        uiTabLabels->lineEditCustomerName->setText(VAbstractValApplication::VApp()->GetCustomerName());
        uiTabLabels->lineEditCustomerName->setDisabled(true);
        uiTabLabels->lineEditCustomerName->setToolTip(tr("The customer name from individual measurements"));

        uiTabLabels->lineEditCustomerEmail->setText(VAbstractValApplication::VApp()->CustomerEmail());
        uiTabLabels->lineEditCustomerEmail->setDisabled(true);
        uiTabLabels->lineEditCustomerEmail->setToolTip(tr("The customer email from individual measurements"));

        uiTabLabels->dateEditCustomerBirthDate->setDate(VAbstractValApplication::VApp()->GetCustomerBirthDate());
        uiTabLabels->dateEditCustomerBirthDate->setDisabled(true);
        uiTabLabels->dateEditCustomerBirthDate->setToolTip(tr("The customer birth date from individual measurements"));
    }
    else
    {
        uiTabLabels->lineEditCustomerName->setText(m_doc->GetCustomerName());
        uiTabLabels->lineEditCustomerEmail->setText(m_doc->GetCustomerEmail());
        uiTabLabels->dateEditCustomerBirthDate->setDate(m_doc->GetCustomerBirthDate());
    }

    connect(uiTabLabels->lineEditPatternName, &QLineEdit::editingFinished, this,
            &DialogSeamAllowance::PatternLabelDataChanged);
    connect(uiTabLabels->lineEditPatternNumber, &QLineEdit::editingFinished, this,
            &DialogSeamAllowance::PatternLabelDataChanged);
    connect(uiTabLabels->lineEditCompanyName, &QLineEdit::editingFinished, this,
            &DialogSeamAllowance::PatternLabelDataChanged);
    connect(uiTabLabels->lineEditCustomerName, &QLineEdit::editingFinished, this,
            &DialogSeamAllowance::PatternLabelDataChanged);
    connect(uiTabLabels->lineEditCustomerEmail, &QLineEdit::editingFinished, this,
            &DialogSeamAllowance::PatternLabelDataChanged);
    connect(uiTabLabels->dateEditCustomerBirthDate, &QDateEdit::editingFinished, this,
            &DialogSeamAllowance::PatternLabelDataChanged);

    connect(uiTabLabels->pushButtonEditPatternLabel, &QPushButton::clicked, this,
            &DialogSeamAllowance::EditPatternLabel);
    connect(uiTabLabels->pushButtonPatternMaterials, &QPushButton::clicked, this,
            &DialogSeamAllowance::ManagePatternMaterials);

    connect(uiTabLabels->comboBoxDateFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogSeamAllowance::PatternLabelDataChanged);
    connect(uiTabLabels->comboBoxTimeFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogSeamAllowance::PatternLabelDataChanged);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitGrainlineTab()
{
    uiTabGrainline->lineEditLenFormula->setPlainText(
        QString().setNum(UnitConvertor(10, Unit::Cm, *data->GetPatternUnit())));

    connect(uiTabGrainline->groupBoxGrainline, &QGroupBox::toggled, this, &DialogSeamAllowance::EnabledGrainline);
    connect(uiTabGrainline->pushButtonRot, &QPushButton::clicked, this, &DialogSeamAllowance::EditGrainlineFormula);
    connect(uiTabGrainline->pushButtonLen, &QPushButton::clicked, this, &DialogSeamAllowance::EditGrainlineFormula);
    connect(uiTabGrainline->lineEditLenFormula, &QPlainTextEdit::textChanged, this,
            &DialogSeamAllowance::UpdateGrainlineValues);
    connect(uiTabGrainline->lineEditRotFormula, &QPlainTextEdit::textChanged, this,
            &DialogSeamAllowance::UpdateGrainlineValues);

    connect(uiTabGrainline->pushButtonShowRot, &QPushButton::clicked, this,
            &DialogSeamAllowance::DeployGrainlineRotation);
    connect(uiTabGrainline->pushButtonShowLen, &QPushButton::clicked, this,
            &DialogSeamAllowance::DeployGrainlineLength);

    EnabledGrainline();

    uiTabGrainline->comboBoxArrow->addItem(tr("Two ways (Up/Down)", "grainline direction"),
                                           static_cast<int>(GrainlineArrowDirection::twoWaysUpDown));
    uiTabGrainline->comboBoxArrow->addItem(tr("One way (Up)", "grainline direction"),
                                           static_cast<int>(GrainlineArrowDirection::oneWayUp));
    uiTabGrainline->comboBoxArrow->addItem(tr("One way (Down)", "grainline direction"),
                                           static_cast<int>(GrainlineArrowDirection::oneWayDown));
    uiTabGrainline->comboBoxArrow->addItem(tr("Four ways", "grainline direction"),
                                           static_cast<int>(GrainlineArrowDirection::fourWays));
    uiTabGrainline->comboBoxArrow->addItem(tr("Two ways (Up/Left)", "grainline direction"),
                                           static_cast<int>(GrainlineArrowDirection::twoWaysUpLeft));
    uiTabGrainline->comboBoxArrow->addItem(tr("Two ways (Up/Right)", "grainline direction"),
                                           static_cast<int>(GrainlineArrowDirection::twoWaysUpRight));
    uiTabGrainline->comboBoxArrow->addItem(tr("Two ways (Down/Left)", "grainline direction"),
                                           static_cast<int>(GrainlineArrowDirection::twoWaysDownLeft));
    uiTabGrainline->comboBoxArrow->addItem(tr("Two ways (Down/Right)", "grainline direction"),
                                           static_cast<int>(GrainlineArrowDirection::twoWaysDownRight));
    uiTabGrainline->comboBoxArrow->addItem(tr("Three ways (Up/Down/Left)", "grainline direction"),
                                           static_cast<int>(GrainlineArrowDirection::threeWaysUpDownLeft));
    uiTabGrainline->comboBoxArrow->addItem(tr("Three ways (Up/Down/Right)", "grainline direction"),
                                           static_cast<int>(GrainlineArrowDirection::threeWaysUpDownRight));
    uiTabGrainline->comboBoxArrow->addItem(tr("Three ways (Up/Left/Right)", "grainline direction"),
                                           static_cast<int>(GrainlineArrowDirection::threeWaysUpLeftRight));
    uiTabGrainline->comboBoxArrow->addItem(tr("Three ways (Down/Left/Right)", "grainline direction"),
                                           static_cast<int>(GrainlineArrowDirection::threeWaysDownLeftRight));

    m_iRotBaseHeight = uiTabGrainline->lineEditRotFormula->height();
    m_iLenBaseHeight = uiTabGrainline->lineEditLenFormula->height();

    InitPinPoint(uiTabGrainline->comboBoxGrainlineCenterPin);
    InitPinPoint(uiTabGrainline->comboBoxGrainlineTopPin);
    InitPinPoint(uiTabGrainline->comboBoxGrainlineBottomPin);

    connect(uiTabGrainline->comboBoxGrainlineTopPin, &QComboBox::currentTextChanged, this,
            &DialogSeamAllowance::GrainlinePinPointChanged);
    connect(uiTabGrainline->comboBoxGrainlineBottomPin, &QComboBox::currentTextChanged, this,
            &DialogSeamAllowance::GrainlinePinPointChanged);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitPinsTab()
{
    uiTabPins->listWidgetPins->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(uiTabPins->listWidgetPins, &QListWidget::customContextMenuRequested, this,
            &DialogSeamAllowance::ShowPinsContextMenu);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitPassmarksTab()
{
    // Length formula
    this->m_formulaBasePassmarkLength = uiTabPassmarks->plainTextEditPassmarkLength->height();
    uiTabPassmarks->plainTextEditPassmarkLength->installEventFilter(this);
    m_timerPassmarkLength->setSingleShot(true);

    connect(m_timerPassmarkLength, &QTimer::timeout, this, &DialogSeamAllowance::EvalPassmarkLength);
    connect(uiTabPassmarks->groupBoxManualLength, &QGroupBox::toggled, this,
            &DialogSeamAllowance::EnabledManualPassmarkLength);
    connect(uiTabPassmarks->toolButtonExprLength, &QPushButton::clicked, this, &DialogSeamAllowance::FXPassmarkLength);
    connect(uiTabPassmarks->plainTextEditPassmarkLength, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerPassmarkLength->start(formulaTimerTimeout); });
    connect(uiTabPassmarks->pushButtonGrowPassmarkLength, &QPushButton::clicked, this,
            &DialogSeamAllowance::DeployPassmarkLength);

    // Width formula
    this->m_formulaBasePassmarkWidth = uiTabPassmarks->plainTextEditPassmarkWidth->height();
    uiTabPassmarks->plainTextEditPassmarkWidth->installEventFilter(this);
    m_timerPassmarkWidth->setSingleShot(true);

    connect(m_timerPassmarkWidth, &QTimer::timeout, this, &DialogSeamAllowance::EvalPassmarkWidth);
    connect(uiTabPassmarks->groupBoxManualWidth, &QGroupBox::toggled, this,
            &DialogSeamAllowance::EnabledManualPassmarkWidth);
    connect(uiTabPassmarks->toolButtonExprWidth, &QPushButton::clicked, this, &DialogSeamAllowance::FXPassmarkWidth);
    connect(uiTabPassmarks->plainTextEditPassmarkWidth, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerPassmarkWidth->start(formulaTimerTimeout); });
    connect(uiTabPassmarks->pushButtonGrowPassmarkWidth, &QPushButton::clicked, this,
            &DialogSeamAllowance::DeployPassmarkWidth);

    // Angle formula
    this->m_formulaBasePassmarkAngle = uiTabPassmarks->plainTextEditPassmarkAngle->height();
    uiTabPassmarks->plainTextEditPassmarkAngle->installEventFilter(this);
    m_timerPassmarkAngle->setSingleShot(true);

    connect(m_timerPassmarkAngle, &QTimer::timeout, this, &DialogSeamAllowance::EvalPassmarkAngle);
    connect(uiTabPassmarks->groupBoxManualAngle, &QGroupBox::toggled, this,
            &DialogSeamAllowance::EnabledManualPassmarkAngle);
    connect(uiTabPassmarks->toolButtonExprAngle, &QPushButton::clicked, this, &DialogSeamAllowance::FXPassmarkAngle);
    connect(uiTabPassmarks->plainTextEditPassmarkAngle, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerPassmarkAngle->start(formulaTimerTimeout); });
    connect(uiTabPassmarks->pushButtonGrowPassmarkAngle, &QPushButton::clicked, this,
            &DialogSeamAllowance::DeployPassmarkAngle);

    // notch list
    InitPassmarksList();
    connect(uiTabPassmarks->comboBoxPassmarks, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogSeamAllowance::PassmarkChanged);

    connect(uiTabPassmarks->buttonGroupLineType, &QButtonGroup::idClicked, this,
            &DialogSeamAllowance::PassmarkLineTypeChanged);
    connect(uiTabPassmarks->buttonGroupAngleType, &QButtonGroup::idClicked, this,
            &DialogSeamAllowance::PassmarkAngleTypeChanged);
    connect(uiTabPassmarks->checkBoxShowSecondPassmark, CHECKBOX_STATE_CHANGED, this,
            &DialogSeamAllowance::PassmarkShowSecondChanged);
    connect(uiTabPassmarks->checkBoxClockwiseOpening, CHECKBOX_STATE_CHANGED, this,
            &DialogSeamAllowance::PassmarkClockwiseOrientationChanged);
    connect(uiTabPassmarks->checkBoxNotMirrored, CHECKBOX_STATE_CHANGED, this,
            &DialogSeamAllowance::PassmarkNotMirroredStateChanged);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitPlaceLabelsTab()
{
    uiTabPlaceLabels->listWidgetPlaceLabels->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(uiTabPlaceLabels->listWidgetPlaceLabels, &QListWidget::currentRowChanged, this,
            [this]()
            {
                if (not m_visSpecialPoints.isNull())
                {
                    m_visSpecialPoints->SetShowRect(true);
                    m_visSpecialPoints->SetRect(CurrentRect());
                    m_visSpecialPoints->RefreshGeometry();
                }
            });
    connect(uiTabPlaceLabels->listWidgetPlaceLabels, &QListWidget::customContextMenuRequested, this,
            &DialogSeamAllowance::ShowPlaceLabelsContextMenu);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitFoldLineTab()
{
    InitFoldLineType();

    InitFoldLabelFontSizes();

    // Height formula
    this->m_formulaBaseFoldHeight = uiTabFoldLine->plainTextEditHeight->height();
    uiTabFoldLine->plainTextEditHeight->installEventFilter(this);
    m_timerFoldHeight->setSingleShot(true);

    connect(m_timerFoldHeight, &QTimer::timeout, this, &DialogSeamAllowance::EvalFoldHeight);
    connect(uiTabFoldLine->groupBoxManualHeight, &QGroupBox::toggled, this,
            &DialogSeamAllowance::EnabledManualFoldHeight);
    connect(uiTabFoldLine->toolButtonExprHeight, &QPushButton::clicked, this, &DialogSeamAllowance::FXFoldHeight);
    connect(uiTabFoldLine->plainTextEditHeight, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerFoldHeight->start(formulaTimerTimeout); });
    connect(uiTabFoldLine->pushButtonGrowHeight, &QPushButton::clicked, this, &DialogSeamAllowance::DeployFoldHeight);

    // Width formula
    this->m_formulaBaseFoldWidth = uiTabFoldLine->plainTextEditWidth->height();
    uiTabFoldLine->plainTextEditWidth->installEventFilter(this);
    m_timerFoldWidth->setSingleShot(true);

    connect(m_timerFoldWidth, &QTimer::timeout, this, &DialogSeamAllowance::EvalFoldWidth);
    connect(uiTabFoldLine->groupBoxManualWidth, &QGroupBox::toggled, this,
            &DialogSeamAllowance::EnabledManualFoldWidth);
    connect(uiTabFoldLine->toolButtonExprWidth, &QPushButton::clicked, this, &DialogSeamAllowance::FXFoldWidth);
    connect(uiTabFoldLine->plainTextEditWidth, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerFoldWidth->start(formulaTimerTimeout); });
    connect(uiTabFoldLine->pushButtonGrowWidth, &QPushButton::clicked, this, &DialogSeamAllowance::DeployFoldWidth);

    // Center formula
    this->m_formulaBaseFoldCenter = uiTabFoldLine->plainTextEditCenter->height();
    uiTabFoldLine->plainTextEditCenter->installEventFilter(this);
    m_timerFoldCenter->setSingleShot(true);

    connect(m_timerFoldCenter, &QTimer::timeout, this, &DialogSeamAllowance::EvalFoldCenter);
    connect(uiTabFoldLine->groupBoxManualCenter, &QGroupBox::toggled, this,
            &DialogSeamAllowance::EnabledManualFoldCenter);
    connect(uiTabFoldLine->toolButtonExprCenter, &QPushButton::clicked, this, &DialogSeamAllowance::FXFoldCenter);
    connect(uiTabFoldLine->plainTextEditCenter, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerFoldCenter->start(formulaTimerTimeout); });
    connect(uiTabFoldLine->pushButtonGrowCenter, &QPushButton::clicked, this, &DialogSeamAllowance::DeployFoldCenter);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitAllPinComboboxes()
{
    InitPinPoint(uiTabGrainline->comboBoxGrainlineCenterPin);
    InitPinPoint(uiTabGrainline->comboBoxGrainlineTopPin);
    InitPinPoint(uiTabGrainline->comboBoxGrainlineBottomPin);

    InitPinPoint(uiTabLabels->comboBoxDLCenterPin);
    InitPinPoint(uiTabLabels->comboBoxDLTopLeftPin);
    InitPinPoint(uiTabLabels->comboBoxDLBottomRightPin);

    InitPinPoint(uiTabLabels->comboBoxPLCenterPin);
    InitPinPoint(uiTabLabels->comboBoxPLTopLeftPin);
    InitPinPoint(uiTabLabels->comboBoxPLBottomRightPin);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitFoldLineType()
{
    uiTabFoldLine->comboBoxType->clear();

    uiTabFoldLine->comboBoxType->addItem(tr("Two arrows (text above)"),
                                         static_cast<unsigned int>(FoldLineType::TwoArrowsTextAbove));
    uiTabFoldLine->comboBoxType->addItem(tr("Two arrows (text under)"),
                                         static_cast<unsigned int>(FoldLineType::TwoArrowsTextUnder));
    uiTabFoldLine->comboBoxType->addItem(tr("Two arrows"), static_cast<unsigned int>(FoldLineType::TwoArrows));
    uiTabFoldLine->comboBoxType->addItem(tr("Text"), static_cast<unsigned int>(FoldLineType::Text));
    uiTabFoldLine->comboBoxType->addItem(tr("Three dots"), static_cast<unsigned int>(FoldLineType::ThreeDots));
    uiTabFoldLine->comboBoxType->addItem(tr("Three X"), static_cast<unsigned int>(FoldLineType::ThreeX));
    uiTabFoldLine->comboBoxType->addItem(tr("None"), static_cast<unsigned int>(FoldLineType::None));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitFoldLabelFontSizes()
{
    uiTabFoldLine->comboBoxLabelFontSize->clear();

    // Get the available font sizes
    QList<int> const sizes = QFontDatabase::standardSizes();
    for (auto size : sizes)
    {
        if (size >= VCommonSettings::MinPieceLabelFontPointSize())
        {
            uiTabFoldLine->comboBoxLabelFontSize->addItem(QString::number(size), size);
        }
    }

    {
        const qint32 indexSize = uiTabFoldLine->comboBoxLabelFontSize->findData(defFoldLineFontSize);
        if (indexSize != -1)
        {
            uiTabFoldLine->comboBoxLabelFontSize->setCurrentIndex(indexSize);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitLabelFontSize(QComboBox *box)
{
    SCASSERT(box != nullptr);
    box->clear();
    box->addItem(tr("Default"), 0);

    // Get the available font sizes
    for (auto size : QFontDatabase::standardSizes())
    {
        if (size >= VCommonSettings::MinPieceLabelFontPointSize())
        {
            box->addItem(QString::number(size), size);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSeamAllowance::GetFormulaSAWidth() const -> QString
{
    QString const width = uiTabPaths->plainTextEditFormulaWidth->toPlainText();
    return VTranslateVars::TryFormulaFromUser(width, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSeamAllowance::UndoStack() -> QVector<QPointer<VUndoCommand>> &
{
    return m_undoStack;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetFormulaSAWidth(const QString &formula)
{
    const QString width = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (width.length() > 80)
    {
        this->DeployWidthFormulaTextEdit();
    }
    uiTabPaths->plainTextEditFormulaWidth->setPlainText(width);

    if (not applyAllowed)
    {
        auto *path = qobject_cast<VisToolPiece *>(vis);
        SCASSERT(path != nullptr)
        const VPiece p = CreatePiece();
        path->SetPiece(p);
    }

    MoveCursorToEnd(uiTabPaths->plainTextEditFormulaWidth);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetFormulaPassmarkLength(const QString &formula)
{
    const QString width = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (width.length() > 80)
    {
        this->DeployPassmarkLength();
    }
    uiTabPassmarks->plainTextEditPassmarkLength->setPlainText(width);

    MoveCursorToEnd(uiTabPassmarks->plainTextEditPassmarkLength);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetFormulaPassmarkWidth(const QString &formula)
{
    const QString width = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (width.length() > 80)
    {
        this->DeployPassmarkWidth();
    }
    uiTabPassmarks->plainTextEditPassmarkWidth->setPlainText(width);

    MoveCursorToEnd(uiTabPassmarks->plainTextEditPassmarkWidth);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetFormulaPassmarkAngle(const QString &formula)
{
    const QString width = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (width.length() > 80)
    {
        this->DeployPassmarkAngle();
    }
    uiTabPassmarks->plainTextEditPassmarkAngle->setPlainText(width);

    MoveCursorToEnd(uiTabPassmarks->plainTextEditPassmarkAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetFormulaFoldHeight(const QString &formula)
{
    const QString width = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (width.length() > 80)
    {
        this->DeployFoldHeight();
    }
    uiTabFoldLine->plainTextEditHeight->setPlainText(width);

    MoveCursorToEnd(uiTabFoldLine->plainTextEditHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetFormulaFoldWidth(const QString &formula)
{
    const QString width = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (width.length() > 80)
    {
        this->DeployFoldWidth();
    }
    uiTabFoldLine->plainTextEditWidth->setPlainText(width);

    MoveCursorToEnd(uiTabFoldLine->plainTextEditWidth);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetFormulaFoldCenter(const QString &formula)
{
    const QString width = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (width.length() > 80)
    {
        this->DeployFoldCenter();
    }
    uiTabFoldLine->plainTextEditCenter->setPlainText(width);

    MoveCursorToEnd(uiTabFoldLine->plainTextEditCenter);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::UpdateCurrentCustomSARecord()
{
    const int row = uiTabPaths->listWidgetCustomSA->currentRow();
    if (uiTabPaths->listWidgetCustomSA->count() == 0 || row == -1)
    {
        return;
    }

    QListWidgetItem *item = uiTabPaths->listWidgetCustomSA->item(row);
    SCASSERT(item != nullptr);
    const auto record = qvariant_cast<CustomSARecord>(item->data(Qt::UserRole));
    item->setText(GetPathName(record.path, record.reverse));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::UpdateCurrentInternalPathRecord()
{
    const int row = uiTabPaths->listWidgetInternalPaths->currentRow();
    if (uiTabPaths->listWidgetInternalPaths->count() == 0 || row == -1)
    {
        return;
    }

    QListWidgetItem *item = uiTabPaths->listWidgetInternalPaths->item(row);
    SCASSERT(item != nullptr);
    const auto path = qvariant_cast<quint32>(item->data(Qt::UserRole));
    item->setText(GetPathName(path));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::UpdateCurrentPlaceLabelRecords()
{
    const int row = uiTabPlaceLabels->listWidgetPlaceLabels->currentRow();
    if (uiTabPlaceLabels->listWidgetPlaceLabels->count() == 0 || row == -1)
    {
        return;
    }

    QListWidgetItem *item = uiTabPlaceLabels->listWidgetPlaceLabels->item(row);
    SCASSERT(item != nullptr);
    const auto labelId = qvariant_cast<quint32>(item->data(Qt::UserRole));
    item->setText(CurrentPlaceLabel(labelId).name());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetGrainlineAngle(QString angleFormula)
{
    if (angleFormula.isEmpty())
    {
        angleFormula = '0';
    }

    const QString formula = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        angleFormula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (formula.length() > 80)
    {
        this->DeployGrainlineRotation();
    }
    uiTabGrainline->lineEditRotFormula->setPlainText(formula);

    MoveCursorToEnd(uiTabGrainline->lineEditRotFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetGrainlineLength(QString lengthFormula)
{
    if (lengthFormula.isEmpty())
    {
        lengthFormula = QString().setNum(UnitConvertor(10, Unit::Cm, *data->GetPatternUnit()));
    }

    const QString formula = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        lengthFormula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (formula.length() > 80)
    {
        this->DeployGrainlineLength();
    }

    uiTabGrainline->lineEditLenFormula->setPlainText(formula);

    MoveCursorToEnd(uiTabGrainline->lineEditLenFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetDLWidth(QString widthFormula)
{
    if (widthFormula.isEmpty())
    {
        widthFormula = QString().setNum(UnitConvertor(10, Unit::Cm, *data->GetPatternUnit()));
    }

    const QString formula = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        widthFormula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (formula.length() > 80)
    {
        this->DeployDLWidth();
    }

    uiTabLabels->lineEditDLWidthFormula->setPlainText(formula);

    MoveCursorToEnd(uiTabLabels->lineEditDLWidthFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetDLHeight(QString heightFormula)
{
    if (heightFormula.isEmpty())
    {
        heightFormula = QString().setNum(UnitConvertor(10, Unit::Cm, *data->GetPatternUnit()));
    }

    const QString formula = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        heightFormula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (formula.length() > 80)
    {
        this->DeployDLHeight();
    }

    uiTabLabels->lineEditDLHeightFormula->setPlainText(formula);

    MoveCursorToEnd(uiTabLabels->lineEditDLHeightFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetDLAngle(QString angleFormula)
{
    if (angleFormula.isEmpty())
    {
        angleFormula = '0';
    }

    const QString formula = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        angleFormula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (formula.length() > 80)
    {
        this->DeployDLAngle();
    }

    uiTabLabels->lineEditDLAngleFormula->setPlainText(formula);

    MoveCursorToEnd(uiTabLabels->lineEditDLAngleFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetPLWidth(QString widthFormula)
{
    if (widthFormula.isEmpty())
    {
        widthFormula = QString().setNum(UnitConvertor(10, Unit::Cm, *data->GetPatternUnit()));
    }

    const QString formula = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        widthFormula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (formula.length() > 80)
    {
        this->DeployPLWidth();
    }

    uiTabLabels->lineEditPLWidthFormula->setPlainText(formula);

    MoveCursorToEnd(uiTabLabels->lineEditPLWidthFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetPLHeight(QString heightFormula)
{
    if (heightFormula.isEmpty())
    {
        heightFormula = QString().setNum(UnitConvertor(10, Unit::Cm, *data->GetPatternUnit()));
    }

    const QString formula = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        heightFormula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (formula.length() > 80)
    {
        this->DeployPLHeight();
    }

    uiTabLabels->lineEditPLHeightFormula->setPlainText(formula);

    MoveCursorToEnd(uiTabLabels->lineEditPLHeightFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetPLAngle(QString angleFormula)
{
    if (angleFormula.isEmpty())
    {
        angleFormula = '0';
    }

    const QString formula = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        angleFormula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (formula.length() > 80)
    {
        this->DeployPLAngle();
    }

    uiTabLabels->lineEditPLAngleFormula->setPlainText(formula);

    MoveCursorToEnd(uiTabLabels->lineEditPLAngleFormula);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSeamAllowance::CurrentRect() const -> QRectF
{
    QRectF rect;
    if (QListWidgetItem *item = uiTabPlaceLabels->listWidgetPlaceLabels->currentItem())
    {
        VPlaceLabelItem const label = CurrentPlaceLabel(qvariant_cast<quint32>(item->data(Qt::UserRole)));
        rect = QRectF(QPointF(label.x() - label.GetWidth() / 2.0, label.y() - label.GetHeight() / 2.0),
                      QPointF(label.x() + label.GetWidth() / 2.0, label.y() + label.GetHeight() / 2.0));
    }
    return rect;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ShowPieceSpecialPointsWithRect(const QListWidget *list, bool showRect)
{
    SCASSERT(list != nullptr)
    if (m_visSpecialPoints.isNull())
    {
        m_visSpecialPoints = new VisPieceSpecialPoints(data);
    }

    m_visSpecialPoints->SetSpecialPoints(GetListInternals<quint32>(list));
    m_visSpecialPoints->SetShowRect(showRect);
    m_visSpecialPoints->SetRect(CurrentRect());

    if (not VAbstractValApplication::VApp()->getCurrentScene()->items().contains(m_visSpecialPoints))
    {
        m_visSpecialPoints->VisualMode();
        m_visSpecialPoints->setZValue(10); // pins should be on top
        auto *tool = qobject_cast<VToolSeamAllowance *>(VAbstractPattern::getTool(toolId));
        SCASSERT(tool != nullptr);
        m_visSpecialPoints->setParentItem(tool);
    }
    else
    {
        m_visSpecialPoints->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSeamAllowance::CurrentPath(quint32 id) const -> VPiecePath
{
    return m_newPaths.contains(id) ? m_newPaths.value(id) : data->GetPiecePath(id);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSeamAllowance::CurrentPlaceLabel(quint32 id) const -> VPlaceLabelItem
{
    return m_newPlaceLabels.contains(id) ? m_newPlaceLabels.value(id) : *data->GeometricObject<VPlaceLabelItem>(id);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSeamAllowance::GetDefaultPieceName() const -> QString
{
    QList<VPiece> const pieces = data->DataPieces()->values();
    QSet<QString> names;

    for (const auto &piece : pieces)
    {
        names.insert(piece.GetName());
    }

    const QString defName = tr("Detail");
    QString name = defName;
    int i = 0;

    while (names.contains(name))
    {
        name = defName + QStringLiteral("_%1").arg(++i);
    }
    return name;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EnableGrainlineFormulaControls(bool enable)
{
    uiTabGrainline->pushButtonRot->setEnabled(enable);
    uiTabGrainline->lineEditRotFormula->setEnabled(enable);
    uiTabGrainline->pushButtonShowRot->setEnabled(enable);

    uiTabGrainline->pushButtonLen->setEnabled(enable);
    uiTabGrainline->lineEditLenFormula->setEnabled(enable);
    uiTabGrainline->pushButtonShowLen->setEnabled(enable);

    uiTabGrainline->comboBoxGrainlineCenterPin->setEnabled(enable);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EnableDetailLabelFormulaControls(bool enable)
{
    uiTabLabels->pushButtonDLWidth->setEnabled(enable);
    uiTabLabels->lineEditDLWidthFormula->setEnabled(enable);
    uiTabLabels->pushButtonShowDLWidth->setEnabled(enable);

    uiTabLabels->pushButtonDLHeight->setEnabled(enable);
    uiTabLabels->lineEditDLHeightFormula->setEnabled(enable);
    uiTabLabels->pushButtonShowDLHeight->setEnabled(enable);

    uiTabLabels->comboBoxDLCenterPin->setEnabled(enable);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EnablePatternLabelFormulaControls(bool enable)
{
    uiTabLabels->pushButtonPLWidth->setEnabled(enable);
    uiTabLabels->lineEditPLWidthFormula->setEnabled(enable);
    uiTabLabels->pushButtonShowPLWidth->setEnabled(enable);

    uiTabLabels->pushButtonPLHeight->setEnabled(enable);
    uiTabLabels->lineEditPLHeightFormula->setEnabled(enable);
    uiTabLabels->pushButtonShowPLHeight->setEnabled(enable);

    uiTabLabels->comboBoxPLCenterPin->setEnabled(enable);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SavePatternLabelData()
{
    if (m_patternLabelDataChanged && m_doc != nullptr)
    {
        m_doc->SetPatternName(uiTabLabels->lineEditPatternName->text());
        m_doc->SetPatternNumber(uiTabLabels->lineEditPatternNumber->text());
        m_doc->SetCompanyName(uiTabLabels->lineEditCompanyName->text());
        if (VAbstractValApplication::VApp()->GetMeasurementsType() != MeasurementsType::Individual)
        {
            m_doc->SetCustomerName(uiTabLabels->lineEditCustomerName->text());
            m_doc->SetCustomerBirthDate(uiTabLabels->dateEditCustomerBirthDate->date());
            m_doc->SetCustomerEmail(uiTabLabels->lineEditCustomerEmail->text());
        }
        m_doc->SetLabelDateFormat(uiTabLabels->comboBoxDateFormat->currentText());
        m_doc->SetLabelTimeFormat(uiTabLabels->comboBoxTimeFormat->currentText());

        m_patternLabelDataChanged = false;
        m_askSavePatternLabelData = false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SavePatternTemplateData()
{
    if (m_patternTemplateDataChanged && m_doc != nullptr)
    {
        m_doc->SetPatternLabelTemplate(m_patternTemplateLines);
        m_patternTemplateDataChanged = false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SavePatternMaterialData()
{
    if (m_patternMaterialsChanged && m_doc != nullptr)
    {
        m_doc->SetPatternMaterials(m_patternMaterials);
        m_patternMaterialsChanged = false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitGradationPlaceholdersMenu()
{
    QChar const per('%');
    auto i = m_gradationPlaceholders.constBegin();
    while (i != m_gradationPlaceholders.constEnd())
    {
        auto [first, second] = i.value();
        QAction *action = m_placeholdersMenu->addAction(first);
        action->setData(per + i.key() + per);
        connect(action, &QAction::triggered, this, &DialogSeamAllowance::InsertGradationPlaceholder);
        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitGradationPlaceholders()
{
    // Pattern tags
    QString const heightValue = QString::number(VAbstractValApplication::VApp()->GetDimensionHeight());
    m_gradationPlaceholders.insert(pl_height, qMakePair(tr("Height", "dimension"), heightValue));
    m_gradationPlaceholders.insert(pl_dimensionX, qMakePair(tr("Dimension X", "dimension"), heightValue));

    QString const sizeValue = QString::number(VAbstractValApplication::VApp()->GetDimensionSize());
    m_gradationPlaceholders.insert(pl_size, qMakePair(tr("Size", "dimension"), sizeValue));
    m_gradationPlaceholders.insert(pl_dimensionY, qMakePair(tr("Dimension Y", "dimension"), sizeValue));

    QString const hipValue = QString::number(VAbstractValApplication::VApp()->GetDimensionHip());
    m_gradationPlaceholders.insert(pl_hip, qMakePair(tr("Hip", "dimension"), hipValue));
    m_gradationPlaceholders.insert(pl_dimensionZ, qMakePair(tr("Dimension Z", "dimension"), hipValue));

    QString const waistValue = QString::number(VAbstractValApplication::VApp()->GetDimensionWaist());
    m_gradationPlaceholders.insert(pl_waist, qMakePair(tr("Waist", "dimension"), waistValue));
    m_gradationPlaceholders.insert(pl_dimensionW, qMakePair(tr("Dimension W", "dimension"), waistValue));

    {
        QString label = VAbstractValApplication::VApp()->GetDimensionHeightLabel();
        m_gradationPlaceholders.insert(
            pl_heightLabel, qMakePair(tr("Height label", "dimension"), not label.isEmpty() ? label : heightValue));
        m_gradationPlaceholders.insert(pl_dimensionXLabel, qMakePair(tr("Dimension X label", "dimension"),
                                                                     not label.isEmpty() ? label : heightValue));

        label = VAbstractValApplication::VApp()->GetDimensionSizeLabel();
        m_gradationPlaceholders.insert(
            pl_sizeLabel, qMakePair(tr("Size label", "dimension"), not label.isEmpty() ? label : sizeValue));
        m_gradationPlaceholders.insert(pl_dimensionYLabel, qMakePair(tr("Dimension Y label", "dimension"),
                                                                     not label.isEmpty() ? label : sizeValue));

        label = VAbstractValApplication::VApp()->GetDimensionHipLabel();
        m_gradationPlaceholders.insert(pl_hipLabel,
                                       qMakePair(tr("Hip label", "dimension"), not label.isEmpty() ? label : hipValue));
        m_gradationPlaceholders.insert(pl_dimensionZLabel, qMakePair(tr("Dimension Z label", "dimension"),
                                                                     not label.isEmpty() ? label : hipValue));

        label = VAbstractValApplication::VApp()->GetDimensionWaistLabel();
        m_gradationPlaceholders.insert(
            pl_waistLabel, qMakePair(tr("Waist label", "dimension"), not label.isEmpty() ? label : waistValue));
        m_gradationPlaceholders.insert(pl_dimensionWLabel, qMakePair(tr("Dimension W label", "dimension"),
                                                                     not label.isEmpty() ? label : waistValue));
    }

    {
        const QMap<QString, QSharedPointer<VMeasurement>> measurements = data->DataMeasurements();
        auto i = measurements.constBegin();
        while (i != measurements.constEnd())
        {
            QString const description = i.value()->GetGuiText().isEmpty() ? i.key() : i.value()->GetGuiText();
            m_gradationPlaceholders.insert(
                pl_measurement + i.key(),
                qMakePair(tr("Measurement: %1").arg(description), QString::number(*i.value()->GetValue())));
            m_gradationPlaceholders.insert(pl_measurement + i.key() + pl_valueAlias,
                                           qMakePair(tr("Measurement %1 value alias").arg(description),
                                                     i.value()->GetValueAlias()));
            ++i;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitPassmarkLengthFormula(const VPieceNode &node)
{
    // notch depth
    uiTabPassmarks->groupBoxManualLength->setEnabled(true);

    if (node.IsManualPassmarkLength())
    {
        uiTabPassmarks->groupBoxManualLength->setChecked(true);

        uiTabPassmarks->toolButtonExprLength->setEnabled(uiTabPassmarks->groupBoxManualLength->isChecked());
        uiTabPassmarks->plainTextEditPassmarkLength->setEnabled(uiTabPassmarks->groupBoxManualLength->isChecked());
        uiTabPassmarks->pushButtonGrowPassmarkLength->setEnabled(uiTabPassmarks->groupBoxManualLength->isChecked());
        uiTabPassmarks->labelEditPassmarkLength->setEnabled(uiTabPassmarks->groupBoxManualLength->isChecked());
        uiTabPassmarks->label_3->setEnabled(uiTabPassmarks->groupBoxManualLength->isChecked());

        QString passmarkLength = node.GetFormulaPassmarkLength();
        passmarkLength = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
            passmarkLength, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
        if (passmarkLength.length() > 80) // increase height if needed.
        {
            this->DeployPassmarkLength();
        }

        if (passmarkLength.isEmpty())
        {
            qreal const length = UnitConvertor(1, Unit::Cm, VAbstractValApplication::VApp()->patternUnits());
            uiTabPassmarks->plainTextEditPassmarkLength->setPlainText(
                VAbstractApplication::VApp()->LocaleToString(length));
        }
        else
        {
            uiTabPassmarks->plainTextEditPassmarkLength->setPlainText(passmarkLength);
        }
    }
    else
    {
        qreal const length = UnitConvertor(1, Unit::Cm, VAbstractValApplication::VApp()->patternUnits());
        uiTabPassmarks->plainTextEditPassmarkLength->setPlainText(VAbstractApplication::VApp()->LocaleToString(length));
    }

    MoveCursorToEnd(uiTabPassmarks->plainTextEditPassmarkLength);
    ChangeColor(uiTabPassmarks->labelEditPassmarkLength, OkColor(this));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitPassmarkWidthFormula(const VPieceNode &node)
{
    // notch width
    if (node.GetPassmarkLineType() != PassmarkLineType::OneLine)
    {
        uiTabPassmarks->groupBoxManualWidth->setEnabled(true);

        if (node.IsManualPassmarkWidth())
        {
            uiTabPassmarks->groupBoxManualWidth->setChecked(true);

            uiTabPassmarks->toolButtonExprWidth->setEnabled(uiTabPassmarks->groupBoxManualWidth->isChecked());
            uiTabPassmarks->plainTextEditPassmarkWidth->setEnabled(uiTabPassmarks->groupBoxManualWidth->isChecked());
            uiTabPassmarks->pushButtonGrowPassmarkWidth->setEnabled(uiTabPassmarks->groupBoxManualWidth->isChecked());
            uiTabPassmarks->labelEditPassmarkWidth->setEnabled(uiTabPassmarks->groupBoxManualWidth->isChecked());
            uiTabPassmarks->label_4->setEnabled(uiTabPassmarks->groupBoxManualWidth->isChecked());

            QString passmarkWidth = node.GetFormulaPassmarkWidth();
            passmarkWidth = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
                passmarkWidth, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
            if (passmarkWidth.length() > 80) // increase height if needed.
            {
                this->DeployPassmarkWidth();
            }

            if (passmarkWidth.isEmpty())
            {
                qreal const width = UnitConvertor(0.85, Unit::Cm, VAbstractValApplication::VApp()->patternUnits());
                uiTabPassmarks->plainTextEditPassmarkWidth->setPlainText(
                    VAbstractApplication::VApp()->LocaleToString(width));
            }
            else
            {
                uiTabPassmarks->plainTextEditPassmarkWidth->setPlainText(passmarkWidth);
            }
        }
        else
        {
            qreal const length = UnitConvertor(0.85, Unit::Cm, VAbstractValApplication::VApp()->patternUnits());
            uiTabPassmarks->plainTextEditPassmarkWidth->setPlainText(
                VAbstractApplication::VApp()->LocaleToString(length));
        }

        MoveCursorToEnd(uiTabPassmarks->plainTextEditPassmarkWidth);
    }
    else
    {
        qreal const length = UnitConvertor(0.85, Unit::Cm, VAbstractValApplication::VApp()->patternUnits());
        uiTabPassmarks->plainTextEditPassmarkWidth->setPlainText(VAbstractApplication::VApp()->LocaleToString(length));
    }
    ChangeColor(uiTabPassmarks->labelEditPassmarkWidth, OkColor(this));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitPassmarkAngleFormula(const VPieceNode &node)
{
    // notch angle
    if (node.GetPassmarkAngleType() == PassmarkAngleType::Straightforward)
    {
        uiTabPassmarks->groupBoxManualAngle->setEnabled(true);

        if (node.IsManualPassmarkAngle())
        {
            uiTabPassmarks->groupBoxManualAngle->setChecked(true);

            uiTabPassmarks->toolButtonExprAngle->setEnabled(uiTabPassmarks->groupBoxManualAngle->isChecked());
            uiTabPassmarks->plainTextEditPassmarkAngle->setEnabled(uiTabPassmarks->groupBoxManualAngle->isChecked());
            uiTabPassmarks->pushButtonGrowPassmarkAngle->setEnabled(uiTabPassmarks->groupBoxManualAngle->isChecked());
            uiTabPassmarks->labelEditPassmarkAngle->setEnabled(uiTabPassmarks->groupBoxManualAngle->isChecked());
            uiTabPassmarks->label_5->setEnabled(uiTabPassmarks->groupBoxManualAngle->isChecked());

            QString passmarkAngle = node.GetFormulaPassmarkAngle();
            passmarkAngle = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
                passmarkAngle, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
            if (passmarkAngle.length() > 80) // increase height if needed.
            {
                this->DeployPassmarkAngle();
            }

            uiTabPassmarks->plainTextEditPassmarkAngle->setPlainText(passmarkAngle.isEmpty() ? QString::number(0)
                                                                                             : passmarkAngle);
        }
        else
        {
            uiTabPassmarks->plainTextEditPassmarkAngle->setPlainText(QString::number(0));
        }

        MoveCursorToEnd(uiTabPassmarks->plainTextEditPassmarkAngle);
    }
    else
    {
        uiTabPassmarks->plainTextEditPassmarkAngle->setPlainText(QString::number(0));
    }
    ChangeColor(uiTabPassmarks->labelEditPassmarkAngle, OkColor(this));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitPassmarkShapeType(const VPieceNode &node)
{
    // Line type
    uiTabPassmarks->groupBoxMarkType->setEnabled(true);

    switch (node.GetPassmarkLineType())
    {
        case PassmarkLineType::OneLine:
            uiTabPassmarks->radioButtonOneLine->setChecked(true);
            break;
        case PassmarkLineType::TwoLines:
            uiTabPassmarks->radioButtonTwoLines->setChecked(true);
            break;
        case PassmarkLineType::ThreeLines:
            uiTabPassmarks->radioButtonThreeLines->setChecked(true);
            break;
        case PassmarkLineType::TMark:
            uiTabPassmarks->radioButtonTMark->setChecked(true);
            break;
        case PassmarkLineType::ExternalVMark:
            uiTabPassmarks->radioButtonVMark->setChecked(true);
            break;
        case PassmarkLineType::InternalVMark:
            uiTabPassmarks->radioButtonVMark2->setChecked(true);
            break;
        case PassmarkLineType::UMark:
            uiTabPassmarks->radioButtonUMark->setChecked(true);
            break;
        case PassmarkLineType::BoxMark:
            uiTabPassmarks->radioButtonBoxMark->setChecked(true);
            break;
        case PassmarkLineType::CheckMark:
            uiTabPassmarks->radioButtonCheckMark->setChecked(true);
            break;
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitPassmarkAngleType(const VPieceNode &node)
{
    // Angle type
    uiTabPassmarks->groupBoxAngleType->setEnabled(true);

    switch (node.GetPassmarkAngleType())
    {
        case PassmarkAngleType::Straightforward:
            uiTabPassmarks->radioButtonStraightforward->setChecked(true);
            break;
        case PassmarkAngleType::Bisector:
            uiTabPassmarks->radioButtonBisector->setChecked(true);
            break;
        case PassmarkAngleType::Intersection:
            uiTabPassmarks->radioButtonIntersection->setChecked(true);
            break;
        case PassmarkAngleType::IntersectionOnlyLeft:
            uiTabPassmarks->radioButtonIntersectionOnlyLeft->setChecked(true);
            break;
        case PassmarkAngleType::IntersectionOnlyRight:
            uiTabPassmarks->radioButtonIntersectionOnlyRight->setChecked(true);
            break;
        case PassmarkAngleType::Intersection2:
            uiTabPassmarks->radioButtonIntersection2->setChecked(true);
            break;
        case PassmarkAngleType::Intersection2OnlyLeft:
            uiTabPassmarks->radioButtonIntersection2OnlyLeft->setChecked(true);
            break;
        case PassmarkAngleType::Intersection2OnlyRight:
            uiTabPassmarks->radioButtonIntersection2OnlyRight->setChecked(true);
            break;
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitIcons()
{
    const auto resource = QStringLiteral("icon");

    const auto fxIcon = QStringLiteral("24x24/fx.png");
    uiTabGrainline->pushButtonRot->setIcon(VTheme::GetIconResource(resource, fxIcon));
    uiTabGrainline->pushButtonLen->setIcon(VTheme::GetIconResource(resource, fxIcon));
    uiTabLabels->pushButtonDLWidth->setIcon(VTheme::GetIconResource(resource, fxIcon));
    uiTabLabels->pushButtonDLHeight->setIcon(VTheme::GetIconResource(resource, fxIcon));
    uiTabLabels->pushButtonDLAngle->setIcon(VTheme::GetIconResource(resource, fxIcon));
    uiTabLabels->pushButtonPLWidth->setIcon(VTheme::GetIconResource(resource, fxIcon));
    uiTabLabels->pushButtonPLHeight->setIcon(VTheme::GetIconResource(resource, fxIcon));
    uiTabLabels->pushButtonPLAngle->setIcon(VTheme::GetIconResource(resource, fxIcon));
    uiTabPassmarks->toolButtonExprLength->setIcon(VTheme::GetIconResource(resource, fxIcon));
    uiTabPassmarks->toolButtonExprWidth->setIcon(VTheme::GetIconResource(resource, fxIcon));
    uiTabPassmarks->toolButtonExprAngle->setIcon(VTheme::GetIconResource(resource, fxIcon));
    uiTabPaths->toolButtonExprWidth->setIcon(VTheme::GetIconResource(resource, fxIcon));
    uiTabPaths->toolButtonExprBefore->setIcon(VTheme::GetIconResource(resource, fxIcon));
    uiTabPaths->toolButtonExprAfter->setIcon(VTheme::GetIconResource(resource, fxIcon));

    const auto equalIcon = QStringLiteral("24x24/equal.png");
    uiTabGrainline->labelEqual->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    uiTabGrainline->labelEqual_2->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    uiTabLabels->labelEqual_3->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    uiTabLabels->labelEqual_4->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    uiTabLabels->labelEqual_5->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    uiTabLabels->labelEqual_7->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    uiTabLabels->labelEqual_6->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    uiTabLabels->labelEqual_8->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    uiTabPaths->label_2->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    uiTabPaths->label_6->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    uiTabPaths->label_10->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));

    uiTabPaths->label_3->setPixmap(VTheme::GetPixmapResource(resource, QStringLiteral("32x32/clockwise.png")));
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSeamAllowance::InitMainPathContextMenu(QMenu *menu, const VPieceNode &rowNode) const -> QHash<int, QAction *>
{
    SCASSERT(menu != nullptr)

    QHash<int, QAction *> contextMenu;

    if (rowNode.GetTypeTool() != Tool::NodePoint)
    {
        QAction *actionReverse = menu->addAction(FromTheme(VThemeIcon::ListRemove),
                                                 QApplication::translate("DialogSeamAllowance", "Reverse"));
        actionReverse->setCheckable(true);
        actionReverse->setChecked(rowNode.GetReverse());
        contextMenu.insert(static_cast<int>(MainPathContextMenuOption::Reverse), actionReverse);

        QAction *actionDuplicate = menu->addAction(FromTheme(VThemeIcon::DuplicateNode),
                                                  QApplication::translate("DialogSeamAllowance", "Duplicate"));
        contextMenu.insert(static_cast<int>(MainPathContextMenuOption::DuplicateNode), actionDuplicate);
    }
    else
    {
        if (applyAllowed)
        {
            QMenu *passmarkSubmenu = menu->addMenu(FromTheme(VThemeIcon::AddPlacemark),
                                                   QApplication::translate("DialogSeamAllowance", "Passmark"));

            QAction *actionNonePassmark =
                passmarkSubmenu->addAction(QApplication::translate("DialogSeamAllowance", "None"));
            actionNonePassmark->setCheckable(true);
            actionNonePassmark->setChecked(!rowNode.IsPassmark());
            contextMenu.insert(static_cast<int>(MainPathContextMenuOption::NonePassmark), actionNonePassmark);

            Q_STATIC_ASSERT_X(static_cast<int>(PassmarkLineType::LAST_ONE_DO_NOT_USE) == 9,
                              "Not all types were handled.");

            auto InitPassmarkLineTypeAction = [passmarkSubmenu, rowNode](const QString &name, PassmarkLineType lineType)
            {
                QAction *action = passmarkSubmenu->addAction(name);
                action->setCheckable(true);
                action->setChecked(rowNode.IsPassmark() && lineType == rowNode.GetPassmarkLineType());
                return action;
            };

            contextMenu.insert(static_cast<int>(MainPathContextMenuOption::OneLine),
                               InitPassmarkLineTypeAction(QApplication::translate("DialogSeamAllowance", "One line"),
                                                          PassmarkLineType::OneLine));
            contextMenu.insert(static_cast<int>(MainPathContextMenuOption::TwoLines),
                               InitPassmarkLineTypeAction(QApplication::translate("DialogSeamAllowance", "Two lines"),
                                                          PassmarkLineType::TwoLines));
            contextMenu.insert(static_cast<int>(MainPathContextMenuOption::ThreeLines),
                               InitPassmarkLineTypeAction(QApplication::translate("DialogSeamAllowance", "Three lines"),
                                                          PassmarkLineType::ThreeLines));
            contextMenu.insert(static_cast<int>(MainPathContextMenuOption::TMark),
                               InitPassmarkLineTypeAction(QApplication::translate("DialogSeamAllowance", "T mark"),
                                                          PassmarkLineType::TMark));
            contextMenu.insert(
                static_cast<int>(MainPathContextMenuOption::ExternalVMark),
                InitPassmarkLineTypeAction(QApplication::translate("DialogSeamAllowance", "External V mark"),
                                           PassmarkLineType::ExternalVMark));
            contextMenu.insert(
                static_cast<int>(MainPathContextMenuOption::InternalVMark),
                InitPassmarkLineTypeAction(QApplication::translate("DialogSeamAllowance", "Internal V mark"),
                                           PassmarkLineType::InternalVMark));
            contextMenu.insert(static_cast<int>(MainPathContextMenuOption::UMark),
                               InitPassmarkLineTypeAction(QApplication::translate("DialogSeamAllowance", "U mark"),
                                                          PassmarkLineType::UMark));
            contextMenu.insert(static_cast<int>(MainPathContextMenuOption::BoxMark),
                               InitPassmarkLineTypeAction(QApplication::translate("DialogSeamAllowance", "Box mark"),
                                                          PassmarkLineType::BoxMark));
            contextMenu.insert(static_cast<int>(MainPathContextMenuOption::CheckMark),
                               InitPassmarkLineTypeAction(QApplication::translate("DialogSeamAllowance", "Check mark"),
                                                          PassmarkLineType::CheckMark));
        }

        QAction *actionUniqueness = menu->addAction(FromTheme(VThemeIcon::DrawStar),
                                                    QApplication::translate("DialogSeamAllowance", "Check uniqueness"));
        actionUniqueness->setCheckable(true);
        actionUniqueness->setChecked(rowNode.IsCheckUniqueness());
        contextMenu.insert(static_cast<int>(MainPathContextMenuOption::Uniqueness), actionUniqueness);

        QAction *actionTurnPoint = menu->addAction(FromTheme(VThemeIcon::SnapNodesRotationCenter),
                                                   QApplication::translate("DialogSeamAllowance", "Turn point"));
        actionTurnPoint->setCheckable(true);
        actionTurnPoint->setChecked(rowNode.IsTurnPoint());
        contextMenu.insert(static_cast<int>(MainPathContextMenuOption::TurnPoint), actionTurnPoint);
    }

    QAction *actionExcluded = menu->addAction(FromTheme(VThemeIcon::FormatTextStrikethrough),
                                              QApplication::translate("DialogSeamAllowance", "Excluded"));
    actionExcluded->setCheckable(true);
    actionExcluded->setChecked(rowNode.IsExcluded());
    contextMenu.insert(static_cast<int>(MainPathContextMenuOption::Excluded), actionExcluded);

    QAction *actionDelete =
        menu->addAction(FromTheme(VThemeIcon::EditDelete), QApplication::translate("DialogSeamAllowance", "Delete"));
    actionDelete->setShortcut(QKeySequence::Delete);
    contextMenu.insert(static_cast<int>(MainPathContextMenuOption::Delete), actionDelete);

    return contextMenu;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetMoveControls()
{
    uiTabPaths->toolButtonTop->setEnabled(false);
    uiTabPaths->toolButtonUp->setEnabled(false);
    uiTabPaths->toolButtonDown->setEnabled(false);
    uiTabPaths->toolButtonBottom->setEnabled(false);

    if (uiTabPaths->listWidgetMainPath->count() >= 2)
    {
        if (uiTabPaths->listWidgetMainPath->currentRow() == 0)
        {
            uiTabPaths->toolButtonDown->setEnabled(true);
            uiTabPaths->toolButtonBottom->setEnabled(true);
        }
        else if (uiTabPaths->listWidgetMainPath->currentRow() == uiTabPaths->listWidgetMainPath->count() - 1)
        {
            uiTabPaths->toolButtonTop->setEnabled(true);
            uiTabPaths->toolButtonUp->setEnabled(true);
        }
        else
        {
            uiTabPaths->toolButtonTop->setEnabled(true);
            uiTabPaths->toolButtonUp->setEnabled(true);
            uiTabPaths->toolButtonDown->setEnabled(true);
            uiTabPaths->toolButtonBottom->setEnabled(true);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetOptionControls()
{
    uiTabPaths->toolButtonReverse->setEnabled(false);
    uiTabPaths->toolButtonExcluded->setEnabled(false);
    uiTabPaths->toolButtonTurnPoint->setEnabled(false);
    uiTabPaths->toolButtonCheckUniqness->setEnabled(false);
    uiTabPaths->toolButtonPassmark->setEnabled(false);
    uiTabPaths->toolButtonDuplicateNode->setEnabled(false);
    uiTabPaths->toolButtonDelete->setEnabled(false);

    auto SetChecked = [](QToolButton *toolButton, bool checked = false)
    {
        const QSignalBlocker blocker(toolButton);
        toolButton->setChecked(checked);
    };

    SetChecked(uiTabPaths->toolButtonReverse);
    SetChecked(uiTabPaths->toolButtonExcluded);
    SetChecked(uiTabPaths->toolButtonTurnPoint);
    SetChecked(uiTabPaths->toolButtonCheckUniqness);
    SetChecked(uiTabPaths->toolButtonPassmark);
    SetChecked(uiTabPaths->toolButtonDuplicateNode);
    SetChecked(uiTabPaths->toolButtonDelete);

    const int row = uiTabPaths->listWidgetMainPath->currentRow();
    if (row < 0)
    {
        return;
    }

    const QListWidgetItem *rowItem = uiTabPaths->listWidgetMainPath->item(row);
    SCASSERT(rowItem != nullptr)
    auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

    if (rowNode.GetTypeTool() != Tool::NodePoint)
    {
        uiTabPaths->toolButtonReverse->setEnabled(true);
        uiTabPaths->toolButtonDuplicateNode->setEnabled(true);
        SetChecked(uiTabPaths->toolButtonReverse, rowNode.GetReverse());
    }
    else
    {
        uiTabPaths->toolButtonPassmark->setEnabled(true);
        SetChecked(uiTabPaths->toolButtonPassmark, rowNode.IsPassmark());

        uiTabPaths->toolButtonCheckUniqness->setEnabled(true);
        SetChecked(uiTabPaths->toolButtonCheckUniqness, rowNode.IsCheckUniqueness());

        uiTabPaths->toolButtonTurnPoint->setEnabled(true);
        SetChecked(uiTabPaths->toolButtonTurnPoint, rowNode.IsTurnPoint());
    }

    uiTabPaths->toolButtonExcluded->setEnabled(true);
    SetChecked(uiTabPaths->toolButtonExcluded, rowNode.IsExcluded());

    uiTabPaths->toolButtonDelete->setEnabled(true);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::PatternLabelDataChanged()
{
    m_patternLabelDataChanged = true;
    m_askSavePatternLabelData = true;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EditPatternLabel()
{
    if (m_patternLabelDataChanged && m_askSavePatternLabelData)
    {
        if (QMessageBox::StandardButton const answer = QMessageBox::question(
                this, tr("Save label data."),
                tr("Label data were changed. Do you want to save them before editing label template?"),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            answer == QMessageBox::Yes)
        {
            SavePatternLabelData();
        }
        else
        {
            m_askSavePatternLabelData = false;
        }
    }

    if (m_doc == nullptr)
    {
        return;
    }

    DialogEditLabel editor(m_doc, data);

    m_patternTemplateDataChanged ? editor.SetTemplate(m_patternTemplateLines)
                                 : editor.SetTemplate(m_doc->GetPatternLabelTemplate());

    if (QDialog::Accepted == editor.exec())
    {
        m_patternTemplateLines = editor.GetTemplate();
        uiTabLabels->groupBoxPatternLabel->setEnabled(not m_patternTemplateLines.isEmpty());
        m_patternTemplateDataChanged = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ManagePatternMaterials()
{
    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();

    DialogPatternMaterials editor(m_patternMaterials, settings->IsRememberPatternMaterials());

    if (QDialog::Accepted == editor.exec())
    {
        m_patternMaterials = editor.GetPatternMaterials();
        m_patternMaterialsChanged = true;

        if (settings->IsRememberPatternMaterials())
        {
            settings->SetKnownMaterials(editor.GetKnownMaterials());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InsertGradationPlaceholder()
{
    auto *action = qobject_cast<QAction *>(sender());
    if (action)
    {
        uiTabPaths->lineEditGradationLabel->insert(action->data().toString());
        uiTabPaths->lineEditGradationLabel->setFocus();
    }
}
