/************************************************************************
 **
 **  @file   dialoggraduatedcurve.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   4 12, 2025
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2025 Valentina project
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
#include "dialoggraduatedcurve.h"
#include "../../tools/toolsdef.h"
#include "../../visualization/path/vistoolgraduatedcurve.h"
#include "../../visualization/visualization.h"
#include "../qmuparser/qmudef.h"
#include "../support/dialogeditwrongformula.h"
#include "../vgeometry/vsplinepath.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vvalentinasettings.h"
#include "../vpatterndb/variables/vincrement.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/global.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "ui_dialoggraduatedcurve.h"
#include "vpatterndb/vformula.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

constexpr int DIALOG_MAX_FORMULA_HEIGHT = 64;

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto AddCell(QTableWidget *table, const QString &text, int row, int column, int aligment, bool ok = true)
    -> QTableWidgetItem *
{
    SCASSERT(table != nullptr)

    auto *item = new QTableWidgetItem(text);
    SetTextAlignment(item, static_cast<Qt::Alignment>(aligment));
    item->setToolTip(text);

    // set the item non-editable (view only), and non-selectable
    Qt::ItemFlags flags = item->flags();
    flags &= ~(Qt::ItemIsEditable); // reset/clear the flag
    item->setFlags(flags);

    if (not ok)
    {
        QBrush brush = item->foreground();
        brush.setColor(Qt::red);
        item->setForeground(brush);
    }

    table->setItem(row, column, item);

    return item;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
DialogGraduatedCurve::DialogGraduatedCurve(const VContainer *data,
                                           VAbstractPattern *doc,
                                           quint32 toolId,
                                           QWidget *parent)
  : DialogTool(data, doc, toolId, parent),
    ui(new Ui::DialogGraduatedCurve)
{
    ui->setupUi(this);

    if (this->toolId == 0)
    { // fake id
        this->toolId = std::numeric_limits<quint32>::max();
    }

    InitIcons();

    FillComboBoxCurves(ui->comboBoxCurve);

    InitColorPicker(ui->pushButtonColor, VAbstractValApplication::VApp()->ValentinaSettings()->GetUserToolColors());
    ui->pushButtonColor->setUseNativeDialog(!VAbstractApplication::VApp()->Settings()->IsDontUseNativeDialog());
    FillComboBoxTypeLine(ui->comboBoxPenStyle,
                         CurvePenStylesPics(ui->comboBoxPenStyle->palette().color(QPalette::Base),
                                            ui->comboBoxPenStyle->palette().color(QPalette::Text)));

    ui->doubleSpinBoxApproximationScale->setMaximum(maxCurveApproximationScale);

    connect(ui->lineEditCurveName, &QLineEdit::textEdited, this, &DialogGraduatedCurve::ValidateName);
    connect(ui->lineEditAlias, &QLineEdit::textEdited, this, &DialogGraduatedCurve::ValidateAlias);

    vis = new VisToolGraduatedCurve(data);

    ui->tabWidget->setCurrentIndex(0);
    SetTabStopDistance(ui->plainTextEditToolNotes);

    m_formulaBaseHeight = ui->plainTextEditFormula->height();
    ui->plainTextEditFormula->installEventFilter(this);

    ShowHeaderUnits(1); // calculated value
    ShowHeaderUnits(2); // formula

    FillOffsets();

    auto *validator = new QRegularExpressionValidator(QRegularExpression(QStringLiteral("^$|") + NameRegExp()), this);
    ui->lineEditName->setValidator(validator);

    connect(ui->tableWidget, &QTableWidget::itemSelectionChanged, this, &DialogGraduatedCurve::ShowOffsetDetails);

    connect(ui->toolButtonAdd, &QToolButton::clicked, this, &DialogGraduatedCurve::AddOffset);
    connect(ui->toolButtonRemove, &QToolButton::clicked, this, &DialogGraduatedCurve::RemoveOffset);
    connect(ui->toolButtonUp, &QToolButton::clicked, this, &DialogGraduatedCurve::MoveUp);
    connect(ui->toolButtonDown, &QToolButton::clicked, this, &DialogGraduatedCurve::MoveDown);
    connect(ui->pushButtonGrow, &QPushButton::clicked, this, &DialogGraduatedCurve::DeployFormula);
    connect(ui->toolButtonExpr, &QToolButton::clicked, this, &DialogGraduatedCurve::Fx);
    connect(ui->lineEditName, &QLineEdit::textEdited, this, &DialogGraduatedCurve::SaveOffsetName);
    connect(ui->plainTextEditDescription,
            &QPlainTextEdit::textChanged,
            this,
            &DialogGraduatedCurve::SaveOffsetDescription);
    connect(ui->plainTextEditFormula, &QPlainTextEdit::textChanged, this, &DialogGraduatedCurve::SaveOffsetFormula);

    if (ui->tableWidget->rowCount() > 0)
    {
        ui->tableWidget->selectRow(0);
    }

    InitOkCancelApply(ui);
}

//---------------------------------------------------------------------------------------------------------------------
DialogGraduatedCurve::~DialogGraduatedCurve()
{
    for (const auto &offset : std::as_const(m_offsets))
    {
        data->RemoveUniqueName(offset.name);
    }

    VAbstractValApplication::VApp()->ValentinaSettings()->SetUserToolColors(ui->pushButtonColor->CustomColors());
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogGraduatedCurve::GetOriginCurveId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxCurve);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::SetOriginCurveId(const quint32 &value)
{
    ChangeCurrentData(ui->comboBoxCurve, value);
    auto *path = qobject_cast<VisToolGraduatedCurve *>(vis);
    SCASSERT(path != nullptr)
    path->SetCurveId(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogGraduatedCurve::GetOffsets() const -> QVector<VRawGraduatedCurveOffset>
{
    QVector<VRawGraduatedCurveOffset> rawOffsets;
    rawOffsets.reserve(m_offsets.size());

    for (const auto &formulaData : m_offsets)
    {
        rawOffsets.append({.name = formulaData.name,
                           .formula = formulaData.formula.GetFormula(FormulaType::ToSystem),
                           .description = formulaData.description});
    }

    return rawOffsets;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::SetOffsets(const QVector<VRawGraduatedCurveOffset> &offsets)
{
    m_offsets.clear();
    m_offsets.reserve(offsets.size());

    auto localData = QSharedPointer<VContainer>(new VContainer(*data));

    for (const auto &offset : offsets)
    {
        VFormula widthFormula(offset.formula, localData.data());
        widthFormula.setToolId(toolId);
        widthFormula.setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits()));
        widthFormula.Eval();

        m_offsets.append(
            {.name = offset.name, .formula = widthFormula, .formulaData = localData, .description = offset.description});

        auto newData = QSharedPointer<VContainer>(new VContainer(*localData.data()));

        auto *offsetVal = new VIncrement(newData.data(), offset.name);
        offsetVal->SetFormula(widthFormula.getDoubleValue(), offset.formula, widthFormula.error());
        offsetVal->SetDescription(
            offset.description.isEmpty()
                ? tr("Offset %1").arg(VAbstractApplication::VApp()->TrVars()->InternalVarToUser(offset.name))
                : offset.description);

        newData->AddUniqueVariable(offsetVal);

        localData = QSharedPointer<VContainer>(new VContainer(*newData.data()));
    }

    const int currentRow = ui->tableWidget->currentRow();
    FillOffsets();
    ui->tableWidget->selectRow(currentRow);

    auto *path = qobject_cast<VisToolGraduatedCurve *>(vis);
    SCASSERT(path != nullptr)
    path->SetOffsets(VisualizationOffsets());
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogGraduatedCurve::GetPenStyle() const -> QString
{
    return GetComboBoxCurrentData(ui->comboBoxPenStyle, TypeLineLine);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::SetPenStyle(const QString &value)
{
    ChangeCurrentData(ui->comboBoxPenStyle, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogGraduatedCurve::GetColor() const -> QString
{
    return ui->pushButtonColor->currentColor().name();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::SetColor(const QString &value)
{
    ui->pushButtonColor->setCurrentColor(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogGraduatedCurve::GetApproximationScale() const -> qreal
{
    return ui->doubleSpinBoxApproximationScale->value();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::SetApproximationScale(qreal value)
{
    ui->doubleSpinBoxApproximationScale->setValue(value);

    auto *path = qobject_cast<VisToolGraduatedCurve *>(vis);
    SCASSERT(path != nullptr)
    path->SetApproximationScale(value);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::SetNotes(const QString &notes)
{
    ui->plainTextEditToolNotes->setPlainText(notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogGraduatedCurve::GetNotes() const -> QString
{
    return ui->plainTextEditToolNotes->toPlainText();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::SetName(const QString &name)
{
    m_originName = name;
    ui->lineEditCurveName->setText(m_originName);
    ValidateName();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogGraduatedCurve::GetName() const -> QString
{
    return ui->lineEditCurveName->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::SetAliasSuffix(const QString &alias)
{
    m_originAliasSuffix = alias;
    ui->lineEditAlias->setText(m_originAliasSuffix);
    ValidateAlias();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogGraduatedCurve::GetAliasSuffix() const -> QString
{
    return ui->lineEditAlias->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::ShowDialog(bool click)
{
    if (not prepare)
    {
        return;
    }

    if (click)
    {
        // The check need to ignore first release of mouse button.
        // User can select point by clicking on a label.
        if (not m_firstRelease)
        {
            m_firstRelease = true;
            return;
        }

        auto *scene = qobject_cast<VMainGraphicsScene *>(VAbstractValApplication::VApp()->getCurrentScene());
        SCASSERT(scene != nullptr)

        const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(GetOriginCurveId());
        QPointF const p = curve->ClosestPoint(scene->getScenePos());

        auto const line = QLineF(p, scene->getScenePos());
        if (line.length() <= ScaledRadius(SceneScale(VAbstractValApplication::VApp()->getCurrentScene())) * 1.5)
        {
            return;
        }

        qreal len = line.length();
        if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
        {
            len = -len;
        }

        QVector<VRawGraduatedCurveOffset> rawOffsets = GetOffsets();

        rawOffsets.append(
            {.name = GetOffsetName(false), .formula = QString::number(FromPixel(len, *data->GetPatternUnit()))});

        SetOffsets(rawOffsets);
        vis->RefreshGeometry();
    }
    else
    {
        FinishCreating();
        if (!m_offsets.isEmpty())
        {
            ui->tableWidget->selectRow(0);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::CheckDependencyTreeComplete()
{
    const bool ready = m_doc->IsPatternGraphComplete();
    ui->lineEditCurveName->setEnabled(ready);
    ui->lineEditAlias->setEnabled(ready);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare) // After first choose we ignore all objects
    {
        return;
    }

    if ((type == SceneObject::Arc || type == SceneObject::ElArc || type == SceneObject::Spline
         || type == SceneObject::SplinePath)
        && SetObject(id, ui->comboBoxCurve, QString()))
    {
        if (vis != nullptr)
        {
            vis->VisualMode(id);
        }
        prepare = true;

        SetName(GenerateDefName());

        auto *window = qobject_cast<VAbstractMainWindow *>(VAbstractValApplication::VApp()->getMainWindow());
        SCASSERT(window != nullptr)
        connect(vis.data(), &Visualization::ToolTip, window, &VAbstractMainWindow::ShowToolTip);

        if (not VAbstractValApplication::VApp()->Settings()->IsInteractiveTools())
        {
            FinishCreating();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::ShowVisualization()
{
    AddVisualization<VisToolGraduatedCurve>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::SaveData()
{
    auto *path = qobject_cast<VisToolGraduatedCurve *>(vis);
    SCASSERT(path != nullptr)

    path->SetCurveId(GetOriginCurveId());
    path->SetOffsets(GetOffsets());
    path->SetApproximationScale(ui->doubleSpinBoxApproximationScale->value());
    path->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditFormula->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::changeEvent(QEvent *event)
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
void DialogGraduatedCurve::ValidateName()
{
    const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(GetOriginCurveId());
    VSplinePath const splPath = curve->Outline({0}, GetName());

    if (QRegularExpression const rx(NameRegExp()); not GetName().isEmpty() || not rx.match(splPath.name()).hasMatch()
                                                   || (m_originName != GetName() && not data->IsUnique(splPath.name())))
    {
        m_flagSuffix = false;
        ChangeColor(ui->labelName, errorColor);
    }
    else
    {
        m_flagSuffix = true;
        ChangeColor(ui->labelName, OkColor(this));
    }

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::ValidateAlias()
{
    const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(GetOriginCurveId());
    VSplinePath splPath = curve->Offset(0, GetName());

    splPath.SetAliasSuffix(GetAliasSuffix());
    if (QRegularExpression const rx(NameRegExp());
        not GetAliasSuffix().isEmpty()
        && (not rx.match(splPath.GetAlias()).hasMatch()
            || (m_originAliasSuffix != GetAliasSuffix() && not data->IsUnique(splPath.GetAlias()))))
    {
        m_flagAlias = false;
        ChangeColor(ui->labelAlias, errorColor);
    }
    else
    {
        m_flagAlias = true;
        ChangeColor(ui->labelAlias, OkColor(this));
    }

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::ShowOffsetDetails()
{
    if (ui->tableWidget->rowCount() > 0 && ui->tableWidget->currentRow() != -1)
    {
        EnableDetails(true);

        const VGraduatedCurveOffsetFormula &offsetData = m_offsets.at(ui->tableWidget->currentRow());

        {
            const QSignalBlocker blocker(ui->lineEditName);
            ui->lineEditName->setText(VAbstractApplication::VApp()->TrVars()->InternalVarToUser(offsetData.name));
        }

        if (QString const unitSuffix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits());
            offsetData.formula.error())
        {
            ui->labelCalculatedValue->setText(tr("Error") + " ("_L1 + unitSuffix + "). "_L1);
            ui->labelCalculatedValue->setToolTip(offsetData.formula.Reason());
        }
        else
        {
            ui->labelCalculatedValue->setText(offsetData.formula.getStringValue());
            ui->labelCalculatedValue->setToolTip(tr("Value"));
        }

        {
            const QSignalBlocker blocker(ui->plainTextEditFormula);
            QString const formula = offsetData.formula.GetFormula(FormulaType::ToUser);
            ui->plainTextEditFormula->setPlainText(formula);
        }

        const QSignalBlocker blocker(ui->plainTextEditDescription);
        ui->plainTextEditDescription->setPlainText(offsetData.description);

        return;
    }

    EnableDetails(false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::AddOffset()
{
    const QString name = GetOffsetName(false);
    qint32 currentRow = -1;

    QVector<VRawGraduatedCurveOffset> rawOffsets = GetOffsets();

    if (ui->tableWidget->currentRow() == -1)
    {
        currentRow = ui->tableWidget->rowCount();

        rawOffsets.append({.name = name, .formula = "0"_L1});
    }
    else
    {
        currentRow = ui->tableWidget->currentRow() + 1;

        if (currentRow >= 0 && currentRow <= rawOffsets.size())
        {
            rawOffsets.insert(currentRow, {.name = name, .formula = "0"_L1});
        }
    }

    SetOffsets(rawOffsets);

    ui->tableWidget->selectRow(currentRow);
    ui->tableWidget->repaint(); // Force repain to fix paint artifacts on Mac OS
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::RemoveOffset()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    QVector<VRawGraduatedCurveOffset> rawOffsets = GetOffsets();

    if (row >= 0 && row < rawOffsets.size())
    {
        data->RemoveUniqueName(rawOffsets.at(row).name);
        rawOffsets.removeAt(row);
    }

    SetOffsets(rawOffsets);

    ui->tableWidget->rowCount() > 0 ? ui->tableWidget->selectRow(0) : EnableDetails(false);
    ui->tableWidget->repaint(); // Force repain to fix paint artifacts on Mac OS X

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::MoveUp()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1 || row == 0)
    {
        return;
    }

    QVector<VRawGraduatedCurveOffset> rawOffsets = GetOffsets();

    if (row > 0 && row < rawOffsets.size())
    {
        rawOffsets.move(row, row - 1);
    }

    SetOffsets(rawOffsets);

    ui->tableWidget->selectRow(row - 1);
    ui->tableWidget->repaint(); // Force repain to fix paint artifacts on Mac OS X
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::MoveDown()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1 || row == ui->tableWidget->rowCount() - 1)
    {
        return;
    }

    QVector<VRawGraduatedCurveOffset> rawOffsets = GetOffsets();

    if (row >= 0 && row < rawOffsets.size() - 1)
    {
        rawOffsets.move(row, row + 1);
    }

    SetOffsets(rawOffsets);

    ui->tableWidget->selectRow(row + 1);
    ui->tableWidget->repaint(); // Force repain to fix paint artifacts on Mac OS X
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::DeployFormula()
{
    const QTextCursor cursor = ui->plainTextEditFormula->textCursor();

    if (ui->plainTextEditFormula->height() < DIALOG_MAX_FORMULA_HEIGHT)
    {
        ui->plainTextEditFormula->setFixedHeight(DIALOG_MAX_FORMULA_HEIGHT);
        // Set icon from theme (internal for Windows system)
        ui->pushButtonGrow->setIcon(FromTheme(VThemeIcon::GoNext));
    }
    else
    {
        ui->plainTextEditFormula->setFixedHeight(m_formulaBaseHeight);
        // Set icon from theme (internal for Windows system)
        ui->pushButtonGrow->setIcon(FromTheme(VThemeIcon::GoDown));
    }

    // I found that after change size of formula field, it was filed for angle formula, field for formula became black.
    // This code prevents this.
    setUpdatesEnabled(false);
    repaint();
    setUpdatesEnabled(true);

    ui->plainTextEditFormula->setFocus();
    ui->plainTextEditFormula->setTextCursor(cursor);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::Fx()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    QVector<VRawGraduatedCurveOffset> rawOffsets = GetOffsets();

    if (row < 0 || row >= rawOffsets.size() || row >= m_offsets.size())
    {
        return;
    }

    VRawGraduatedCurveOffset &offsetData = rawOffsets[row];

    QScopedPointer<DialogEditWrongFormula> const dialog(
        new DialogEditWrongFormula(m_offsets.at(row).formulaData.data(), toolId, this));
    dialog->setWindowTitle(tr("Edit offset"));
    dialog->SetFormula(offsetData.formula);
    const QString postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
    dialog->setPostfix(postfix); // Show unit in dialog lable (cm, mm or inch)

    if (dialog->exec() == QDialog::Accepted)
    {
        offsetData.formula = dialog->GetFormula();
        SetOffsets(rawOffsets);

        ui->tableWidget->selectRow(row);
        ShowOffsetDetails();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::SaveOffsetName(const QString &text)
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1 || text.isEmpty())
    {
        return;
    }

    QString const newName = VAbstractApplication::VApp()->TrVars()->InternalVarFromUser(text);
    if (const VContainer *localData = m_offsets.isEmpty() ? data : m_offsets.constLast().formulaData.data();
        not localData->IsUnique(newName))
    {
        return;
    }

    QVector<VRawGraduatedCurveOffset> rawOffsets = GetOffsets();

    for (int i = row + 1; i < rawOffsets.size(); ++i)
    {
        if (VRawGraduatedCurveOffset &offset = rawOffsets[i]; offset.formula.contains(rawOffsets.at(row).name))
        {
            offset.formula.replace(rawOffsets.at(row).name, newName);
        }
    }

    rawOffsets[row].name = newName;

    SetOffsets(rawOffsets);

    const QSignalBlocker blocker(ui->tableWidget);
    ui->tableWidget->selectRow(row);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::SaveOffsetDescription()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1 || row >= m_offsets.size())
    {
        return;
    }

    const QTextCursor cursor = ui->plainTextEditDescription->textCursor();

    QVector<VRawGraduatedCurveOffset> rawOffsets = GetOffsets();
    rawOffsets[row].description = ui->plainTextEditDescription->toPlainText();
    SetOffsets(rawOffsets);

    {
        const QSignalBlocker blocker(ui->tableWidget);
        ui->tableWidget->selectRow(row);
    }
    ui->plainTextEditDescription->setTextCursor(cursor);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::SaveOffsetFormula()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1 || row >= m_offsets.size())
    {
        return;
    }

    QString const text = ui->plainTextEditFormula->toPlainText();
    QString const units = UnitsToStr(VAbstractValApplication::VApp()->patternUnits());

    VFormula widthFormula(m_offsets.at(row).formulaData.data());
    widthFormula.SetFormula(text, FormulaType::FromUser);
    widthFormula.setToolId(toolId);
    widthFormula.setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits()));
    widthFormula.Eval();

    if (widthFormula.error())
    {
        ui->labelCalculatedValue->setText(tr("Error") + " ("_L1 + units + "). "_L1 + widthFormula.Reason());
        ui->labelCalculatedValue->setToolTip(tr("Parser error: %1").arg(widthFormula.Reason()));
        return;
    }

    ui->labelCalculatedValue->setText(widthFormula.getStringValue());
    ui->labelCalculatedValue->setToolTip(tr("Value"));

    QVector<VRawGraduatedCurveOffset> rawOffsets = GetOffsets();
    rawOffsets[row].formula = widthFormula.GetFormula(FormulaType::ToSystem);

    const QTextCursor cursor = ui->plainTextEditFormula->textCursor();

    SetOffsets(rawOffsets);

    {
        const QSignalBlocker blocker(ui->tableWidget);
        ui->tableWidget->selectRow(row);
    }
    ui->plainTextEditFormula->setTextCursor(cursor);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::FinishCreating()
{
    vis->SetMode(Mode::Show);
    vis->RefreshGeometry();

    emit ToolTip(QString());

    setModal(true);
    show();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::InitIcons()
{
    const auto resource = QStringLiteral("icon");

    const auto fxIcon = QStringLiteral("24x24/fx.png");
    ui->toolButtonExpr->setIcon(VTheme::GetIconResource(resource, fxIcon));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::FillOffsets()
{
    const QSignalBlocker blocker(ui->tableWidget);
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(static_cast<int>(m_offsets.size()));

    for (int i = 0; i < m_offsets.size(); ++i)
    {
        const VGraduatedCurveOffsetFormula &offsetData = m_offsets.at(i);
        AddCell(ui->tableWidget, offsetData.name, i, 0, Qt::AlignVCenter); // name

        VFormula const formula = offsetData.formula;
        QString const calculatedValue = VAbstractApplication::VApp()->LocaleToString(formula.getDoubleValue());
        AddCell(ui->tableWidget, calculatedValue, i, 1, Qt::AlignCenter, !formula.error()); // calculated value

        QString const formulaStr = formula.GetFormula(FormulaType::ToUser);
        AddCell(ui->tableWidget, formulaStr, i, 2, Qt::AlignVCenter); // formula
    }

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::Controls()
{
    ui->toolButtonRemove->setEnabled(ui->tableWidget->rowCount() > 0);

    if (ui->tableWidget->rowCount() >= 2)
    {
        if (ui->tableWidget->currentRow() == 0)
        {
            ui->toolButtonUp->setEnabled(false);
            ui->toolButtonDown->setEnabled(true);
        }
        else if (ui->tableWidget->currentRow() == ui->tableWidget->rowCount() - 1)
        {
            ui->toolButtonUp->setEnabled(true);
            ui->toolButtonDown->setEnabled(false);
        }
        else
        {
            ui->toolButtonUp->setEnabled(true);
            ui->toolButtonDown->setEnabled(true);
        }
    }
    else
    {
        ui->toolButtonUp->setEnabled(false);
        ui->toolButtonDown->setEnabled(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::EnableDetails(bool enabled)
{
    if (enabled)
    {
        Controls();
    }
    else
    {
        ui->toolButtonRemove->setEnabled(enabled);
        ui->toolButtonUp->setEnabled(enabled);
        ui->toolButtonDown->setEnabled(enabled);
    }

    if (not enabled)
    { // Clear
        {
            const QSignalBlocker blocker(ui->lineEditName);
            ui->lineEditName->clear();
        }

        {
            const QSignalBlocker blocker(ui->plainTextEditDescription);
            ui->plainTextEditDescription->clear();
        }

        {
            const QSignalBlocker blocker(ui->labelCalculatedValue);
            ui->labelCalculatedValue->clear();
        }

        const QSignalBlocker blocker(ui->plainTextEditFormula);
        ui->plainTextEditFormula->clear();
    }

    ui->pushButtonGrow->setEnabled(enabled);
    ui->toolButtonExpr->setEnabled(enabled);
    ui->lineEditName->setEnabled(enabled);
    ui->plainTextEditDescription->setEnabled(enabled);
    ui->plainTextEditFormula->setEnabled(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogGraduatedCurve::GetOffsetName(bool translate) const -> QString
{
    const VContainer *localData = m_offsets.isEmpty() ? data : m_offsets.constLast().formulaData.data();
    qint32 num = 1;
    QString name;
    QString const subName = translate ? VAbstractApplication::VApp()->TrVars()->InternalVarToUser(offset_) : offset_;
    do
    {
        name = subName + QString::number(num);
        num++;
    } while (not localData->IsUnique(name));
    return name;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogGraduatedCurve::VisualizationOffsets() const -> QVector<VRawGraduatedCurveOffset>
{
    QVector<VRawGraduatedCurveOffset> toUserOffsets;
    toUserOffsets.reserve(m_offsets.size());
    for (const auto &offset : std::as_const(m_offsets))
    {
        toUserOffsets.append({.name = offset.name,
                              .formula = offset.formula.GetFormula(FormulaType::ToUser),
                              .description = offset.description});
    }
    return toUserOffsets;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogGraduatedCurve::ShowHeaderUnits(int column)
{
    const QString unit = UnitsToStr(VAbstractValApplication::VApp()->patternUnits());
    const QString header = ui->tableWidget->horizontalHeaderItem(column)->text();
    const auto unitHeader = QStringLiteral("%1 (%2)").arg(header, unit);
    ui->tableWidget->horizontalHeaderItem(column)->setText(unitHeader);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogGraduatedCurve::GenerateDefName() const -> QString
{
    auto GenerateName = [this](const QString &base) -> QString
    {
        VSplinePath path;
        qint32 num = 1;
        QString name;
        do
        {
            name = base + QString::number(num++);
            path.SetNameSuffix(name);

        } while (!data->IsUnique(path.name()));

        return name;
    };
    QSharedPointer<VAbstractCurve> const curve = data->GeometricObject<VAbstractCurve>(GetOriginCurveId());
    if (!curve->IsDerivative())
    {
        return GenerateName(curve->HeadlessName() + "__o"_L1);
    }

    return GenerateName("Curve"_L1 + offset_);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogGraduatedCurve::IsValid() const -> bool
{
    bool const flagOffset = std::any_of(m_offsets.constBegin(),
                                        m_offsets.constEnd(),
                                        [](const auto &offset) -> auto { return !offset.formula.error(); });

    return !m_offsets.isEmpty() && flagOffset && m_flagSuffix && m_flagAlias;
}
