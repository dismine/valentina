/************************************************************************
 **
 **  @file   dialogeditlabel.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 8, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2017 Valentina project
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

#include "dialogeditlabel.h"
#include "../vmisc/vabstractapplication.h"
#include "ui_dialogeditlabel.h"
#include "vabstractvalapplication.h"
#include "vvalentinasettings.h"
#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#include "../vmisc/backport/qoverload.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#include "../ifc/exception/vexception.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vlabeltemplateconverter.h"
#include "../tools/dialogtool.h"
#include "../vformat/vlabeltemplate.h"
#include "../vpatterndb/calculator.h"
#include "../vpatterndb/floatItemData/vpiecelabeldata.h"
#include "../vpatterndb/variables/vmeasurement.h"
#include "../vpatterndb/variables/vpiecearea.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpiece.h"

#include <QDate>
#include <QDir>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
DialogEditLabel::DialogEditLabel(const VAbstractPattern *doc, const VContainer *data, QWidget *parent)
  : QDialog(parent),
    ui(new Ui::DialogEditLabel),
    m_placeholdersMenu(new QMenu(this)),
    m_doc(doc),
    m_data(data)
{
    ui->setupUi(this);

    ui->lineEditLine->setClearButtonEnabled(true);

    connect(ui->toolButtonAdd, &QToolButton::clicked, this, &DialogEditLabel::AddLine);
    connect(ui->toolButtonRemove, &QToolButton::clicked, this, &DialogEditLabel::RemoveLine);
    connect(ui->lineEditLine, &QLineEdit::textEdited, this, &DialogEditLabel::SaveLineText);
    connect(ui->toolButtonBold, &QToolButton::toggled, this, &DialogEditLabel::SaveFontStyle);
    connect(ui->toolButtonItalic, &QToolButton::toggled, this, &DialogEditLabel::SaveFontStyle);
    connect(ui->toolButtonTextLeft, &QToolButton::toggled, this, &DialogEditLabel::SaveTextFormating);
    connect(ui->toolButtonTextCenter, &QToolButton::toggled, this, &DialogEditLabel::SaveTextFormating);
    connect(ui->toolButtonTextRight, &QToolButton::toggled, this, &DialogEditLabel::SaveTextFormating);
    connect(ui->listWidgetEdit, &QListWidget::itemSelectionChanged, this, &DialogEditLabel::ShowLineDetails);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &DialogEditLabel::TabChanged);
    connect(ui->toolButtonNewLabel, &QToolButton::clicked, this, &DialogEditLabel::NewTemplate);
    connect(ui->toolButtonExportLabel, &QToolButton::clicked, this, &DialogEditLabel::ExportTemplate);
    connect(ui->toolButtonImportLabel, &QToolButton::clicked, this, &DialogEditLabel::ImportTemplate);
    connect(ui->spinBoxFontSize, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &DialogEditLabel::SaveAdditionalFontSize);
    connect(ui->toolButtonTop, &QToolButton::clicked, this,
            [this]() { DialogTool::MoveListRowTop(ui->listWidgetEdit); });
    connect(ui->toolButtonUp, &QToolButton::clicked, this, [this]() { DialogTool::MoveListRowUp(ui->listWidgetEdit); });
    connect(ui->toolButtonDown, &QToolButton::clicked, this,
            [this]() { DialogTool::MoveListRowDown(ui->listWidgetEdit); });
    connect(ui->toolButtonBottom, &QToolButton::clicked, this,
            [this]() { DialogTool::MoveListRowBottom(ui->listWidgetEdit); });

    InitPlaceholders();
    InitPlaceholdersMenu();

    m_placeholdersMenu->setStyleSheet(QStringLiteral("QMenu { menu-scrollable: 1; }"));
    m_placeholdersMenu->setToolTipsVisible(true);
    ui->pushButtonInsert->setMenu(m_placeholdersMenu);
}

//---------------------------------------------------------------------------------------------------------------------
DialogEditLabel::~DialogEditLabel()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEditLabel::ShowLineDetails()
{
    if (ui->listWidgetEdit->count() > 0)
    {
        const QListWidgetItem *line = ui->listWidgetEdit->currentItem();
        if (line)
        {
            ui->lineEditLine->blockSignals(true);
            ui->lineEditLine->setText(line->text());
            ui->lineEditLine->blockSignals(false);

            const QFont lineFont = line->font();

            ui->toolButtonBold->blockSignals(true);
            ui->toolButtonBold->setChecked(lineFont.bold());
            ui->toolButtonBold->blockSignals(false);

            ui->toolButtonItalic->blockSignals(true);
            ui->toolButtonItalic->setChecked(lineFont.italic());
            ui->toolButtonItalic->blockSignals(false);

            ui->toolButtonTextLeft->blockSignals(true);
            ui->toolButtonTextCenter->blockSignals(true);
            ui->toolButtonTextRight->blockSignals(true);

            const int lineAlignment = line->textAlignment();

            if (lineAlignment == 0 || lineAlignment & Qt::AlignLeft)
            {
                ui->toolButtonTextLeft->setChecked(true);
                ui->toolButtonTextCenter->setChecked(false);
                ui->toolButtonTextRight->setChecked(false);
            }
            else if (lineAlignment & Qt::AlignHCenter)
            {
                ui->toolButtonTextLeft->setChecked(false);
                ui->toolButtonTextCenter->setChecked(true);
                ui->toolButtonTextRight->setChecked(false);
            }
            else if (lineAlignment & Qt::AlignRight)
            {
                ui->toolButtonTextLeft->setChecked(false);
                ui->toolButtonTextCenter->setChecked(false);
                ui->toolButtonTextRight->setChecked(true);
            }

            ui->toolButtonTextLeft->blockSignals(false);
            ui->toolButtonTextCenter->blockSignals(false);
            ui->toolButtonTextRight->blockSignals(false);

            ui->spinBoxFontSize->blockSignals(true);
            ui->spinBoxFontSize->setValue(line->data(Qt::UserRole).toInt());
            ui->spinBoxFontSize->blockSignals(false);
        }
    }

    SetupControls();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEditLabel::AddLine()
{
    int row = ui->listWidgetEdit->currentRow();
    ui->listWidgetEdit->insertItem(++row, new QListWidgetItem(tr("<empty>")));
    ui->listWidgetEdit->setCurrentRow(row);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEditLabel::RemoveLine()
{
    ui->listWidgetEdit->blockSignals(true);
    QListWidgetItem *curLine = ui->listWidgetEdit->takeItem(ui->listWidgetEdit->currentRow());
    delete curLine;
    ui->listWidgetEdit->blockSignals(false);
    ShowLineDetails();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEditLabel::SaveLineText(const QString &text)
{
    QListWidgetItem *curLine = ui->listWidgetEdit->currentItem();
    if (curLine)
    {
        curLine->setText(text);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEditLabel::SaveFontStyle(bool checked)
{
    QListWidgetItem *curLine = ui->listWidgetEdit->currentItem();
    if (curLine)
    {
        QFont lineFont = curLine->font();

        auto *button = qobject_cast<QToolButton *>(sender());
        if (button)
        {
            if (button == ui->toolButtonBold)
            {
                lineFont.setBold(checked);
            }
            else if (button == ui->toolButtonItalic)
            {
                lineFont.setItalic(checked);
            }
        }

        curLine->setFont(lineFont);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEditLabel::SaveTextFormating(bool checked)
{
    QListWidgetItem *curLine = ui->listWidgetEdit->currentItem();
    if (curLine)
    {
        auto *button = qobject_cast<QToolButton *>(sender());
        if (button)
        {
            ui->toolButtonTextLeft->blockSignals(true);
            ui->toolButtonTextCenter->blockSignals(true);
            ui->toolButtonTextRight->blockSignals(true);

            if (button == ui->toolButtonTextLeft)
            {
                if (checked)
                {
                    curLine->setTextAlignment(Qt::AlignLeft);

                    ui->toolButtonTextCenter->setChecked(false);
                    ui->toolButtonTextRight->setChecked(false);
                }
                else
                {
                    button->setChecked(true);
                }
            }
            else if (button == ui->toolButtonTextCenter)
            {
                if (checked)
                {
                    curLine->setTextAlignment(Qt::AlignHCenter);

                    ui->toolButtonTextLeft->setChecked(false);
                    ui->toolButtonTextRight->setChecked(false);
                }
                else
                {
                    button->setChecked(true);
                }
            }
            else if (button == ui->toolButtonTextRight)
            {
                if (checked)
                {
                    curLine->setTextAlignment(Qt::AlignRight);

                    ui->toolButtonTextCenter->setChecked(false);
                    ui->toolButtonTextRight->setChecked(false);
                }
                else
                {
                    button->setChecked(true);
                }
            }

            ui->toolButtonTextLeft->blockSignals(false);
            ui->toolButtonTextCenter->blockSignals(false);
            ui->toolButtonTextRight->blockSignals(false);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEditLabel::NewTemplate()
{
    if (ui->listWidgetEdit->count() > 0)
    {
        const QMessageBox::StandardButton answer =
            QMessageBox::question(this, tr("Create new template"),
                                  tr("Creating new template will overwrite the current, do "
                                     "you want to continue?"),
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (answer == QMessageBox::No)
        {
            return;
        }
    }

    ui->listWidgetEdit->blockSignals(true);
    ui->listWidgetEdit->clear();
    ui->listWidgetEdit->blockSignals(false);
    ShowLineDetails();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEditLabel::ExportTemplate()
{
    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();

    QString filters(tr("Label template") + "(*.xml)"_L1);
    const QString path = settings->GetPathLabelTemplate();

    QString fileName =
        QFileDialog::getSaveFileName(this, tr("Export label template"), path + '/'_L1 + tr("template") + ".xml"_L1,
                                     filters, nullptr, VAbstractApplication::VApp()->NativeFileDialog());

    if (fileName.isEmpty())
    {
        return;
    }

    QFileInfo f(fileName);
    if (f.suffix().isEmpty() && f.suffix() != "xml"_L1)
    {
        fileName += ".xml"_L1;
    }

    settings->SetPathLabelTemplate(QFileInfo(fileName).absolutePath());

    VLabelTemplate ltemplate;
    ltemplate.CreateEmptyTemplate();
    ltemplate.AddLines(GetTemplate());

    QString error;
    const bool result = ltemplate.SaveDocument(fileName, error);
    if (not result)
    {
        QMessageBox messageBox(this);
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.setInformativeText(tr("Could not save file"));
        messageBox.setDefaultButton(QMessageBox::Ok);
        messageBox.setDetailedText(error);
        messageBox.setStandardButtons(QMessageBox::Ok);
        messageBox.exec();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEditLabel::ImportTemplate()
{
    if (ui->listWidgetEdit->count() > 0)
    {
        const QMessageBox::StandardButton answer =
            QMessageBox::question(this, tr("Import template"),
                                  tr("Import template will overwrite the current, do "
                                     "you want to continue?"),
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (answer == QMessageBox::No)
        {
            return;
        }
    }

    QString filter(tr("Label template") + " (*.xml)"_L1);
    // Use standard path to label templates
    const QString path = VAbstractValApplication::VApp()->ValentinaSettings()->GetPathLabelTemplate();
    const QString fileName = QFileDialog::getOpenFileName(this, tr("Import template"), path, filter, nullptr,
                                                          VAbstractApplication::VApp()->NativeFileDialog());
    if (fileName.isEmpty())
    {
        return;
    }

    try
    {
        VLabelTemplate ltemplate;
        ltemplate.setXMLContent(VLabelTemplateConverter(fileName).Convert());
        SetTemplate(ltemplate.ReadLines());
    }
    catch (VException &e)
    {
        qCritical("%s\n\n%s\n\n%s", qUtf8Printable(tr("File error.")), qUtf8Printable(e.ErrorMessage()),
                  qUtf8Printable(e.DetailedInformation()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEditLabel::InsertPlaceholder()
{
    auto *action = qobject_cast<QAction *>(sender());
    if (action)
    {
        ui->lineEditLine->insert(action->data().toString());
        ui->lineEditLine->setFocus();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEditLabel::TabChanged(int index)
{
    if (index == ui->tabWidget->indexOf(ui->tabPreview))
    {
        ui->toolButtonNewLabel->setDisabled(true);
        ui->toolButtonImportLabel->setDisabled(true);
        InitPreviewLines(GetTemplate());
    }
    else
    {
        ui->toolButtonNewLabel->setEnabled(ui->listWidgetEdit->count() > 0);
        ui->toolButtonImportLabel->setEnabled(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEditLabel::SaveAdditionalFontSize(int i)
{
    QListWidgetItem *curLine = ui->listWidgetEdit->currentItem();
    if (curLine)
    {
        QFont lineFont = curLine->font();
        lineFont.setPointSize(qMax(lineFont.pointSize() - curLine->data(Qt::UserRole).toInt() + i, 1));
        curLine->setFont(lineFont);
        curLine->setData(Qt::UserRole, i);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEditLabel::SetupControls()
{
    const bool enabled = ui->listWidgetEdit->count() > 0;

    if (not enabled)
    {
        ui->lineEditLine->blockSignals(true);
        ui->lineEditLine->clear();
        ui->lineEditLine->blockSignals(false);
    }

    ui->toolButtonAdd->setEnabled(true);
    ui->toolButtonImportLabel->setEnabled(true);

    ui->toolButtonRemove->setEnabled(enabled);
    ui->toolButtonBold->setEnabled(enabled);
    ui->toolButtonItalic->setEnabled(enabled);
    ui->toolButtonTextLeft->setEnabled(enabled);
    ui->toolButtonTextCenter->setEnabled(enabled);
    ui->toolButtonTextRight->setEnabled(enabled);
    ui->pushButtonInsert->setEnabled(enabled);
    ui->toolButtonNewLabel->setEnabled(enabled);
    ui->toolButtonExportLabel->setEnabled(enabled);
    ui->lineEditLine->setEnabled(enabled);
    ui->spinBoxFontSize->setEnabled(enabled);

    ui->toolButtonTop->setEnabled(false);
    ui->toolButtonUp->setEnabled(false);
    ui->toolButtonDown->setEnabled(false);
    ui->toolButtonBottom->setEnabled(false);

    if (ui->listWidgetEdit->count() >= 2)
    {
        if (ui->listWidgetEdit->currentRow() == 0)
        {
            ui->toolButtonDown->setEnabled(true);
            ui->toolButtonBottom->setEnabled(true);
        }
        else if (ui->listWidgetEdit->currentRow() == ui->listWidgetEdit->count() - 1)
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
auto DialogEditLabel::SortedActions() const -> QMap<QString, QString>
{
    QMap<QString, QString> sortedActions;
    QChar per('%');
    auto i = m_placeholders.constBegin();
    while (i != m_placeholders.constEnd())
    {
        sortedActions.insert(i.value().first, per + i.key() + per);
        ++i;
    }
    return sortedActions;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEditLabel::InitPlaceholdersMenu()
{
    QMap<QString, QString> sortedActions = SortedActions();
    auto i = sortedActions.constBegin();
    while (i != sortedActions.constEnd())
    {
        QAction *action = m_placeholdersMenu->addAction(i.key());
        action->setData(i.value());
        action->setToolTip(i.value());
        connect(action, &QAction::triggered, this, &DialogEditLabel::InsertPlaceholder);
        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEditLabel::InitPlaceholders()
{
    // Pattern tags
    QLocale locale(VAbstractApplication::VApp()->Settings()->GetLocale());

    const QString date = locale.toString(QDate::currentDate(), m_doc->GetLabelDateFormat());
    m_placeholders.insert(pl_date, qMakePair(tr("Date"), date));

    const QString time = locale.toString(QTime::currentTime(), m_doc->GetLabelTimeFormat());
    m_placeholders.insert(pl_time, qMakePair(tr("Time"), time));

    m_placeholders.insert(pl_patternName, qMakePair(tr("Pattern name"), m_doc->GetPatternName()));
    m_placeholders.insert(pl_patternNumber, qMakePair(tr("Pattern number"), m_doc->GetPatternNumber()));
    m_placeholders.insert(pl_author, qMakePair(tr("Company name or designer name"), m_doc->GetCompanyName()));

    m_placeholders.insert(pl_mUnits, qMakePair(tr("Measurements units"),
                                               UnitsToStr(VAbstractValApplication::VApp()->MeasurementsUnits(), true)));

    const QString pUnits = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
    m_placeholders.insert(pl_pUnits, qMakePair(tr("Pattern units"), pUnits));
    m_placeholders.insert(
        pl_mSizeUnits,
        qMakePair(tr("Size units"), UnitsToStr(VAbstractValApplication::VApp()->DimensionSizeUnits(), true)));
    m_placeholders.insert(pl_areaUnits, qMakePair(tr("Area units"), pUnits + QStringLiteral("²")));

    if (VAbstractValApplication::VApp()->GetMeasurementsType() == MeasurementsType::Individual)
    {
        m_placeholders.insert(pl_customer,
                              qMakePair(tr("Customer name"), VAbstractValApplication::VApp()->GetCustomerName()));

        const QString birthDate =
            locale.toString(VAbstractValApplication::VApp()->GetCustomerBirthDate(), m_doc->GetLabelDateFormat());
        m_placeholders.insert(pl_birthDate, qMakePair(tr("Customer birth date"), birthDate));

        m_placeholders.insert(pl_email,
                              qMakePair(tr("Customer email"), VAbstractValApplication::VApp()->CustomerEmail()));
    }
    else
    {
        m_placeholders.insert(pl_customer, qMakePair(tr("Customer name"), m_doc->GetCustomerName()));

        const QString birthDate = locale.toString(m_doc->GetCustomerBirthDate(), m_doc->GetLabelDateFormat());
        m_placeholders.insert(pl_birthDate, qMakePair(tr("Customer birth date"), birthDate));

        m_placeholders.insert(pl_email, qMakePair(tr("Customer email"), m_doc->GetCustomerEmail()));
    }

    m_placeholders.insert(pl_pExt, qMakePair(tr("Pattern extension"), QStringLiteral("val")));

    const QString patternFilePath = QFileInfo(VAbstractValApplication::VApp()->GetPatternPath()).baseName();
    m_placeholders.insert(pl_pFileName, qMakePair(tr("Pattern file name"), patternFilePath));

    const QString measurementsFilePath = QFileInfo(m_doc->MPath()).baseName();
    m_placeholders.insert(pl_mFileName, qMakePair(tr("Measurments file name"), measurementsFilePath));

    QString heightValue = QString::number(VAbstractValApplication::VApp()->GetDimensionHeight());
    m_placeholders.insert(pl_height, qMakePair(tr("Height", "dimension"), heightValue));
    m_placeholders.insert(pl_dimensionX, qMakePair(tr("Dimension X", "dimension"), heightValue));

    QString sizeValue = QString::number(VAbstractValApplication::VApp()->GetDimensionSize());
    m_placeholders.insert(pl_size, qMakePair(tr("Size", "dimension"), sizeValue));
    m_placeholders.insert(pl_dimensionY, qMakePair(tr("Dimension Y", "dimension"), sizeValue));

    QString hipValue = QString::number(VAbstractValApplication::VApp()->GetDimensionHip());
    m_placeholders.insert(pl_hip, qMakePair(tr("Hip", "dimension"), hipValue));
    m_placeholders.insert(pl_dimensionZ, qMakePair(tr("Dimension Z", "dimension"), hipValue));

    QString waistValue = QString::number(VAbstractValApplication::VApp()->GetDimensionWaist());
    m_placeholders.insert(pl_waist, qMakePair(tr("Waist", "dimension"), waistValue));
    m_placeholders.insert(pl_dimensionW, qMakePair(tr("Dimension W", "dimension"), waistValue));

    {
        QString label = VAbstractValApplication::VApp()->GetDimensionHeightLabel();
        m_placeholders.insert(pl_heightLabel,
                              qMakePair(tr("Height label", "dimension"), not label.isEmpty() ? label : heightValue));
        m_placeholders.insert(
            pl_dimensionX, qMakePair(tr("Dimension X label", "dimension"), not label.isEmpty() ? label : heightValue));

        label = VAbstractValApplication::VApp()->GetDimensionSizeLabel();
        m_placeholders.insert(pl_sizeLabel,
                              qMakePair(tr("Size label", "dimension"), not label.isEmpty() ? label : sizeValue));
        m_placeholders.insert(pl_dimensionY,
                              qMakePair(tr("Dimension Y label", "dimension"), not label.isEmpty() ? label : sizeValue));

        label = VAbstractValApplication::VApp()->GetDimensionHipLabel();
        m_placeholders.insert(pl_hipLabel,
                              qMakePair(tr("Hip label", "dimension"), not label.isEmpty() ? label : hipValue));
        m_placeholders.insert(pl_dimensionZ,
                              qMakePair(tr("Dimension Z label", "dimension"), not label.isEmpty() ? label : hipValue));

        label = VAbstractValApplication::VApp()->GetDimensionWaistLabel();
        m_placeholders.insert(pl_waistLabel,
                              qMakePair(tr("Waist label", "dimension"), not label.isEmpty() ? label : waistValue));
        m_placeholders.insert(
            pl_dimensionW, qMakePair(tr("Dimension W label", "dimension"), not label.isEmpty() ? label : waistValue));
    }

    m_placeholders.insert(
        pl_mExt, qMakePair(tr("Measurments extension"),
                           VAbstractValApplication::VApp()->GetMeasurementsType() == MeasurementsType::Multisize
                               ? QStringLiteral("vst")
                               : QStringLiteral("vit")));

    const QString userMaterialStr = tr("User material");
    const QMap<int, QString> materials = m_doc->GetPatternMaterials();
    for (int i = 0; i < userMaterialPlaceholdersQuantity; ++i)
    {
        const QString number = QString::number(i + 1);
        QString materialDescription;

        QString value;
        if (materials.contains(i + 1))
        {
            value = materials.value(i + 1);
            materialDescription = QStringLiteral("%1 %2 <%3>").arg(userMaterialStr, number, value);
        }
        else
        {
            materialDescription = QStringLiteral("%1 %2").arg(userMaterialStr, number);
        }

        m_placeholders.insert(pl_userMaterial + number, qMakePair(materialDescription, value));
    }

    {
        const QMap<QString, QSharedPointer<VMeasurement>> measurements = m_data->DataMeasurements();
        auto i = measurements.constBegin();
        while (i != measurements.constEnd())
        {
            QString description = i.value()->GetGuiText().isEmpty() ? i.key() : i.value()->GetGuiText();
            m_placeholders.insert(pl_measurement + i.key(), qMakePair(tr("Measurement: %1").arg(description),
                                                                      QString::number(*i.value()->GetValue())));
            ++i;
        }
    }

    {
        const QVector<VFinalMeasurement> measurements = m_doc->GetFinalMeasurements();
        VContainer completeData = m_doc->GetCompleteData();
        completeData.FillPiecesAreas(VAbstractValApplication::VApp()->patternUnits());

        m_placeholders.insert(pl_currentArea, qMakePair(tr("Piece full area"), QString()));
        m_placeholders.insert(pl_currentSeamLineArea, qMakePair(tr("Piece seam line area"), QString()));

        for (int i = 0; i < measurements.size(); ++i)
        {
            const VFinalMeasurement &m = measurements.at(i);

            try
            {
                QScopedPointer<Calculator> cal(new Calculator());
                const qreal result = cal->EvalFormula(completeData.DataVariables(), m.formula);

                m_placeholders.insert(pl_finalMeasurement + m.name,
                                      qMakePair(tr("Final measurement: %1").arg(m.name), QString::number(result)));
            }
            catch (qmu::QmuParserError &e)
            {
                const QString errorMsg = QObject::tr("Failed to prepare final measurement placeholder. Parser error at "
                                                     "line %1: %2.")
                                             .arg(i + 1)
                                             .arg(e.GetMsg());
                VAbstractApplication::VApp()->IsPedantic()
                    ? throw VException(errorMsg)
                    : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
            }
        }
    }

    QSharedPointer<QTranslator> phTr = VAbstractValApplication::VApp()->GetPlaceholderTranslator();

    // Piece tags
    m_placeholders.insert(pl_pLetter, qMakePair(tr("Piece letter"), QString()));
    m_placeholders.insert(pl_pAnnotation, qMakePair(tr("Piece annotation"), QString()));
    m_placeholders.insert(pl_pOrientation, qMakePair(tr("Piece orientation"), QString()));
    m_placeholders.insert(pl_pRotation, qMakePair(tr("Piece rotation"), QString()));
    m_placeholders.insert(pl_pTilt, qMakePair(tr("Piece tilt"), QString()));
    m_placeholders.insert(pl_pFoldPosition, qMakePair(tr("Piece fold position"), QString()));
    m_placeholders.insert(pl_pName, qMakePair(tr("Piece name"), QString()));
    m_placeholders.insert(pl_pQuantity, qMakePair(tr("Quantity"), QString()));
    m_placeholders.insert(pl_mFabric, qMakePair(tr("Material: Fabric"), phTr->translate("Placeholder", "Fabric")));
    m_placeholders.insert(pl_mLining, qMakePair(tr("Material: Lining"), phTr->translate("Placeholder", "Lining")));
    m_placeholders.insert(pl_mInterfacing,
                          qMakePair(tr("Material: Interfacing"), phTr->translate("Placeholder", "Interfacing")));
    m_placeholders.insert(pl_mInterlining,
                          qMakePair(tr("Material: Interlining"), phTr->translate("Placeholder", "Interlining")));
    m_placeholders.insert(pl_wCut, qMakePair(tr("Word: Cut"), phTr->translate("Placeholder", "Cut")));
    m_placeholders.insert(pl_wOnFold, qMakePair(tr("Word: on fold"), QString())); // By default should be empty
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogEditLabel::ReplacePlaceholders(QString line) const -> QString
{
    QChar per('%');

    auto TestDimension = [per, this, line](const QString &placeholder, const QString &errorMsg)
    {
        if (line.contains(per + placeholder + per) && m_placeholders.value(placeholder).second == '0'_L1)
        {
            VAbstractApplication::VApp()->IsPedantic()
                ? throw VException(errorMsg)
                : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        }
    };

    TestDimension(pl_height, tr("No data for the height dimension."));
    TestDimension(pl_size, tr("No data for the size dimension."));
    TestDimension(pl_hip, tr("No data for the hip dimension."));
    TestDimension(pl_waist, tr("No data for the waist dimension."));

    TestDimension(pl_dimensionX, tr("No data for the X dimension."));
    TestDimension(pl_dimensionY, tr("No data for the Y dimension."));
    TestDimension(pl_dimensionZ, tr("No data for the Z dimension."));
    TestDimension(pl_dimensionW, tr("No data for the W dimension."));

    auto i = m_placeholders.constBegin();
    while (i != m_placeholders.constEnd())
    {
        line.replace(per + i.key() + per, i.value().second);
        ++i;
    }
    return line;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogEditLabel::GetTemplate() const -> QVector<VLabelTemplateLine>
{
    QVector<VLabelTemplateLine> lines;
    lines.reserve(ui->listWidgetEdit->count());

    for (int i = 0; i < ui->listWidgetEdit->count(); ++i)
    {
        const QListWidgetItem *lineItem = ui->listWidgetEdit->item(i);
        if (lineItem)
        {
            VLabelTemplateLine line;
            line.line = lineItem->text();
            line.alignment = lineItem->textAlignment();
            line.fontSizeIncrement = lineItem->data(Qt::UserRole).toInt();

            const QFont font = lineItem->font();
            line.bold = font.bold();
            line.italic = font.italic();

            lines.append(line);
        }
    }

    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEditLabel::SetTemplate(const QVector<VLabelTemplateLine> &lines)
{
    ui->listWidgetEdit->blockSignals(true);
    ui->listWidgetEdit->clear();

    int row = -1;

    for (const auto &line : lines)
    {
        auto *item = new QListWidgetItem(line.line);
        item->setTextAlignment(line.alignment);
        item->setData(Qt::UserRole, line.fontSizeIncrement);

        QFont font = item->font();
        font.setBold(line.bold);
        font.setItalic(line.italic);
        font.setPointSize(qMax(font.pointSize() + line.fontSizeIncrement, 1));
        item->setFont(font);

        ui->listWidgetEdit->insertItem(++row, item);
    }

    ui->listWidgetEdit->blockSignals(false);

    if (ui->listWidgetEdit->count() > 0)
    {
        ui->listWidgetEdit->setCurrentRow(0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEditLabel::SetPiece(const VPiece &piece)
{
    const VPieceLabelData &pieceData = piece.GetPieceLabelData();
    m_placeholders[pl_pLetter].second = pieceData.GetLetter();
    m_placeholders[pl_pAnnotation].second = pieceData.GetAnnotation();
    m_placeholders[pl_pOrientation].second = pieceData.GetOrientation();
    m_placeholders[pl_pRotation].second = pieceData.GetRotationWay();
    m_placeholders[pl_pTilt].second = pieceData.GetTilt();
    m_placeholders[pl_pFoldPosition].second = pieceData.GetFoldPosition();
    m_placeholders[pl_pName].second = piece.GetName();
    m_placeholders[pl_pQuantity].second = QString::number(pieceData.GetQuantity());
    if (pieceData.IsOnFold())
    {
        QSharedPointer<QTranslator> phTr = VAbstractValApplication::VApp()->GetPlaceholderTranslator();
        m_placeholders[pl_wOnFold].second = phTr->translate("Placeholder", "on fold");
    }

    VContainer completeData = m_doc->GetCompleteData();
    completeData.FillPiecesAreas(VAbstractValApplication::VApp()->patternUnits());

    QScopedPointer<Calculator> cal(new Calculator());

    try
    {
        const QString formula = pieceArea_ + VPieceArea::PieceShortName(piece);
        const qreal result = cal->EvalFormula(completeData.DataVariables(), formula);
        m_placeholders[pl_currentArea].second = QString::number(result);
    }
    catch (qmu::QmuParserError &e)
    {
        const QString errorMsg = QObject::tr("Failed to prepare full piece area placeholder. %1.").arg(e.GetMsg());
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
    }

    try
    {
        const QString formula = pieceSeamLineArea_ + VPieceArea::PieceShortName(piece);
        const qreal result = cal->EvalFormula(completeData.DataVariables(), formula);
        m_placeholders[pl_currentSeamLineArea].second = QString::number(result);
    }
    catch (qmu::QmuParserError &e)
    {
        const QString errorMsg = QObject::tr("Failed to prepare piece seam line area placeholder. %1.").arg(e.GetMsg());
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEditLabel::InitPreviewLines(const QVector<VLabelTemplateLine> &lines)
{
    ui->listWidgetPreview->clear();

    int row = -1;

    for (const auto &line : lines)
    {
        auto *item = new QListWidgetItem(ReplacePlaceholders(line.line));
        item->setTextAlignment(line.alignment);
        item->setData(Qt::UserRole, line.fontSizeIncrement);

        QFont font = item->font();
        font.setBold(line.bold);
        font.setItalic(line.italic);
        font.setPointSize(qMax(font.pointSize() + line.fontSizeIncrement, 1));
        item->setFont(font);

        ui->listWidgetPreview->insertItem(++row, item);
    }

    if (ui->listWidgetPreview->count() > 0)
    {
        ui->listWidgetPreview->setCurrentRow(0);
    }
}
