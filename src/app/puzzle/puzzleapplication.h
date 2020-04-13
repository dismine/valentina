/************************************************************************
 **
 **  @file   puzzleapplication.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 2, 2020
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
#ifndef PUZZLEAPPLICATION_H
#define PUZZLEAPPLICATION_H

#include "../vmisc/def.h"
#include "../vmisc/vpuzzlesettings.h"
#include "../vmisc/vabstractapplication.h"
#include "vpuzzlecommandline.h"

#include <memory>

class PuzzleApplication;// use in define
class PuzzleMainWindow;
class QLocalServer;

#if defined(qApp)
#undef qApp
#endif
#define qApp (static_cast<PuzzleApplication*>(VAbstractApplication::instance()))

enum class SocketConnection : bool {Client = false, Server = true};

class PuzzleApplication : public VAbstractApplication
{
    Q_OBJECT
public:
    PuzzleApplication(int &argc, char **argv);
    virtual ~PuzzleApplication() override;

    virtual bool notify(QObject * receiver, QEvent * event) override;

    bool IsTestMode() const;
    virtual bool IsAppInGUIMode() const override;
    PuzzleMainWindow *MainWindow();
    QList<PuzzleMainWindow*> MainWindows();
    PuzzleMainWindow *NewMainWindow();

    void InitOptions();


    virtual const VTranslateVars *TrVars() override;

    virtual void  OpenSettings() override;
    VPuzzleSettings *PuzzleSettings();
    void ActivateDarkMode();

    void ParseCommandLine(const SocketConnection &connection, const QStringList &arguments);
    const VPuzzleCommandLinePtr CommandLine();
public slots:
    void ProcessCMD();

protected:
    virtual void InitTrVars() override;
    virtual bool event(QEvent *e) override;

protected slots:
    virtual void AboutToQuit() override;

private slots:
    void NewLocalSocketConnection();

private:
    Q_DISABLE_COPY(PuzzleApplication)
    QList<QPointer<PuzzleMainWindow> > mainWindows;
    QLocalServer *localServer;
    bool testMode;

    void Clean();
};

#endif // PUZZLEAPPLICATION_H
