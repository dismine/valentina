/************************************************************************
 **
 **  @file   dialogincrements.cpp
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

#include "dialogincrements.h"
#include "../vmisc/vvalentinasettings.h"
#include "ui_dialogincrements.h"
#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#include "../vmisc/backport/qoverload.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#include "../qmuparser/qmudef.h"
#include "../qmuparser/qmutokenparser.h"
#include "../vmisc/theme/vtheme.h"
#include "../vpatterndb/calculator.h"
#include "../vpatterndb/variables/varcradius.h"
#include "../vpatterndb/variables/vcurveangle.h"
#include "../vpatterndb/variables/vcurveclength.h"
#include "../vpatterndb/variables/vcurvelength.h"
#include "../vpatterndb/variables/vincrement.h"
#include "../vpatterndb/variables/vlineangle.h"
#include "../vpatterndb/variables/vlinelength.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vtools/dialogs/support/dialogeditwrongformula.h"

#include <QCloseEvent>
#include <QDir>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QtNumeric>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

constexpr int DIALOG_MAX_FORMULA_HEIGHT = 64;

namespace
{
enum class IncrUnits : qint8
{
    Pattern,
    Degrees
};
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogIncrements create dialog
 * @param data container with data
 * @param doc dom document container
 * @param parent parent widget
 */
