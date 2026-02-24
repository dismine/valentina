/************************************************************************
 **
 **  @file   puzzlepreferenceslayoutpage.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 8, 2021
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2021 Valentina project
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
#include "puzzlepreferenceslayoutpage.h"
#include "../../vpapplication.h"
#include "../vmisc/compatibility.h"
#include "ui_puzzlepreferenceslayoutpage.h"

//---------------------------------------------------------------------------------------------------------------------
PuzzlePreferencesLayoutPage::PuzzlePreferencesLayoutPage(QWidget *parent)
  : QWidget(parent),
    ui(std::make_unique<Ui::PuzzlePreferencesLayoutPage>())
{
    ui->setupUi(this);

    VPApplication::VApp()->PuzzleSettings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    InitLayoutUnits();
    VAbstractLayoutDialog::InitTemplates(ui->comboBoxSheetTemplates);
    VAbstractLayoutDialog::InitTileTemplates(ui->comboBoxTileTemplates);
    MinimumSheetPaperSize();
    MinimumTilePaperSize();

    ReadSettings();

    connect(ui->comboBoxLayoutUnit, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &PuzzlePreferencesLayoutPage::ConvertPaperSize);

    connect(ui->comboBoxSheetTemplates, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this]
            {
                SheetSize(SheetTemplate());
                m_settingsChanged = true;
            });
    connect(ui->comboBoxTileTemplates, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this]
            {
                TileSize(TileTemplate());
                m_settingsChanged = true;
            });

    connect(ui->doubleSpinBoxSheetPaperWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzlePreferencesLayoutPage::SheetPaperSizeChanged);
    connect(ui->doubleSpinBoxSheetPaperHeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzlePreferencesLayoutPage::SheetPaperSizeChanged);
    connect(ui->doubleSpinBoxTilePaperWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzlePreferencesLayoutPage::TilePaperSizeChanged);
    connect(ui->doubleSpinBoxTilePaperHeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzlePreferencesLayoutPage::TilePaperSizeChanged);

    connect(ui->doubleSpinBoxSheetPaperWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzlePreferencesLayoutPage::FindSheetTemplate);
    connect(ui->doubleSpinBoxSheetPaperHeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzlePreferencesLayoutPage::FindSheetTemplate);
    connect(ui->doubleSpinBoxTilePaperWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzlePreferencesLayoutPage::FindTileTemplate);
    connect(ui->doubleSpinBoxTilePaperHeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzlePreferencesLayoutPage::FindTileTemplate);

    connect(ui->doubleSpinBoxSheetPaperWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzlePreferencesLayoutPage::CorrectMaxMargins);
    connect(ui->doubleSpinBoxSheetPaperHeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzlePreferencesLayoutPage::CorrectMaxMargins);
    connect(ui->doubleSpinBoxTilePaperWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzlePreferencesLayoutPage::CorrectMaxMargins);
    connect(ui->doubleSpinBoxTilePaperHeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzlePreferencesLayoutPage::CorrectMaxMargins);

    connect(ui->checkBoxLayoutIgnoreFileds,
            CHECKBOX_STATE_CHANGED,
            this,
            &PuzzlePreferencesLayoutPage::LayoutSheetIgnoreMargins);
    connect(ui->checkBoxTileIgnoreFileds,
            CHECKBOX_STATE_CHANGED,
            this,
            &PuzzlePreferencesLayoutPage::LayoutTileIgnoreMargins);

    connect(ui->toolButtonSheetPortraitOritation, &QToolButton::toggled, this,
            &PuzzlePreferencesLayoutPage::SwapSheetOrientation);
    connect(ui->toolButtonSheetLandscapeOrientation, &QToolButton::toggled, this,
            &PuzzlePreferencesLayoutPage::SwapSheetOrientation);
    connect(ui->toolButtonTilePortraitOrientation, &QToolButton::toggled, this,
            &PuzzlePreferencesLayoutPage::SwapTileOrientation);
    connect(ui->toolButtonTileLandscapeOrientation, &QToolButton::toggled, this,
            &PuzzlePreferencesLayoutPage::SwapTileOrientation);

    connect(ui->doubleSpinBoxSheetMarginLeft, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            [this]() { m_settingsChanged = true; });
    connect(ui->doubleSpinBoxSheetMarginRight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            [this]() { m_settingsChanged = true; });
    connect(ui->doubleSpinBoxSheetMarginTop, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            [this]() { m_settingsChanged = true; });
    connect(ui->doubleSpinBoxSheetMarginBottom, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            [this]() { m_settingsChanged = true; });

    connect(ui->checkBoxTileShowTiles, CHECKBOX_STATE_CHANGED, this, [this]() { m_settingsChanged = true; });
    connect(ui->checkBoxTileShowWatermark, CHECKBOX_STATE_CHANGED, this, [this]() { m_settingsChanged = true; });
    connect(ui->checkBoxTileShowWatermark, CHECKBOX_STATE_CHANGED, this, [this]() { m_settingsChanged = true; });

    connect(ui->doubleSpinBoxPiecesGap, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            [this]() { m_settingsChanged = true; });
    ui->doubleSpinBoxPiecesGap->setSuffix(UnitsToStr(m_oldLayoutUnit));

    ui->spinBoxLineWidth->setSuffix(UnitsToStr(Unit::Px));

    connect(ui->checkBoxWarningPiecesSuperposition,
            CHECKBOX_STATE_CHANGED,
            this,
            [this]() { m_settingsChanged = true; });
    connect(ui->checkBoxStickyEdges, CHECKBOX_STATE_CHANGED, this, [this]() { m_settingsChanged = true; });
    connect(ui->checkBoxWarningPiecesOutOfBound, CHECKBOX_STATE_CHANGED, this, [this]() { m_settingsChanged = true; });
    connect(ui->checkBoxFollowGrainline, CHECKBOX_STATE_CHANGED, this, [this]() { m_settingsChanged = true; });
    connect(ui->checkBoxWarningPieceGapePosition, CHECKBOX_STATE_CHANGED, this, [this]() { m_settingsChanged = true; });
}

//---------------------------------------------------------------------------------------------------------------------
PuzzlePreferencesLayoutPage::~PuzzlePreferencesLayoutPage() = default;

//---------------------------------------------------------------------------------------------------------------------
auto PuzzlePreferencesLayoutPage::Apply() -> QStringList
{
    QStringList preferences;
    VPSettings *settings = VPApplication::VApp()->PuzzleSettings();

    settings->SetLayoutUnit(LayoutUnit());

    settings->SetLayoutSheetPaperHeight(
        UnitConvertor(ui->doubleSpinBoxSheetPaperHeight->value(), m_oldLayoutUnit, Unit::Px));
    settings->SetLayoutSheetPaperWidth(
        UnitConvertor(ui->doubleSpinBoxSheetPaperWidth->value(), m_oldLayoutUnit, Unit::Px));

    settings->SetLayoutSheetIgnoreMargins(ui->checkBoxLayoutIgnoreFileds->isChecked());
    settings->SetLayoutSheetMargins(GetSheetMargins());

    settings->SetLayoutTileShowTiles(ui->checkBoxTileShowTiles->isChecked());
    settings->SetLayoutTileShowWatermark(ui->checkBoxTileShowWatermark->isChecked());

    settings->SetLayoutTilePaperHeight(
        UnitConvertor(ui->doubleSpinBoxTilePaperHeight->value(), m_oldLayoutUnit, Unit::Px));
    settings->SetLayoutTilePaperWidth(
        UnitConvertor(ui->doubleSpinBoxTilePaperWidth->value(), m_oldLayoutUnit, Unit::Px));

    settings->SetLayoutTileIgnoreMargins(ui->checkBoxTileIgnoreFileds->isChecked());
    settings->SetLayoutTileMargins(GetTileMargins());

    settings->SetLayoutPieceGap(UnitConvertor(ui->doubleSpinBoxPiecesGap->value(), m_oldLayoutUnit, Unit::Px));

    settings->SetLayoutWarningPiecesSuperposition(ui->checkBoxWarningPiecesSuperposition->isChecked());
    settings->SetLayoutStickyEdges(ui->checkBoxStickyEdges->isChecked());
    settings->SetLayoutWarningPiecesOutOfBound(ui->checkBoxWarningPiecesOutOfBound->isChecked());
    settings->SetLayoutFollowGrainline(ui->checkBoxFollowGrainline->isChecked());
    settings->SetLayoutWarningPieceGapePosition(ui->checkBoxWarningPieceGapePosition->isChecked());

    settings->SetLayoutLineWidth(ui->spinBoxLineWidth->value());

    if (m_settingsChanged)
    {
        preferences.append(tr("default layout settings"));
        m_settingsChanged = false;
    }
    return preferences;
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesLayoutPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
    }
    // remember to call base class implementation
    QWidget::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesLayoutPage::ConvertPaperSize()
{
    const Unit layoutUnit = LayoutUnit();

    const qreal sheetWidth = ui->doubleSpinBoxSheetPaperWidth->value();
    const qreal sheetHeight = ui->doubleSpinBoxSheetPaperHeight->value();

    const qreal sheetLeftMargin = ui->doubleSpinBoxSheetMarginLeft->value();
    const qreal sheetRightMargin = ui->doubleSpinBoxSheetMarginRight->value();
    const qreal sheetTopMargin = ui->doubleSpinBoxSheetMarginTop->value();
    const qreal sheetBottomMargin = ui->doubleSpinBoxSheetMarginBottom->value();

    {
        const QSignalBlocker blockerSheetPaperWidth(ui->doubleSpinBoxSheetPaperWidth);
        const QSignalBlocker blockerSheetPaperHeight(ui->doubleSpinBoxSheetPaperHeight);
        ui->doubleSpinBoxSheetPaperWidth->setMaximum(FromPixel(QIMAGE_MAX, layoutUnit));
        ui->doubleSpinBoxSheetPaperHeight->setMaximum(FromPixel(QIMAGE_MAX, layoutUnit));
    }

    const qreal newSheetWidth = UnitConvertor(sheetWidth, m_oldLayoutUnit, layoutUnit);
    const qreal newSheetHeight = UnitConvertor(sheetHeight, m_oldLayoutUnit, layoutUnit);

    const qreal newSheetLeftMargin = UnitConvertor(sheetLeftMargin, m_oldLayoutUnit, layoutUnit);
    const qreal newSheetRightMargin = UnitConvertor(sheetRightMargin, m_oldLayoutUnit, layoutUnit);
    const qreal newSheetTopMargin = UnitConvertor(sheetTopMargin, m_oldLayoutUnit, layoutUnit);
    const qreal newSheetBottomMargin = UnitConvertor(sheetBottomMargin, m_oldLayoutUnit, layoutUnit);

    const qreal tileWidth = ui->doubleSpinBoxTilePaperWidth->value();
    const qreal tileHeight = ui->doubleSpinBoxTilePaperHeight->value();

    const qreal tileLeftMargin = ui->doubleSpinBoxTileMarginLeft->value();
    const qreal tileRightMargin = ui->doubleSpinBoxTileMarginRight->value();
    const qreal tileTopMargin = ui->doubleSpinBoxTileMarginTop->value();
    const qreal tileBottomMargin = ui->doubleSpinBoxTileMarginBottom->value();

    {
        const QSignalBlocker blockerTilePaperWidth(ui->doubleSpinBoxTilePaperWidth);
        const QSignalBlocker blockerTilePaperHeight(ui->doubleSpinBoxTilePaperHeight);
        ui->doubleSpinBoxTilePaperWidth->setMaximum(FromPixel(QIMAGE_MAX, layoutUnit));
        ui->doubleSpinBoxTilePaperHeight->setMaximum(FromPixel(QIMAGE_MAX, layoutUnit));
    }

    const qreal newTileWidth = UnitConvertor(tileWidth, m_oldLayoutUnit, layoutUnit);
    const qreal newTileHeight = UnitConvertor(tileHeight, m_oldLayoutUnit, layoutUnit);

    const qreal newTileLeftMargin = UnitConvertor(tileLeftMargin, m_oldLayoutUnit, layoutUnit);
    const qreal newTileRightMargin = UnitConvertor(tileRightMargin, m_oldLayoutUnit, layoutUnit);
    const qreal newTileTopMargin = UnitConvertor(tileTopMargin, m_oldLayoutUnit, layoutUnit);
    const qreal newTileBottomMargin = UnitConvertor(tileBottomMargin, m_oldLayoutUnit, layoutUnit);

    qreal const newGap = UnitConvertor(ui->doubleSpinBoxPiecesGap->value(), m_oldLayoutUnit, layoutUnit);

    m_oldLayoutUnit = layoutUnit;
    CorrectPaperDecimals();
    MinimumSheetPaperSize();
    MinimumTilePaperSize();

    ui->doubleSpinBoxSheetPaperWidth->setValue(newSheetWidth);
    ui->doubleSpinBoxSheetPaperHeight->setValue(newSheetHeight);

    ui->doubleSpinBoxSheetMarginLeft->setValue(newSheetLeftMargin);
    ui->doubleSpinBoxSheetMarginRight->setValue(newSheetRightMargin);
    ui->doubleSpinBoxSheetMarginTop->setValue(newSheetTopMargin);
    ui->doubleSpinBoxSheetMarginBottom->setValue(newSheetBottomMargin);

    ui->doubleSpinBoxTilePaperWidth->setValue(newTileWidth);
    ui->doubleSpinBoxTilePaperHeight->setValue(newTileHeight);

    ui->doubleSpinBoxTileMarginLeft->setValue(newTileLeftMargin);
    ui->doubleSpinBoxTileMarginRight->setValue(newTileRightMargin);
    ui->doubleSpinBoxTileMarginTop->setValue(newTileTopMargin);
    ui->doubleSpinBoxTileMarginBottom->setValue(newTileBottomMargin);

    ui->doubleSpinBoxPiecesGap->setMaximum(UnitConvertor(VPSettings::GetMaxLayoutPieceGap(), Unit::Cm, layoutUnit));
    ui->doubleSpinBoxPiecesGap->setValue(newGap);
    ui->doubleSpinBoxPiecesGap->setSuffix(UnitsToStr(layoutUnit));

    m_settingsChanged = true;
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesLayoutPage::LayoutSheetIgnoreMargins(int state)
{
    ui->doubleSpinBoxSheetMarginLeft->setDisabled(state != 0);
    ui->doubleSpinBoxSheetMarginRight->setDisabled(state != 0);
    ui->doubleSpinBoxSheetMarginTop->setDisabled(state != 0);
    ui->doubleSpinBoxSheetMarginBottom->setDisabled(state != 0);

    m_settingsChanged = true;
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesLayoutPage::LayoutTileIgnoreMargins(int state)
{
    ui->doubleSpinBoxTileMarginLeft->setDisabled(state != 0);
    ui->doubleSpinBoxTileMarginRight->setDisabled(state != 0);
    ui->doubleSpinBoxTileMarginTop->setDisabled(state != 0);
    ui->doubleSpinBoxTileMarginBottom->setDisabled(state != 0);

    m_settingsChanged = true;
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesLayoutPage::FindSheetTemplate()
{
    const qreal width = ui->doubleSpinBoxSheetPaperWidth->value();
    const qreal height = ui->doubleSpinBoxSheetPaperHeight->value();
    FindTemplate(ui->comboBoxSheetTemplates, width, height);
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesLayoutPage::FindTileTemplate()
{
    const qreal width = ui->doubleSpinBoxTilePaperWidth->value();
    const qreal height = ui->doubleSpinBoxTilePaperHeight->value();
    FindTemplate(ui->comboBoxTileTemplates, width, height);
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesLayoutPage::CorrectMaxMargins()
{
    const qreal sheetWidth = ui->doubleSpinBoxSheetPaperWidth->value();
    const qreal sheetHeight = ui->doubleSpinBoxSheetPaperHeight->value();

    // 80%/2 of paper size for each field
    const qreal sheetWidthMargin = (sheetWidth * 80.0 / 100.0) / 2.0;
    const qreal sheetHeightMargin = (sheetHeight * 80.0 / 100.0) / 2.0;

    ui->doubleSpinBoxSheetMarginLeft->setMaximum(sheetWidthMargin);
    ui->doubleSpinBoxSheetMarginRight->setMaximum(sheetWidthMargin);
    ui->doubleSpinBoxSheetMarginTop->setMaximum(sheetHeightMargin);
    ui->doubleSpinBoxSheetMarginBottom->setMaximum(sheetHeightMargin);

    const qreal tileWidth = ui->doubleSpinBoxTilePaperWidth->value();
    const qreal tileHeight = ui->doubleSpinBoxTilePaperHeight->value();

    // 80%/2 of paper size for each field
    const qreal tileWidthMargin = (tileWidth * 80.0 / 100.0) / 2.0;
    const qreal tileHeightMargin = (tileHeight * 80.0 / 100.0) / 2.0;

    ui->doubleSpinBoxTileMarginLeft->setMaximum(tileWidthMargin);
    ui->doubleSpinBoxTileMarginRight->setMaximum(tileWidthMargin);
    ui->doubleSpinBoxTileMarginTop->setMaximum(tileHeightMargin);
    ui->doubleSpinBoxTileMarginBottom->setMaximum(tileHeightMargin);
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesLayoutPage::SwapSheetOrientation(bool checked)
{
    if (checked)
    {
        {
            const QSignalBlocker blocker(ui->doubleSpinBoxSheetPaperWidth);
            const qreal height = ui->doubleSpinBoxSheetPaperHeight->value();
            ui->doubleSpinBoxSheetPaperWidth->setValue(height);
        }

        {
            const QSignalBlocker blocker(ui->doubleSpinBoxSheetPaperHeight);
            const qreal width = ui->doubleSpinBoxSheetPaperWidth->value();
            ui->doubleSpinBoxSheetPaperHeight->setValue(width);
        }

        SheetPaperSizeChanged();

        m_settingsChanged = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesLayoutPage::SwapTileOrientation(bool checked)
{
    if (checked)
    {
        {
            const QSignalBlocker blocker(ui->doubleSpinBoxTilePaperWidth);
            const qreal height = ui->doubleSpinBoxTilePaperHeight->value();
            ui->doubleSpinBoxTilePaperWidth->setValue(height);
        }

        {
            const QSignalBlocker blocker(ui->doubleSpinBoxTilePaperHeight);
            const qreal width = ui->doubleSpinBoxTilePaperWidth->value();
            ui->doubleSpinBoxTilePaperHeight->setValue(width);
        }

        TilePaperSizeChanged();

        m_settingsChanged = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesLayoutPage::InitLayoutUnits()
{
    ui->comboBoxLayoutUnit->addItem(tr("Millimiters"), QVariant(UnitsToStr(Unit::Mm)));
    ui->comboBoxLayoutUnit->addItem(tr("Centimeters"), QVariant(UnitsToStr(Unit::Cm)));
    ui->comboBoxLayoutUnit->addItem(tr("Inches"), QVariant(UnitsToStr(Unit::Inch)));
    ui->comboBoxLayoutUnit->addItem(tr("Pixels"), QVariant(UnitsToStr(Unit::Px)));

    // set default unit
    VPSettings *settings = VPApplication::VApp()->PuzzleSettings();
    m_oldLayoutUnit = StrToUnits(settings->GetUnit());
    const qint32 indexUnit = ui->comboBoxLayoutUnit->findData(settings->GetUnit());
    if (indexUnit != -1)
    {
        ui->comboBoxLayoutUnit->setCurrentIndex(indexUnit);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto PuzzlePreferencesLayoutPage::Template(VAbstractLayoutDialog::PaperSizeTemplate t) const -> QSizeF
{
    const Unit layoutUnit = LayoutUnit();

    switch (t)
    {
        case VAbstractLayoutDialog::PaperSizeTemplate::A0:
        case VAbstractLayoutDialog::PaperSizeTemplate::A1:
        case VAbstractLayoutDialog::PaperSizeTemplate::A2:
        case VAbstractLayoutDialog::PaperSizeTemplate::A3:
        case VAbstractLayoutDialog::PaperSizeTemplate::A4:
        case VAbstractLayoutDialog::PaperSizeTemplate::Letter:
        case VAbstractLayoutDialog::PaperSizeTemplate::Legal:
        case VAbstractLayoutDialog::PaperSizeTemplate::Tabloid:
        case VAbstractLayoutDialog::PaperSizeTemplate::Roll24in:
        case VAbstractLayoutDialog::PaperSizeTemplate::Roll30in:
        case VAbstractLayoutDialog::PaperSizeTemplate::Roll36in:
        case VAbstractLayoutDialog::PaperSizeTemplate::Roll42in:
        case VAbstractLayoutDialog::PaperSizeTemplate::Roll44in:
        case VAbstractLayoutDialog::PaperSizeTemplate::Roll48in:
        case VAbstractLayoutDialog::PaperSizeTemplate::Roll62in:
        case VAbstractLayoutDialog::PaperSizeTemplate::Roll72in:
        case VAbstractLayoutDialog::PaperSizeTemplate::Custom:
            return VAbstractLayoutDialog::GetTemplateSize(t, layoutUnit);
        default:
            break;
    }
    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto PuzzlePreferencesLayoutPage::SheetTemplate() const -> QSizeF
{
    auto t = static_cast<VAbstractLayoutDialog::PaperSizeTemplate>(ui->comboBoxSheetTemplates->currentData().toInt());
    return Template(t);
}

//---------------------------------------------------------------------------------------------------------------------
auto PuzzlePreferencesLayoutPage::TileTemplate() const -> QSizeF
{
    auto t = static_cast<VAbstractLayoutDialog::PaperSizeTemplate>(ui->comboBoxTileTemplates->currentData().toInt());
    return Template(t);
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesLayoutPage::MinimumSheetPaperSize()
{
    const qreal value = UnitConvertor(1, Unit::Px, m_oldLayoutUnit);
    ui->doubleSpinBoxSheetPaperWidth->setMinimum(value);
    ui->doubleSpinBoxSheetPaperHeight->setMinimum(value);
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesLayoutPage::MinimumTilePaperSize()
{
    const qreal value = UnitConvertor(1, Unit::Px, m_oldLayoutUnit);
    ui->doubleSpinBoxTilePaperWidth->setMinimum(value);
    ui->doubleSpinBoxTilePaperHeight->setMinimum(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto PuzzlePreferencesLayoutPage::LayoutUnit() const -> Unit
{
    return StrToUnits(ui->comboBoxLayoutUnit->currentData().toString());
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesLayoutPage::SetLayoutUnit(Unit unit)
{
    const qint32 indexUnit = ui->comboBoxLayoutUnit->findData(UnitsToStr(unit));
    if (indexUnit != -1)
    {
        ui->comboBoxLayoutUnit->setCurrentIndex(indexUnit);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesLayoutPage::SheetSize(const QSizeF &size)
{
    m_oldLayoutUnit = LayoutUnit();
    ui->doubleSpinBoxSheetPaperWidth->setMaximum(FromPixel(QIMAGE_MAX, m_oldLayoutUnit));
    ui->doubleSpinBoxSheetPaperHeight->setMaximum(FromPixel(QIMAGE_MAX, m_oldLayoutUnit));

    ui->doubleSpinBoxSheetPaperWidth->setValue(size.width());
    ui->doubleSpinBoxSheetPaperHeight->setValue(size.height());

    CorrectPaperDecimals();
    SheetPaperSizeChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesLayoutPage::TileSize(const QSizeF &size)
{
    m_oldLayoutUnit = LayoutUnit();
    ui->doubleSpinBoxTilePaperWidth->setMaximum(FromPixel(QIMAGE_MAX, m_oldLayoutUnit));
    ui->doubleSpinBoxTilePaperHeight->setMaximum(FromPixel(QIMAGE_MAX, m_oldLayoutUnit));

    ui->doubleSpinBoxTilePaperWidth->setValue(size.width());
    ui->doubleSpinBoxTilePaperHeight->setValue(size.height());

    CorrectPaperDecimals();
    TilePaperSizeChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesLayoutPage::CorrectPaperDecimals()
{
    switch (m_oldLayoutUnit)
    {
        case Unit::Cm:
        case Unit::Mm:
        case Unit::Px:
            ui->doubleSpinBoxSheetPaperWidth->setDecimals(2);
            ui->doubleSpinBoxSheetPaperHeight->setDecimals(2);

            ui->doubleSpinBoxTilePaperWidth->setDecimals(2);
            ui->doubleSpinBoxTilePaperHeight->setDecimals(2);

            ui->doubleSpinBoxSheetMarginLeft->setDecimals(4);
            ui->doubleSpinBoxSheetMarginRight->setDecimals(4);
            ui->doubleSpinBoxSheetMarginTop->setDecimals(4);
            ui->doubleSpinBoxSheetMarginBottom->setDecimals(4);

            ui->doubleSpinBoxTileMarginLeft->setDecimals(4);
            ui->doubleSpinBoxTileMarginRight->setDecimals(4);
            ui->doubleSpinBoxTileMarginTop->setDecimals(4);
            ui->doubleSpinBoxTileMarginBottom->setDecimals(4);

            ui->doubleSpinBoxPiecesGap->setDecimals(2);
            break;
        case Unit::Inch:
            ui->doubleSpinBoxSheetPaperWidth->setDecimals(5);
            ui->doubleSpinBoxSheetPaperHeight->setDecimals(5);

            ui->doubleSpinBoxTilePaperWidth->setDecimals(5);
            ui->doubleSpinBoxTilePaperHeight->setDecimals(5);

            ui->doubleSpinBoxSheetMarginLeft->setDecimals(5);
            ui->doubleSpinBoxSheetMarginRight->setDecimals(5);
            ui->doubleSpinBoxSheetMarginTop->setDecimals(5);
            ui->doubleSpinBoxSheetMarginBottom->setDecimals(5);

            ui->doubleSpinBoxTileMarginLeft->setDecimals(5);
            ui->doubleSpinBoxTileMarginRight->setDecimals(5);
            ui->doubleSpinBoxTileMarginTop->setDecimals(5);
            ui->doubleSpinBoxTileMarginBottom->setDecimals(5);

            ui->doubleSpinBoxPiecesGap->setDecimals(5);
            break;
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesLayoutPage::SheetPaperSizeChanged()
{
    bool const portrait = ui->doubleSpinBoxSheetPaperHeight->value() > ui->doubleSpinBoxSheetPaperWidth->value();

    {
        const QSignalBlocker blocker(ui->toolButtonSheetPortraitOritation);
        ui->toolButtonSheetPortraitOritation->setChecked(portrait);
    }

    {
        const QSignalBlocker blocker(ui->toolButtonSheetLandscapeOrientation);
        ui->toolButtonSheetLandscapeOrientation->setChecked(not portrait);
    }

    m_settingsChanged = true;
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesLayoutPage::TilePaperSizeChanged()
{
    bool const portrait = ui->doubleSpinBoxTilePaperHeight->value() > ui->doubleSpinBoxTilePaperWidth->value();

    {
        const QSignalBlocker blocker(ui->toolButtonTilePortraitOrientation);
        ui->toolButtonTilePortraitOrientation->setChecked(portrait);
    }

    {
        const QSignalBlocker blocker(ui->toolButtonTileLandscapeOrientation);
        ui->toolButtonTileLandscapeOrientation->setChecked(not portrait);
    }

    m_settingsChanged = true;
}

//---------------------------------------------------------------------------------------------------------------------
auto PuzzlePreferencesLayoutPage::GetSheetMargins() const -> QMarginsF
{
    QMarginsF fields;
    fields.setLeft(UnitConvertor(ui->doubleSpinBoxSheetMarginLeft->value(), m_oldLayoutUnit, Unit::Px));
    fields.setRight(UnitConvertor(ui->doubleSpinBoxSheetMarginRight->value(), m_oldLayoutUnit, Unit::Px));
    fields.setTop(UnitConvertor(ui->doubleSpinBoxSheetMarginTop->value(), m_oldLayoutUnit, Unit::Px));
    fields.setBottom(UnitConvertor(ui->doubleSpinBoxSheetMarginBottom->value(), m_oldLayoutUnit, Unit::Px));
    return fields;
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesLayoutPage::SetSheetMargins(const QMarginsF &value)
{
    ui->doubleSpinBoxSheetMarginLeft->setValue(UnitConvertor(value.left(), Unit::Px, LayoutUnit()));
    ui->doubleSpinBoxSheetMarginRight->setValue(UnitConvertor(value.right(), Unit::Px, LayoutUnit()));
    ui->doubleSpinBoxSheetMarginTop->setValue(UnitConvertor(value.top(), Unit::Px, LayoutUnit()));
    ui->doubleSpinBoxSheetMarginBottom->setValue(UnitConvertor(value.bottom(), Unit::Px, LayoutUnit()));
}

//---------------------------------------------------------------------------------------------------------------------
auto PuzzlePreferencesLayoutPage::GetTileMargins() const -> QMarginsF
{
    QMarginsF fields;
    fields.setLeft(UnitConvertor(ui->doubleSpinBoxTileMarginLeft->value(), m_oldLayoutUnit, Unit::Px));
    fields.setRight(UnitConvertor(ui->doubleSpinBoxTileMarginRight->value(), m_oldLayoutUnit, Unit::Px));
    fields.setTop(UnitConvertor(ui->doubleSpinBoxTileMarginTop->value(), m_oldLayoutUnit, Unit::Px));
    fields.setBottom(UnitConvertor(ui->doubleSpinBoxTileMarginBottom->value(), m_oldLayoutUnit, Unit::Px));
    return fields;
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesLayoutPage::SetTileMargins(const QMarginsF &value)
{
    ui->doubleSpinBoxTileMarginLeft->setValue(UnitConvertor(value.left(), Unit::Px, LayoutUnit()));
    ui->doubleSpinBoxTileMarginRight->setValue(UnitConvertor(value.right(), Unit::Px, LayoutUnit()));
    ui->doubleSpinBoxTileMarginTop->setValue(UnitConvertor(value.top(), Unit::Px, LayoutUnit()));
    ui->doubleSpinBoxTileMarginBottom->setValue(UnitConvertor(value.bottom(), Unit::Px, LayoutUnit()));
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesLayoutPage::SetPieceGap(qreal gap)
{
    qreal const value = UnitConvertor(gap, Unit::Px, LayoutUnit());
    ui->doubleSpinBoxPiecesGap->setValue(value);
    CorrectPaperDecimals();
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesLayoutPage::ReadSettings()
{
    VPSettings *settings = VPApplication::VApp()->PuzzleSettings();

    SetLayoutUnit(settings->LayoutUnit());
    m_oldLayoutUnit = LayoutUnit();

    const qreal sheetWidth = UnitConvertor(settings->GetLayoutSheetPaperWidth(), Unit::Px, LayoutUnit());
    const qreal sheetHeight = UnitConvertor(settings->GetLayoutSheetPaperHeight(), Unit::Px, LayoutUnit());
    SheetSize(QSizeF(sheetWidth, sheetHeight));

    ui->checkBoxLayoutIgnoreFileds->setChecked(settings->GetLayoutSheetIgnoreMargins());
    SetSheetMargins(settings->GetLayoutSheetMargins());

    const qreal tileWidth = UnitConvertor(settings->GetLayoutTilePaperWidth(), Unit::Px, LayoutUnit());
    const qreal tileHeight = UnitConvertor(settings->GetLayoutTilePaperHeight(), Unit::Px, LayoutUnit());
    TileSize(QSizeF(tileWidth, tileHeight));

    ui->checkBoxTileShowTiles->setChecked(settings->GetLayoutTileShowTiles());
    ui->checkBoxTileShowWatermark->setChecked(settings->GetLayoutTileShowWatermark());
    ui->checkBoxTileIgnoreFileds->setChecked(settings->GetLayoutTileIgnoreMargins());
    SetTileMargins(settings->GetLayoutSheetMargins());

    ui->checkBoxWarningPiecesSuperposition->setChecked(settings->GetLayoutWarningPiecesSuperposition());
    ui->checkBoxStickyEdges->setChecked(settings->GetLayoutStickyEdges());
    ui->checkBoxWarningPiecesOutOfBound->setChecked(settings->GetLayoutWarningPiecesOutOfBound());
    ui->checkBoxFollowGrainline->setChecked(settings->GetLayoutFollowGrainline());
    ui->checkBoxWarningPieceGapePosition->setChecked(settings->GetLayoutWarningPieceGapePosition());

    ui->doubleSpinBoxPiecesGap->setMaximum(UnitConvertor(VPSettings::GetMaxLayoutPieceGap(), Unit::Px, LayoutUnit()));
    SetPieceGap(settings->GetLayoutPieceGap());

    FindSheetTemplate();
    FindTileTemplate();

    SheetPaperSizeChanged();
    TilePaperSizeChanged();

    LayoutSheetIgnoreMargins(static_cast<int>(ui->checkBoxLayoutIgnoreFileds->isChecked()));
    LayoutTileIgnoreMargins(static_cast<int>(ui->checkBoxTileIgnoreFileds->isChecked()));

    ui->spinBoxLineWidth->setValue(settings->GetLayoutLineWidth());

    m_settingsChanged = false;
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesLayoutPage::FindTemplate(QComboBox *box, qreal width, qreal height) const
{
    SCASSERT(box != nullptr)
    const Unit paperUnit = LayoutUnit();

    const auto max = static_cast<int>(VAbstractLayoutDialog::PaperSizeTemplate::Custom);
    for (int i = 0; i < max; ++i)
    {
        const QSizeF tmplSize =
            VAbstractLayoutDialog::GetTemplateSize(static_cast<VAbstractLayoutDialog::PaperSizeTemplate>(i), paperUnit);
        if (VAbstractLayoutDialog::RoundTemplateSize(width, height, paperUnit) == tmplSize ||
            VAbstractLayoutDialog::RoundTemplateSize(height, width, paperUnit) == tmplSize)
        {
            const QSignalBlocker blocker(box);
            if (const int index = box->findData(i); index != -1)
            {
                box->setCurrentIndex(index);
            }
            return;
        }
    }

    const QSignalBlocker blocker(box);
    if (const int index = box->findData(max); index != -1)
    {
        box->setCurrentIndex(index);
    }
}
