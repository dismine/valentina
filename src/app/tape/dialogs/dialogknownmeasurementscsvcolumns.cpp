/************************************************************************
 **
 **  @file   dialogknownmeasurementscsvcolumns.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   7 11, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#include "dialogknownmeasurementscsvcolumns.h"
#include "ui_dialogknownmeasurementscsvcolumns.h"

#include "../vmisc/qxtcsvmodel.h"
#include "../vtools/dialogs/dialogtoolbox.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#ifdef WITH_TEXTCODEC
#include "../vmisc/codecs/qtextcodec.h"
#else
#include "../vmisc/vtextcodec.h"
using QTextCodec = VTextCodec;
#endif // WITH_TEXTCODEC
#else
#include <QTextCodec>
#endif // QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

#include <QPushButton>
#include <QShowEvent>
#include <utility>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
DialogKnownMeasurementsCSVColumns::DialogKnownMeasurementsCSVColumns(QString filename, QWidget *parent)
  : QDialog(parent),
    ui(std::make_unique<Ui::DialogKnownMeasurementsCSVColumns>()),
    m_fileName{std::move(filename)}
{
    ui->setupUi(this);
}

//---------------------------------------------------------------------------------------------------------------------
DialogKnownMeasurementsCSVColumns::~DialogKnownMeasurementsCSVColumns() = default;

//---------------------------------------------------------------------------------------------------------------------
void DialogKnownMeasurementsCSVColumns::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);

        RetranslateLabels();
        InitColumnsControls();
        InitImportHeaders();
        ShowImportPreview();
        CheckStatus();
    }

    // remember to call base class implementation
    QDialog::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogKnownMeasurementsCSVColumns::showEvent(QShowEvent *event)
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

    if (not m_fileName.isEmpty())
    {
        InitColumnsMap();
        ShowInputPreview();
        InitColumnsControls();
        RetranslateLabels();
        SetDefaultColumns();
        InitImportHeaders();
        ShowImportPreview();

        connect(ui->comboBoxName, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                &DialogKnownMeasurementsCSVColumns::ColumnChanged);
        connect(ui->comboBoxGroup, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                &DialogKnownMeasurementsCSVColumns::ColumnChanged);
        connect(ui->comboBoxFullName, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                &DialogKnownMeasurementsCSVColumns::ColumnChanged);
        connect(ui->comboBoxFormula, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                &DialogKnownMeasurementsCSVColumns::ColumnChanged);
        connect(ui->comboBoxDescription, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                &DialogKnownMeasurementsCSVColumns::ColumnChanged);

        CheckStatus();
    }

    m_isInitialized = true; // first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void DialogKnownMeasurementsCSVColumns::ColumnChanged()
{
    auto *control = qobject_cast<QComboBox *>(sender());

    if (control == ui->comboBoxName)
    {
        SaveColum(control, KnownMeasurementsColumns::Name);
    }
    else if (control == ui->comboBoxGroup)
    {
        SaveColum(control, KnownMeasurementsColumns::Group);
    }
    else if (control == ui->comboBoxFullName)
    {
        SaveColum(control, KnownMeasurementsColumns::FullName);
    }
    else if (control == ui->comboBoxFormula)
    {
        SaveColum(control, KnownMeasurementsColumns::Formula);
    }
    else if (control == ui->comboBoxDescription)
    {
        SaveColum(control, KnownMeasurementsColumns::Description);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogKnownMeasurementsCSVColumns::ColumnMandatory(int column) const -> bool
{
    return column < static_cast<int>(KnownMeasurementsColumns::Group);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogKnownMeasurementsCSVColumns::ColumnHeader(int column) const -> QString
{
    const auto individualColumn = static_cast<KnownMeasurementsColumns>(column);
    switch (individualColumn)
    {
        case KnownMeasurementsColumns::Name:
            return QCoreApplication::translate("DialogKnownMeasurementsCSVColumns", "Name", "measurement column");
        case KnownMeasurementsColumns::Group:
            return QCoreApplication::translate("DialogKnownMeasurementsCSVColumns", "Group", "measurement column");
        case KnownMeasurementsColumns::FullName:
            return QCoreApplication::translate("DialogKnownMeasurementsCSVColumns", "Full name", "measurement column");
        case KnownMeasurementsColumns::Formula:
            return QCoreApplication::translate("DialogKnownMeasurementsCSVColumns", "Formula", "measurement column");
        case KnownMeasurementsColumns::Description:
            return QCoreApplication::translate("DialogKnownMeasurementsCSVColumns", "Description",
                                               "measurement column");
        default:
            return {};
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogKnownMeasurementsCSVColumns::ImportColumnCount() const -> int
{
    return static_cast<int>(KnownMeasurementsColumns::LAST_DO_NOT_USE);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogKnownMeasurementsCSVColumns::MinimumColumns() const -> int
{
    return 1;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogKnownMeasurementsCSVColumns::ColumnsValid() -> bool
{
    ClearColumnCollor();

    bool columnNameFlag = true;
    bool columnGroupFlag = true;
    bool columnFullNameFlag = true;
    bool columnFormulaFlag = true;
    bool columnDescriptionFlag = true;

    const QColor errorColor = Qt::red;

    auto ChangeColumnColor = [this, errorColor](QLabel *label, bool &columnFlag, KnownMeasurementsColumns column)
    {
        if (not ColumnValid(column))
        {
            ChangeColor(label, errorColor);
            columnFlag = false;
        }
    };

    ChangeColumnColor(ui->labelName, columnNameFlag, KnownMeasurementsColumns::Name);
    ChangeColumnColor(ui->labelGroup, columnGroupFlag, KnownMeasurementsColumns::Group);
    ChangeColumnColor(ui->labelFullName, columnFullNameFlag, KnownMeasurementsColumns::FullName);
    ChangeColumnColor(ui->labelFormula, columnFormulaFlag, KnownMeasurementsColumns::Formula);
    ChangeColumnColor(ui->labelDescription, columnDescriptionFlag, KnownMeasurementsColumns::Description);

    return columnNameFlag && columnGroupFlag && columnFullNameFlag && columnFormulaFlag && columnDescriptionFlag;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogKnownMeasurementsCSVColumns::ClearColumnCollor()
{
    ChangeColor(ui->labelName, OkColor(this));
    ChangeColor(ui->labelGroup, OkColor(this));
    ChangeColor(ui->labelFullName, OkColor(this));
    ChangeColor(ui->labelFormula, OkColor(this));
    ChangeColor(ui->labelDescription, OkColor(this));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogKnownMeasurementsCSVColumns::InitColumnsMap()
{
    QSharedPointer<QxtCsvModel> const csv = DialogKnownMeasurementsCSVColumns::CSVModel();
    m_columnsMap.clear();

    auto InitColumn = [this, csv](int column, int &index)
    {
        ++index;
        if (ColumnMandatory(column))
        {
            m_columnsMap[column] = index;
        }
        else
        {
            m_columnsMap[column] = csv->columnCount() >= index ? index : -1;
        }
    };

    m_columnsMap.resize(static_cast<int>(KnownMeasurementsColumns::LAST_DO_NOT_USE));
    int index = -1;

    for (int column = 0; column < static_cast<int>(KnownMeasurementsColumns::LAST_DO_NOT_USE); ++column)
    {
        InitColumn(column, index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogKnownMeasurementsCSVColumns::InitColumnsControls()
{
    const int inputColumnCount = CSVModel()->columnCount();

    auto InitControl = [this, inputColumnCount](QComboBox *control, int column)
    {
        SCASSERT(control != nullptr)

        int currentColumn = -2;
        int index = control->currentIndex();
        if (index != -1)
        {
            currentColumn = control->currentData().toInt();
        }

        QSignalBlocker blocker(control);
        control->clear();

        for (int i = 0; i < inputColumnCount; ++i)
        {
            control->addItem(QString::number(i + 1), i);
        }

        if (not ColumnMandatory(column))
        {
            control->addItem(QCoreApplication::translate("DialogKnownMeasurementsCSVColumns", "Skip"), -1);
        }

        control->setCurrentIndex(-1);

        index = control->findData(currentColumn);
        if (index != -1)
        {
            control->setCurrentIndex(index);
            blocker.unblock();
        }
        else
        {
            blocker.unblock();

            if (not ColumnMandatory(column))
            {
                control->setCurrentIndex(control->findData(-1));
            }
            else
            {
                control->setCurrentIndex(0);
            }
        }
    };

    InitControl(ui->comboBoxName, static_cast<int>(KnownMeasurementsColumns::Name));
    InitControl(ui->comboBoxGroup, static_cast<int>(KnownMeasurementsColumns::Group));
    InitControl(ui->comboBoxFullName, static_cast<int>(KnownMeasurementsColumns::FullName));
    InitControl(ui->comboBoxFormula, static_cast<int>(KnownMeasurementsColumns::Formula));
    InitControl(ui->comboBoxDescription, static_cast<int>(KnownMeasurementsColumns::Description));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogKnownMeasurementsCSVColumns::InitImportHeaders()
{
    if (m_fileName.isEmpty())
    {
        return;
    }

    const int columns = ImportColumnCount();

    ui->tableWidgetImport->clear();
    ui->tableWidgetImport->setColumnCount(columns);

    auto AddHeader = [this](int column, bool visible = true)
    {
        auto *header = new QTableWidgetItem(ColumnHeader(column));
        ui->tableWidgetImport->setHorizontalHeaderItem(column, header);
        ui->tableWidgetImport->setColumnHidden(column, not visible);
    };

    AddHeader(static_cast<int>(KnownMeasurementsColumns::Name));
    AddHeader(static_cast<int>(KnownMeasurementsColumns::Group));
    AddHeader(static_cast<int>(KnownMeasurementsColumns::FullName));
    AddHeader(static_cast<int>(KnownMeasurementsColumns::Formula));
    AddHeader(static_cast<int>(KnownMeasurementsColumns::Description));
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogKnownMeasurementsCSVColumns::CSVModel() const -> QSharedPointer<QxtCsvModel>
{
    return QSharedPointer<QxtCsvModel>::create(m_fileName, nullptr, m_withHeader, m_separator, m_codec);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogKnownMeasurementsCSVColumns::ShowInputPreview()
{
    if (m_fileName.isEmpty())
    {
        return;
    }

    QSharedPointer<QxtCsvModel> const csv = DialogKnownMeasurementsCSVColumns::CSVModel();

    const int columns = csv->columnCount();
    const int rows = csv->rowCount();

    ui->tableWidgetInput->clear();
    ui->tableWidgetInput->setColumnCount(columns);
    ui->tableWidgetInput->setRowCount(rows);

    ui->tableWidgetInput->horizontalHeader()->setVisible(m_withHeader);
    if (m_withHeader)
    {
        for (int column = 0; column < columns; ++column)
        {
            auto *header = new QTableWidgetItem(csv->headerText(column));
            header->setToolTip(QString::number(column + 1));
            ui->tableWidgetInput->setHorizontalHeaderItem(column, header);
        }
    }

    for (int row = 0; row < rows; ++row)
    {
        for (int column = 0; column < columns; ++column)
        {
            const QString text = csv->text(row, column);
            auto *item = new QTableWidgetItem(text);
            item->setToolTip(text);

            // set the item non-editable (view only), and non-selectable
            Qt::ItemFlags flags = item->flags();
            flags &= ~(Qt::ItemIsEditable); // reset/clear the flag
            item->setFlags(flags);

            ui->tableWidgetInput->setItem(row, column, item);
        }
    }

    ui->tableWidgetInput->resizeColumnsToContents();
    ui->tableWidgetInput->resizeRowsToContents();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogKnownMeasurementsCSVColumns::ShowImportPreview()
{
    if (m_fileName.isEmpty())
    {
        return;
    }

    QSharedPointer<QxtCsvModel> const csv = DialogKnownMeasurementsCSVColumns::CSVModel();

    const int importColumns = ImportColumnCount();
    const int columns = csv->columnCount();
    const int rows = csv->rowCount();

    ui->tableWidgetImport->clearContents();
    ui->tableWidgetImport->setRowCount(rows);

    for (int row = 0; row < rows; ++row)
    {
        for (int column = 0; column < importColumns; ++column)
        {
            const int tableColumn = m_columnsMap.at(column);
            if (tableColumn >= 0 && tableColumn < columns)
            {
                const QString text = csv->text(row, tableColumn);
                auto *item = new QTableWidgetItem(text);
                item->setToolTip(text);

                // set the item non-editable (view only), and non-selectable
                Qt::ItemFlags flags = item->flags();
                flags &= ~(Qt::ItemIsEditable); // reset/clear the flag
                item->setFlags(flags);

                ui->tableWidgetImport->setItem(row, column, item);
            }
        }
    }

    ui->tableWidgetImport->resizeColumnsToContents();
    ui->tableWidgetImport->resizeRowsToContents();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogKnownMeasurementsCSVColumns::RetranslateLabels()
{
    ui->labelName->setText(QCoreApplication::translate("DialogKnownMeasurementsCSVColumns", "Name") + "*:"_L1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogKnownMeasurementsCSVColumns::SetDefaultColumns()
{
    auto SetDefault = [this](QComboBox *control, int column)
    {
        SCASSERT(control != nullptr)

        int const index = control->findData(m_columnsMap.at(column));
        if (index != -1)
        {
            control->setCurrentIndex(index);
        }
    };

    SetDefault(ui->comboBoxName, static_cast<int>(KnownMeasurementsColumns::Name));
    SetDefault(ui->comboBoxGroup, static_cast<int>(KnownMeasurementsColumns::Group));
    SetDefault(ui->comboBoxFullName, static_cast<int>(KnownMeasurementsColumns::FullName));
    SetDefault(ui->comboBoxFormula, static_cast<int>(KnownMeasurementsColumns::Formula));
    SetDefault(ui->comboBoxDescription, static_cast<int>(KnownMeasurementsColumns::Description));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogKnownMeasurementsCSVColumns::CheckStatus()
{
    auto SetStatus = [this](bool status)
    {
        QPushButton *bOk = ui->buttonBox->button(QDialogButtonBox::Ok);
        // cppcheck-suppress unknownMacro
        SCASSERT(bOk != nullptr)
        bOk->setEnabled(status);
    };

    if (m_fileName.isEmpty())
    {
        SetStatus(false);
        ui->labelStatus->setText(
            QCoreApplication::translate("DialogKnownMeasurementsCSVColumns", "File path is empty"));
        return;
    }

    QSharedPointer<QxtCsvModel> const csv = DialogKnownMeasurementsCSVColumns::CSVModel();

    if (const int columns = csv->columnCount(); columns < MinimumColumns())
    {
        SetStatus(false);
        ui->labelStatus->setText(
            QCoreApplication::translate("DialogKnownMeasurementsCSVColumns", "Not enough columns"));
        return;
    }

    if (const int rows = csv->rowCount(); rows < 1)
    {
        SetStatus(false);
        ui->labelStatus->setText(
            QCoreApplication::translate("DialogKnownMeasurementsCSVColumns", "Not enough data to import"));
        return;
    }

    if (not ColumnsValid())
    {
        SetStatus(false);
        ui->labelStatus->setText(QCoreApplication::translate("DialogKnownMeasurementsCSVColumns",
                                                             "Please, select unique number for each column"));
        return;
    }

    SetStatus(true);
    ui->labelStatus->setText(QCoreApplication::translate("DialogKnownMeasurementsCSVColumns", "Ready"));
}

//---------------------------------------------------------------------------------------------------------------------
template <class T> auto DialogKnownMeasurementsCSVColumns::ColumnValid(T column) const -> bool
{
    const auto columnNumber = static_cast<int>(column);
    int const value = m_columnsMap.at(columnNumber);

    if (value == -1 && not ColumnMandatory(columnNumber))
    {
        return true;
    }

    for (int c = 0; c < m_columnsMap.size(); ++c)
    {
        if (c == columnNumber)
        {
            continue;
        }

        if (value == m_columnsMap.at(c))
        {
            return false;
        }
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
template <class T> void DialogKnownMeasurementsCSVColumns::SaveColum(QComboBox *control, T column)
{
    SCASSERT(control != nullptr)

    const auto columnNumber = static_cast<int>(column);
    m_columnsMap[columnNumber] = control->currentData().toInt();
    ShowImportPreview();
    CheckStatus();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogKnownMeasurementsCSVColumns::SetCodec(QTextCodec *codec)
{
    m_codec = codec;
}
