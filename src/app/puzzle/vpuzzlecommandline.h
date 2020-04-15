/************************************************************************
 **
 **  @file   vpuzzlecommandline.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
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
#ifndef VPUZZLECOMMANDLINE_H
#define VPUZZLECOMMANDLINE_H

#include <memory>
#include <QCoreApplication>
#include <QCommandLineParser>

class VPuzzleCommandLine;
using VPuzzleCommandLinePtr = std::shared_ptr<VPuzzleCommandLine>;

class VPuzzleCommandLine: public QObject
{
    Q_OBJECT
public:   
    virtual ~VPuzzleCommandLine() = default;

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
protected:
    VPuzzleCommandLine();

    /** @brief create the single instance of the class inside puzzleapplication */
    static VPuzzleCommandLinePtr Instance(const QCoreApplication &app);
private:
    Q_DISABLE_COPY(VPuzzleCommandLine)
    static VPuzzleCommandLinePtr instance;
    QCommandLineParser parser;
    bool isGuiEnabled;
    friend class PuzzleApplication;

    /** @brief add options to the QCommandLineParser that there are in the cmd can be */
    void InitCommandLineOptions();

    bool IsOptionSet(const QString &option)const;
    QString OptionValue(const QString &option) const;
    QStringList OptionValues(const QString &option) const;
};

#endif // VPUZZLECOMMANDLINE_H
