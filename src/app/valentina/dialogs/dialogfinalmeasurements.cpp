/************************************************************************
 **
 **  @file   dialogfinalmeasurements.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 9, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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

#include "dialogfinalmeasurements.h"
#include "../qmuparser/qmudef.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vmisc/vvalentinasettings.h"
#include "../vpatterndb/calculator.h"
#include "../vtools/dialogs/dialogtoolbox.h"
#include "../vtools/dialogs/support/dialogeditwrongformula.h"
#include "ui_dialogfinalmeasurements.h"

#include <QMenu>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

constexpr int DIALOG_MAX_FORMULA_HEIGHT = 64;

//---------------------------------------------------------------------------------------------------------------------
DialogFinalMeasurements::DialogFinalMeasurements(VPattern *doc, QWidget *parent)
  : QDialog(parent),
    ui(new Ui::DialogFinalMeasurements),
    m_doc(doc),
    m_data(doc->GetCompleteData()),
    m_measurements(doc->GetFinalMeasurements()),
    m_searchHistory(new QMenu(this))
{
    ui->setupUi(this);

#if defined(Q_OS_MAC)
    setWindowFlags(Qt::Window);
#endif

    InitIcons();

    m_data.FillPiecesAreas(VAbstractValApplication::VApp()->patternUnits());

    ui->lineEditName->setClearButtonEnabled(true);

    ui->lineEditFind->installEventFilter(this);

    InitSearch();

    formulaBaseHeight = ui->plainTextEditFormula->height();
    ui->plainTextEditFormula->installEventFilter(this);

    VAbstractApplication::VApp()->Settings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    ShowUnits();

    const bool freshCall = true;
    FillFinalMeasurements(freshCall);

    connect(m_doc, &VPattern::FullUpdateFromFile, this, &DialogFinalMeasurements::FullUpdateFromFile);

    ui->lineEditName->setValidator(
        new QRegularExpressionValidator(QRegularExpression(QStringLiteral("^$|") + NameRegExp()), this));

    connect(ui->tableWidget, &QTableWidget::itemSelectionChanged, this,
            &DialogFinalMeasurements::ShowFinalMeasurementDetails);

    connect(ui->toolButtonAdd, &QToolButton::clicked, this, &DialogFinalMeasurements::Add);
    connect(ui->toolButtonRemove, &QToolButton::clicked, this, &DialogFinalMeasurements::Remove);
    connect(ui->toolButtonUp, &QToolButton::clicked, this, &DialogFinalMeasurements::MoveUp);
    connect(ui->toolButtonDown, &QToolButton::clicked, this, &DialogFinalMeasurements::MoveDown);
    connect(ui->pushButtonGrow, &QPushButton::clicked, this, &DialogFinalMeasurements::DeployFormula);
    connect(ui->toolButtonExpr, &QToolButton::clicked, this, &DialogFinalMeasurements::Fx);
    connect(ui->lineEditName, &QLineEdit::textEdited, this, &DialogFinalMeasurements::SaveName);
    connect(ui->plainTextEditDescription, &QPlainTextEdit::textChanged, this,
            &DialogFinalMeasurements::SaveDescription);
    connect(ui->plainTextEditFormula, &QPlainTextEdit::textChanged, this, &DialogFinalMeasurements::SaveFormula);

    if (ui->tableWidget->rowCount() > 0)
    {
        ui->tableWidget->selectRow(0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
DialogFinalMeasurements::~DialogFinalMeasurements()
{
    ui->lineEditFind->blockSignals(true); // prevents crash
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFinalMeasurements::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditFormula->blockSignals(true);
    ui->lineEditName->blockSignals(true);
    ui->plainTextEditDescription->blockSignals(true);

    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
    settings->SetFinalMeasurementsSearchOptionMatchCase(m_search->IsMatchCase());
    settings->SetFinalMeasurementsSearchOptionWholeWord(m_search->IsMatchWord());
    settings->SetFinalMeasurementsSearchOptionRegexp(m_search->IsMatchRegexp());
    settings->SetFinalMeasurementsSearchOptionUseUnicodeProperties(m_search->IsUseUnicodePreperties());

    QDialog::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFinalMeasurements::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
        ui->lineEditFind->setPlaceholderText(m_search->SearchPlaceholder());
        UpdateSearchControlsTooltips();
        FullUpdateFromFile();
    }

    if (event->type() == QEvent::PaletteChange)
    {
        InitIcons();
        InitDialogButtonBoxIcons(ui->buttonBox);
    }

    // remember to call base class implementation
    QDialog::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFinalMeasurements::eventFilter(QObject *object, QEvent *event) -> bool
{
    if (auto *textEdit = qobject_cast<QLineEdit *>(object))
    {
        if (event->type() == QEvent::KeyPress)
        {
            auto *keyEvent = static_cast<QKeyEvent *>(event); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
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
    else
    {
        // pass the event on to the parent class
        return QDialog::eventFilter(object, event);
    }
    return false; // pass the event to the widget // clazy:exclude=base-class-event
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFinalMeasurements::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    if (event->spontaneous())
    {
        return;
    }

    if (m_isInitialized)
    {
        return;
    }
    // do your init stuff here

    const QSize sz = VAbstractApplication::VApp()->Settings()->GetFinalMeasurementsDialogSize();
    if (not sz.isEmpty())
    {
        resize(sz);
    }

    m_isInitialized = true; // first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFinalMeasurements::resizeEvent(QResizeEvent *event)
{
    // remember the size for the next time this dialog is opened, but only
    // if widget was already initialized, which rules out the resize at
    // dialog creating, which would
    if (m_isInitialized)
    {
        VAbstractApplication::VApp()->Settings()->SetFinalMeasurementsDialogSize(size());
    }
    QDialog::resizeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFinalMeasurements::ShowFinalMeasurementDetails()
{
    if (ui->tableWidget->rowCount() > 0 && m_measurements.size() == ui->tableWidget->rowCount())
    {
        EnableDetails(true);

        const VFinalMeasurement &m = m_measurements.at(ui->tableWidget->currentRow());

        ui->lineEditName->blockSignals(true);
        ui->lineEditName->setText(m.name);
        ui->lineEditName->blockSignals(false);

        ui->plainTextEditDescription->blockSignals(true);
        ui->plainTextEditDescription->setPlainText(m.description);
        ui->plainTextEditDescription->blockSignals(false);

        EvalUserFormula(m.formula, false);
        ui->plainTextEditFormula->blockSignals(true);

        const QString formula =
            VTranslateVars::TryFormulaToUser(m.formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());

        ui->plainTextEditFormula->setPlainText(formula);
        ui->plainTextEditFormula->blockSignals(false);
    }
    else
    {
        EnableDetails(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFinalMeasurements::Add()
{
    const int currentRow = ui->tableWidget->currentRow() + 1;

    VFinalMeasurement m;
    m.name = tr("measurement");
    m.formula = '0'_L1;

    m_measurements.append(m);

    UpdateTree();
    ui->tableWidget->selectRow(currentRow);
    ui->tableWidget->repaint(); // Force repain to fix paint artifacts on Mac OS X
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFinalMeasurements::Remove()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1 || row >= m_measurements.size())
    {
        return;
    }

    m_measurements.remove(row);

    UpdateTree();

    if (ui->tableWidget->rowCount() > 0)
    {
        ui->tableWidget->selectRow(0);
    }
    else
    {
        EnableDetails(false);
    }
    ui->tableWidget->repaint(); // Force repain to fix paint artifacts on Mac OS X
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFinalMeasurements::MoveUp()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1 || row == 0 || row >= m_measurements.size())
    {
        return;
    }

    m_measurements.move(row, row - 1);
    UpdateTree();

    ui->tableWidget->selectRow(row - 1);
    ui->tableWidget->repaint(); // Force repain to fix paint artifacts on Mac OS X
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFinalMeasurements::MoveDown()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1 || row == ui->tableWidget->rowCount() - 1 || row >= m_measurements.size())
    {
        return;
    }

    m_measurements.move(row, row + 1);
    UpdateTree();

    ui->tableWidget->selectRow(row + 1);
    ui->tableWidget->repaint(); // Force repain to fix paint artifacts on Mac OS X
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFinalMeasurements::SaveName(const QString &text)
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1 || row >= m_measurements.size())
    {
        return;
    }

    m_measurements[row].name = text.isEmpty() ? tr("measurement") : text;

    UpdateTree();

    ui->tableWidget->blockSignals(true);
    ui->tableWidget->selectRow(row);
    ui->tableWidget->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFinalMeasurements::SaveDescription()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1 || row >= m_measurements.size())
    {
        return;
    }

    const QTextCursor cursor = ui->plainTextEditDescription->textCursor();

    m_measurements[row].description = ui->plainTextEditDescription->toPlainText();

    UpdateTree();

    ui->tableWidget->blockSignals(true);
    ui->tableWidget->selectRow(row);
    ui->tableWidget->blockSignals(false);
    ui->plainTextEditDescription->setTextCursor(cursor);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFinalMeasurements::SaveFormula()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1 || row >= m_measurements.size())
    {
        return;
    }

    // Replace line return character with spaces for calc if exist
    QString text = ui->plainTextEditFormula->toPlainText();

    QTableWidgetItem *formulaField = ui->tableWidget->item(row, 2);
    if (formulaField->text() == text)
    {
        QTableWidgetItem *result = ui->tableWidget->item(row, 1);
        // Show unit in dialog lable (cm, mm or inch)
        const QString postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits());
        ui->labelCalculatedValue->setText(result->text() + QChar(QChar::Space) + postfix);
        return;
    }

    if (text.isEmpty())
    {
        // Show unit in dialog lable (cm, mm or inch)
        const QString postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits());
        ui->labelCalculatedValue->setText(tr("Error") + " (" + postfix + "). " + tr("Empty field."));
        return;
    }

    if (not EvalUserFormula(text, true))
    {
        return;
    }

    try
    {
        m_measurements[row].formula = VAbstractApplication::VApp()->TrVars()->FormulaFromUser(
            text, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    }
    catch (qmu::QmuParserError &e) // Just in case something bad will happen
    {
        Q_UNUSED(e)
        return;
    }

    const QTextCursor cursor = ui->plainTextEditFormula->textCursor();

    UpdateTree();

    ui->tableWidget->blockSignals(true);
    ui->tableWidget->selectRow(row);
    ui->tableWidget->blockSignals(false);
    ui->plainTextEditFormula->setTextCursor(cursor);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFinalMeasurements::DeployFormula()
{
    const QTextCursor cursor = ui->plainTextEditFormula->textCursor();

    if (ui->plainTextEditFormula->height() < DIALOG_MAX_FORMULA_HEIGHT)
    {
        ui->plainTextEditFormula->setFixedHeight(DIALOG_MAX_FORMULA_HEIGHT);
        // Set icon from theme (internal for Windows system)
        ui->pushButtonGrow->setIcon(QIcon::fromTheme(QStringLiteral("go-next")));
    }
    else
    {
        ui->plainTextEditFormula->setFixedHeight(formulaBaseHeight);
        // Set icon from theme (internal for Windows system)
        ui->pushButtonGrow->setIcon(QIcon::fromTheme(QStringLiteral("go-down")));
    }

    // I found that after change size of formula field, it was filed for angle formula, field for formula became black.
    // This code prevent this.
    setUpdatesEnabled(false);
    repaint();
    setUpdatesEnabled(true);

    ui->plainTextEditFormula->setFocus();
    ui->plainTextEditFormula->setTextCursor(cursor);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFinalMeasurements::Fx()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1 || row >= m_measurements.size())
    {
        return;
    }

    QScopedPointer<DialogEditWrongFormula> dialog(new DialogEditWrongFormula(&m_data, NULL_ID, this));
    dialog->setWindowTitle(tr("Edit measurement"));
    dialog->SetFormula(VTranslateVars::TryFormulaFromUser(ui->plainTextEditFormula->toPlainText(),
                                                          VAbstractApplication::VApp()->Settings()->GetOsSeparator()));
    dialog->ShowPieceArea(true);
    const QString postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
    dialog->setPostfix(postfix); // Show unit in dialog lable (cm, mm or inch)

    if (dialog->exec() == QDialog::Accepted)
    {
        m_measurements[row].formula = dialog->GetFormula();
        UpdateTree();

        ui->tableWidget->selectRow(row);
        ShowFinalMeasurementDetails();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFinalMeasurements::FullUpdateFromFile()
{
    m_data = m_doc->GetCompleteData();
    m_data.FillPiecesAreas(VAbstractValApplication::VApp()->patternUnits());
    m_measurements = m_doc->GetFinalMeasurements();

    FillFinalMeasurements();

    m_search->RefreshList(ui->lineEditFind->text());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFinalMeasurements::FillFinalMeasurements(bool freshCall)
{
    ui->tableWidget->blockSignals(true);
    ui->tableWidget->clearContents();

    ui->tableWidget->setRowCount(static_cast<int>(m_measurements.size()));
    for (int i = 0; i < m_measurements.size(); ++i)
    {
        const VFinalMeasurement &m = m_measurements.at(i);

        AddCell(m.name, i, 0, Qt::AlignVCenter); // name

        bool ok = true;
        const qreal result = EvalFormula(m.formula, ok);
        AddCell(VAbstractApplication::VApp()->LocaleToString(result), i, 1, Qt::AlignHCenter | Qt::AlignVCenter,
                ok); // calculated value

        const QString formula =
            VTranslateVars::TryFormulaFromUser(m.formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
        AddCell(formula, i, 2, Qt::AlignVCenter); // formula
    }

    if (freshCall)
    {
        ui->tableWidget->resizeColumnsToContents();
        ui->tableWidget->resizeRowsToContents();
    }
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFinalMeasurements::ShowUnits()
{
    const QString unit = UnitsToStr(VAbstractValApplication::VApp()->patternUnits());

    {
        // calculated value
        const QString header = ui->tableWidget->horizontalHeaderItem(1)->text();
        const QString unitHeader = QStringLiteral("%1 (%2)").arg(header, unit);
        ui->tableWidget->horizontalHeaderItem(1)->setText(unitHeader);
    }

    {
        // formula
        const QString header = ui->tableWidget->horizontalHeaderItem(2)->text();
        const QString unitHeader = QStringLiteral("%1 (%2)").arg(header, unit);
        ui->tableWidget->horizontalHeaderItem(2)->setText(unitHeader);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFinalMeasurements::AddCell(const QString &text, int row, int column, int aligment, bool ok)
{
    auto *item = new QTableWidgetItem(text);
    item->setTextAlignment(aligment);

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

    ui->tableWidget->setItem(row, column, item);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFinalMeasurements::EvalUserFormula(const QString &formula, bool fromUser) -> bool
{
    const QString postfix =
        UnitsToStr(VAbstractValApplication::VApp()->patternUnits()); // Show unit in dialog lable (cm, mm or inch)
    if (formula.isEmpty())
    {
        ui->labelCalculatedValue->setText(tr("Error") + " (" + postfix + "). " + tr("Empty field."));
        ui->labelCalculatedValue->setToolTip(tr("Empty field"));
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
        const qreal result = cal->EvalFormula(m_data.DataVariables(), f);

        if (qIsInf(result) || qIsNaN(result))
        {
            ui->labelCalculatedValue->setText(tr("Error") + " (" + postfix + ").");
            ui->labelCalculatedValue->setToolTip(tr("Invalid result. Value is infinite or NaN. Please, check your "
                                                    "calculations."));
            return false;
        }

        ui->labelCalculatedValue->setText(VAbstractApplication::VApp()->LocaleToString(result) + QChar(QChar::Space) +
                                          postfix);
        ui->labelCalculatedValue->setToolTip(tr("Value"));
        return true;
    }
    catch (qmu::QmuParserError &e)
    {
        ui->labelCalculatedValue->setText(tr("Error") + " (" + postfix + "). " +
                                          tr("Parser error: %1").arg(e.GetMsg()));
        ui->labelCalculatedValue->setToolTip(tr("Parser error: %1").arg(e.GetMsg()));
        return false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFinalMeasurements::Controls()
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
void DialogFinalMeasurements::EnableDetails(bool enabled)
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
        ui->lineEditName->blockSignals(true);
        ui->lineEditName->clear();
        ui->lineEditName->blockSignals(false);

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

    ui->pushButtonGrow->setEnabled(enabled);
    ui->toolButtonExpr->setEnabled(enabled);
    ui->lineEditName->setEnabled(enabled);
    ui->plainTextEditDescription->setEnabled(enabled);
    ui->plainTextEditFormula->setEnabled(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFinalMeasurements::UpdateTree()
{
    int row = ui->tableWidget->currentRow();
    FillFinalMeasurements();
    ui->tableWidget->blockSignals(true);
    ui->tableWidget->selectRow(row);
    ui->tableWidget->blockSignals(false);

    m_search->RefreshList(ui->lineEditFind->text());
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFinalMeasurements::EvalFormula(const QString &formula, bool &ok) -> qreal
{
    qreal result = 0;
    if (formula.isEmpty())
    {
        ok = false;
        return result;
    }

    try
    {
        // Replace line return character with spaces for calc if exist
        QScopedPointer<Calculator> cal(new Calculator());
        result = cal->EvalFormula(m_data.DataVariables(), formula);

        if (qIsInf(result) || qIsNaN(result))
        {
            ok = false;
            return 0;
        }
    }
    catch (qmu::QmuParserError &)
    {
        ok = false;
        return 0;
    }

    ok = true;
    return result;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFinalMeasurements::InitSearch()
{
    m_search = QSharedPointer<VTableSearch>(new VTableSearch(ui->tableWidget));

    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
    m_search->SetUseUnicodePreperties(settings->GetFinalMeasurementsSearchOptionUseUnicodeProperties());
    m_search->SetMatchWord(settings->GetFinalMeasurementsSearchOptionWholeWord());
    m_search->SetMatchRegexp(settings->GetFinalMeasurementsSearchOptionRegexp());
    m_search->SetMatchCase(settings->GetFinalMeasurementsSearchOptionMatchCase());

    ui->lineEditFind->setPlaceholderText(m_search->SearchPlaceholder());

    UpdateSearchControlsTooltips();

    connect(ui->lineEditFind, &QLineEdit::textEdited, this, [this](const QString &term) { m_search->Find(term); });
    connect(ui->lineEditFind, &QLineEdit::editingFinished, this,
            [this]()
            {
                SaveSearchRequest();
                InitSearchHistory();
                m_search->Find(ui->lineEditFind->text());
            });
    connect(ui->toolButtonFindPrevious, &QToolButton::clicked, this,
            [this]()
            {
                SaveSearchRequest();
                InitSearchHistory();
                m_search->FindPrevious();
                ui->labelResults->setText(
                    QStringLiteral("%1/%2").arg(m_search->MatchIndex() + 1).arg(m_search->MatchCount()));
            });
    connect(ui->toolButtonFindNext, &QToolButton::clicked, this,
            [this]()
            {
                SaveSearchRequest();
                InitSearchHistory();
                m_search->FindNext();
                ui->labelResults->setText(
                    QStringLiteral("%1/%2").arg(m_search->MatchIndex() + 1).arg(m_search->MatchCount()));
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

    connect(ui->toolButtonCaseSensitive, &QToolButton::toggled, this,
            [this](bool checked)
            {
                m_search->SetMatchCase(checked);
                m_search->Find(ui->lineEditFind->text());
                ui->lineEditFind->setPlaceholderText(m_search->SearchPlaceholder());
            });

    connect(ui->toolButtonWholeWord, &QToolButton::toggled, this,
            [this](bool checked)
            {
                m_search->SetMatchWord(checked);
                m_search->Find(ui->lineEditFind->text());
                ui->lineEditFind->setPlaceholderText(m_search->SearchPlaceholder());
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

    connect(ui->toolButtonUseUnicodeProperties, &QToolButton::toggled, this,
            [this](bool checked)
            {
                m_search->SetUseUnicodePreperties(checked);
                m_search->Find(ui->lineEditFind->text());
            });

    m_searchHistory->setStyleSheet(QStringLiteral("QMenu { menu-scrollable: 1; }"));
    InitSearchHistory();
    ui->pushButtonSearch->setMenu(m_searchHistory);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFinalMeasurements::InitSearchHistory()
{
    QStringList searchHistory =
        VAbstractValApplication::VApp()->ValentinaSettings()->GetFinalMeasurementsSearchHistory();
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
void DialogFinalMeasurements::SaveSearchRequest()
{
    QStringList searchHistory =
        VAbstractValApplication::VApp()->ValentinaSettings()->GetFinalMeasurementsSearchHistory();
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
    VAbstractValApplication::VApp()->ValentinaSettings()->SetFinalMeasurementsSearchHistory(searchHistory);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFinalMeasurements::UpdateSearchControlsTooltips()
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
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFinalMeasurements::InitIcons()
{
    QString resource = QStringLiteral("icon");

    ui->toolButtonExpr->setIcon(VTheme::GetIconResource(resource, QStringLiteral("24x24/fx.png")));
}