DialogIncrements::DialogIncrements(VContainer *data, VPattern *doc, QWidget *parent)
  : DialogTool(data, doc, NULL_ID, parent),
    ui(new Ui::DialogIncrements),
    m_data(data),
    m_doc(doc),
    m_completeData(doc->GetCompleteData()),
    m_searchHistory(new QMenu(this)),
    m_searchHistoryPC(new QMenu(this))
{
    ui->setupUi(this);

#if defined(Q_OS_MAC)
    setWindowFlags(Qt::Window);
#endif

    InitIcons();

    ui->lineEditName->setClearButtonEnabled(true);
    ui->lineEditNamePC->setClearButtonEnabled(true);

    ui->lineEditFind->installEventFilter(this);
    ui->lineEditFindPC->installEventFilter(this);

    m_search = QSharedPointer<VTableSearch>(new VTableSearch(ui->tableWidgetIncrement));
    m_searchPC = QSharedPointer<VTableSearch>(new VTableSearch(ui->tableWidgetPC));

    m_formulaBaseHeight = ui->plainTextEditFormula->height();
    ui->plainTextEditFormula->installEventFilter(this);
    m_formulaBaseHeightPC = ui->plainTextEditFormulaPC->height();
    ui->plainTextEditFormulaPC->installEventFilter(this);

    VAbstractApplication::VApp()->Settings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    qCDebug(vDialog, "Showing variables.");
    ShowUnits();

    InitIncrementUnits(ui->comboBoxIncrementUnits);
    InitIncrementUnits(ui->comboBoxPreviewCalculationUnits);

    ui->comboBoxIncrementUnits->blockSignals(true);
    ui->comboBoxIncrementUnits->setCurrentIndex(-1);
    ui->comboBoxIncrementUnits->blockSignals(true);

    ui->comboBoxPreviewCalculationUnits->blockSignals(true);
    ui->comboBoxPreviewCalculationUnits->setCurrentIndex(-1);
    ui->comboBoxPreviewCalculationUnits->blockSignals(true);

    connect(ui->comboBoxIncrementUnits, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogIncrements::SaveIncrUnits);

    FillIncrements();
    FillPreviewCalculations();
    FillLengthsLines();
    FillLengthLinesAngles();
    FillLengthsCurves();
    FillCurvesCLengths();
    FillRadiusesArcs();
    FillAnglesCurves();

    connect(this->m_doc, &VPattern::FullUpdateFromFile, this, &DialogIncrements::FullUpdateFromFile);

    ui->tabWidget->setCurrentIndex(0);
    auto *validator = new QRegularExpressionValidator(QRegularExpression(QStringLiteral("^$|") + NameRegExp()), this);
    ui->lineEditName->setValidator(validator);
    ui->lineEditNamePC->setValidator(validator);

    connect(ui->tableWidgetIncrement, &QTableWidget::itemSelectionChanged, this,
            &DialogIncrements::ShowIncrementDetails);
    connect(ui->tableWidgetPC, &QTableWidget::itemSelectionChanged, this, &DialogIncrements::ShowIncrementDetails);

    ui->toolButtonAdd->setMenu(InitVarTypeMenu(ui->toolButtonAdd->menu(), true /*increments tab*/));
    ui->toolButtonAddPC->setMenu(InitVarTypeMenu(ui->toolButtonAddPC->menu(), false /*preview calculations tab*/));

    connect(ui->toolButtonAdd, &QToolButton::clicked, this, &DialogIncrements::AddIncrement);
    connect(ui->toolButtonAddPC, &QToolButton::clicked, this, &DialogIncrements::AddIncrement);
    connect(ui->toolButtonRemove, &QToolButton::clicked, this, &DialogIncrements::RemoveIncrement);
    connect(ui->toolButtonRemovePC, &QToolButton::clicked, this, &DialogIncrements::RemoveIncrement);
    connect(ui->toolButtonUp, &QToolButton::clicked, this, &DialogIncrements::MoveUp);
    connect(ui->toolButtonUpPC, &QToolButton::clicked, this, &DialogIncrements::MoveUp);
    connect(ui->toolButtonDown, &QToolButton::clicked, this, &DialogIncrements::MoveDown);
    connect(ui->toolButtonDownPC, &QToolButton::clicked, this, &DialogIncrements::MoveDown);
    connect(ui->pushButtonGrow, &QPushButton::clicked, this, &DialogIncrements::DeployFormula);
    connect(ui->pushButtonGrowPC, &QPushButton::clicked, this, &DialogIncrements::DeployFormula);
    connect(ui->toolButtonExpr, &QToolButton::clicked, this, &DialogIncrements::Fx);
    connect(ui->toolButtonExprPC, &QToolButton::clicked, this, &DialogIncrements::Fx);
    connect(ui->lineEditName, &QLineEdit::textEdited, this, &DialogIncrements::SaveIncrName);
    connect(ui->lineEditNamePC, &QLineEdit::textEdited, this, &DialogIncrements::SaveIncrName);
    connect(ui->plainTextEditDescription, &QPlainTextEdit::textChanged, this, &DialogIncrements::SaveIncrDescription);
    connect(ui->plainTextEditDescriptionPC, &QPlainTextEdit::textChanged, this, &DialogIncrements::SaveIncrDescription);
    connect(ui->plainTextEditFormula, &QPlainTextEdit::textChanged, this, &DialogIncrements::SaveIncrFormula);
    connect(ui->plainTextEditFormulaPC, &QPlainTextEdit::textChanged, this, &DialogIncrements::SaveIncrFormula);
    connect(ui->pushButtonRefresh, &QPushButton::clicked, this, &DialogIncrements::RefreshPattern);
    connect(ui->pushButtonRefreshPC, &QPushButton::clicked, this, &DialogIncrements::RefreshPattern);

    InitSearch();

    if (ui->tableWidgetIncrement->rowCount() > 0)
    {
        ui->tableWidgetIncrement->selectRow(0);
    }

    if (ui->tableWidgetPC->rowCount() > 0)
    {
        ui->tableWidgetPC->selectRow(0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FillIncrementTable fill data for increment table
 */
void DialogIncrements::FillIncrements()
{
    FillIncrementsTable(ui->tableWidgetIncrement, m_data->DataIncrementsWithSeparators(), false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::FillPreviewCalculations()
{
    FillIncrementsTable(ui->tableWidgetPC, m_data->DataIncrementsWithSeparators(), true);
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> void DialogIncrements::FillTable(const QMap<QString, T> &varTable, QTableWidget *table)
{
    SCASSERT(table != nullptr)

    qint32 currentRow = -1;
    QMapIterator<QString, T> i(varTable);
    while (i.hasNext())
    {
        i.next();
        qreal length = *i.value()->GetValue();
        currentRow++;
        table->setRowCount(static_cast<int>(varTable.size()));

        auto *item = new QTableWidgetItem(i.key());
        item->setTextAlignment(Qt::AlignLeft);
        QFont font = item->font();
        font.setBold(true);
        item->setFont(font);
        table->setItem(currentRow, 0, item);

        item = new QTableWidgetItem(VAbstractApplication::VApp()->LocaleToString(length));
        item->setTextAlignment(Qt::AlignHCenter);
        table->setItem(currentRow, 1, item);
    }
    table->resizeColumnsToContents();
    table->resizeRowsToContents();
    table->verticalHeader()->setDefaultSectionSize(20);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FillLengthLines fill data for table of lines lengths
 */
void DialogIncrements::FillLengthsLines()
{
    FillTable(m_completeData.DataLengthLines(), ui->tableWidgetLines);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::FillLengthLinesAngles()
{
    FillTable(m_completeData.DataAngleLines(), ui->tableWidgetLinesAngles);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FillLengthSplines fill data for table of splines lengths
 */
void DialogIncrements::FillLengthsCurves()
{
    FillTable(m_completeData.DataLengthCurves(), ui->tableWidgetSplines);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::FillCurvesCLengths()
{
    FillTable(m_completeData.DataCurvesCLength(), ui->tableWidgetCLength);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::FillRadiusesArcs()
{
    FillTable(m_completeData.DataRadiusesArcs(), ui->tableWidgetRadiusesArcs);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::FillAnglesCurves()
{
    FillTable(m_completeData.DataAnglesCurves(), ui->tableWidgetAnglesCurves);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::ShowUnits()
{
    const QString unit = UnitsToStr(VAbstractValApplication::VApp()->patternUnits());

    ShowHeaderUnits(ui->tableWidgetIncrement, 1, unit); // calculated value
    ShowHeaderUnits(ui->tableWidgetIncrement, 2, unit); // formula
    ShowHeaderUnits(ui->tableWidgetPC, 1, unit);        // calculated value
    ShowHeaderUnits(ui->tableWidgetPC, 2, unit);        // formula

    ShowHeaderUnits(ui->tableWidgetLines, 1, unit);                // lengths
    ShowHeaderUnits(ui->tableWidgetSplines, 1, unit);              // lengths
    ShowHeaderUnits(ui->tableWidgetCLength, 1, unit);              // lengths
    ShowHeaderUnits(ui->tableWidgetLinesAngles, 1, degreeSymbol);  // angle
    ShowHeaderUnits(ui->tableWidgetRadiusesArcs, 1, unit);         // radius
    ShowHeaderUnits(ui->tableWidgetAnglesCurves, 1, degreeSymbol); // angle
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::ShowHeaderUnits(QTableWidget *table, int column, const QString &unit)
{
    SCASSERT(table != nullptr)

    const QString header = table->horizontalHeaderItem(column)->text();
    const QString unitHeader = QStringLiteral("%1 (%2)").arg(header, unit);
    table->horizontalHeaderItem(column)->setText(unitHeader);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogIncrements::AddCell(QTableWidget *table, const QString &text, int row, int column, int aligment, bool ok)
    -> QTableWidgetItem *
{
    SCASSERT(table != nullptr)

    auto *item = new QTableWidgetItem(text);
    item->setTextAlignment(aligment);
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

//---------------------------------------------------------------------------------------------------------------------
auto DialogIncrements::AddSeparatorCell(QTableWidget *table, const QString &text, int row, int column, int aligment,
                                        bool ok) -> QTableWidgetItem *
{
    QTableWidgetItem *item = AddCell(table, text, row, column, aligment, ok);

    QFont itemFont = item->font();
    itemFont.setBold(true);
    itemFont.setItalic(true);
    item->setFont(itemFont);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogIncrements::GetCustomName() const -> QString
{
    qint32 num = 1;
    QString name;
    do
    {
        name = CustomIncrSign + VAbstractApplication::VApp()->TrVars()->InternalVarToUser(increment_) +
               QString::number(num);
        num++;
    } while (not m_data->IsUnique(name));
    return name;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogIncrements::ClearIncrementName(const QString &name) -> QString
{
    QString clear = name;
    const auto index = clear.indexOf(CustomIncrSign);
    if (index == 0)
    {
        clear.remove(0, 1);
    }
    return clear;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogIncrements::EvalIncrementFormula(const QString &formula, bool fromUser, VContainer *data, QLabel *label,
                                            bool special) -> bool
{
    QString postfix;
    if (special)
    {
        postfix = degreeSymbol;
    }
    else
    {
        postfix =
            UnitsToStr(VAbstractValApplication::VApp()->patternUnits()); // Show unit in dialog lable (cm, mm or inch)
    }
    if (formula.isEmpty())
    {
        label->setText(tr("Error") + " (" + postfix + "). " + tr("Empty field."));
        label->setToolTip(tr("Empty field"));
        return false;
    }

    try
    {
        QString f;
        // Replace line return character with spaces for calc if exist
        if (fromUser)
        {
            f = VAbstractApplication::VApp()->TrVars()->FormulaFromUser(
                formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
        }
        else
        {
            f = formula;
        }
        QScopedPointer<Calculator> cal(new Calculator());
        const qreal result = cal->EvalFormula(data->DataVariables(), f);

        if (qIsInf(result) || qIsNaN(result))
        {
            label->setText(tr("Error") + " (" + postfix + ").");
            label->setToolTip(tr("Invalid result. Value is infinite or NaN. Please, check your calculations."));
            return false;
        }

        label->setText(VAbstractApplication::VApp()->LocaleToString(result) + QChar(QChar::Space) + postfix);
        label->setToolTip(tr("Value"));
        return true;
    }
    catch (qmu::QmuParserError &e)
    {
        label->setText(tr("Error") + " (" + postfix + "). " + tr("Parser error: %1").arg(e.GetMsg()));
        label->setToolTip(tr("Parser error: %1").arg(e.GetMsg()));
        return false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::Controls(QTableWidget *table)
{
    SCASSERT(table != nullptr)

    if (table->rowCount() > 0)
    {
        const QTableWidgetItem *nameField = table->item(table->currentRow(), 0);
        SCASSERT(nameField != nullptr)

        if (table == ui->tableWidgetIncrement)
        {
            ui->toolButtonRemove->setEnabled(not IncrementUsed(nameField->text()));
        }
        else if (table == ui->tableWidgetPC)
        {
            ui->toolButtonRemovePC->setEnabled(not IncrementUsed(nameField->text()));
        }
    }
    else
    {
        if (table == ui->tableWidgetIncrement)
        {
            ui->toolButtonRemove->setEnabled(false);
        }
        else if (table == ui->tableWidgetPC)
        {
            ui->toolButtonRemovePC->setEnabled(false);
        }
    }

    if (table->rowCount() >= 2)
    {
        if (table->currentRow() == 0)
        {
            if (table == ui->tableWidgetIncrement)
            {
                ui->toolButtonUp->setEnabled(false);
                ui->toolButtonDown->setEnabled(true);
            }
            else if (table == ui->tableWidgetPC)
            {
                ui->toolButtonUpPC->setEnabled(false);
                ui->toolButtonDownPC->setEnabled(true);
            }
        }
        else if (table->currentRow() == table->rowCount() - 1)
        {
            if (table == ui->tableWidgetIncrement)
            {
                ui->toolButtonUp->setEnabled(true);
                ui->toolButtonDown->setEnabled(false);
            }
            else if (table == ui->tableWidgetPC)
            {
                ui->toolButtonUpPC->setEnabled(true);
                ui->toolButtonDownPC->setEnabled(false);
            }
        }
        else
        {
            if (table == ui->tableWidgetIncrement)
            {
                ui->toolButtonUp->setEnabled(true);
                ui->toolButtonDown->setEnabled(true);
            }
            else if (table == ui->tableWidgetPC)
            {
                ui->toolButtonUpPC->setEnabled(true);
                ui->toolButtonDownPC->setEnabled(true);
            }
        }
    }
    else
    {
        if (table == ui->tableWidgetIncrement)
        {
            ui->toolButtonUp->setEnabled(false);
            ui->toolButtonDown->setEnabled(false);
        }
        else if (table == ui->tableWidgetPC)
        {
            ui->toolButtonUpPC->setEnabled(false);
            ui->toolButtonDownPC->setEnabled(false);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::EnableDetails(QTableWidget *table, bool enabled)
{
    SCASSERT(table != nullptr)

    if (enabled)
    {
        Controls(table);
    }
    else
    {
        if (table == ui->tableWidgetIncrement)
        {
            ui->toolButtonRemove->setEnabled(enabled);
            ui->toolButtonUp->setEnabled(enabled);
            ui->toolButtonDown->setEnabled(enabled);
        }
        else if (table == ui->tableWidgetPC)
        {
            ui->toolButtonRemovePC->setEnabled(enabled);
            ui->toolButtonUpPC->setEnabled(enabled);
            ui->toolButtonDownPC->setEnabled(enabled);
        }
    }

    if (not enabled)
    { // Clear
        if (table == ui->tableWidgetIncrement)
        {
            ui->lineEditName->blockSignals(true);
            ui->lineEditName->clear();
            ui->lineEditName->blockSignals(false);

            ui->comboBoxIncrementUnits->blockSignals(true);
            ui->comboBoxIncrementUnits->setCurrentIndex(-1);
            ui->comboBoxIncrementUnits->blockSignals(false);

            ui->plainTextEditDescription->blockSignals(true);
            ui->plainTextEditDescription->clear();
            ui->plainTextEditDescription->blockSignals(false);

            ui->labelCalculatedValue->blockSignals(true);
            ui->labelCalculatedValue->clear();
            ui->labelCalculatedValue->blockSignals(false);

            ui->plainTextEditFormula->blockSignals(true);
            ui->plainTextEditFormula->clear();
            ui->plainTextEditFormula->blockSignals(false);
        }
        else if (table == ui->tableWidgetPC)
        {
            ui->lineEditNamePC->blockSignals(true);
            ui->lineEditNamePC->clear();
            ui->lineEditNamePC->blockSignals(false);

            ui->comboBoxPreviewCalculationUnits->blockSignals(true);
            ui->comboBoxPreviewCalculationUnits->setCurrentIndex(-1);
            ui->comboBoxPreviewCalculationUnits->blockSignals(false);

            ui->plainTextEditDescriptionPC->blockSignals(true);
            ui->plainTextEditDescriptionPC->clear();
            ui->plainTextEditDescriptionPC->blockSignals(false);

            ui->labelCalculatedValuePC->blockSignals(true);
            ui->labelCalculatedValuePC->clear();
            ui->labelCalculatedValuePC->blockSignals(false);

            ui->plainTextEditFormulaPC->blockSignals(true);
            ui->plainTextEditFormulaPC->clear();
            ui->plainTextEditFormulaPC->blockSignals(false);
        }
    }

    if (table == ui->tableWidgetIncrement)
    {
        ui->pushButtonGrow->setEnabled(enabled);
        ui->toolButtonExpr->setEnabled(enabled);
        ui->lineEditName->setEnabled(enabled);
        ui->comboBoxIncrementUnits->setEnabled(enabled);
        ui->plainTextEditDescription->setEnabled(enabled);
        ui->plainTextEditFormula->setEnabled(enabled);
    }
    else if (table == ui->tableWidgetPC)
    {
        ui->pushButtonGrowPC->setEnabled(enabled);
        ui->toolButtonExprPC->setEnabled(enabled);
        ui->lineEditNamePC->setEnabled(enabled);
        ui->comboBoxPreviewCalculationUnits->setEnabled(enabled);
        ui->plainTextEditDescriptionPC->setEnabled(enabled);
        ui->plainTextEditFormulaPC->setEnabled(enabled);
    }

    if (table->rowCount() > 0)
    {
        const QTableWidgetItem *nameField = table->item(table->currentRow(), 0);
        SCASSERT(nameField != nullptr)
        QSharedPointer<VIncrement> incr = m_data->GetVariable<VIncrement>(nameField->text());
        const bool isSeparator = incr->GetIncrementType() == IncrementType::Separator;

        if (table == ui->tableWidgetIncrement)
        {
            ui->labelCalculated->setVisible(not isSeparator);
            ui->labelCalculatedValue->setVisible(not isSeparator);
            ui->labelFormula->setVisible(not isSeparator);
            ui->plainTextEditFormula->setVisible(not isSeparator);
            ui->pushButtonGrow->setVisible(not isSeparator);
            ui->toolButtonExpr->setVisible(not isSeparator);
            ui->labelIncrementUnits->setVisible(not isSeparator);
            ui->comboBoxIncrementUnits->setVisible(not isSeparator);
        }
        else if (table == ui->tableWidgetPC)
        {
            ui->labelCalculatedPC->setVisible(not isSeparator);
            ui->labelCalculatedValuePC->setVisible(not isSeparator);
            ui->labelFormulaPC->setVisible(not isSeparator);
            ui->plainTextEditFormulaPC->setVisible(not isSeparator);
            ui->pushButtonGrowPC->setVisible(not isSeparator);
            ui->toolButtonExprPC->setVisible(not isSeparator);
            ui->labelPreviewCalculationUnits->setVisible(not isSeparator);
            ui->comboBoxPreviewCalculationUnits->setVisible(not isSeparator);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::LocalUpdateTree()
{
    m_doc->LiteParseIncrements();

    int row = ui->tableWidgetIncrement->currentRow();

    FillIncrements();

    ui->tableWidgetIncrement->blockSignals(true);
    ui->tableWidgetIncrement->selectRow(row);
    ui->tableWidgetIncrement->blockSignals(false);

    row = ui->tableWidgetPC->currentRow();

    FillPreviewCalculations();

    ui->tableWidgetPC->blockSignals(true);
    ui->tableWidgetPC->selectRow(row);
    ui->tableWidgetPC->blockSignals(false);

    m_search->RefreshList(ui->lineEditFind->text());
    m_searchPC->RefreshList(ui->lineEditFindPC->text());
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogIncrements::IncrementUsed(const QString &name) const -> bool
{
    const QVector<VFormulaField> expressions = m_doc->ListExpressions();

    return std::any_of(expressions.constBegin(), expressions.constEnd(),
                       [name](const auto &field)
                       {
                           if (field.expression.indexOf(name) == -1)
                           {
                               return false;
                           }

                           // Eval formula
                           try
                           {
                               QScopedPointer<qmu::QmuTokenParser> cal(
                                   new qmu::QmuTokenParser(field.expression, false, false));

                               // Tokens (variables, measurements)
                               if (cal->GetTokens().values().contains(name))
                               {
                                   return true;
                               }
                           }
                           catch (const qmu::QmuParserError &)
                           {
                               // Do nothing. Because we not sure if used. A formula is broken.
                           }
                           return false;
                       });
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::CacheRename(const QString &name, const QString &newName)
{
    for (auto &i : m_renameList)
    {
        if (i.second == name)
        {
            i.second = newName;
            return;
        }
    }

    m_renameList.append(qMakePair(name, newName));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::ShowTableIncrementDetails(QTableWidget *table)
{
    QLineEdit *lineEditName = nullptr;
    QPlainTextEdit *plainTextEditDescription = nullptr;
    QPlainTextEdit *plainTextEditFormula = nullptr;
    QLabel *labelCalculatedValue = nullptr;
    QComboBox *comboboxUnits = nullptr;

    if (table == ui->tableWidgetIncrement)
    {
        lineEditName = ui->lineEditName;
        plainTextEditDescription = ui->plainTextEditDescription;
        plainTextEditFormula = ui->plainTextEditFormula;
        labelCalculatedValue = ui->labelCalculatedValue;
        comboboxUnits = ui->comboBoxIncrementUnits;
    }
    else if (table == ui->tableWidgetPC)
    {
        lineEditName = ui->lineEditNamePC;
        plainTextEditDescription = ui->plainTextEditDescriptionPC;
        plainTextEditFormula = ui->plainTextEditFormulaPC;
        labelCalculatedValue = ui->labelCalculatedValuePC;
        comboboxUnits = ui->comboBoxPreviewCalculationUnits;
    }
    else
    {
        return;
    }

    if (table->rowCount() > 0 && table->currentRow() != -1)
    {
        EnableDetails(table, true);

        // name
        const QTableWidgetItem *nameField = table->item(table->currentRow(), 0);
        QSharedPointer<VIncrement> incr;

        try
        {
            incr = m_data->GetVariable<VIncrement>(nameField->text());
        }
        catch (const VExceptionBadId &e)
        {
            Q_UNUSED(e)
            EnableDetails(table, false);
            return;
        }

        lineEditName->blockSignals(true);
        lineEditName->setText(ClearIncrementName(incr->GetName()));
        lineEditName->blockSignals(false);

        comboboxUnits->blockSignals(true);
        comboboxUnits->setCurrentIndex(comboboxUnits->findData(
            static_cast<int>(incr->IsSpecialUnits() ? IncrUnits::Degrees : IncrUnits::Pattern)));
        comboboxUnits->blockSignals(false);

        plainTextEditDescription->blockSignals(true);
        plainTextEditDescription->setPlainText(incr->GetDescription());
        plainTextEditDescription->blockSignals(false);

        EvalIncrementFormula(incr->GetFormula(), false, incr->GetData(), labelCalculatedValue, incr->IsSpecialUnits());
        plainTextEditFormula->blockSignals(true);

        QString formula = VTranslateVars::TryFormulaToUser(incr->GetFormula(),
                                                           VAbstractApplication::VApp()->Settings()->GetOsSeparator());

        plainTextEditFormula->setPlainText(formula);
        plainTextEditFormula->blockSignals(false);
    }
    else
    {
        EnableDetails(table, false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogIncrements::InitVarTypeMenu(QMenu *menu, bool incrementTab) -> QMenu *
{
    if (menu == nullptr)
    {
        menu = new QMenu(this);
    }
    else
    {
        menu->clear();
    }

    QAction *action = menu->addAction(tr("Increment"));
    action->setData(incrementTab); // Increments tab
    connect(action, &QAction::triggered, this, &DialogIncrements::AddIncrement);

    action = menu->addAction(tr("Separator"));
    action->setData(incrementTab); // Increments tab
    connect(action, &QAction::triggered, this, &DialogIncrements::AddSeparator);

    return menu;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::AddNewIncrement(IncrementType type)
{
    qCDebug(vDialog, "Add new increment");

    bool incrementMode = true;
    if (auto *action = qobject_cast<QAction *>(sender()))
    {
        incrementMode = action->data().toBool();
    }
    else if (auto *button = qobject_cast<QToolButton *>(sender()))
    {
        incrementMode = button == ui->toolButtonAdd;
    }
    else
    {
        return;
    }

    QTableWidget *table = incrementMode ? ui->tableWidgetIncrement : ui->tableWidgetPC;

    const QString name = GetCustomName();
    qint32 currentRow = -1;

    if (table->currentRow() == -1)
    {
        currentRow = table->rowCount();
        incrementMode ? m_doc->AddEmptyIncrement(name, type) : m_doc->AddEmptyPreviewCalculation(name, type);
    }
    else
    {
        currentRow = table->currentRow() + 1;
        const QTableWidgetItem *nameField = table->item(table->currentRow(), 0);

        incrementMode ? m_doc->AddEmptyIncrementAfter(nameField->text(), name, type)
                      : m_doc->AddEmptyPreviewCalculationAfter(nameField->text(), name, type);
    }

    m_hasChanges = true;
    LocalUpdateTree();

    table->selectRow(currentRow);
    table->repaint(); // Force repain to fix paint artifacts on Mac OS X
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::InitSearch()
{
    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();

    m_search->SetUseUnicodePreperties(settings->GetIncrementsSearchOptionUseUnicodeProperties());
    m_search->SetMatchWord(settings->GetIncrementsSearchOptionWholeWord());
    m_search->SetMatchRegexp(settings->GetIncrementsSearchOptionRegexp());
    m_search->SetMatchCase(settings->GetIncrementsSearchOptionMatchCase());

    m_searchPC->SetUseUnicodePreperties(settings->GetPreviewCalculationsSearchOptionUseUnicodeProperties());
    m_searchPC->SetMatchWord(settings->GetPreviewCalculationsSearchOptionWholeWord());
    m_searchPC->SetMatchRegexp(settings->GetPreviewCalculationsSearchOptionRegexp());
    m_searchPC->SetMatchCase(settings->GetPreviewCalculationsSearchOptionMatchCase());

    ui->lineEditFind->setPlaceholderText(m_search->SearchPlaceholder());
    ui->lineEditFindPC->setPlaceholderText(m_searchPC->SearchPlaceholder());

    UpdateSearchControlsTooltips();

    connect(ui->lineEditFind, &QLineEdit::textEdited, this, [this](const QString &term) { m_search->Find(term); });
    connect(ui->lineEditFind, &QLineEdit::editingFinished, this,
            [this]()
            {
                SaveIncrementsSearchRequest();
                InitIncrementsSearchHistory();
                m_search->Find(ui->lineEditFind->text());
            });
    connect(ui->lineEditFindPC, &QLineEdit::textEdited, this, [this](const QString &term) { m_searchPC->Find(term); });
    connect(ui->lineEditFindPC, &QLineEdit::editingFinished, this,
            [this]()
            {
                SavePreviewCalculationsSearchRequest();
                InitPreviewCalculationsSearchHistory();
                m_searchPC->Find(ui->lineEditFindPC->text());
            });

    connect(ui->toolButtonFindPrevious, &QToolButton::clicked, this,
            [this]()
            {
                SaveIncrementsSearchRequest();
                InitPreviewCalculationsSearchHistory();
                m_search->FindPrevious();
                ui->labelResults->setText(
                    QStringLiteral("%1/%2").arg(m_search->MatchIndex() + 1).arg(m_search->MatchCount()));
            });
    connect(ui->toolButtonFindPreviousPC, &QToolButton::clicked, this,
            [this]()
            {
                SavePreviewCalculationsSearchRequest();
                InitPreviewCalculationsSearchHistory();
                m_searchPC->FindPrevious();
                ui->labelResultsPC->setText(
                    QStringLiteral("%1/%2").arg(m_searchPC->MatchIndex() + 1).arg(m_searchPC->MatchCount()));
            });
    connect(ui->toolButtonFindNext, &QToolButton::clicked, this,
            [this]()
            {
                SaveIncrementsSearchRequest();
                InitIncrementsSearchHistory();
                m_search->FindNext();
                ui->labelResults->setText(
                    QStringLiteral("%1/%2").arg(m_search->MatchIndex() + 1).arg(m_search->MatchCount()));
            });
    connect(ui->toolButtonFindNextPC, &QToolButton::clicked, this,
            [this]()
            {
                SavePreviewCalculationsSearchRequest();
                InitPreviewCalculationsSearchHistory();
                m_searchPC->FindNext();
                ui->labelResultsPC->setText(
                    QStringLiteral("%1/%2").arg(m_searchPC->MatchIndex() + 1).arg(m_searchPC->MatchCount()));
            });

    connect(m_search.data(), &VTableSearch::HasResult, this,
            [this](bool state)
            {
                ui->toolButtonFindPrevious->setEnabled(state);
                ui->toolButtonFindNext->setEnabled(state);

                if (state)
                {
                    ui->labelResults->setText(
                        QStringLiteral("%1/%2").arg(m_search->MatchIndex() + 1).arg(m_search->MatchCount()));
                }
                else
                {
                    ui->labelResults->setText(tr("0 results"));
                }

                QPalette palette;

                if (not state && not ui->lineEditFind->text().isEmpty())
                {
                    palette.setColor(QPalette::Text, Qt::red);
                    ui->lineEditFind->setPalette(palette);

                    palette.setColor(QPalette::Active, ui->labelResults->foregroundRole(), Qt::red);
                    palette.setColor(QPalette::Inactive, ui->labelResults->foregroundRole(), Qt::red);
                    ui->labelResults->setPalette(palette);
                }
                else
                {
                    ui->lineEditFind->setPalette(palette);
                    ui->labelResults->setPalette(palette);
                }
            });

    connect(m_searchPC.data(), &VTableSearch::HasResult, this,
            [this](bool state)
            {
                ui->toolButtonFindPreviousPC->setEnabled(state);
                ui->toolButtonFindNextPC->setEnabled(state);

                if (state)
                {
                    ui->labelResultsPC->setText(
                        QStringLiteral("%1/%2").arg(m_searchPC->MatchIndex() + 1).arg(m_searchPC->MatchCount()));
                }
                else
                {
                    ui->labelResultsPC->setText(tr("0 results"));
                }

                QPalette palette;

                if (not state && not ui->lineEditFindPC->text().isEmpty())
                {
                    palette.setColor(QPalette::Text, Qt::red);
                    ui->lineEditFindPC->setPalette(palette);

                    palette.setColor(QPalette::Active, ui->labelResultsPC->foregroundRole(), Qt::red);
                    palette.setColor(QPalette::Inactive, ui->labelResultsPC->foregroundRole(), Qt::red);
                    ui->labelResultsPC->setPalette(palette);
                }
                else
                {
                    ui->lineEditFindPC->setPalette(palette);
                    ui->labelResultsPC->setPalette(palette);
                }
            });

    connect(ui->toolButtonCaseSensitive, &QToolButton::toggled, this,
            [this](bool checked)
            {
                m_search->SetMatchCase(checked);
                m_search->Find(ui->lineEditFind->text());
                ui->lineEditFind->setPlaceholderText(m_search->SearchPlaceholder());
            });

    connect(ui->toolButtonCaseSensitivePC, &QToolButton::toggled, this,
            [this](bool checked)
            {
                m_searchPC->SetMatchCase(checked);
                m_searchPC->Find(ui->lineEditFindPC->text());
                ui->lineEditFindPC->setPlaceholderText(m_searchPC->SearchPlaceholder());
            });

    connect(ui->toolButtonWholeWord, &QToolButton::toggled, this,
            [this](bool checked)
            {
                m_search->SetMatchWord(checked);
                m_search->Find(ui->lineEditFind->text());
                ui->lineEditFind->setPlaceholderText(m_search->SearchPlaceholder());
            });

    connect(ui->toolButtonWholeWordPC, &QToolButton::toggled, this,
            [this](bool checked)
            {
                m_searchPC->SetMatchWord(checked);
                m_searchPC->Find(ui->lineEditFindPC->text());
                ui->lineEditFindPC->setPlaceholderText(m_searchPC->SearchPlaceholder());
            });

    connect(ui->toolButtonRegexp, &QToolButton::toggled, this,
            [this](bool checked)
            {
                m_search->SetMatchRegexp(checked);

                if (checked)
                {
                    ui->toolButtonWholeWord->blockSignals(true);
                    ui->toolButtonWholeWord->setChecked(false);
                    ui->toolButtonWholeWord->blockSignals(false);
                    ui->toolButtonWholeWord->setEnabled(false);

                    ui->toolButtonUseUnicodeProperties->setEnabled(true);
                }
                else
                {
                    ui->toolButtonWholeWord->setEnabled(true);
                    ui->toolButtonUseUnicodeProperties->blockSignals(true);
                    ui->toolButtonUseUnicodeProperties->setChecked(false);
                    ui->toolButtonUseUnicodeProperties->blockSignals(false);
                    ui->toolButtonUseUnicodeProperties->setEnabled(false);
                }
                m_search->Find(ui->lineEditFind->text());
                ui->lineEditFind->setPlaceholderText(m_search->SearchPlaceholder());
            });

    connect(ui->toolButtonRegexpPC, &QToolButton::toggled, this,
            [this](bool checked)
            {
                m_searchPC->SetMatchRegexp(checked);

                if (checked)
                {
                    ui->toolButtonWholeWordPC->blockSignals(true);
                    ui->toolButtonWholeWordPC->setChecked(false);
                    ui->toolButtonWholeWordPC->blockSignals(false);
                    ui->toolButtonWholeWordPC->setEnabled(false);

                    ui->toolButtonUseUnicodePropertiesPC->setEnabled(true);
                }
                else
                {
                    ui->toolButtonWholeWordPC->setEnabled(true);
                    ui->toolButtonUseUnicodePropertiesPC->blockSignals(true);
                    ui->toolButtonUseUnicodePropertiesPC->setChecked(false);
                    ui->toolButtonUseUnicodePropertiesPC->blockSignals(false);
                    ui->toolButtonUseUnicodePropertiesPC->setEnabled(false);
                }
                m_searchPC->Find(ui->lineEditFindPC->text());
                ui->lineEditFindPC->setPlaceholderText(m_searchPC->SearchPlaceholder());
            });

    connect(ui->toolButtonUseUnicodeProperties, &QToolButton::toggled, this,
            [this](bool checked)
            {
                m_search->SetUseUnicodePreperties(checked);
                m_search->Find(ui->lineEditFind->text());
            });

    connect(ui->toolButtonUseUnicodePropertiesPC, &QToolButton::toggled, this,
            [this](bool checked)
            {
                m_searchPC->SetUseUnicodePreperties(checked);
                m_searchPC->Find(ui->lineEditFindPC->text());
            });

    m_searchHistory->setStyleSheet(QStringLiteral("QMenu { menu-scrollable: 1; }"));
    m_searchHistoryPC->setStyleSheet(QStringLiteral("QMenu { menu-scrollable: 1; }"));

    InitIncrementsSearchHistory();
    InitPreviewCalculationsSearchHistory();

    ui->pushButtonSearch->setMenu(m_searchHistory);
    ui->pushButtonSearchPC->setMenu(m_searchHistoryPC);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::InitIncrementsSearchHistory()
{
    QStringList searchHistory = VAbstractValApplication::VApp()->ValentinaSettings()->GetIncrementsSearchHistory();
    m_searchHistory->clear();
    for (const auto &term : searchHistory)
    {
        QAction *action = m_searchHistory->addAction(term);
        action->setData(term);
        connect(action, &QAction::triggered, this,
                [this]()
                {
                    auto *action = qobject_cast<QAction *>(sender());
                    if (action != nullptr)
                    {
                        QString term = action->data().toString();
                        ui->lineEditFind->setText(term);
                        m_search->Find(term);
                        ui->lineEditFind->setFocus();
                    }
                });
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::InitPreviewCalculationsSearchHistory()
{
    QStringList searchHistory =
        VAbstractValApplication::VApp()->ValentinaSettings()->GetPreviewCalculationsSearchHistory();
    m_searchHistoryPC->clear();
    for (const auto &term : searchHistory)
    {
        QAction *action = m_searchHistoryPC->addAction(term);
        action->setData(term);
        connect(action, &QAction::triggered, this,
                [this]()
                {
                    auto *action = qobject_cast<QAction *>(sender());
                    if (action != nullptr)
                    {
                        QString term = action->data().toString();
                        ui->lineEditFindPC->setText(term);
                        m_searchPC->Find(term);
                        ui->lineEditFindPC->setFocus();
                    }
                });
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::SaveIncrementsSearchRequest()
{
    QStringList searchHistory = VAbstractValApplication::VApp()->ValentinaSettings()->GetIncrementsSearchHistory();
    QString term = ui->lineEditFind->text();
    if (term.isEmpty())
    {
        return;
    }

    searchHistory.removeAll(term);
    searchHistory.prepend(term);
    while (searchHistory.size() > VTableSearch::MaxHistoryRecords)
    {
        searchHistory.removeLast();
    }
    VAbstractValApplication::VApp()->ValentinaSettings()->SetIncrementsSearchHistory(searchHistory);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::SavePreviewCalculationsSearchRequest()
{
    QStringList searchHistory =
        VAbstractValApplication::VApp()->ValentinaSettings()->GetPreviewCalculationsSearchHistory();
    QString term = ui->lineEditFindPC->text();
    if (term.isEmpty())
    {
        return;
    }

    searchHistory.removeAll(term);
    searchHistory.prepend(term);
    while (searchHistory.size() > VTableSearch::MaxHistoryRecords)
    {
        searchHistory.removeLast();
    }
    VAbstractValApplication::VApp()->ValentinaSettings()->SetPreviewCalculationsSearchHistory(searchHistory);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::UpdateSearchControlsTooltips()
{
    auto UpdateToolTip = [](QAbstractButton *button)
    {
        if (button->toolTip().contains("%1"_L1))
        {
            button->setToolTip(button->toolTip().arg(button->shortcut().toString(QKeySequence::NativeText)));
        }
    };

    UpdateToolTip(ui->toolButtonCaseSensitive);
    UpdateToolTip(ui->toolButtonWholeWord);
    UpdateToolTip(ui->toolButtonRegexp);
    UpdateToolTip(ui->toolButtonUseUnicodeProperties);
    UpdateToolTip(ui->pushButtonSearch);
    UpdateToolTip(ui->toolButtonFindPrevious);
    UpdateToolTip(ui->toolButtonFindNext);

    UpdateToolTip(ui->toolButtonCaseSensitivePC);
    UpdateToolTip(ui->toolButtonWholeWordPC);
    UpdateToolTip(ui->toolButtonRegexpPC);
    UpdateToolTip(ui->toolButtonUseUnicodePropertiesPC);
    UpdateToolTip(ui->pushButtonSearchPC);
    UpdateToolTip(ui->toolButtonFindPreviousPC);
    UpdateToolTip(ui->toolButtonFindNextPC);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::InitIncrementUnits(QComboBox *combo)
{
    SCASSERT(combo != nullptr)

    combo->blockSignals(true);

    int current = -1;
    if (combo->currentIndex() != -1)
    {
        current = combo->currentData().toInt();
    }

    QString units;
    switch (VAbstractValApplication::VApp()->patternUnits())
    {
        case Unit::Mm:
            units = tr("Millimeters");
            break;
        case Unit::Inch:
            units = tr("Inches");
            break;
        case Unit::Cm:
            units = tr("Centimeters");
            break;
        default:
            units = QStringLiteral("<Invalid>");
            break;
    }

    combo->clear();
    combo->addItem(units, QVariant(static_cast<int>(IncrUnits::Pattern)));
    combo->addItem(tr("Degrees"), QVariant(static_cast<int>(IncrUnits::Degrees)));

    int i = combo->findData(current);
    if (i != -1)
    {
        combo->setCurrentIndex(i);
    }

    combo->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::InitIcons()
{
    const QString resource = QStringLiteral("icon");
    const QString fxIcon = QStringLiteral("24x24/fx.png");

    ui->toolButtonExpr->setIcon(VTheme::GetIconResource(resource, fxIcon));
    ui->toolButtonExprPC->setIcon(VTheme::GetIconResource(resource, fxIcon));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FullUpdateFromFile update information in tables form file
 */
void DialogIncrements::FullUpdateFromFile()
{
    m_hasChanges = false;

    const int incrementRow = ui->tableWidgetIncrement->currentRow();
    const int pcRow = ui->tableWidgetPC->currentRow();

    ui->tableWidgetLines->clearContents();
    ui->tableWidgetSplines->clearContents();
    ui->tableWidgetAnglesCurves->clearContents();
    ui->tableWidgetLinesAngles->clearContents();
    ui->tableWidgetRadiusesArcs->clearContents();

    m_completeData = m_doc->GetCompleteData();

    FillIncrements();
    FillPreviewCalculations();
    FillLengthsLines();
    FillLengthLinesAngles();
    FillLengthsCurves();
    FillCurvesCLengths();
    FillRadiusesArcs();
    FillAnglesCurves();

    m_search->RefreshList(ui->lineEditFind->text());
    m_searchPC->RefreshList(ui->lineEditFindPC->text());

    ui->tableWidgetIncrement->selectRow(incrementRow);
    ui->tableWidgetPC->selectRow(pcRow);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::RefreshPattern()
{
    if (m_hasChanges)
    {
        QVector<VFormulaField> expressions = m_doc->ListExpressions();
        for (auto &item : m_renameList)
        {
            m_doc->ReplaceNameInFormula(expressions, item.first, item.second);
        }
        m_renameList.clear();

        const int row = ui->tableWidgetIncrement->currentRow();
        const int rowPC = ui->tableWidgetPC->currentRow();

        m_doc->LiteParseTree(Document::FullLiteParse);

        ui->tableWidgetIncrement->blockSignals(true);
        ui->tableWidgetIncrement->selectRow(row);
        ui->tableWidgetIncrement->blockSignals(false);

        ui->tableWidgetPC->blockSignals(true);
        ui->tableWidgetPC->selectRow(rowPC);
        ui->tableWidgetPC->blockSignals(false);

        m_hasChanges = false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::FillIncrementsTable(QTableWidget *table,
                                           const QMap<QString, QSharedPointer<VIncrement>> &increments,
                                           bool takePreviewCalculations)
{
    SCASSERT(table != nullptr)

    table->blockSignals(true);
    table->clearContents();

    QMap<QString, QSharedPointer<VIncrement>>::const_iterator i;
    QMap<quint32, QString> map;
    // Sorting QHash by id
    for (i = increments.constBegin(); i != increments.constEnd(); ++i)
    {
        const QSharedPointer<VIncrement> &incr = i.value();
        if (takePreviewCalculations == incr->IsPreviewCalculation())
        {
            map.insert(incr->GetIndex(), i.key());
        }
    }

    qint32 currentRow = -1;
    QMapIterator<quint32, QString> iMap(map);
    table->setRowCount(static_cast<int>(map.size()));
    while (iMap.hasNext())
    {
        iMap.next();
        const QSharedPointer<VIncrement> &incr = increments.value(iMap.value());
        ++currentRow;

        if (incr->GetType() == VarType::Increment)
        {
            AddCell(table, incr->GetName(), currentRow, 0, Qt::AlignVCenter); // name

            QString calculatedValue = VAbstractApplication::VApp()->LocaleToString(*incr->GetValue());
            if (incr->IsSpecialUnits())
            {
                calculatedValue = calculatedValue + degreeSymbol;
            }
            AddCell(table, calculatedValue, currentRow, 1, Qt::AlignCenter, incr->IsFormulaOk()); // calculated value

            QString formula = VTranslateVars::TryFormulaToUser(
                incr->GetFormula(), VAbstractApplication::VApp()->Settings()->GetOsSeparator());

            AddCell(table, formula, currentRow, 2, Qt::AlignVCenter); // formula

            if (table->columnSpan(currentRow, 1) > 1)
            {
                table->setSpan(currentRow, 1, 1, 1);
            }
        }
        else if (incr->GetType() == VarType::IncrementSeparator)
        {
            AddSeparatorCell(table, incr->GetName(), currentRow, 0, Qt::AlignVCenter); // name
            AddCell(table, incr->GetDescription(), currentRow, 1, Qt::AlignCenter);    // description
            table->setSpan(currentRow, 1, 1, 2);
        }
    }

    table->horizontalHeader()->setStretchLastSection(true);
    table->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief clickedToolButtonAdd create new row in table
 */
void DialogIncrements::AddIncrement()
{
    AddNewIncrement(IncrementType::Increment);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::AddSeparator()
{
    AddNewIncrement(IncrementType::Separator);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief clickedToolButtonRemove remove one row from table
 */
void DialogIncrements::RemoveIncrement()
{
    auto *button = qobject_cast<QToolButton *>(sender());

    QTableWidget *table = nullptr;

    if (button == ui->toolButtonRemove)
    {
        table = ui->tableWidgetIncrement;
    }
    else if (button == ui->toolButtonRemovePC)
    {
        table = ui->tableWidgetPC;
    }
    else
    {
        return;
    }

    const int row = table->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = table->item(row, 0);

    if (button == ui->toolButtonRemove)
    {
        m_doc->RemoveIncrement(nameField->text());
    }
    else if (button == ui->toolButtonRemovePC)
    {
        m_doc->RemovePreviewCalculation(nameField->text());
    }

    m_hasChanges = true;
    LocalUpdateTree();

    table->rowCount() > 0 ? table->selectRow(0) : EnableDetails(table, false);
    table->repaint(); // Force repain to fix paint artifacts on Mac OS X
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::MoveUp()
{
    auto *button = qobject_cast<QToolButton *>(sender());

    QTableWidget *table = nullptr;

    if (button == ui->toolButtonUp)
    {
        table = ui->tableWidgetIncrement;
    }
    else if (button == ui->toolButtonUpPC)
    {
        table = ui->tableWidgetPC;
    }
    else
    {
        return;
    }

    const int row = table->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = table->item(row, 0);

    if (button == ui->toolButtonUp)
    {
        m_doc->MoveUpIncrement(nameField->text());
    }
    else if (button == ui->toolButtonUpPC)
    {
        m_doc->MoveUpPreviewCalculation(nameField->text());
    }

    m_hasChanges = true;
    LocalUpdateTree();

    table->selectRow(row - 1);
    table->repaint(); // Force repain to fix paint artifacts on Mac OS X
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::MoveDown()
{
    auto *button = qobject_cast<QToolButton *>(sender());

    QTableWidget *table = nullptr;

    if (button == ui->toolButtonDown)
    {
        table = ui->tableWidgetIncrement;
    }
    else if (button == ui->toolButtonDownPC)
    {
        table = ui->tableWidgetPC;
    }
    else
    {
        return;
    }

    const int row = table->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = table->item(row, 0);

    if (button == ui->toolButtonDown)
    {
        m_doc->MoveDownIncrement(nameField->text());
    }
    else if (button == ui->toolButtonDownPC)
    {
        m_doc->MoveDownPreviewCalculation(nameField->text());
    }

    m_hasChanges = true;
    LocalUpdateTree();

    table->selectRow(row + 1);
    table->repaint(); // Force repain to fix paint artifacts on Mac OS X
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::SaveIncrName(const QString &text)
{
    auto *lineEdit = qobject_cast<QLineEdit *>(sender());

    QTableWidget *table = nullptr;

    if (lineEdit == ui->lineEditName)
    {
        table = ui->tableWidgetIncrement;
    }
    else if (lineEdit == ui->lineEditNamePC)
    {
        table = ui->tableWidgetPC;
    }
    else
    {
        return;
    }

    const int row = table->currentRow();

    if (row == -1)
    {
        return;
    }

    QTableWidgetItem *nameField = table->item(row, 0);

    QString newName = text.isEmpty() ? GetCustomName() : CustomIncrSign + text;
    bool updateFlag = not text.isEmpty();

    if (not m_data->IsUnique(newName))
    {
        updateFlag = false;
        qint32 num = 2;
        QString name = newName;
        do
        {
            name = name + '_' + QString::number(num);
            num++;
        } while (not m_data->IsUnique(name));
        newName = name;
    }

    m_doc->SetIncrementName(nameField->text(), newName);
    QVector<VFormulaField> expression = m_doc->ListIncrementExpressions();
    m_doc->ReplaceNameInFormula(expression, nameField->text(), newName);

    CacheRename(nameField->text(), newName);

    m_hasChanges = true;
    if (updateFlag)
    {
        LocalUpdateTree();
    }
    else
    {
        nameField->setText(newName);
        m_doc->LiteParseIncrements();
    }

    table->blockSignals(true);
    table->selectRow(row);
    table->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::SaveIncrUnits()
{
    auto *combo = qobject_cast<QComboBox *>(sender());

    QTableWidget *table = nullptr;

    if (combo == ui->comboBoxIncrementUnits)
    {
        table = ui->tableWidgetIncrement;
    }
    else if (combo == ui->comboBoxPreviewCalculationUnits)
    {
        table = ui->tableWidgetPC;
    }
    else
    {
        return;
    }

    const int row = table->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = table->item(row, 0);

    const IncrUnits units = static_cast<IncrUnits>(combo->currentData().toInt());
    m_doc->SetIncrementSpecialUnits(nameField->text(), units == IncrUnits::Degrees);
    LocalUpdateTree();

    table->blockSignals(true);
    table->selectRow(row);
    table->blockSignals(false);

    ShowTableIncrementDetails(table);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::SaveIncrDescription()
{
    auto *textEdit = qobject_cast<QPlainTextEdit *>(sender());

    QTableWidget *table = nullptr;

    if (textEdit == ui->plainTextEditDescription)
    {
        table = ui->tableWidgetIncrement;
    }
    else if (textEdit == ui->plainTextEditDescriptionPC)
    {
        table = ui->tableWidgetPC;
    }
    else
    {
        return;
    }

    const int row = table->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTextCursor cursor = textEdit->textCursor();

    const QTableWidgetItem *nameField = table->item(row, 0);
    m_doc->SetIncrementDescription(nameField->text(), textEdit->toPlainText());
    LocalUpdateTree();

    table->blockSignals(true);
    table->selectRow(row);
    table->blockSignals(false);
    textEdit->setTextCursor(cursor);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::SaveIncrFormula()
{
    auto *textEdit = qobject_cast<QPlainTextEdit *>(sender());

    QTableWidget *table = nullptr;
    QLabel *labelCalculatedValue = nullptr;

    if (textEdit == ui->plainTextEditFormula)
    {
        table = ui->tableWidgetIncrement;
        labelCalculatedValue = ui->labelCalculatedValue;
    }
    else if (textEdit == ui->plainTextEditFormulaPC)
    {
        table = ui->tableWidgetPC;
        labelCalculatedValue = ui->labelCalculatedValuePC;
    }
    else
    {
        return;
    }

    const int row = table->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = table->item(row, 0);

    QString text = textEdit->toPlainText();
    QSharedPointer<VIncrement> incr = m_data->GetVariable<VIncrement>(nameField->text());

    QTableWidgetItem *formulaField = table->item(row, 2);
    if (formulaField->text() == text)
    {
        QTableWidgetItem *result = table->item(row, 1);
        if (incr->IsSpecialUnits())
        {
            labelCalculatedValue->setText(result->text() + QChar(QChar::Space) + degreeSymbol);
        }
        else
        {
            // Show unit in dialog lable (cm, mm or inch)
            const QString postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits());
            labelCalculatedValue->setText(result->text() + QChar(QChar::Space) + postfix);
        }
        return;
    }

    if (text.isEmpty())
    {
        if (incr->IsSpecialUnits())
        {
            labelCalculatedValue->setText(tr("Error") + " (" + degreeSymbol + "). " + tr("Empty field."));
        }
        else
        {
            // Show unit in dialog lable (cm, mm or inch)
            const QString postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits());
            labelCalculatedValue->setText(tr("Error") + " (" + postfix + "). " + tr("Empty field."));
        }
        return;
    }

    if (incr->GetIncrementType() == IncrementType::Separator)
    {
        return;
    }

    if (not EvalIncrementFormula(text, true, incr->GetData(), labelCalculatedValue, incr->IsSpecialUnits()))
    {
        return;
    }

    try
    {
        const QString formula = VAbstractApplication::VApp()->TrVars()->FormulaFromUser(
            text, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
        m_doc->SetIncrementFormula(nameField->text(), formula);
    }
    catch (qmu::QmuParserError &e) // Just in case something bad will happen
    {
        Q_UNUSED(e)
        return;
    }

    const QTextCursor cursor = textEdit->textCursor();

    m_hasChanges = true;
    LocalUpdateTree();

    table->blockSignals(true);
    table->selectRow(row);
    table->blockSignals(false);
    textEdit->setTextCursor(cursor);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::DeployFormula()
{
    SCASSERT(ui->plainTextEditFormula != nullptr)
    SCASSERT(ui->pushButtonGrow != nullptr)
    SCASSERT(ui->plainTextEditFormulaPC != nullptr)
    SCASSERT(ui->pushButtonGrowPC != nullptr)

    auto *button = qobject_cast<QPushButton *>(sender());

    QPlainTextEdit *plainTextEditFormula = nullptr;
    QPushButton *pushButtonGrow = nullptr;
    int baseHeight = 0;

    if (button == ui->pushButtonGrow)
    {
        plainTextEditFormula = ui->plainTextEditFormula;
        pushButtonGrow = ui->pushButtonGrow;
        baseHeight = m_formulaBaseHeight;
    }
    else if (button == ui->pushButtonGrowPC)
    {
        plainTextEditFormula = ui->plainTextEditFormulaPC;
        pushButtonGrow = ui->pushButtonGrowPC;
        baseHeight = m_formulaBaseHeightPC;
    }
    else
    {
        return;
    }

    const QTextCursor cursor = plainTextEditFormula->textCursor();

    if (plainTextEditFormula->height() < DIALOG_MAX_FORMULA_HEIGHT)
    {
        plainTextEditFormula->setFixedHeight(DIALOG_MAX_FORMULA_HEIGHT);
        // Set icon from theme (internal for Windows system)
        pushButtonGrow->setIcon(QIcon::fromTheme(QStringLiteral("go-next")));
    }
    else
    {
        plainTextEditFormula->setFixedHeight(baseHeight);
        // Set icon from theme (internal for Windows system)
        pushButtonGrow->setIcon(QIcon::fromTheme(QStringLiteral("go-down")));
    }

    // I found that after change size of formula field, it was filed for angle formula, field for formula became black.
    // This code prevent this.
    setUpdatesEnabled(false);
    repaint();
    setUpdatesEnabled(true);

    plainTextEditFormula->setFocus();
    plainTextEditFormula->setTextCursor(cursor);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::Fx()
{
    auto *button = qobject_cast<QToolButton *>(sender());

    QTableWidget *table = nullptr;
    QPlainTextEdit *plainTextEditFormula = nullptr;

    bool incrementMode = true;
    if (button == ui->toolButtonExpr)
    {
        incrementMode = true;
        table = ui->tableWidgetIncrement;
        plainTextEditFormula = ui->plainTextEditFormula;
    }
    else if (button == ui->toolButtonExprPC)
    {
        incrementMode = false;
        table = ui->tableWidgetPC;
        plainTextEditFormula = ui->plainTextEditFormulaPC;
    }
    else
    {
        return;
    }

    const int row = table->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = table->item(row, 0);
    QSharedPointer<VIncrement> incr = m_data->GetVariable<VIncrement>(nameField->text());

    QScopedPointer<DialogEditWrongFormula> dialog(new DialogEditWrongFormula(incr->GetData(), NULL_ID, this));
    dialog->setWindowTitle(tr("Edit increment"));
    incrementMode ? dialog->SetIncrementsMode() : dialog->SetPreviewCalculationsMode();

    dialog->SetFormula(VTranslateVars::TryFormulaFromUser(plainTextEditFormula->toPlainText(),
                                                          VAbstractApplication::VApp()->Settings()->GetOsSeparator()));
    const QString postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
    dialog->setPostfix(postfix); // Show unit in dialog lable (cm, mm or inch)

    if (dialog->exec() == QDialog::Accepted)
    {
        // Fix the bug #492. https://bitbucket.org/dismine/valentina/issues/492/valentina-crashes-when-add-an-increment
        // Because of the bug need to take QTableWidgetItem twice time. Previous update "killed" the pointer.
        m_doc->SetIncrementFormula(table->item(row, 0)->text(), dialog->GetFormula());

        m_hasChanges = true;
        LocalUpdateTree();

        table->selectRow(row);
        ShowTableIncrementDetails(table);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::closeEvent(QCloseEvent *event)
{
    RefreshPattern();

    ui->plainTextEditFormula->blockSignals(true);
    ui->lineEditName->blockSignals(true);
    ui->plainTextEditDescription->blockSignals(true);

    disconnect(this->m_doc, &VPattern::FullUpdateFromFile, this, &DialogIncrements::FullUpdateFromFile);

    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();

    settings->SetIncrementsSearchOptionMatchCase(m_search->IsMatchCase());
    settings->SetIncrementsSearchOptionWholeWord(m_search->IsMatchWord());
    settings->SetIncrementsSearchOptionRegexp(m_search->IsMatchRegexp());
    settings->SetIncrementsSearchOptionUseUnicodeProperties(m_search->IsUseUnicodePreperties());

    settings->SetPreviewCalculationsSearchOptionMatchCase(m_searchPC->IsMatchCase());
    settings->SetPreviewCalculationsSearchOptionWholeWord(m_searchPC->IsMatchWord());
    settings->SetPreviewCalculationsSearchOptionRegexp(m_searchPC->IsMatchRegexp());
    settings->SetPreviewCalculationsSearchOptionUseUnicodeProperties(m_searchPC->IsUseUnicodePreperties());

    emit UpdateProperties();
    emit DialogClosed(QDialog::Accepted);
    event->accept();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);

        ui->toolButtonAdd->setMenu(InitVarTypeMenu(ui->toolButtonAdd->menu(), true /*increments tab*/));
        ui->toolButtonAddPC->setMenu(InitVarTypeMenu(ui->toolButtonAddPC->menu(), false /*preview calculations tab*/));

        ui->lineEditFind->setPlaceholderText(m_search->SearchPlaceholder());
        ui->lineEditFindPC->setPlaceholderText(m_searchPC->SearchPlaceholder());
        UpdateSearchControlsTooltips();

        FullUpdateFromFile();
    }

    if (event->type() == QEvent::PaletteChange)
    {
        InitIcons();
    }

    // remember to call base class implementation
    QWidget::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogIncrements::eventFilter(QObject *object, QEvent *event) -> bool
{
    if (auto *textEdit = qobject_cast<QLineEdit *>(object))
    {
        if (event->type() == QEvent::KeyPress)
        {
            auto *keyEvent = static_cast<QKeyEvent *>(event);
            if ((keyEvent->key() == Qt::Key_Period) && (keyEvent->modifiers() & Qt::KeypadModifier))
            {
                if (VAbstractApplication::VApp()->Settings()->GetOsSeparator())
                {
                    textEdit->insert(LocaleDecimalPoint(QLocale()));
                }
                else
                {
                    textEdit->insert(LocaleDecimalPoint(QLocale::c()));
                }
                return true;
            }
        }
    }

    // pass the event on to the parent class
    return DialogTool::eventFilter(object, event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::showEvent(QShowEvent *event)
{
    // Skip DialogTool implementation
    QDialog::showEvent(event); // NOLINT(bugprone-parent-virtual-call)
    if (event->spontaneous())
    {
        return;
    }

    if (isInitialized)
    {
        return;
    }
    // do your init stuff here

    const QSize sz = VAbstractApplication::VApp()->Settings()->GetIncrementsDialogSize();
    if (not sz.isEmpty())
    {
        resize(sz);
    }

    isInitialized = true; // first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::resizeEvent(QResizeEvent *event)
{
    // remember the size for the next time this dialog is opened, but only
    // if widget was already initialized, which rules out the resize at
    // dialog creating, which would
    if (isInitialized)
    {
        VAbstractApplication::VApp()->Settings()->SetIncrementsDialogSize(size());
    }
    DialogTool::resizeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::ShowIncrementDetails()
{
    ShowTableIncrementDetails(qobject_cast<QTableWidget *>(sender()));
}

//---------------------------------------------------------------------------------------------------------------------
DialogIncrements::~DialogIncrements()
{
    ui->lineEditFind->blockSignals(true);   // prevents crash
    ui->lineEditFindPC->blockSignals(true); // prevents crash
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrements::RestoreAfterClose()
{
    // Because of bug on Mac OS with Qt 5.11 closing this dialog causes a crash. Instead of closing we will keep
    // dialog in memory and reuse it again. This function redo some moves made after close.

    connect(this->m_doc, &VPattern::FullUpdateFromFile, this, &DialogIncrements::FullUpdateFromFile);

    ui->tabWidget->setCurrentIndex(0);

    if (ui->tableWidgetIncrement->rowCount() > 0)
    {
        ui->tableWidgetIncrement->selectRow(0);
    }

    if (ui->tableWidgetPC->rowCount() > 0)
    {
        ui->tableWidgetPC->selectRow(0);
    }
}
