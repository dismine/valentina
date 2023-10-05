/************************************************************************
 **
 **  @file   vtapesettings.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 7, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#ifndef VTAPESETTINGS_H
#define VTAPESETTINGS_H

#include <QByteArray>
#include <QChar>
#include <QMetaObject>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QtGlobal>

#include "vcommonsettings.h"

class VTapeSettings : public VCommonSettings
{
    Q_OBJECT // NOLINT

public:
    VTapeSettings(Format format, Scope scope, const QString &organization, const QString &application = QString(),
                  QObject *parent = nullptr);
    ~VTapeSettings() override = default;

    auto GetPathTemplate() const -> QString;
    void SetPathTemplate(const QString &value);

    auto GetDataBaseGeometry() const -> QByteArray;
    void SetDataBaseGeometry(const QByteArray &value);

    auto GetTapeSearchHistory() const -> QStringList;
    void SetTapeSearchHistory(const QStringList &history);

    auto GetTapeSearchOptionUseUnicodeProperties() const -> bool;
    void SetTapeSearchOptionUseUnicodeProperties(bool value);

    auto GetTapeSearchOptionWholeWord() const -> bool;
    void SetTapeSearchOptionWholeWord(bool value);

    auto GetTapeSearchOptionRegexp() const -> bool;
    void SetTapeSearchOptionRegexp(bool value);

    auto GetTapeSearchOptionMatchCase() const -> bool;
    void SetTapeSearchOptionMatchCase(bool value);

private:
    Q_DISABLE_COPY_MOVE(VTapeSettings) // NOLINT
};

#endif // VTAPESETTINGS_H
