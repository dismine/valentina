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
#include <QVector>
#include <QStyle>

#include "../vgeometry/vgobject.h"
#include "../qmuparser/qmudef.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpropertyexplorer/checkablemessagebox.h"
#include "../vmisc/vabstractvalapplication.h"

//---------------------------------------------------------------------------------------------------------------------
auto SourceToObjects(const QVector<SourceItem> &source) -> QVector<quint32>
{
    QVector<quint32> ids;
    ids.reserve(source.size());

    for (auto s: source)
    {
        ids.append(s.id);
    }

    return ids;
}

//---------------------------------------------------------------------------------------------------------------------
auto SourceAliasValid(const SourceItem &item, const QSharedPointer<VGObject> &obj, const VContainer *data,
                      const QString &originAlias) -> bool
{
    SCASSERT(data != nullptr)

    QRegularExpression rx(NameRegExp());

    QString alias;

    if (obj->getType() == GOType::Point)
    {
        alias = item.alias;
    }
    else
    {
        const QString oldAlias = obj->GetAliasSuffix();
        obj->SetAliasSuffix(item.alias);
        alias = obj->GetAlias();
        obj->SetAliasSuffix(oldAlias);
    }

    if (not alias.isEmpty() && originAlias != alias &&
        (not rx.match(alias).hasMatch() || not data->IsUnique(alias)))
    {
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto OriginAlias(quint32 id, const QVector<SourceItem> &source, const QSharedPointer<VGObject> &obj) -> QString
{
    auto item = std::find_if(source.begin(), source.end(),
                            [id](const SourceItem &sItem) { return sItem.id == id; });
    if (item != source.end())
    {
        if (obj->getType() == GOType::Point)
        {
            return item->alias;
        }

        const QString oldAlias = obj->GetAliasSuffix();
        obj->SetAliasSuffix(item->alias);
        QString alias = obj->GetAlias();
        obj->SetAliasSuffix(oldAlias);
        return alias;
    }

    return QString();
}

//---------------------------------------------------------------------------------------------------------------------
auto OperationLineStylesPics() -> QMap<QString, QIcon>
{
    QMap<QString, QIcon> map = LineStylesPics();
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
    msgBox.setIconPixmap(QApplication::style()->standardIcon(QStyle::SP_MessageBoxQuestion).pixmap(32, 32) );

    int dialogResult = msgBox.exec();

    if (dialogResult == QDialog::Accepted)
    {
        VAbstractApplication::VApp()->Settings()->SetConfirmItemDelete(not msgBox.isChecked());
    }

    return dialogResult == QDialog::Accepted ? QMessageBox::Yes : QMessageBox::No;
}
