/************************************************************************
 **
 **  @file   dialogbulkrename.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 3, 2026
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project
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
#include "dialogbulkrename.h"
#include "ui_dialogbulkrename.h"

#include <QCheckBox>
#include <QPushButton>

#include "../ifc/exception/vexceptionbadid.h"
#include "../qmuparser/qmudef.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vgobject.h"
#include "../vpatterndb/vcontainer.h"

static constexpr int COL_ENABLE = 0;
static constexpr int COL_BASENAME = 1;
static constexpr int COL_NEWNAME = 2;

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto ApplyPlaceholders(const QString &text, int orderIndex) -> QString
{
    const int n = orderIndex + 1; // 1-based

    QString result = text;
    // Longest patterns first to avoid partial substitution
    result.replace(QLatin1String("%000n"), QString::asprintf("%04d", n));
    result.replace(QLatin1String("%00n"), QString::asprintf("%03d", n));
    result.replace(QLatin1String("%0n"), QString::asprintf("%02d", n));
    result.replace(QLatin1String("%n"), QString::number(n));
    return result;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
DialogBulkRename::DialogBulkRename(const QVector<SourceItem> &items, const VContainer *data, QWidget *parent)
  : QDialog(parent),
    ui(new Ui::DialogBulkRename),
    m_items(items),
    m_data(data)
{
    ui->setupUi(this);

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(COL_ENABLE, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(COL_BASENAME, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(COL_NEWNAME, QHeaderView::Stretch);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);

    PopulateTable();

    connect(ui->lineEdit, &QLineEdit::textChanged, this, &DialogBulkRename::UpdatePreview);
    connect(ui->radioButtonAppend, &QRadioButton::toggled, this, &DialogBulkRename::UpdatePreview);
    connect(ui->radioButtonPrepend, &QRadioButton::toggled, this, &DialogBulkRename::UpdatePreview);
    connect(ui->radioButtonReplace, &QRadioButton::toggled, this, &DialogBulkRename::UpdatePreview);

    ValidateNames();
}

//---------------------------------------------------------------------------------------------------------------------
DialogBulkRename::~DialogBulkRename()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogBulkRename::HasChanges() const -> bool
{
    for (int row = 0; row < m_items.size(); ++row)
    {
        if (!IsRowEnabled(row))
        {
            continue;
        }

        if (m_items.at(row).name != ui->tableWidget->item(row, COL_NEWNAME)->data(Qt::UserRole).value<QString>())
        {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogBulkRename::RenamedItems() const -> QVector<SourceItem>
{
    QVector<SourceItem> result = m_items; // start from original list

    for (int row = 0; row < m_items.size(); ++row)
    {
        if (!IsRowEnabled(row))
        {
            continue; // leave this SourceItem's name untouched
        }

        if (const auto newName = ui->tableWidget->item(row, COL_NEWNAME)->data(Qt::UserRole).value<QString>();
            newName != result.at(row).name)
        {
            result[row].name = newName; // only overwrite if actually changed
        }
    }

    return result;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogBulkRename::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::PaletteChange)
    {
        UpdatePreview();
    }

    // remember to call base class implementation
    QDialog::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogBulkRename::UpdatePreview()
{
    int enabledCount = 0;

    for (int row = 0; row < m_items.size(); ++row)
    {
        const bool enabled = IsRowEnabled(row);
        const auto id = ui->tableWidget->item(row, COL_BASENAME)->data(Qt::UserRole).value<quint32>();

        const QString newBaseName = BuildNewName(enabled, id, enabledCount);
        ui->tableWidget->item(row, COL_NEWNAME)->setData(Qt::UserRole, newBaseName);

        const QString newFullBaseName = NewFullBaseName(newBaseName, id);
        ui->tableWidget->item(row, COL_NEWNAME)->setText(newFullBaseName);

        const QColor fg = enabled ? ui->tableWidget->palette().color(QPalette::Text)
                                  : ui->tableWidget->palette().color(QPalette::Disabled, QPalette::Text);

        ui->tableWidget->item(row, COL_BASENAME)->setForeground(fg);
        ui->tableWidget->item(row, COL_NEWNAME)->setForeground(fg);
    }

    ValidateNames();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogBulkRename::PopulateTable()
{
    ui->tableWidget->setRowCount(static_cast<int>(m_items.size()));

    for (int row = 0; row < m_items.size(); ++row)
    {
        const SourceItem &src = m_items.at(row);

        // Column 0: enable checkbox
        auto *chk = new QCheckBox();
        chk->setChecked(true);
        chk->setToolTip(tr("Enable renaming for this item"));

        auto *cell = new QWidget();
        auto *lay = new QHBoxLayout(cell);
        lay->addWidget(chk);
        lay->setAlignment(Qt::AlignCenter);
        lay->setContentsMargins(4, 0, 4, 0);
        ui->tableWidget->setCellWidget(row, COL_ENABLE, cell);

        connect(chk, &QCheckBox::toggled, this, &DialogBulkRename::UpdatePreview);

        // Column 1: base name
        const QString base = FullBaseName(src.id);
        auto *baseItem = new QTableWidgetItem(base);
        baseItem->setFlags(baseItem->flags() & ~Qt::ItemIsEditable);
        baseItem->setData(Qt::UserRole, src.id);
        ui->tableWidget->setItem(row, COL_BASENAME, baseItem);

        // Column 2: new name
        auto *newItem = new QTableWidgetItem(NewFullBaseName(src.name, src.id));
        newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
        ui->tableWidget->setItem(row, COL_NEWNAME, newItem);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogBulkRename::BuildNewName(bool enabled, quint32 id, int &orderIndex) const -> QString
{
    const QString raw = ui->lineEdit->text();

    if (!enabled || raw.isEmpty())
    {
        const auto it = std::find_if(m_items.cbegin(),
                                     m_items.cend(),
                                     [id](const SourceItem &item) -> bool { return item.id == id; });
        return it != m_items.cend() ? it->name : QString{};
    }

    if (ui->radioButtonReplace->isChecked())
    {
        return ApplyPlaceholders(raw, orderIndex++);
    }

    const QString base = BaseName(id);

    if (ui->radioButtonPrepend->isChecked())
    {
        return ApplyPlaceholders(raw, orderIndex++) + base;
    }

    // Append (default)
    return base + ApplyPlaceholders(raw, orderIndex++);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogBulkRename::FullBaseName(quint32 id) const -> QString
{
    if (m_data == nullptr)
    {
        return {};
    }

    try
    {
        return m_data->GetGObject(id)->name();
    }
    catch (const VExceptionBadId &)
    {
        return GetName(id);
    }
    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogBulkRename::NewFullBaseName(const QString &base, quint32 id) const -> QString
{
    try
    {
        if (const QSharedPointer<VGObject> obj = m_data->GetGObject(id); obj->getType() == GOType::Point)
        {
            return base;
        }
        const QSharedPointer<VAbstractCurve> curve = m_data->GeometricObject<VAbstractCurve>(id);
        return curve->GetTypeHead() + base;
    }
    catch (const VExceptionBadId &)
    {
        return base;
    }
    return base;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogBulkRename::BaseName(quint32 id) const -> QString
{
    if (m_data == nullptr)
    {
        return {};
    }

    try
    {
        if (const QSharedPointer<VGObject> obj = m_data->GetGObject(id); obj->getType() == GOType::Point)
        {
            return obj->name();
        }
        const QSharedPointer<VAbstractCurve> curve = m_data->GeometricObject<VAbstractCurve>(id);
        return curve->HeadlessName();
    }
    catch (const VExceptionBadId &)
    {
        return GetName(id);
    }
    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogBulkRename::IsRowEnabled(int row) const -> bool
{
    if (auto *cellWidget = ui->tableWidget->cellWidget(row, COL_ENABLE))
    {
        if (auto *chk = cellWidget->findChild<QCheckBox *>())
        {
            return chk->isChecked();
        }
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogBulkRename::GetName(quint32 id) const -> QString
{
    const auto it = std::find_if(m_items.cbegin(),
                                 m_items.cend(),
                                 [id](const SourceItem &item) -> bool { return item.id == id; });
    return it != m_items.cend() ? it->name : QString{};
}

//---------------------------------------------------------------------------------------------------------------------
void DialogBulkRename::ValidateNames()
{
    bool allValid = true;

    const QColor errorColor(Qt::red);
    QColor bg = errorColor;
    bg.setAlpha(50);

    for (int row = 0; row < m_items.size(); ++row)
    {
        if (!IsRowEnabled(row))
        {
            // Reset background for disabled rows
            ui->tableWidget->item(row, COL_NEWNAME)->setBackground(QBrush());
            continue;
        }

        const QString newName = ui->tableWidget->item(row, COL_NEWNAME)->text();
        const bool valid = IsNameValid(newName, row);

        ui->tableWidget->item(row, COL_NEWNAME)->setBackground(valid ? QBrush() : QBrush(bg));

        if (!valid)
        {
            allValid = false;
        }
    }

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(allValid);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogBulkRename::IsNameValid(const QString &name, int currentRow) const -> bool
{
    // Must match naming rules
    static const QRegularExpression rx(NameRegExp());
    if (!rx.match(name).hasMatch())
    {
        return false;
    }

    if (const QString raw = ui->lineEdit->text(); raw.isEmpty())
    {
        return true;
    }

    if (currentRow >= m_items.size())
    {
        return false;
    }

    // Must be unique in the data container
    if (NewFullBaseName(m_items.at(currentRow).name, m_items.at(currentRow).id) != name && (m_data != nullptr)
        && !m_data->IsUnique(name))
    {
        return false;
    }

    // Must not duplicate any other new name in this dialog
    for (int row = 0; row < m_items.size(); ++row)
    {
        if (row == currentRow)
        {
            continue;
        }

        if (ui->tableWidget->item(row, COL_NEWNAME)->text() == name)
        {
            return false;
        }
    }

    return true;
}
