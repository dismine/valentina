/************************************************************************
 **
 **  @file   vwidgetbackgroundimages.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 1, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2022 Valentina project
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
#include "vwidgetbackgroundimages.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vbackgroundpatternimage.h"
#include "../vmisc/def.h"
#include "../vmisc/lambdaconstants.h"
#include "../vmisc/theme/themeDef.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vabstractapplication.h"
#include "../vtools/undocommands/image/hideallbackgroundimages.h"
#include "../vtools/undocommands/image/hidebackgroundimage.h"
#include "../vtools/undocommands/image/holdallbackgroundimages.h"
#include "../vtools/undocommands/image/holdbackgroundimage.h"
#include "../vtools/undocommands/image/movebackgroundimage.h"
#include "../vtools/undocommands/image/renamebackgroundimage.h"
#include "../vtools/undocommands/image/resetbackgroundimage.h"
#include "../vtools/undocommands/image/rotatebackgroundimage.h"
#include "../vtools/undocommands/image/scalebackgroundimage.h"
#include "../vtools/undocommands/image/zvaluemovebackgroundimage.h"
#include "../vwidgets/vmousewheelwidgetadjustmentguard.h"
#include "ui_vwidgetbackgroundimages.h"

#include <QMenu>
#include <QPushButton>

namespace
{
enum ImageData
{
    Hold = 0,
    Visibility = 1,
    Name = 2
};

//---------------------------------------------------------------------------------------------------------------------
void SetImageHold(QTableWidgetItem *item, const VBackgroundPatternImage &image)
{
    if (item)
    {
        const auto resource = QStringLiteral("icon");
        (image.Hold()) ? item->setIcon(VTheme::GetIconResource(resource, QStringLiteral("16x16/hold_image.png")))
                       : item->setIcon(VTheme::GetIconResource(resource, QStringLiteral("16x16/not_hold_image.png")));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void SetImageVisibility(QTableWidgetItem *item, const VBackgroundPatternImage &image)
{
    if (item)
    {
        const auto resource = QStringLiteral("icon");
        item->setIcon(image.Visible() ? VTheme::GetIconResource(resource, QStringLiteral("16x16/open_eye.png"))
                                      : VTheme::GetIconResource(resource, QStringLiteral("16x16/closed_eye.png")));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void SetImageName(QTableWidgetItem *item, const VBackgroundPatternImage &image, const QString &def)
{
    QString imageName = def;
    if (not image.Name().isEmpty())
    {
        imageName = image.Name();
    }
    item->setText(imageName);
    item->setToolTip(imageName);
}

//---------------------------------------------------------------------------------------------------------------------
void SetCheckBoxValue(QCheckBox *checkbox, bool value)
{
    const QSignalBlocker blocker(checkbox);
    checkbox->setChecked(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto ScaleUnitConvertor(qreal base, qreal value, ScaleUnit from, ScaleUnit to) -> qreal
{
    auto FromPxTo = [base, to](qreal value)
    {
        switch (to)
        {
            case ScaleUnit::Mm:
                return (value / PrintDPI) * 25.4;
            case ScaleUnit::Cm:
                return ((value / PrintDPI) * 25.4) / 10.0;
            case ScaleUnit::Inch:
                return value / PrintDPI;
            case ScaleUnit::Px:
                return value;
            case ScaleUnit::Percent:
                if (qFuzzyIsNull(base))
                {
                    return 0.0;
                }
                return value / base * 100.;
            default:
                break;
        }

        return 0.0;
    };

    switch (from)
    {
        case ScaleUnit::Percent:
            return FromPxTo(base * (value / 100.));
        case ScaleUnit::Px:
            return FromPxTo(value);
        case ScaleUnit::Mm:
            return FromPxTo((value / 25.4) * PrintDPI);
        case ScaleUnit::Cm:
            return FromPxTo(((value * 10.0) / 25.4) * PrintDPI);
        case ScaleUnit::Inch:
            return FromPxTo(value * PrintDPI);
        default:
            break;
    }

    return 0;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VWidgetBackgroundImages::VWidgetBackgroundImages(VAbstractPattern *doc, QWidget *parent)
  : QWidget(parent),
    ui(new Ui::VWidgetBackgroundImages),
    m_doc(doc)
{
    ui->setupUi(this);

    // Prevent stealing focus when scrolling
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(ui->doubleSpinBoxImageHorizontalTranslate);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(ui->comboBoxTranslateUnit);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(ui->doubleSpinBoxImageVerticalTranslate);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(ui->doubleSpinBoxScaleWidth);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(ui->comboBoxScaleUnit);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(ui->doubleSpinBoxScaleHeight);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(ui->doubleSpinBoxRotationAngle);

    SCASSERT(doc != nullptr)

    UpdateImages();

    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(doc, &VAbstractPattern::BackgroundImageHoldChanged, this, &VWidgetBackgroundImages::UpdateImage);
    connect(doc, &VAbstractPattern::BackgroundImageVisibilityChanged, this, &VWidgetBackgroundImages::UpdateImage);
    connect(doc, &VAbstractPattern::BackgroundImageNameChanged, this, &VWidgetBackgroundImages::UpdateImage);
    connect(doc, &VAbstractPattern::BackgroundImagesHoldChanged, this, &VWidgetBackgroundImages::UpdateImages);
    connect(doc, &VAbstractPattern::BackgroundImagesVisibilityChanged, this, &VWidgetBackgroundImages::UpdateImages);
    connect(doc, &VAbstractPattern::BackgroundImagePositionChanged, this,
            &VWidgetBackgroundImages::ImagePositionChanged);
    connect(doc, &VAbstractPattern::BackgroundImageTransformationChanged, this,
            &VWidgetBackgroundImages::ImagePositionChanged);

    connect(ui->tableWidget, &QTableWidget::cellClicked, this, &VWidgetBackgroundImages::ImageHoldChanged);
    connect(ui->tableWidget, &QTableWidget::cellClicked, this, &VWidgetBackgroundImages::ImageVisibilityChanged);
    connect(ui->tableWidget, &QTableWidget::cellChanged, this, &VWidgetBackgroundImages::ImageNameChanged);
    connect(ui->tableWidget, &QTableWidget::customContextMenuRequested, this, &VWidgetBackgroundImages::ContextMenu);
    connect(ui->tableWidget, &QTableWidget::currentCellChanged, this, &VWidgetBackgroundImages::CurrentImageChanged);

    connect(ui->toolButtonTop, &QToolButton::clicked, this, &VWidgetBackgroundImages::MoveImageOnTop);
    connect(ui->toolButtonUp, &QToolButton::clicked, this, &VWidgetBackgroundImages::MoveImageUp);
    connect(ui->toolButtonDown, &QToolButton::clicked, this, &VWidgetBackgroundImages::MoveImageDown);
    connect(ui->toolButtonBottom, &QToolButton::clicked, this, &VWidgetBackgroundImages::MoveImageBottom);

    InitImageTranslation();
}

//---------------------------------------------------------------------------------------------------------------------
VWidgetBackgroundImages::~VWidgetBackgroundImages()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetBackgroundImages::UpdateImages()
{
    FillTable(m_doc->GetBackgroundImages());
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetBackgroundImages::UpdateImage(const QUuid &id)
{
    int const row = ImageRow(id);
    if (row == -1)
    {
        return;
    }

    VBackgroundPatternImage const image = m_doc->GetBackgroundImage(id);
    if (image.IsNull())
    {
        return;
    }

    const QSignalBlocker blocker(ui->tableWidget);

    QTableWidgetItem *item = ui->tableWidget->item(row, ImageData::Hold);
    SetImageHold(item, image);

    item = ui->tableWidget->item(row, ImageData::Visibility);
    SetImageVisibility(item, image);

    item = ui->tableWidget->item(row, ImageData::Name);
    SetImageName(item, image, tr("Background image"));
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetBackgroundImages::ImageSelected(const QUuid &id)
{
    int const row = ImageRow(id);

    {
        const QSignalBlocker blocker(ui->tableWidget);
        ui->tableWidget->setCurrentCell(row, ImageData::Name);
    }

    if (row != -1 && not ui->checkBoxRelativeTranslation->isChecked())
    {
        SetAbsolutePisition(id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetBackgroundImages::ImageHoldChanged(int row, int column)
{
    if (column != ImageData::Hold)
    {
        return;
    }

    QTableWidgetItem *item = ui->tableWidget->item(row, column);
    if (item != nullptr)
    {
        ToggleImageHold(item->data(Qt::UserRole).toUuid());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetBackgroundImages::ImageVisibilityChanged(int row, int column)
{
    if (column != ImageData::Visibility)
    {
        return;
    }

    QTableWidgetItem *item = ui->tableWidget->item(row, column);
    if (item != nullptr)
    {
        ToggleImageVisibility(item->data(Qt::UserRole).toUuid());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetBackgroundImages::ImageNameChanged(int row, int column)
{
    if (column != ImageData::Name)
    {
        return;
    }

    QTableWidgetItem *item = ui->tableWidget->item(row, column);
    if (item != nullptr)
    {
        auto *command = new RenameBackgroundImage(item->data(Qt::UserRole).toUuid(), item->text(), m_doc);
        VAbstractApplication::VApp()->getUndoStack()->push(command);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetBackgroundImages::ContextMenu(const QPoint &pos)
{
    QTableWidgetItem *item = ui->tableWidget->itemAt(pos);
    if (item == nullptr)
    {
        return;
    }

    const int row = item->row();
    item = ui->tableWidget->item(row, 0);
    const QUuid id = item->data(Qt::UserRole).toUuid();
    VBackgroundPatternImage const image = m_doc->GetBackgroundImage(id);
    if (image.IsNull())
    {
        return;
    }

    auto MultipleChangeHoldTo = [this](bool visibility)
    {
        for (int r = 0; r < ui->tableWidget->rowCount(); ++r)
        {
            QTableWidgetItem *rowItem = ui->tableWidget->item(r, ImageData::Visibility);
            if (rowItem && visibility != m_doc->GetBackgroundImage(rowItem->data(Qt::UserRole).toUuid()).Hold())
            {
                return true;
            }
        }

        return false;
    };

    auto MultipleChangeVisibilityTo = [this](bool visibility)
    {
        for (int r = 0; r < ui->tableWidget->rowCount(); ++r)
        {
            QTableWidgetItem *rowItem = ui->tableWidget->item(r, ImageData::Visibility);
            if (rowItem && visibility != m_doc->GetBackgroundImage(rowItem->data(Qt::UserRole).toUuid()).Visible())
            {
                return true;
            }
        }

        return false;
    };

    QMenu menu;

    QAction *holdOption = menu.addAction(tr("Hold"));
    holdOption->setCheckable(true);
    holdOption->setChecked(image.Hold());

    QAction *actionVisible = menu.addAction(tr("Visible"));
    actionVisible->setCheckable(true);
    actionVisible->setChecked(image.Visible());

    QAction *actionReset = menu.addAction(tr("Reset transformation"));
    actionReset->setEnabled(not image.Hold());

    const QAction *actionDelete = menu.addAction(FromTheme(VThemeIcon::EditDelete), tr("Delete"));

    menu.addSeparator();
    QAction *actionHoldAll = menu.addAction(tr("Hold All"));
    actionHoldAll->setEnabled(MultipleChangeHoldTo(true));
    QAction *actionUnholdAll = menu.addAction(tr("Unhold All"));
    actionUnholdAll->setEnabled(MultipleChangeHoldTo(false));

    menu.addSeparator();
    QAction *actionHideAll = menu.addAction(tr("Hide All"));
    actionHideAll->setEnabled(MultipleChangeVisibilityTo(false));
    QAction *actionShowAll = menu.addAction(tr("Show All"));
    actionShowAll->setEnabled(MultipleChangeVisibilityTo(true));

    QAction *selectedAction = menu.exec(ui->tableWidget->viewport()->mapToGlobal(pos));

    if (selectedAction == holdOption)
    {
        ToggleImageHold(id);
    }
    else if (selectedAction == actionVisible)
    {
        ToggleImageVisibility(id);
    }
    else if (selectedAction == actionReset)
    {
        VAbstractApplication::VApp()->getUndoStack()->push(new ResetBackgroundImage(image.Id(), m_doc));
    }
    else if (selectedAction == actionDelete)
    {
        emit DeleteImage(id);
    }
    else if (selectedAction == actionHoldAll)
    {
        auto *command = new HoldAllBackgroundImages(true, m_doc);
        VAbstractApplication::VApp()->getUndoStack()->push(command);
    }
    else if (selectedAction == actionUnholdAll)
    {
        auto *command = new HoldAllBackgroundImages(false, m_doc);
        VAbstractApplication::VApp()->getUndoStack()->push(command);
    }
    else if (selectedAction == actionHideAll)
    {
        auto *command = new HideAllBackgroundImages(true, m_doc);
        VAbstractApplication::VApp()->getUndoStack()->push(command);
    }
    else if (selectedAction == actionShowAll)
    {
        auto *command = new HideAllBackgroundImages(false, m_doc);
        VAbstractApplication::VApp()->getUndoStack()->push(command);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetBackgroundImages::CurrentImageChanged(int currentRow, int currentColumn, int previousRow,
                                                  int previousColumn)
{
    Q_UNUSED(currentColumn)
    Q_UNUSED(previousColumn)
    if (previousRow != currentRow)
    {
        QTableWidgetItem *item = ui->tableWidget->item(currentRow, 0);
        if (item != nullptr)
        {
            emit SelectImage(item->data(Qt::UserRole).toUuid());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetBackgroundImages::MoveImageOnTop()
{
    int const row = ui->tableWidget->currentRow();
    if (row == -1)
    {
        return;
    }

    QTableWidgetItem *item = ui->tableWidget->item(row, 0);
    if (item != nullptr)
    {
        QUuid const id = item->data(Qt::UserRole).toUuid();
        auto *command = new ZValueMoveBackgroundImage(id, ZValueMoveType::Top, m_doc);
        VAbstractApplication::VApp()->getUndoStack()->push(command);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetBackgroundImages::MoveImageUp()
{
    int const row = ui->tableWidget->currentRow();
    if (row == -1)
    {
        return;
    }

    QTableWidgetItem *item = ui->tableWidget->item(row, 0);
    if (item != nullptr)
    {
        QUuid const id = item->data(Qt::UserRole).toUuid();
        auto *command = new ZValueMoveBackgroundImage(id, ZValueMoveType::Up, m_doc);
        VAbstractApplication::VApp()->getUndoStack()->push(command);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetBackgroundImages::MoveImageDown()
{
    int const row = ui->tableWidget->currentRow();
    if (row == -1)
    {
        return;
    }

    QTableWidgetItem *item = ui->tableWidget->item(row, 0);
    if (item != nullptr)
    {
        QUuid const id = item->data(Qt::UserRole).toUuid();
        auto *command = new ZValueMoveBackgroundImage(id, ZValueMoveType::Down, m_doc);
        VAbstractApplication::VApp()->getUndoStack()->push(command);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetBackgroundImages::MoveImageBottom()
{
    int const row = ui->tableWidget->currentRow();
    if (row == -1)
    {
        return;
    }

    QTableWidgetItem *item = ui->tableWidget->item(row, 0);
    if (item != nullptr)
    {
        QUuid const id = item->data(Qt::UserRole).toUuid();
        auto *command = new ZValueMoveBackgroundImage(id, ZValueMoveType::Bottom, m_doc);
        VAbstractApplication::VApp()->getUndoStack()->push(command);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetBackgroundImages::ApplyImageTransformation()
{
    int const row = ui->tableWidget->currentRow();
    if (row == -1)
    {
        return;
    }

    QTableWidgetItem *item = ui->tableWidget->item(row, 0);
    if (item == nullptr)
    {
        return;
    }

    QUuid const id = item->data(Qt::UserRole).toUuid();
    VBackgroundPatternImage const image = m_doc->GetBackgroundImage(id);

    if (image.IsNull() || image.Hold())
    {
        return;
    }

    const int index = ui->tabWidgetImageTransformation->currentIndex();
    if (ui->tabWidgetImageTransformation->indexOf(ui->tabTranslate) == index)
    { // translate
        qreal dx = UnitConvertor(ui->doubleSpinBoxImageHorizontalTranslate->value(), CurrentTranslateUnit(), Unit::Px);
        qreal dy = UnitConvertor(ui->doubleSpinBoxImageVerticalTranslate->value(), CurrentTranslateUnit(), Unit::Px);

        if (not ui->checkBoxRelativeTranslation->isChecked())
        {
            QRectF const rect = image.BoundingRect();
            dx = dx - rect.topLeft().x();
            dy = dy - rect.topLeft().y();
        }

        auto *command = new MoveBackgroundImage(id, dx, dy, m_doc);
        VAbstractApplication::VApp()->getUndoStack()->push(command);
    }
    else if (ui->tabWidgetImageTransformation->indexOf(ui->tabScale) == index)
    { // scale
        qreal const sx =
            WidthScaleUnitConvertor(ui->doubleSpinBoxScaleWidth->value(), CurrentScaleUnit(), ScaleUnit::Percent) / 100;
        qreal const sy =
            HeightScaleUnitConvertor(ui->doubleSpinBoxScaleHeight->value(), CurrentScaleUnit(), ScaleUnit::Percent) /
            100;

        QTransform imageMatrix = image.Matrix();
        QPointF const originPos = image.BoundingRect().center();

        QTransform m;
        m.translate(originPos.x(), originPos.y());
        m.scale(sx, sy);
        m.translate(-originPos.x(), -originPos.y());
        imageMatrix *= m;

        auto *command = new ScaleBackgroundImage(id, imageMatrix, m_doc);
        VAbstractApplication::VApp()->getUndoStack()->push(command);
    }
    else if (ui->tabWidgetImageTransformation->indexOf(ui->tabRotate) == index)
    { // rotate
        qreal angle = ui->doubleSpinBoxRotationAngle->value();

        if (ui->toolButtonImageRotationClockwise->isChecked())
        {
            angle *= -1;
        }

        QTransform imageMatrix = image.Matrix();

        QPointF const originPos = image.BoundingRect().center();

        QTransform m;
        m.translate(originPos.x(), originPos.y());
        m.rotate(-angle);
        m.translate(-originPos.x(), -originPos.y());
        imageMatrix *= m;

        auto *command = new RotateBackgroundImage(id, imageMatrix, m_doc);
        VAbstractApplication::VApp()->getUndoStack()->push(command);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetBackgroundImages::ResetImageTransformationSettings()
{
    const int index = ui->tabWidgetImageTransformation->currentIndex();
    if (ui->tabWidgetImageTransformation->indexOf(ui->tabTranslate) == index)
    { // translate
        if (ui->checkBoxRelativeTranslation->isChecked())
        {
            ui->doubleSpinBoxImageHorizontalTranslate->setValue(0);
            ui->doubleSpinBoxImageVerticalTranslate->setValue(0);
        }
        else
        {
            if (ui->tableWidget->currentRow() == -1)
            {
                ui->doubleSpinBoxImageHorizontalTranslate->setValue(0);
                ui->doubleSpinBoxImageVerticalTranslate->setValue(0);
            }
        }

        int const unitIndex = ui->comboBoxTranslateUnit->findData(QVariant(static_cast<int>(Unit::Px)));
        if (unitIndex != -1)
        {
            ui->comboBoxTranslateUnit->setCurrentIndex(unitIndex);
        }
    }
    else if (ui->tabWidgetImageTransformation->indexOf(ui->tabScale) == index)
    { // scale
        if (int const unitIndex = ui->comboBoxScaleUnit->findData(QVariant(static_cast<int>(ScaleUnit::Percent)));
            unitIndex != -1)
        {
            ui->comboBoxScaleUnit->setCurrentIndex(unitIndex);
        }

        ui->doubleSpinBoxScaleHeight->setValue(100);
        ui->doubleSpinBoxScaleWidth->setValue(100);
    }
    else if (ui->tabWidgetImageTransformation->indexOf(ui->tabRotate) == index)
    { // rotate
        ui->doubleSpinBoxRotationAngle->setValue(0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetBackgroundImages::RelativeTranslationChanged(bool checked)
{
    if (checked)
    {
        ui->doubleSpinBoxImageHorizontalTranslate->setValue(0);
        ui->doubleSpinBoxImageVerticalTranslate->setValue(0);
    }
    else
    {
        int const row = ui->tableWidget->currentRow();
        if (row == -1)
        {
            return;
        }

        QTableWidgetItem *item = ui->tableWidget->item(row, 0);
        if (item != nullptr)
        {
            QUuid const id = item->data(Qt::UserRole).toUuid();
            SetAbsolutePisition(id);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetBackgroundImages::ScaleProportionallyChanged(bool checked)
{
    if (checked)
    {
        qreal const value = ui->doubleSpinBoxScaleWidth->value();
        ui->doubleSpinBoxScaleHeight->setValue(value);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetBackgroundImages::ScaleWidthChanged(double value)
{
    if (ui->checkBoxScaleProportionally->isChecked())
    {
        ScaleUnit const unit = CurrentScaleUnit();
        if (unit == ScaleUnit::Percent)
        {
            const QSignalBlocker blocker(ui->doubleSpinBoxScaleHeight);
            ui->doubleSpinBoxScaleHeight->setValue(value);
        }
        else
        {
            qreal const factor = WidthScaleUnitConvertor(value, unit, ScaleUnit::Percent) / 100.;
            qreal const heightPx = ImageHeight() * factor;
            qreal const height = HeightScaleUnitConvertor(heightPx, ScaleUnit::Px, unit);

            const QSignalBlocker blocker(ui->doubleSpinBoxScaleHeight);
            ui->doubleSpinBoxScaleHeight->setValue(height);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetBackgroundImages::ScaleHeightChanged(double value)
{
    if (ui->checkBoxScaleProportionally->isChecked())
    {
        ScaleUnit const unit = CurrentScaleUnit();
        if (unit == ScaleUnit::Percent)
        {
            const QSignalBlocker blocker(ui->doubleSpinBoxScaleWidth);
            ui->doubleSpinBoxScaleWidth->setValue(value);
        }
        else
        {
            qreal const factor = HeightScaleUnitConvertor(value, unit, ScaleUnit::Percent) / 100.;
            qreal const widthPx = ImageWidth() * factor;
            qreal const width = WidthScaleUnitConvertor(widthPx, ScaleUnit::Px, unit);

            const QSignalBlocker blocker(ui->doubleSpinBoxScaleHeight);
            ui->doubleSpinBoxScaleHeight->setValue(width);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetBackgroundImages::ImagePositionChanged(const QUuid &id)
{
    if (ui->checkBoxRelativeTranslation->isChecked())
    {
        return;
    }

    int const row = ui->tableWidget->currentRow();
    if (row == -1)
    {
        return;
    }

    QTableWidgetItem *item = ui->tableWidget->item(row, 0);
    if (item != nullptr)
    {
        if (QUuid const curentId = item->data(Qt::UserRole).toUuid(); curentId != id)
        {
            return;
        }

        SetAbsolutePisition(id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetBackgroundImages::FillTable(const QVector<VBackgroundPatternImage> &images)
{
    const QSignalBlocker blocker(ui->tableWidget);
    ui->tableWidget->clear();

    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->setRowCount(static_cast<int>(images.size()));
    qint32 currentRow = -1;

    auto ReadOnly = [](QTableWidgetItem *item)
    {
        // set the item non-editable (view only), and non-selectable
        Qt::ItemFlags flags = item->flags();
        flags &= ~(Qt::ItemIsEditable); // reset/clear the flag
        item->setFlags(flags);
    };

    for (const auto &image : images)
    {
        ++currentRow;

        // Hold
        auto *item = new QTableWidgetItem();
        item->setTextAlignment(Qt::AlignHCenter);
        SetImageHold(item, image);
        item->setData(Qt::UserRole, image.Id());
        ReadOnly(item);
        ui->tableWidget->setItem(currentRow, 0, item);

        // Visibility
        item = new QTableWidgetItem();
        item->setTextAlignment(Qt::AlignHCenter);
        SetImageVisibility(item, image);
        item->setData(Qt::UserRole, image.Id());
        ReadOnly(item);
        ui->tableWidget->setItem(currentRow, 1, item);

        item = new QTableWidgetItem();
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        item->setData(Qt::UserRole, image.Id());
        SetImageName(item, image, tr("Background image"));
        ui->tableWidget->setItem(currentRow, 2, item);
    }

    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetBackgroundImages::ToggleImageHold(const QUuid &id) const
{
    VBackgroundPatternImage const image = m_doc->GetBackgroundImage(id);
    if (not image.IsNull())
    {
        auto *command = new HoldBackgroundImage(image.Id(), not image.Hold(), m_doc);
        VAbstractApplication::VApp()->getUndoStack()->push(command);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetBackgroundImages::ToggleImageVisibility(const QUuid &id) const
{
    VBackgroundPatternImage const image = m_doc->GetBackgroundImage(id);
    if (not image.IsNull())
    {
        auto *command = new HideBackgroundImage(image.Id(), image.Visible(), m_doc);
        VAbstractApplication::VApp()->getUndoStack()->push(command);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VWidgetBackgroundImages::ImageRow(const QUuid &id) const -> int
{
    for (int r = 0; r < ui->tableWidget->rowCount(); ++r)
    {
        QTableWidgetItem *item = ui->tableWidget->item(r, 0);
        if (item != nullptr && id == item->data(Qt::UserRole).toUuid())
        {
            return r;
        }
    }

    return -1;
}

//---------------------------------------------------------------------------------------------------------------------
auto VWidgetBackgroundImages::CurrentTranslateUnit() const -> Unit
{
    return static_cast<Unit>(ui->comboBoxTranslateUnit->currentData().toInt());
}

//---------------------------------------------------------------------------------------------------------------------
auto VWidgetBackgroundImages::CurrentScaleUnit() const -> ScaleUnit
{
    return static_cast<ScaleUnit>(ui->comboBoxScaleUnit->currentData().toInt());
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetBackgroundImages::InitImageTranslation()
{
    // Translate
    ui->comboBoxTranslateUnit->addItem(tr("Pixels"), QVariant(static_cast<int>(Unit::Px)));
    ui->comboBoxTranslateUnit->addItem(tr("Millimiters"), QVariant(static_cast<int>(Unit::Mm)));
    ui->comboBoxTranslateUnit->addItem(tr("Centimeters"), QVariant(static_cast<int>(Unit::Cm)));
    ui->comboBoxTranslateUnit->addItem(tr("Inches"), QVariant(static_cast<int>(Unit::Inch)));

    {
        const QSignalBlocker blocker(ui->comboBoxTranslateUnit);
        ui->comboBoxTranslateUnit->setCurrentIndex(0);
    }

    const int minTranslate = -10000;
    const int maxTranslate = 10000;

    ui->doubleSpinBoxImageHorizontalTranslate->setMinimum(
        UnitConvertor(minTranslate, Unit::Cm, m_oldImageTranslationUnit));
    ui->doubleSpinBoxImageHorizontalTranslate->setMaximum(
        UnitConvertor(maxTranslate, Unit::Cm, m_oldImageTranslationUnit));
    ui->doubleSpinBoxImageHorizontalTranslate->setValue(0);

    ui->doubleSpinBoxImageVerticalTranslate->setMinimum(
        UnitConvertor(minTranslate, Unit::Cm, m_oldImageTranslationUnit));
    ui->doubleSpinBoxImageVerticalTranslate->setMaximum(
        UnitConvertor(maxTranslate, Unit::Cm, m_oldImageTranslationUnit));
    ui->doubleSpinBoxImageVerticalTranslate->setValue(0);

    connect(ui->comboBoxTranslateUnit, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this V_LAMBDA_CONSTANTS(minTranslate, maxTranslate)]()
            {
                const Unit newUnit = CurrentTranslateUnit();
                const qreal oldTranslateX = ui->doubleSpinBoxImageHorizontalTranslate->value();
                const qreal oldTranslateY = ui->doubleSpinBoxImageVerticalTranslate->value();

                ui->doubleSpinBoxImageHorizontalTranslate->setMinimum(UnitConvertor(minTranslate, Unit::Cm, newUnit));
                ui->doubleSpinBoxImageHorizontalTranslate->setMaximum(UnitConvertor(maxTranslate, Unit::Cm, newUnit));

                ui->doubleSpinBoxImageVerticalTranslate->setMinimum(UnitConvertor(minTranslate, Unit::Cm, newUnit));
                ui->doubleSpinBoxImageVerticalTranslate->setMaximum(UnitConvertor(maxTranslate, Unit::Cm, newUnit));

                ui->doubleSpinBoxImageHorizontalTranslate->setValue(
                    UnitConvertor(oldTranslateX, m_oldImageTranslationUnit, newUnit));
                ui->doubleSpinBoxImageVerticalTranslate->setValue(
                    UnitConvertor(oldTranslateY, m_oldImageTranslationUnit, newUnit));

                m_oldImageTranslationUnit = newUnit;
            });

    SetCheckBoxValue(ui->checkBoxRelativeTranslation, true);
    connect(ui->checkBoxRelativeTranslation, &QCheckBox::toggled, this,
            &VWidgetBackgroundImages::RelativeTranslationChanged);

    // Scale
    ui->comboBoxScaleUnit->addItem(QChar('%'), QVariant(static_cast<int>(ScaleUnit::Percent)));
    ui->comboBoxScaleUnit->addItem(tr("Millimiters"), QVariant(static_cast<int>(ScaleUnit::Mm)));
    ui->comboBoxScaleUnit->addItem(tr("Centimeters"), QVariant(static_cast<int>(ScaleUnit::Cm)));
    ui->comboBoxScaleUnit->addItem(tr("Inches"), QVariant(static_cast<int>(ScaleUnit::Inch)));
    ui->comboBoxScaleUnit->addItem(tr("Pixels"), QVariant(static_cast<int>(ScaleUnit::Px)));

    {
        const QSignalBlocker blocker(ui->comboBoxScaleUnit);
        ui->comboBoxScaleUnit->setCurrentIndex(0);
    }

    const int minScale = -100000;
    const int maxScale = 100000;

    ui->doubleSpinBoxScaleWidth->setMinimum(minScale);
    ui->doubleSpinBoxScaleWidth->setMaximum(maxScale);
    ui->doubleSpinBoxScaleWidth->setValue(100);

    ui->doubleSpinBoxScaleHeight->setMinimum(minScale);
    ui->doubleSpinBoxScaleHeight->setMaximum(maxScale);
    ui->doubleSpinBoxScaleHeight->setValue(100);

    connect(
        ui->comboBoxScaleUnit, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
        [this V_LAMBDA_CONSTANTS(minScale, maxScale)]()
        {
            const ScaleUnit newUnit = CurrentScaleUnit();
            const qreal oldScaleWidth = ui->doubleSpinBoxScaleWidth->value();
            const qreal oldScaleHeight = ui->doubleSpinBoxScaleHeight->value();

            {
                const QSignalBlocker blocker(ui->doubleSpinBoxScaleWidth);

                ui->doubleSpinBoxScaleWidth->setMinimum(WidthScaleUnitConvertor(minScale, ScaleUnit::Percent, newUnit));
                ui->doubleSpinBoxScaleWidth->setMinimum(WidthScaleUnitConvertor(minScale, ScaleUnit::Percent, newUnit));

                ui->doubleSpinBoxScaleWidth->setValue(
                    WidthScaleUnitConvertor(oldScaleWidth, m_oldImageScaleUnit, newUnit));
            }

            {
                const QSignalBlocker blocker(ui->doubleSpinBoxScaleHeight);

                ui->doubleSpinBoxScaleHeight->setMaximum(
                    HeightScaleUnitConvertor(maxScale, ScaleUnit::Percent, newUnit));
                ui->doubleSpinBoxScaleHeight->setMaximum(
                    HeightScaleUnitConvertor(maxScale, ScaleUnit::Percent, newUnit));

                if (ui->checkBoxScaleProportionally->isChecked() && newUnit == ScaleUnit::Percent)
                {
                    ui->doubleSpinBoxScaleHeight->setValue(ui->doubleSpinBoxScaleWidth->value());
                }
                else
                {
                    ui->doubleSpinBoxScaleHeight->setValue(
                        HeightScaleUnitConvertor(oldScaleHeight, m_oldImageScaleUnit, newUnit));
                }
            }

            m_oldImageScaleUnit = newUnit;
        });

    connect(ui->doubleSpinBoxScaleHeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &VWidgetBackgroundImages::ScaleHeightChanged);
    connect(ui->doubleSpinBoxScaleWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &VWidgetBackgroundImages::ScaleWidthChanged);

    SetCheckBoxValue(ui->checkBoxScaleProportionally, true);
    connect(ui->checkBoxScaleProportionally, &QCheckBox::toggled, this,
            &VWidgetBackgroundImages::ScaleProportionallyChanged);

    // Rotate
    ui->doubleSpinBoxRotationAngle->setValue(0);

    ui->toolButtonImageRotationAnticlockwise->setChecked(true);

    QPushButton *bApply = ui->buttonBox->button(QDialogButtonBox::Apply);
    SCASSERT(bApply != nullptr)
    connect(bApply, &QPushButton::clicked, this, &VWidgetBackgroundImages::ApplyImageTransformation);

    QPushButton *bReset = ui->buttonBox->button(QDialogButtonBox::Reset);
    SCASSERT(bReset != nullptr)
    connect(bReset, &QPushButton::clicked, this, &VWidgetBackgroundImages::ResetImageTransformationSettings);
}

//---------------------------------------------------------------------------------------------------------------------
auto VWidgetBackgroundImages::ImageWidth() const -> qreal
{
    qreal width = 0;

    if (int const row = ui->tableWidget->currentRow(); row != -1)
    {
        QTableWidgetItem *item = ui->tableWidget->item(row, 0);
        if (item != nullptr)
        {
            QUuid const id = item->data(Qt::UserRole).toUuid();
            VBackgroundPatternImage const image = m_doc->GetBackgroundImage(id);
            width = image.Size().width();
        }
    }

    return width;
}

//---------------------------------------------------------------------------------------------------------------------
auto VWidgetBackgroundImages::ImageHeight() const -> qreal
{
    qreal height = 0;

    if (int const row = ui->tableWidget->currentRow(); row != -1)
    {
        QTableWidgetItem *item = ui->tableWidget->item(row, 0);
        if (item != nullptr)
        {
            QUuid const id = item->data(Qt::UserRole).toUuid();
            VBackgroundPatternImage const image = m_doc->GetBackgroundImage(id);
            height = image.Size().height();
        }
    }

    return height;
}

//---------------------------------------------------------------------------------------------------------------------
auto VWidgetBackgroundImages::WidthScaleUnitConvertor(qreal value, ScaleUnit from, ScaleUnit to) const -> qreal
{
    return ScaleUnitConvertor(ImageWidth(), value, from, to);
}

//---------------------------------------------------------------------------------------------------------------------
auto VWidgetBackgroundImages::HeightScaleUnitConvertor(qreal value, ScaleUnit from, ScaleUnit to) const -> qreal
{
    return ScaleUnitConvertor(ImageHeight(), value, from, to);
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetBackgroundImages::SetAbsolutePisition(const QUuid &id)
{
    VBackgroundPatternImage const image = m_doc->GetBackgroundImage(id);
    QRectF const rect = image.BoundingRect();

    ui->doubleSpinBoxImageHorizontalTranslate->setValue(
        UnitConvertor(rect.topLeft().x(), Unit::Px, CurrentTranslateUnit()));
    ui->doubleSpinBoxImageVerticalTranslate->setValue(
        UnitConvertor(rect.topLeft().y(), Unit::Px, CurrentTranslateUnit()));
}
