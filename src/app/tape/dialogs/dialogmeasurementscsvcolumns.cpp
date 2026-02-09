/************************************************************************
 **
 **  @file   dialogmeasurementscsvcolumns.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   9 10, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2020 Valentina project
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
#include "dialogmeasurementscsvcolumns.h"
#include "../vmisc/qxtcsvmodel.h"
#include "../vtools/dialogs/dialogtoolbox.h"
#include "ui_dialogmeasurementscsvcolumns.h"

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
DialogMeasurementsCSVColumns::DialogMeasurementsCSVColumns(const QString &filename, MeasurementsType type,
                                                           QWidget *parent)
  : QDialog(parent),
    ui(new Ui::DialogMeasurementsCSVColumns),
    m_fileName{filename},
    m_type(type)
{
    ui->setupUi(this);

    HackColumnControls();
}

//---------------------------------------------------------------------------------------------------------------------
DialogMeasurementsCSVColumns::DialogMeasurementsCSVColumns(const QString &filename, MeasurementsType type,
                                                           const QList<MeasurementDimension_p> &dimensions,
                                                           QWidget *parent)
  : QDialog(parent),
    ui(new Ui::DialogMeasurementsCSVColumns),
    m_fileName{filename},
    m_type(type),
    m_dimensions{dimensions}
{
    ui->setupUi(this);

    HackColumnControls();
}

//---------------------------------------------------------------------------------------------------------------------
DialogMeasurementsCSVColumns::~DialogMeasurementsCSVColumns()
{
    qDeleteAll(m_hackedWidgets);
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMeasurementsCSVColumns::changeEvent(QEvent *event)
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
void DialogMeasurementsCSVColumns::showEvent(QShowEvent *event)
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
                &DialogMeasurementsCSVColumns::ColumnChanged);
        connect(ui->comboBoxValue, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                &DialogMeasurementsCSVColumns::ColumnChanged);

        if (m_type == MeasurementsType::Multisize)
        {
            if (not m_dimensions.empty())
            {
                connect(ui->comboBoxShiftA, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                        &DialogMeasurementsCSVColumns::ColumnChanged);
            }

            if (m_dimensions.size() > 1)
            {
                connect(ui->comboBoxShiftB, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                        &DialogMeasurementsCSVColumns::ColumnChanged);
            }

            if (m_dimensions.size() > 2)
            {
                connect(ui->comboBoxShiftC, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                        &DialogMeasurementsCSVColumns::ColumnChanged);
            }
        }

        connect(ui->comboBoxFullName, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                &DialogMeasurementsCSVColumns::ColumnChanged);
        connect(ui->comboBoxDescription, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                &DialogMeasurementsCSVColumns::ColumnChanged);

        CheckStatus();
    }

    m_isInitialized = true; // first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMeasurementsCSVColumns::ColumnChanged()
{
    auto *control = qobject_cast<QComboBox *>(sender());

    if (control == ui->comboBoxName)
    {
        m_type == MeasurementsType::Individual ? SaveColum(control, IndividualMeasurementsColumns::Name)
                                               : SaveColum(control, MultisizeMeasurementsColumns::Name);
    }
    else if (control == ui->comboBoxValue)
    {
        m_type == MeasurementsType::Individual ? SaveColum(control, IndividualMeasurementsColumns::Value)
                                               : SaveColum(control, MultisizeMeasurementsColumns::BaseValue);
    }
    else if (control == ui->comboBoxShiftA)
    {
        SaveColum(control, MultisizeMeasurementsColumns::ShiftA);
    }
    else if (control == ui->comboBoxShiftB)
    {
        SaveColum(control, MultisizeMeasurementsColumns::ShiftB);
    }
    else if (control == ui->comboBoxShiftC)
    {
        SaveColum(control, MultisizeMeasurementsColumns::ShiftC);
    }
    else if (control == ui->comboBoxFullName)
    {
        if (m_type == MeasurementsType::Individual)
        {
            SaveColum(control, IndividualMeasurementsColumns::FullName);
        }
        else
        {
            SaveColum(control, MultisizeMeasurementsColumns::FullName);
        }
    }
    else if (control == ui->comboBoxDescription)
    {
        if (m_type == MeasurementsType::Individual)
        {
            SaveColum(control, IndividualMeasurementsColumns::Description);
        }
        else
        {
            SaveColum(control, MultisizeMeasurementsColumns::Description);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMeasurementsCSVColumns::ColumnMandatory(int column) const -> bool
{
    if (m_type == MeasurementsType::Individual)
    {
        return column < static_cast<int>(IndividualMeasurementsColumns::FullName);
    }

    int mandatory = 3;

    if (m_dimensions.size() > 1)
    {
        mandatory += qMin(static_cast<int>(m_dimensions.size()), 2);
    }

    return column < mandatory;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMeasurementsCSVColumns::ColumnHeader(int column) const -> QString
{
    if (m_type == MeasurementsType::Individual)
    {
        const auto individualColumn = static_cast<IndividualMeasurementsColumns>(column);
        switch (individualColumn)
        {
            case IndividualMeasurementsColumns::Name:
                return QCoreApplication::translate("DialogMeasurementsCSVColumns", "Name", "measurement column");
            case IndividualMeasurementsColumns::Value:
                return QCoreApplication::translate("DialogMeasurementsCSVColumns", "Value", "measurement column");
            case IndividualMeasurementsColumns::FullName:
                return QCoreApplication::translate("DialogMeasurementsCSVColumns", "Full name", "measurement column");
            case IndividualMeasurementsColumns::Description:
                return QCoreApplication::translate("DialogMeasurementsCSVColumns", "Description", "measurement column");
            default:
                return {};
        }
    }
    else
    {
        const auto suffix = QStringLiteral(" (%1):");
        const auto multisizeColumn = static_cast<MultisizeMeasurementsColumns>(column);
        switch (multisizeColumn)
        {
            case MultisizeMeasurementsColumns::Name:
                return QCoreApplication::translate("DialogMeasurementsCSVColumns", "Name", "measurement column");
            case MultisizeMeasurementsColumns::BaseValue:
                return QCoreApplication::translate("DialogMeasurementsCSVColumns", "Base value", "measurement column");
            case MultisizeMeasurementsColumns::ShiftA:
                if (not m_dimensions.empty())
                {
                    MeasurementDimension_p const dimension = m_dimensions.at(0);
                    return QCoreApplication::translate("DialogMeasurementsCSVColumns", "Shift", "measurement column") +
                           suffix.arg(dimension->Name());
                }
                else
                {
                    return QStringLiteral("Shift A");
                }
            case MultisizeMeasurementsColumns::ShiftB:
                if (m_dimensions.size() > 1)
                {
                    MeasurementDimension_p const dimension = m_dimensions.at(1);
                    return QCoreApplication::translate("DialogMeasurementsCSVColumns", "Shift", "measurement column") +
                           suffix.arg(dimension->Name());
                }
                else
                {
                    return QStringLiteral("Shift B");
                }
            case MultisizeMeasurementsColumns::ShiftC:
                if (m_dimensions.size() > 2)
                {
                    MeasurementDimension_p const dimension = m_dimensions.at(2);
                    return QCoreApplication::translate("DialogMeasurementsCSVColumns", "Shift", "measurement column") +
                           suffix.arg(dimension->Name());
                }
                else
                {
                    return QStringLiteral("Shift C");
                }
            case MultisizeMeasurementsColumns::FullName:
                return QCoreApplication::translate("DialogMeasurementsCSVColumns", "Full name", "measurement column");
            case MultisizeMeasurementsColumns::Description:
                return QCoreApplication::translate("DialogMeasurementsCSVColumns", "Description", "measurement column");
            default:
                return {};
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMeasurementsCSVColumns::ImportColumnCount() const -> int
{
    if (m_type == MeasurementsType::Individual)
    {
        return static_cast<int>(IndividualMeasurementsColumns::LAST_DO_NOT_USE);
    }

    return static_cast<int>(MultisizeMeasurementsColumns::LAST_DO_NOT_USE);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMeasurementsCSVColumns::MinimumColumns() const -> int
{
    if (m_type == MeasurementsType::Individual)
    {
        return 2;
    }

    int mandatory = 3;

    if (m_dimensions.size() > 1)
    {
        mandatory += qMin(static_cast<int>(m_dimensions.size()), 2);
    }

    return mandatory;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMeasurementsCSVColumns::ColumnsValid() -> bool
{
    ClearColumnCollor();

    bool columnNameFlag = true;
    bool columnValueFlag = true;
    bool columnShiftAFlag = true;
    bool columnShiftBFlag = true;
    bool columnShiftCFlag = true;
    bool columnFullNameFlag = true;
    bool columnDescriptionFlag = true;

    const QColor errorColor = Qt::red;

    auto ChangeColumnColor = [this, errorColor](QLabel *label, bool &columnFlag,
                                                MultisizeMeasurementsColumns multisizeColumn,
                                                IndividualMeasurementsColumns individualColumn)
    {
        if (m_type == MeasurementsType::Multisize ? not ColumnValid(multisizeColumn)
                                                  : not ColumnValid(individualColumn))
        {
            ChangeColor(label, errorColor);
            columnFlag = false;
        }
    };

    ChangeColumnColor(ui->labelName, columnNameFlag, MultisizeMeasurementsColumns::Name,
                      IndividualMeasurementsColumns::Name);
    ChangeColumnColor(ui->labelValue, columnValueFlag, MultisizeMeasurementsColumns::BaseValue,
                      IndividualMeasurementsColumns::Value);

    if (m_type == MeasurementsType::Multisize)
    {
        if (not m_dimensions.empty() && not ColumnValid(MultisizeMeasurementsColumns::ShiftA))
        {
            ChangeColor(ui->labelShiftA, errorColor);
            columnShiftAFlag = false;
        }

        if (m_dimensions.size() > 1 && not ColumnValid(MultisizeMeasurementsColumns::ShiftB))
        {
            ChangeColor(ui->labelShiftB, errorColor);
            columnShiftBFlag = false;
        }

        if (m_dimensions.size() > 2 && not ColumnValid(MultisizeMeasurementsColumns::ShiftC))
        {
            ChangeColor(ui->labelShiftC, errorColor);
            columnShiftCFlag = false;
        }
    }

    ChangeColumnColor(ui->labelFullName, columnFullNameFlag, MultisizeMeasurementsColumns::FullName,
                      IndividualMeasurementsColumns::FullName);
    ChangeColumnColor(ui->labelDescription, columnDescriptionFlag, MultisizeMeasurementsColumns::Description,
                      IndividualMeasurementsColumns::Description);

    return columnNameFlag && columnValueFlag && columnShiftAFlag && columnShiftBFlag && columnShiftCFlag &&
           columnFullNameFlag && columnDescriptionFlag;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMeasurementsCSVColumns::ClearColumnCollor()
{
    ChangeColor(ui->labelName, OkColor(this));
    ChangeColor(ui->labelValue, OkColor(this));
    if (m_type == MeasurementsType::Multisize)
    {
        if (not m_dimensions.empty())
        {
            ChangeColor(ui->labelShiftA, OkColor(this));
        }

        if (m_dimensions.size() > 1)
        {
            ChangeColor(ui->labelShiftB, OkColor(this));
        }

        if (m_dimensions.size() > 2)
        {
            ChangeColor(ui->labelShiftC, OkColor(this));
        }
    }
    ChangeColor(ui->labelFullName, OkColor(this));
    ChangeColor(ui->labelDescription, OkColor(this));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMeasurementsCSVColumns::InitColumnsMap()
{
    QSharedPointer<QxtCsvModel> const csv = DialogMeasurementsCSVColumns::CSVModel();
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

    if (m_type == MeasurementsType::Individual)
    {
        m_columnsMap.resize(static_cast<int>(IndividualMeasurementsColumns::LAST_DO_NOT_USE));
        int index = -1;

        for (int column = 0; column < static_cast<int>(IndividualMeasurementsColumns::LAST_DO_NOT_USE); ++column)
        {
            InitColumn(column, index);
        }
    }
    else
    {
        m_columnsMap.resize(static_cast<int>(MultisizeMeasurementsColumns::LAST_DO_NOT_USE));
        int index = -1;

        InitColumn(static_cast<int>(MultisizeMeasurementsColumns::Name), index);
        InitColumn(static_cast<int>(MultisizeMeasurementsColumns::BaseValue), index);
        InitColumn(static_cast<int>(MultisizeMeasurementsColumns::ShiftA), index);
        InitColumn(static_cast<int>(MultisizeMeasurementsColumns::ShiftB), index, m_dimensions.size() < 2);
        InitColumn(static_cast<int>(MultisizeMeasurementsColumns::ShiftC), index, m_dimensions.size() < 3);
        InitColumn(static_cast<int>(MultisizeMeasurementsColumns::FullName), index);
        InitColumn(static_cast<int>(MultisizeMeasurementsColumns::Description), index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMeasurementsCSVColumns::InitColumnsControls()
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
            control->addItem(QCoreApplication::translate("DialogMeasurementsCSVColumns", "Skip"), -1);
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

    if (m_type == MeasurementsType::Individual)
    {
        InitControl(ui->comboBoxName, static_cast<int>(IndividualMeasurementsColumns::Name));
        InitControl(ui->comboBoxValue, static_cast<int>(IndividualMeasurementsColumns::Value));
        InitControl(ui->comboBoxFullName, static_cast<int>(IndividualMeasurementsColumns::FullName));
        InitControl(ui->comboBoxDescription, static_cast<int>(IndividualMeasurementsColumns::Description));
    }
    else
    {
        InitControl(ui->comboBoxName, static_cast<int>(MultisizeMeasurementsColumns::Name));
        InitControl(ui->comboBoxValue, static_cast<int>(MultisizeMeasurementsColumns::BaseValue));

        if (not m_dimensions.empty())
        {
            InitControl(ui->comboBoxShiftA, static_cast<int>(MultisizeMeasurementsColumns::ShiftA));
        }

        if (m_dimensions.size() > 1)
        {
            InitControl(ui->comboBoxShiftB, static_cast<int>(MultisizeMeasurementsColumns::ShiftB));
        }

        if (m_dimensions.size() > 2)
        {
            InitControl(ui->comboBoxShiftC, static_cast<int>(MultisizeMeasurementsColumns::ShiftC));
        }

        InitControl(ui->comboBoxFullName, static_cast<int>(MultisizeMeasurementsColumns::FullName));
        InitControl(ui->comboBoxDescription, static_cast<int>(MultisizeMeasurementsColumns::Description));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMeasurementsCSVColumns::InitImportHeaders()
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

    if (m_type == MeasurementsType::Individual)
    {
        AddHeader(static_cast<int>(IndividualMeasurementsColumns::Name));
        AddHeader(static_cast<int>(IndividualMeasurementsColumns::Value));
        AddHeader(static_cast<int>(IndividualMeasurementsColumns::FullName));
        AddHeader(static_cast<int>(IndividualMeasurementsColumns::Description));
    }
    else
    {
        AddHeader(static_cast<int>(MultisizeMeasurementsColumns::Name));
        AddHeader(static_cast<int>(MultisizeMeasurementsColumns::BaseValue));
        AddHeader(static_cast<int>(MultisizeMeasurementsColumns::ShiftA), not m_dimensions.empty());
        AddHeader(static_cast<int>(MultisizeMeasurementsColumns::ShiftB), m_dimensions.size() > 1);
        AddHeader(static_cast<int>(MultisizeMeasurementsColumns::ShiftC), m_dimensions.size() > 2);
        AddHeader(static_cast<int>(MultisizeMeasurementsColumns::FullName));
        AddHeader(static_cast<int>(MultisizeMeasurementsColumns::Description));
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMeasurementsCSVColumns::CSVModel() const -> QSharedPointer<QxtCsvModel>
{
    return QSharedPointer<QxtCsvModel>::create(m_fileName, nullptr, m_withHeader, m_separator, m_codec);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMeasurementsCSVColumns::ShowInputPreview()
{
    if (m_fileName.isEmpty())
    {
        return;
    }

    QSharedPointer<QxtCsvModel> const csv = DialogMeasurementsCSVColumns::CSVModel();

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
void DialogMeasurementsCSVColumns::ShowImportPreview()
{
    if (m_fileName.isEmpty())
    {
        return;
    }

    QSharedPointer<QxtCsvModel> const csv = DialogMeasurementsCSVColumns::CSVModel();

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
void DialogMeasurementsCSVColumns::HackColumnControls()
{
    if (m_type == MeasurementsType::Individual)
    {
        HackWidget(&ui->labelShiftA);
        HackWidget(&ui->labelShiftB);
        HackWidget(&ui->labelShiftC);

        HackWidget(&ui->comboBoxShiftA);
        HackWidget(&ui->comboBoxShiftB);
        HackWidget(&ui->comboBoxShiftC);
    }
    else
    {
        if (m_dimensions.size() < 2)
        {
            HackWidget(&ui->labelShiftB);
            HackWidget(&ui->comboBoxShiftB);
        }

        if (m_dimensions.size() < 3)
        {
            HackWidget(&ui->labelShiftC);
            HackWidget(&ui->comboBoxShiftC);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMeasurementsCSVColumns::RetranslateLabels()
{
    ui->labelName->setText(QCoreApplication::translate("DialogMeasurementsCSVColumns", "Name", "measurement column") +
                           "*:"_L1);

    if (m_type == MeasurementsType::Individual)
    {
        ui->labelValue->setText(
            QCoreApplication::translate("DialogMeasurementsCSVColumns", "Value", "measurement column") + "*:"_L1);
    }
    else
    {
        ui->labelValue->setText(
            QCoreApplication::translate("DialogMeasurementsCSVColumns", "Base value", "measurement column") + "*:"_L1);

        const auto suffix = QStringLiteral(" (%1)");

        if (not m_dimensions.empty())
        {
            MeasurementDimension_p const dimension = m_dimensions.at(0);
            ui->labelShiftA->setText(
                QCoreApplication::translate("DialogMeasurementsCSVColumns", "Shift", "measurement column") +
                suffix.arg(dimension->Name()));
        }

        if (m_dimensions.size() > 1)
        {
            MeasurementDimension_p const dimension = m_dimensions.at(1);
            ui->labelShiftB->setText(
                QCoreApplication::translate("DialogMeasurementsCSVColumns", "Shift", "measurement column") +
                suffix.arg(dimension->Name()));
        }

        if (m_dimensions.size() > 2)
        {
            MeasurementDimension_p const dimension = m_dimensions.at(2);
            ui->labelShiftC->setText(
                QCoreApplication::translate("DialogMeasurementsCSVColumns", "Shift", "measurement column") +
                suffix.arg(dimension->Name()));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMeasurementsCSVColumns::SetDefaultColumns()
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

    if (m_type == MeasurementsType::Individual)
    {
        SetDefault(ui->comboBoxName, static_cast<int>(IndividualMeasurementsColumns::Name));
        SetDefault(ui->comboBoxValue, static_cast<int>(IndividualMeasurementsColumns::Value));
        SetDefault(ui->comboBoxFullName, static_cast<int>(IndividualMeasurementsColumns::FullName));
        SetDefault(ui->comboBoxDescription, static_cast<int>(IndividualMeasurementsColumns::Description));
    }
    else
    {
        SetDefault(ui->comboBoxName, static_cast<int>(MultisizeMeasurementsColumns::Name));
        SetDefault(ui->comboBoxValue, static_cast<int>(MultisizeMeasurementsColumns::BaseValue));

        if (not m_dimensions.empty())
        {
            SetDefault(ui->comboBoxShiftA, static_cast<int>(MultisizeMeasurementsColumns::ShiftA));
        }

        if (m_dimensions.size() > 1)
        {
            SetDefault(ui->comboBoxShiftB, static_cast<int>(MultisizeMeasurementsColumns::ShiftB));
        }

        if (m_dimensions.size() > 2)
        {
            SetDefault(ui->comboBoxShiftC, static_cast<int>(MultisizeMeasurementsColumns::ShiftC));
        }

        SetDefault(ui->comboBoxFullName, static_cast<int>(MultisizeMeasurementsColumns::FullName));
        SetDefault(ui->comboBoxDescription, static_cast<int>(MultisizeMeasurementsColumns::Description));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMeasurementsCSVColumns::CheckStatus()
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
        ui->labelStatus->setText(QCoreApplication::translate("DialogMeasurementsCSVColumns", "File path is empty"));
        return;
    }

    QSharedPointer<QxtCsvModel> const csv = DialogMeasurementsCSVColumns::CSVModel();

    if (const int columns = csv->columnCount(); columns < MinimumColumns())
    {
        SetStatus(false);
        ui->labelStatus->setText(QCoreApplication::translate("DialogMeasurementsCSVColumns", "Not enough columns"));
        return;
    }

    if (const int rows = csv->rowCount(); rows < 1)
    {
        SetStatus(false);
        ui->labelStatus->setText(
            QCoreApplication::translate("DialogMeasurementsCSVColumns", "Not enough data to import"));
        return;
    }

    if (not ColumnsValid())
    {
        SetStatus(false);
        ui->labelStatus->setText(QCoreApplication::translate("DialogMeasurementsCSVColumns",
                                                             "Please, select unique number for each column"));
        return;
    }

    SetStatus(true);
    ui->labelStatus->setText(QCoreApplication::translate("DialogMeasurementsCSVColumns", "Ready"));
}

//---------------------------------------------------------------------------------------------------------------------
template <class T> void DialogMeasurementsCSVColumns::HackWidget(T **widget)
{
    delete *widget;
    *widget = new T();
    m_hackedWidgets.append(*widget);
}

//---------------------------------------------------------------------------------------------------------------------
template <class T> auto DialogMeasurementsCSVColumns::ColumnValid(T column) const -> bool
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
template <class T> void DialogMeasurementsCSVColumns::SaveColum(QComboBox *control, T column)
{
    SCASSERT(control != nullptr)

    const auto columnNumber = static_cast<int>(column);
    m_columnsMap[columnNumber] = control->currentData().toInt();
    ShowImportPreview();
    CheckStatus();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMeasurementsCSVColumns::SetCodec(QTextCodec *codec)
{
    m_codec = codec;
}
