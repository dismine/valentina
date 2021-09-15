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

class VPCommandLine;
using VPCommandLinePtr = std::shared_ptr<VPCommandLine>;

class VPCommandLine: public QObject
{
    Q_OBJECT
public:   
    virtual ~VPCommandLine() = default;

    /** @brief if user enabled export from cmd */
    bool IsExportEnabled() const;

    /** @brief path to export file or empty string if not */
    QString OptionExportFile() const;

    /** @brief list with paths to the raw layout data files */
    QStringList OptionRawLayouts() const;

    /** @brief if user enabled test mode from cmd */
    bool IsTestModeEnabled() const;

    /** @brief if gui enabled or not */
    bool IsGuiEnabled() const;

    /** @brief the file name which should be loaded */
    QStringList OptionFileNames() const;

    /** @brief if high dpi scaling is enabled */
    bool IsNoScalingEnabled() const;

    Q_NORETURN void ShowHelp(int exitCode = 0);
protected:
    VPCommandLine();

    /** @brief create the single instance of the class inside vpapplication */
    static VPCommandLinePtr Instance(const QCoreApplication &app);
    static void ProcessInstance(VPCommandLinePtr &instance, const QStringList &arguments);
private:
    Q_DISABLE_COPY(VPCommandLine)
    static VPCommandLinePtr instance;
    QCommandLineParser parser;
    bool isGuiEnabled;
    friend class VPApplication;

    /** @brief add options to the QCommandLineParser that there are in the cmd can be */
    void InitCommandLineOptions();

    bool IsOptionSet(const QString &option)const;
    QString OptionValue(const QString &option) const;
    QStringList OptionValues(const QString &option) const;
};

#endif // VPCOMMANDLINE_H
