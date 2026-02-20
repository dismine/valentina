/************************************************************************
 **
 **  @file   toolsdef.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   6 11, 2020
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

#include "toolsdef.h"

#include <QBrush>
#include <QDialogButtonBox>
#include <QIcon>
#include <QMessageBox>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QRegularExpression>
#include <QStyle>
#include <QVector>

#include "../qmuparser/qmudef.h"
#include "../vgeometry/vgobject.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpropertyexplorer/checkablemessagebox.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
auto SourceToObjects(const QVector<SourceItem> &source) -> QVector<quint32>
{
    QVector<quint32> ids;
    ids.reserve(source.size());

    for (const auto &s : source)
    {
        ids.append(s.id);
    }

    return ids;
}

//---------------------------------------------------------------------------------------------------------------------
auto OperationLineStylesPics(QColor backgroundColor, QColor textColor) -> QMap<QString, QIcon>
{
    QMap<QString, QIcon> map = LineStylesPics(backgroundColor, textColor);
    map.insert(TypeLineDefault, QIcon());
    return map;
}

//---------------------------------------------------------------------------------------------------------------------
auto ConfirmDeletion() -> int
{
    if (not VAbstractApplication::VApp()->Settings()->GetConfirmItemDelete())
    {
        return QMessageBox::Yes;
    }

    Utils::CheckableMessageBox msgBox(VAbstractValApplication::VApp()->getMainWindow());
    msgBox.setWindowTitle(QObject::tr("Confirm deletion"));
    msgBox.setText(QObject::tr("Do you really want to delete?"));
    msgBox.setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No);
    msgBox.setDefaultButton(QDialogButtonBox::No);
    msgBox.setIconPixmap(QApplication::style()->standardIcon(QStyle::SP_MessageBoxQuestion).pixmap(32, 32));

    int const dialogResult = msgBox.exec();

    if (dialogResult == QDialog::Accepted)
    {
        VAbstractApplication::VApp()->Settings()->SetConfirmItemDelete(not msgBox.isChecked());
    }

    return dialogResult == QDialog::Accepted ? QMessageBox::Yes : QMessageBox::No;
}

//---------------------------------------------------------------------------------------------------------------------
void FillDefSourceNames(QVector<SourceItem> &source, const VContainer *data, const QString &suffix)
{
    SCASSERT(data != nullptr)
    SCASSERT(!suffix.isEmpty())

    constexpr int maxTries = 100; // Limit number of tries per suffix

    QRegularExpression const rx(NameRegExp());

    // Lambda to get base name for an item
    auto GetBaseName = [data](quint32 id) -> QString
    {
        if (const QSharedPointer<VGObject> obj = data->GetGObject(id); obj->getType() == GOType::Point)
        {
            return obj->name();
        }

        // Assume it's a curve for other types
        const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(id);
        return curve->HeadlessName();
    };

    // Try each suffix number until we find one that works for all items
    for (int suffixIndex = 1; suffixIndex <= maxTries; ++suffixIndex)
    {
        QSet<QString> candidateNames;
        bool allValid = true;

        // Check if this suffix works for all items
        for (const auto &sourceItem : source)
        {
            if (sourceItem.id == NULL_ID)
            {
                continue;
            }

            try
            {
                const QString baseName = GetBaseName(sourceItem.id);
                const QString candidateName = u"%1__%2%3"_s.arg(baseName, suffix).arg(suffixIndex);

                // Check if name matches the regex
                if (!rx.match(candidateName).hasMatch())
                {
                    allValid = false;
                    break;
                }

                // Check if name is unique in data and not already in candidate set
                if (!data->IsUnique(candidateName) || candidateNames.contains(candidateName))
                {
                    allValid = false;
                    break;
                }

                candidateNames.insert(candidateName);
            }
            catch (const VExceptionBadId &)
            {
                // If object not found, skip validation for this item
                continue;
            }
        }

        // If this suffix works for all items, apply it
        if (allValid)
        {
            for (auto &sourceItem : source)
            {
                if (sourceItem.id == NULL_ID)
                {
                    continue;
                }

                try
                {
                    const QString baseName = GetBaseName(sourceItem.id);
                    sourceItem.name = u"%1__%2%3"_s.arg(baseName, suffix).arg(suffixIndex);
                }
                catch (const VExceptionBadId &)
                {
                    // If object not found, leave name empty
                    continue;
                }
            }

            return; // Successfully filled all names with same suffix
        }
    }

    // If no common suffix found after maxTries, names remain empty
}

//---------------------------------------------------------------------------------------------------------------------
auto IsValidSourceName(const QString &newName, quint32 id, const QVector<SourceItem> &source, const VContainer *data)
    -> bool
{
    if (id == NULL_ID || newName.isEmpty())
    {
        return false;
    }

    SCASSERT(data != nullptr)

    auto item = std::find_if(source.begin(),
                             source.end(),
                             [id](const SourceItem &sItem) -> bool { return sItem.id == id; });
    if (item == source.end())
    {
        return false;
    }

    const QString name = GetSourceItemName(newName, id, data);

    if (name.isEmpty())
    {
        return false;
    }

    // Check if name matches regex
    if (QRegularExpression const rx(NameRegExp()); !rx.match(name).hasMatch())
    {
        return false;
    }

    // Check if name is unique in data
    if (newName != item->name && !data->IsUnique(name))
    {
        return false;
    }

    // Check uniqueness among all names in source
    return std::ranges::all_of(source,
                               [id, name, source, data](const SourceItem &sourceItem) -> bool
                               {
                                   if (sourceItem.id == NULL_ID || sourceItem.id == id)
                                   {
                                       return true;
                                   }

                                   // Check against other item's name
                                   if (!sourceItem.name.isEmpty()
                                       && name == GetSourceItemName(sourceItem.name, sourceItem.id, data))
                                   {
                                       return false;
                                   }

                                   return true;
                               });
}

//---------------------------------------------------------------------------------------------------------------------
auto GetSourceItemName(const QString &name, quint32 id, const VContainer *data) -> QString
{
    SCASSERT(data != nullptr)

    if (id == NULL_ID || name.isEmpty())
    {
        return {};
    }

    try
    {
        const QSharedPointer<VGObject> obj = data->GetGObject(id);

        if (obj->getType() == GOType::Point)
        {
            return name;
        }

        const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(id);
        return curve->GetTypeHead() + name;
    }
    catch (const VExceptionBadId &)
    {
        return {};
    }
}
