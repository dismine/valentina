/************************************************************************
 **
 **  @file   vshortcutdialog.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 10, 2023
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
#include "vshortcutdialog.h"
#include "../vabstractapplication.h"
#include "ui_vshortcutdialog.h"

#include <QAbstractButton>
#include <QMessageBox>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto ShortcutAlreadyBound(const QKeySequence &chosenSequence, const VShortcutAction &exemptShortcut) -> QString
{
    if (chosenSequence.isEmpty())
    {
        return {};
    }

    VAbstractShortcutManager *manager = VAbstractApplication::VApp()->GetShortcutManager();
    if (manager == nullptr)
    {
        return {};
    }

    const auto &shortcutsList = manager->GetShortcutsList();
    for (const auto &shortcut : shortcutsList)
    {
        auto sequenceList = VAbstractShortcutManager::StringListToKeySequenceList(shortcut.shortcuts);

        if (sequenceList.contains(chosenSequence) && shortcut.type != exemptShortcut)
        {
            return VAbstractShortcutManager::ReadableName(shortcut.type);
        }
    }
    return {};
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VShortcutDialog::VShortcutDialog(int index, QWidget *parent)
  : QDialog(parent),
    ui(new Ui::VShortcutDialog),
    m_index(index)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint | Qt::CustomizeWindowHint));

    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &VShortcutDialog::ButtonBoxClicked);

    m_shortcutObject = VAbstractApplication::VApp()->GetShortcutManager()->GetShortcutsList().value(index);
    ui->keySequenceEdit->setKeySequence(m_shortcutObject.shortcuts.join(", "_L1));

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    ui->keySequenceEdit->setClearButtonEnabled(true);
#endif
}

//---------------------------------------------------------------------------------------------------------------------
VShortcutDialog::~VShortcutDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void VShortcutDialog::AcceptValidated()
{
    QDialog::done(1);
}

//---------------------------------------------------------------------------------------------------------------------
void VShortcutDialog::ButtonBoxClicked(QAbstractButton *button)
{
    if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
    {
        QStringList const shortcutsStringList = ui->keySequenceEdit->keySequence().toString().split(", "_L1);
        const auto sequenceList = VAbstractShortcutManager::StringListToKeySequenceList(shortcutsStringList);

        for (const auto &sequence : sequenceList)
        {
            auto conflictingShortcut = ShortcutAlreadyBound(sequence, m_shortcutObject.type);
            if (!conflictingShortcut.isEmpty())
            {
                QString const nativeShortcutString = sequence.toString(QKeySequence::NativeText);
                QMessageBox::warning(
                    this, tr("Shortcut Already Used"),
                    tr(R"("%1" is already bound to "%2")").arg(nativeShortcutString, conflictingShortcut));
                return;
            }
        }

        AcceptValidated();

        emit ShortcutsListChanged(m_index, shortcutsStringList);
    }
    else if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ResetRole)
    {
        ui->keySequenceEdit->setKeySequence(m_shortcutObject.defaultShortcuts.join(", "_L1));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VShortcutDialog::done(int r)
{
    if (r == QDialog::Accepted)
    {
        return;
    }

    QDialog::done(r);
}
