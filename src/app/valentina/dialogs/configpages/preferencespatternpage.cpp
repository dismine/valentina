/************************************************************************
 **
 **  @file   preferencespatternpage.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 4, 2017
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

#include "preferencespatternpage.h"
#include "../dialogdatetimeformats.h"
#include "../dialogknownmaterials.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vmisc/vvalentinasettings.h"
#include "ui_preferencespatternpage.h"
#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#include "../vmisc/backport/qoverload.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#include "../ifc/xml/vabstractpattern.h"
#include "../vwidgets/vmaingraphicsview.h"

#include <QDate>
#include <QMessageBox>
#include <QTime>

namespace
{
auto ComboBoxAllStrings(QComboBox *combo) -> QStringList
{
    SCASSERT(combo != nullptr)

    QStringList itemsInComboBox;
    itemsInComboBox.reserve(combo->count());
    for (int index = 0; index < combo->count(); ++index)
    {
        itemsInComboBox << combo->itemText(index);
    }

    return itemsInComboBox;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
PreferencesPatternPage::PreferencesPatternPage(QWidget *parent)
  : QWidget(parent),
    ui(new Ui::PreferencesPatternPage)
{
    ui->setupUi(this);
    RetranslateUi();

    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();

    ui->graphOutputCheck->setChecked(settings->GetGraphicalOutput());
    ui->checkBoxOpenGLRender->setChecked(settings->IsOpenGLRender());
    ui->doubleSpinBoxCurveApproximation->setValue(settings->GetCurveApproximationScale());
    ui->doubleSpinBoxCurveApproximation->setMinimum(minCurveApproximationScale);
    ui->doubleSpinBoxCurveApproximation->setMaximum(maxCurveApproximationScale);
    ui->undoCount->setValue(settings->GetUndoCount());

    //----------------------- Unit setup
    // set default unit
    const Unit defUnit = QLocale().measurementSystem() == QLocale::MetricSystem ? Unit::Mm : Unit::Inch;
    const qint32 indexUnit = ui->comboBoxLineWidthUnit->findData(static_cast<int>(defUnit));
    if (indexUnit != -1)
    {
        ui->comboBoxLineWidthUnit->setCurrentIndex(indexUnit);
    }

    connect(ui->comboBoxLineWidthUnit, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this]()
            {
                const Unit lineUnit = static_cast<Unit>(ui->comboBoxLineWidthUnit->currentData().toInt());
                const qreal value = UnitConvertor(ui->doubleSpinBoxLineWidth->value(), m_oldLineUnit, lineUnit);
                ui->doubleSpinBoxLineWidth->setDecimals(lineUnit == Unit::Mm ? 1 : 6);
                ui->doubleSpinBoxLineWidth->setMinimum(
                    UnitConvertor(ui->doubleSpinBoxLineWidth->minimum(), m_oldLineUnit, lineUnit));
                ui->doubleSpinBoxLineWidth->setMaximum(
                    UnitConvertor(ui->doubleSpinBoxLineWidth->maximum(), m_oldLineUnit, lineUnit));
                ui->doubleSpinBoxLineWidth->setValue(value);
                m_oldLineUnit = lineUnit;
            });

    m_oldLineUnit = static_cast<Unit>(ui->comboBoxLineWidthUnit->currentData().toInt());
    ui->doubleSpinBoxLineWidth->setDecimals(m_oldLineUnit == Unit::Mm ? 1 : 6);
    ui->doubleSpinBoxLineWidth->setMinimum(UnitConvertor(VCommonSettings::MinimalLineWidth(), Unit::Mm, m_oldLineUnit));
    ui->doubleSpinBoxLineWidth->setMaximum(UnitConvertor(VCommonSettings::MaximalLineWidth(), Unit::Mm, m_oldLineUnit));
    ui->doubleSpinBoxLineWidth->setValue(UnitConvertor(settings->GetLineWidth(), Unit::Mm, m_oldLineUnit));

    InitDefaultSeamAllowance();
    InitLabelDateTimeFormats();

    ui->forbidFlippingCheck->setChecked(settings->GetForbidWorkpieceFlipping());
    ui->checkBoxSewLineOnDrawing->setChecked(settings->GetSewLineOnDrawing());
    ui->doublePassmarkCheck->setChecked(settings->IsDoublePassmark());
    ui->checkBoxHideMainPath->setChecked(settings->IsHideMainPath());

    QFont labelFont = settings->GetLabelFont();
    int pointSize = settings->GetPieceLabelFontPointSize();
    labelFont.setPointSize(pointSize);

    ui->fontComboBoxLabelFont->setCurrentFont(labelFont);

    InitLabelFontSizes();
    const qint32 indexSize = ui->comboBoxLabelFontSize->findData(pointSize);
    if (indexSize != -1)
    {
        ui->comboBoxLabelFontSize->setCurrentIndex(indexSize);
    }

    ui->checkBoxRemeberPatternMaterials->setChecked(settings->IsRememberPatternMaterials());
    m_knownMaterials = settings->GetKnownMaterials();

    ui->spinBoxOpacity->setValue(settings->GetBackgroundImageDefOpacity());

    connect(ui->pushButtonKnownMaterials, &QPushButton::clicked, this, &PreferencesPatternPage::ManageKnownMaterials);
}

//---------------------------------------------------------------------------------------------------------------------
PreferencesPatternPage::~PreferencesPatternPage()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto PreferencesPatternPage::Apply() -> QStringList
{
    QStringList preferences;

    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();

    // Scene antialiasing
    if (settings->GetGraphicalOutput() != ui->graphOutputCheck->isChecked())
    {
        if (VAbstractValApplication::VApp()->getSceneView()->IsOpenGLRender())
        {
            preferences.append(tr("antialiasing"));
        }

        settings->SetGraphicalOutput(ui->graphOutputCheck->isChecked());
    }

    if (settings->IsOpenGLRender() != ui->checkBoxOpenGLRender->isChecked())
    {
        preferences.append(tr("scene render"));
        settings->SetOpenGLRender(ui->checkBoxOpenGLRender->isChecked());
    }

    settings->SetCurveApproximationScale(ui->doubleSpinBoxCurveApproximation->value());
    settings->SetLineWidth(UnitConvertor(ui->doubleSpinBoxLineWidth->value(), m_oldLineUnit, Unit::Mm));
    VAbstractValApplication::VApp()->getSceneView()->SetAntialiasing(ui->graphOutputCheck->isChecked());

    /* Maximum number of commands in undo stack may only be set when the undo stack is empty, since setting it on a
     * non-empty stack might delete the command at the current index. Calling setUndoLimit() on a non-empty stack
     * prints a warning and does nothing.*/
    settings->SetUndoCount(ui->undoCount->value());

    settings->SetDefaultSeamAllowance(ui->defaultSeamAllowance->value());

    settings->SetForbidWorkpieceFlipping(ui->forbidFlippingCheck->isChecked());
    settings->SetSewLineOnDrawing(ui->checkBoxSewLineOnDrawing->isChecked());
    settings->SetHideMainPath(ui->checkBoxHideMainPath->isChecked());
    settings->SetLabelFont(ui->fontComboBoxLabelFont->currentFont());
    settings->SetPieceLabelFontPointSize(ui->comboBoxLabelFontSize->currentData().toInt());

    if (settings->IsDoublePassmark() != ui->doublePassmarkCheck->isChecked())
    {
        settings->SetDoublePassmark(ui->doublePassmarkCheck->isChecked());
        VAbstractValApplication::VApp()->getCurrentDocument()->LiteParseTree(Document::LiteParse);
    }

    settings->SetLabelDateFormat(ui->comboBoxDateFormats->currentText());
    settings->SetLabelTimeFormat(ui->comboBoxTimeFormats->currentText());

    settings->SetUserDefinedDateFormats(ComboBoxAllStrings(ui->comboBoxDateFormats));
    settings->SetUserDefinedTimeFormats(ComboBoxAllStrings(ui->comboBoxTimeFormats));

    settings->SetKnownMaterials(m_knownMaterials);
    settings->SetRememberPatternMaterials(ui->checkBoxRemeberPatternMaterials->isChecked());

    settings->SetBackgroundImageDefOpacity(ui->spinBoxOpacity->value());

    return preferences;
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPatternPage::InitDefaultSeamAllowance()
{
    ui->defaultSeamAllowance->setValue(VAbstractValApplication::VApp()->ValentinaSettings()->GetDefaultSeamAllowance());
    ui->defaultSeamAllowance->setSuffix(
        UnitsToStr(StrToUnits(VAbstractValApplication::VApp()->ValentinaSettings()->GetUnit()), true));
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPatternPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        RetranslateUi();
        ui->retranslateUi(this);
    }
    // remember to call base class implementation
    QWidget::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPatternPage::EditDateTimeFormats()
{
    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();

    auto *button = qobject_cast<QPushButton *>(sender());
    if (button == ui->pushButtonEditDateFormats)
    {
        CallDateTimeFormatEditor(QDate::currentDate(), VCommonSettings::PredefinedDateFormats(),
                                 settings->GetUserDefinedDateFormats(), ui->comboBoxDateFormats);
    }
    else if (button == ui->pushButtonEditTimeFormats)
    {
        CallDateTimeFormatEditor(QTime::currentTime(), VCommonSettings::PredefinedTimeFormats(),
                                 settings->GetUserDefinedTimeFormats(), ui->comboBoxTimeFormats);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPatternPage::ManageKnownMaterials()
{
    DialogKnownMaterials editor;
    editor.SetList(m_knownMaterials);

    if (QDialog::Accepted == editor.exec())
    {
        m_knownMaterials = editor.GetList();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPatternPage::InitLabelDateTimeFormats()
{
    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();

    InitComboBoxFormats(ui->comboBoxDateFormats,
                        VCommonSettings::PredefinedDateFormats() + settings->GetUserDefinedDateFormats(),
                        settings->GetLabelDateFormat());
    InitComboBoxFormats(ui->comboBoxTimeFormats,
                        VCommonSettings::PredefinedTimeFormats() + settings->GetUserDefinedTimeFormats(),
                        settings->GetLabelTimeFormat());

    connect(ui->pushButtonEditDateFormats, &QPushButton::clicked, this, &PreferencesPatternPage::EditDateTimeFormats);
    connect(ui->pushButtonEditTimeFormats, &QPushButton::clicked, this, &PreferencesPatternPage::EditDateTimeFormats);
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPatternPage::InitLabelFontSizes()
{
    ui->comboBoxLabelFontSize->clear();

    // Get the available font sizes
    for (auto size : QFontDatabase::standardSizes())
    {
        if (size >= VCommonSettings::MinPieceLabelFontPointSize())
        {
            ui->comboBoxLabelFontSize->addItem(QString::number(size), size);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPatternPage::InitComboBoxFormats(QComboBox *box, const QStringList &items, const QString &currentFormat)
{
    // cppcheck-suppress unknownMacro
    SCASSERT(box != nullptr)

    box->addItems(items);
    int index = box->findText(currentFormat);
    if (index != -1)
    {
        box->setCurrentIndex(index);
    }
    else
    {
        box->setCurrentIndex(0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPatternPage::InitUnits()
{
    ui->comboBoxLineWidthUnit->addItem(tr("Millimiters"), static_cast<int>(Unit::Mm));
    ui->comboBoxLineWidthUnit->addItem(tr("Inches"), static_cast<int>(Unit::Inch));
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPatternPage::RetranslateUi()
{
    ui->comboBoxLineWidthUnit->blockSignals(true);
    const int unit = ui->comboBoxLineWidthUnit->currentData().toInt();
    ui->comboBoxLineWidthUnit->clear();
    InitUnits();
    ui->comboBoxLineWidthUnit->setCurrentIndex(ui->comboBoxLineWidthUnit->findData(unit));
    ui->comboBoxLineWidthUnit->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
void PreferencesPatternPage::CallDateTimeFormatEditor(const T &type, const QStringList &predefinedFormats,
                                                      const QStringList &userDefinedFormats, QComboBox *box)
{
    SCASSERT(box != nullptr)

    DialogDateTimeFormats dialog(type, predefinedFormats, userDefinedFormats);

    if (QDialog::Accepted == dialog.exec())
    {
        const QString currentFormat = box->currentText();
        box->clear();
        box->addItems(dialog.GetFormats());

        int index = box->findText(currentFormat);
        if (index != -1)
        {
            box->setCurrentIndex(index);
        }
        else
        {
            box->setCurrentIndex(0);
        }
    }
}
