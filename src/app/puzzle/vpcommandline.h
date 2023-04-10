/************************************************************************
 **
 **  @file   vpcommandline.h
 **  @author Dmytro Hladkykh <vorzelmir@gmail.com>
 **  @date   12 4, 2020
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
#ifndef VPCOMMANDLINE_H
#define VPCOMMANDLINE_H

#include <memory>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <ciso646>

#include "../vmisc/defglobal.h"

class VPCommandLine;
using VPCommandLinePtr = std::shared_ptr<VPCommandLine>;

class VPCommandLine: public QObject
{
    Q_OBJECT // NOLINT
public:
    ~VPCommandLine() override = default;

    /** @brief list with paths to the raw layout data files */
    auto OptionRawLayouts() const -> QStringList;

    /** @brief if gui enabled or not */
    auto IsGuiEnabled() const -> bool;

    /** @brief the file name which should be loaded */
    auto OptionFileNames() const -> QStringList;

    /** @brief if high dpi scaling is enabled */
    auto IsNoScalingEnabled() const -> bool;

    Q_NORETURN void ShowHelp(int exitCode = 0);
protected:
    VPCommandLine();

    /** @brief create the single instance of the class inside vpapplication */
    static auto Instance() -> VPCommandLinePtr;
    static void ProcessInstance(VPCommandLinePtr &instance, const QStringList &arguments);
private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VPCommandLine) // NOLINT
    static VPCommandLinePtr instance; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
    QCommandLineParser parser{};
    bool isGuiEnabled{true};
    friend class VPApplication;

    /** @brief add options to the QCommandLineParser that there are in the cmd can be */
    void InitCommandLineOptions();

    auto IsOptionSet(const QString &option)const -> bool;
    auto OptionValue(const QString &option) const -> QString;
    auto OptionValues(const QString &option) const -> QStringList;
};

#endif // VPCOMMANDLINE_H
