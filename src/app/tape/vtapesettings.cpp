/************************************************************************
 **
 **  @file   vtapesettings.cpp
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

#include "vtapesettings.h"

#include <QDir>
#include <QGlobalStatic>
#include <QVariant>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPathsTemplates, ("paths/templates"_L1)) // NOLINT

Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingDataBaseGeometry, ("database/geometry"_L1))   // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchHistoryTape, ("searchHistory/tape"_L1)) // NOLINT

// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsTapeUseUnicodeProperties,
                          ("searchOptions/tapeUseUnicodeProperties"_L1))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsTapeWholeWord, ("searchOptions/tapeWholeWord"_L1))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsTapeRegexp, ("searchOptions/tapeRegexp"_L1))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsTapeMatchCase, ("searchOptions/tapeMatchCase"_L1))
QT_WARNING_POP
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VTapeSettings::VTapeSettings(Format format, Scope scope, const QString &organization, const QString &application,
                             QObject *parent)
  : VCommonSettings(format, scope, organization, application, parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VTapeSettings::GetPathTemplate() const -> QString
{
    return value(*settingPathsTemplates, QDir::homePath()).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VTapeSettings::SetPathTemplate(const QString &value)
{
    setValue(*settingPathsTemplates, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VTapeSettings::GetDataBaseGeometry() const -> QByteArray
{
    return value(*settingDataBaseGeometry).toByteArray();
}

//---------------------------------------------------------------------------------------------------------------------
void VTapeSettings::SetDataBaseGeometry(const QByteArray &value)
{
    setValue(*settingDataBaseGeometry, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VTapeSettings::GetTapeSearchHistory() const -> QStringList
{
    return value(*settingSearchHistoryTape).toStringList();
}

//---------------------------------------------------------------------------------------------------------------------
void VTapeSettings::SetTapeSearchHistory(const QStringList &history)
{
    setValue(*settingSearchHistoryTape, history);
}

//---------------------------------------------------------------------------------------------------------------------
auto VTapeSettings::GetTapeSearchOptionUseUnicodeProperties() const -> bool
{
    return value(*settingSearchOptionsTapeUseUnicodeProperties, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VTapeSettings::SetTapeSearchOptionUseUnicodeProperties(bool value)
{
    setValue(*settingSearchOptionsTapeUseUnicodeProperties, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VTapeSettings::GetTapeSearchOptionWholeWord() const -> bool
{
    return value(*settingSearchOptionsTapeWholeWord, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VTapeSettings::SetTapeSearchOptionWholeWord(bool value)
{
    setValue(*settingSearchOptionsTapeWholeWord, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VTapeSettings::GetTapeSearchOptionRegexp() const -> bool
{
    return value(*settingSearchOptionsTapeRegexp, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VTapeSettings::SetTapeSearchOptionRegexp(bool value)
{
    setValue(*settingSearchOptionsTapeRegexp, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VTapeSettings::GetTapeSearchOptionMatchCase() const -> bool
{
    return value(*settingSearchOptionsTapeMatchCase, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VTapeSettings::SetTapeSearchOptionMatchCase(bool value)
{
    setValue(*settingSearchOptionsTapeMatchCase, value);
}
