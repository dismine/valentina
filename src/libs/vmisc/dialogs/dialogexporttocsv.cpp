/************************************************************************
 **
 **  @file   dialogexporttocsv.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   1 6, 2016
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

#include "dialogexporttocsv.h"
#include "ui_dialogexporttocsv.h"

#include "../qxtcsvmodel.h"
#include "../vabstractapplication.h"
#include "../vcommonsettings.h"

#include <QDebug>
#include <QPushButton>
#include <QShowEvent>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include "../vtextcodec.h"
#else
#include <QTextCodec>
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#include "../vmisc/backport/qoverload.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 7, 0)

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
DialogExportToCSV::DialogExportToCSV(QWidget *parent)
  : QDialog(parent),
    ui(new Ui::DialogExportToCSV),
    isInitialized(false),
    m_fileName()
{
    ui->setupUi(this);

    const QList<int> mibs = VTextCodec::availableMibs();
    for (auto mib : mibs)
    {
        if (VTextCodec *codec = VTextCodec::codecForMib(mib))
        {
            ui->comboBoxCodec->addItem(codec->name(), mib);
        }
    }

    ui->comboBoxCodec->setCurrentIndex(ui->comboBoxCodec->findData(VCommonSettings::GetDefCSVCodec()));

    SetSeparator(VCommonSettings::GetDefCSVSeparator());

    QPushButton *bDefaults = ui->buttonBox->button(QDialogButtonBox::RestoreDefaults);
    SCASSERT(bDefaults != nullptr)
    connect(bDefaults, &QPushButton::clicked, this,
            [this]()
            {
                ui->comboBoxCodec->blockSignals(true);
                ui->checkBoxWithHeader->blockSignals(true);
                ui->buttonGroup->blockSignals(true);

                ui->checkBoxWithHeader->setChecked(VAbstractApplication::VApp()->Settings()->GetDefCSVWithHeader());
                ui->comboBoxCodec->setCurrentIndex(ui->comboBoxCodec->findData(VCommonSettings::GetDefCSVCodec()));

                SetSeparator(VCommonSettings::GetDefCSVSeparator());

                ui->comboBoxCodec->blockSignals(false);
                ui->checkBoxWithHeader->blockSignals(false);
                ui->buttonGroup->blockSignals(false);

                ShowPreview();
            });

    ui->groupBoxPreview->setVisible(false);

    connect(ui->comboBoxCodec, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { ShowPreview(); });
    connect(ui->checkBoxWithHeader, &QCheckBox::stateChanged, this, [this]() { ShowPreview(); });
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(ui->buttonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, [this]() { ShowPreview(); });
#else
    connect(ui->buttonGroup, &QButtonGroup::idClicked, this, [this]() { ShowPreview(); });
#endif
}

//---------------------------------------------------------------------------------------------------------------------
DialogExportToCSV::~DialogExportToCSV()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogExportToCSV::IsWithHeader() const -> bool
{
    return ui->checkBoxWithHeader->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogExportToCSV::SetWithHeader(bool value)
{
    ui->checkBoxWithHeader->setChecked(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogExportToCSV::GetSelectedMib() const -> int
{
    if (ui->comboBoxCodec->currentIndex() != -1)
    {
        return ui->comboBoxCodec->currentData().toInt();
    }

    return VCommonSettings::GetDefCSVCodec();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogExportToCSV::SetSelectedMib(int value)
{
    const int index = ui->comboBoxCodec->findData(value);
    if (index != -1)
    {
        ui->comboBoxCodec->setCurrentIndex(index);
    }
    else
    {
        ui->comboBoxCodec->setCurrentIndex(ui->comboBoxCodec->findData(VCommonSettings::GetDefCSVCodec()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogExportToCSV::GetSeparator() const -> QChar
{
    if (ui->radioButtonTab->isChecked())
    {
        return '\t'_L1;
    }

    if (ui->radioButtonSemicolon->isChecked())
    {
        return ';'_L1;
    }

    else if (ui->radioButtonSpace->isChecked())
    {
        return ' '_L1;
    }

    return VCommonSettings::GetDefCSVSeparator();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogExportToCSV::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
    }

    // remember to call base class implementation
    QDialog::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogExportToCSV::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    if (event->spontaneous())
    {
        return;
    }

    if (isInitialized)
    {
        return;
    }
    // do your init stuff here

    resize(1, 1);
    adjustSize();
    if (not m_fileName.isEmpty())
    {
        ShowPreview();
    }
    else
    {
        setMaximumSize(size());
        setMinimumSize(size());
    }

    isInitialized = true; // first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void DialogExportToCSV::ShowPreview()
{
    if (m_fileName.isEmpty())
    {
        return;
    }

    ui->groupBoxPreview->setVisible(true);

    QxtCsvModel csv(m_fileName, nullptr, IsWithHeader(), GetSeparator(), VTextCodec::codecForMib(GetSelectedMib()));

    const int columns = csv.columnCount();
    const int rows = csv.rowCount();

    ui->tableWidget->clear();
    ui->tableWidget->setColumnCount(columns);
    ui->tableWidget->setRowCount(rows);

    ui->tableWidget->horizontalHeader()->setVisible(IsWithHeader());
    if (IsWithHeader())
    {
        for (int column = 0; column < columns; ++column)
        {
            QTableWidgetItem *header = new QTableWidgetItem(csv.headerText(column));
            ui->tableWidget->setHorizontalHeaderItem(column, header);
        }
        ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    }

    for (int row = 0; row < rows; ++row)
    {
        for (int column = 0; column < columns; ++column)
        {
            QTableWidgetItem *item = new QTableWidgetItem(csv.text(row, column));
            item->setToolTip(csv.text(row, column));

            // set the item non-editable (view only), and non-selectable
            Qt::ItemFlags flags = item->flags();
            flags &= ~(Qt::ItemIsEditable); // reset/clear the flag
            item->setFlags(flags);

            ui->tableWidget->setItem(row, column, item);
        }
    }

    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogExportToCSV::SetSeparator(const QChar &separator)
{
    switch (separator.toLatin1())
    {
        case '\t':
            ui->radioButtonTab->setChecked(true);
            break;
        case ';':
            ui->radioButtonSemicolon->setChecked(true);
            break;
        case ' ':
            ui->radioButtonSpace->setChecked(true);
            break;
        case ',':
        default:
            ui->radioButtonComma->setChecked(true);
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogExportToCSV::ShowFilePreview(const QString &fileName)
{
    m_fileName = fileName;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogExportToCSV::MakeHelpCodecsList() -> QString
{
    QString out = QStringLiteral("\n");
    const QList<int> list = VTextCodec::availableMibs();
    for (int i = 0; i < list.size(); ++i)
    {
        if (VTextCodec *codec = VTextCodec::codecForMib(list.at(i)))
        {
            out += QStringLiteral("\t* ") + codec->name();
            out += i < list.size() - 1 ? ",\n"_L1 : ".\n"_L1;
        }
        else
        {
            qDebug() << "Can't get codec for MIBenum " << i;
        }
    }
    return out;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogExportToCSV::MakeHelpSeparatorList() -> QString
{
    QString out = "\n"_L1;
    out += "\t* 'Tab',\n"_L1;
    out += "\t* ';',\n"_L1;
    out += "\t* 'Space',\n"_L1;
    out += "\t* ','.\n"_L1;
    return out;
}
