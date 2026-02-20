/************************************************************************
 **
 **  @file   dialogincrementscsvcolumns.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 10, 2025
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
#include "dialogincrementscsvcolumns.h"
#include "../vmisc/qxtcsvmodel.h"
#include "../vtools/dialogs/dialogtoolbox.h"
#include "ui_dialogincrementscsvcolumns.h"

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

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
DialogIncrementsCSVColumns::DialogIncrementsCSVColumns(const QString &filename, QWidget *parent)
  : QDialog(parent),
    ui(new Ui::DialogIncrementsCSVColumns),
    m_fileName(filename)
{
    ui->setupUi(this);
}

//---------------------------------------------------------------------------------------------------------------------
DialogIncrementsCSVColumns::~DialogIncrementsCSVColumns()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrementsCSVColumns::SetCodec(QTextCodec *codec)
{
    m_codec = codec;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrementsCSVColumns::changeEvent(QEvent *event)
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
void DialogIncrementsCSVColumns::showEvent(QShowEvent *event)
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

        connect(ui->comboBoxName,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                this,
                &DialogIncrementsCSVColumns::ColumnChanged);
        connect(ui->comboBoxValue,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                this,
                &DialogIncrementsCSVColumns::ColumnChanged);
        connect(ui->comboBoxDescription,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                this,
                &DialogIncrementsCSVColumns::ColumnChanged);

        CheckStatus();
    }

    m_isInitialized = true; // first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrementsCSVColumns::ColumnChanged()
{
    if (auto *control = qobject_cast<QComboBox *>(sender()); control == ui->comboBoxName)
    {
        SaveColum(control, IncrementsColumns::Name);
    }
    else if (control == ui->comboBoxValue)
    {
        SaveColum(control, IncrementsColumns::Value);
    }
    else if (control == ui->comboBoxDescription)
    {
        SaveColum(control, IncrementsColumns::Description);
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogIncrementsCSVColumns::ColumnMandatory(int column) const
{
    return column < static_cast<int>(IncrementsColumns::Description);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogIncrementsCSVColumns::ColumnHeader(int column) const
{
    const auto individualColumn = static_cast<IncrementsColumns>(column);
    switch (individualColumn)
    {
        case IncrementsColumns::Name:
            return QCoreApplication::translate("DialogIncrementsCSVColumns", "Name", "measurement column");
        case IncrementsColumns::Value:
            return QCoreApplication::translate("DialogIncrementsCSVColumns", "Value", "measurement column");
        case IncrementsColumns::Description:
            return QCoreApplication::translate("DialogIncrementsCSVColumns", "Description", "measurement column");
        default:
            return {};
    }
}

//---------------------------------------------------------------------------------------------------------------------
int DialogIncrementsCSVColumns::ImportColumnCount() const
{
    return static_cast<int>(IncrementsColumns::LAST_DO_NOT_USE);
}

//---------------------------------------------------------------------------------------------------------------------
int DialogIncrementsCSVColumns::MinimumColumns() const
{
    return 2;
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogIncrementsCSVColumns::ColumnsValid()
{
    ClearColumnCollor();

    bool columnNameFlag = true;
    bool columnValueFlag = true;
    bool columnFullNameFlag = true;
    bool columnDescriptionFlag = true;

    const QColor errorColor = Qt::red;

    auto ChangeColumnColor = [this, errorColor](QLabel *label, bool &columnFlag, IncrementsColumns individualColumn)
    {
        if (!ColumnValid(individualColumn))
        {
            ChangeColor(label, errorColor);
            columnFlag = false;
        }
    };

    ChangeColumnColor(ui->labelName, columnNameFlag, IncrementsColumns::Name);
    ChangeColumnColor(ui->labelValue, columnValueFlag, IncrementsColumns::Value);
    ChangeColumnColor(ui->labelDescription, columnDescriptionFlag, IncrementsColumns::Description);

    return columnNameFlag && columnValueFlag && columnFullNameFlag && columnDescriptionFlag;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrementsCSVColumns::ClearColumnCollor()
{
    ChangeColor(ui->labelName, OkColor(this));
    ChangeColor(ui->labelValue, OkColor(this));
    ChangeColor(ui->labelDescription, OkColor(this));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrementsCSVColumns::InitColumnsMap()
{
    QSharedPointer<QxtCsvModel> const csv = DialogIncrementsCSVColumns::CSVModel();
    m_columnsMap.clear();

    auto InitColumn = [this, csv](int column, int &index, bool forceSkip = false)
    {
        if (forceSkip)
        {
            m_columnsMap[column] = -1;
        }
        else
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
        }
    };

    m_columnsMap.resize(static_cast<int>(IncrementsColumns::LAST_DO_NOT_USE));
    int index = -1;

    for (int column = 0; column < static_cast<int>(IncrementsColumns::LAST_DO_NOT_USE); ++column)
    {
        InitColumn(column, index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrementsCSVColumns::InitColumnsControls()
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
            control->addItem(QCoreApplication::translate("DialogIncrementsCSVColumns", "Skip"), -1);
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

    InitControl(ui->comboBoxName, static_cast<int>(IncrementsColumns::Name));
    InitControl(ui->comboBoxValue, static_cast<int>(IncrementsColumns::Value));
    InitControl(ui->comboBoxDescription, static_cast<int>(IncrementsColumns::Description));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrementsCSVColumns::InitImportHeaders()
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

    AddHeader(static_cast<int>(IncrementsColumns::Name));
    AddHeader(static_cast<int>(IncrementsColumns::Value));
    AddHeader(static_cast<int>(IncrementsColumns::Description));
}

//---------------------------------------------------------------------------------------------------------------------
QSharedPointer<QxtCsvModel> DialogIncrementsCSVColumns::CSVModel() const
{
    return QSharedPointer<QxtCsvModel>::create(m_fileName, nullptr, m_withHeader, m_separator, m_codec);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrementsCSVColumns::ShowInputPreview()
{
    if (m_fileName.isEmpty())
    {
        return;
    }

    QSharedPointer<QxtCsvModel> const csv = DialogIncrementsCSVColumns::CSVModel();

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
void DialogIncrementsCSVColumns::ShowImportPreview()
{
    if (m_fileName.isEmpty())
    {
        return;
    }

    QSharedPointer<QxtCsvModel> const csv = DialogIncrementsCSVColumns::CSVModel();

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
void DialogIncrementsCSVColumns::RetranslateLabels()
{
    ui->labelName->setText(QCoreApplication::translate("DialogIncrementsCSVColumns", "Name", "measurement column")
                           + "*:"_L1);
    ui->labelValue->setText(QCoreApplication::translate("DialogIncrementsCSVColumns", "Value", "measurement column")
                            + "*:"_L1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrementsCSVColumns::SetDefaultColumns()
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

    SetDefault(ui->comboBoxName, static_cast<int>(IncrementsColumns::Name));
    SetDefault(ui->comboBoxValue, static_cast<int>(IncrementsColumns::Value));
    SetDefault(ui->comboBoxDescription, static_cast<int>(IncrementsColumns::Description));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogIncrementsCSVColumns::CheckStatus()
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
        ui->labelStatus->setText(QCoreApplication::translate("DialogIncrementsCSVColumns", "File path is empty"));
        return;
    }

    QSharedPointer<QxtCsvModel> const csv = DialogIncrementsCSVColumns::CSVModel();

    if (const int columns = csv->columnCount(); columns < MinimumColumns())
    {
        SetStatus(false);
        ui->labelStatus->setText(QCoreApplication::translate("DialogIncrementsCSVColumns", "Not enough columns"));
        return;
    }

    if (const int rows = csv->rowCount(); rows < 1)
    {
        SetStatus(false);
        ui->labelStatus->setText(QCoreApplication::translate("DialogIncrementsCSVColumns", "Not enough data to import"));
        return;
    }

    if (not ColumnsValid())
    {
        SetStatus(false);
        ui->labelStatus->setText(
            QCoreApplication::translate("DialogIncrementsCSVColumns", "Please, select unique number for each column"));
        return;
    }

    SetStatus(true);
    ui->labelStatus->setText(QCoreApplication::translate("DialogIncrementsCSVColumns", "Ready"));
}

//---------------------------------------------------------------------------------------------------------------------
template<class T>
bool DialogIncrementsCSVColumns::ColumnValid(T column) const
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
template<class T>
void DialogIncrementsCSVColumns::SaveColum(QComboBox *control, T column)
{
    SCASSERT(control != nullptr)

    const auto columnNumber = static_cast<int>(column);
    m_columnsMap[columnNumber] = control->currentData().toInt();
    ShowImportPreview();
    CheckStatus();
}
