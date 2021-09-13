/************************************************************************
 **
 **  @file   vpapplication.h
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
#ifndef VPAPPLICATION_H
#define VPAPPLICATION_H

#include "../vmisc/def.h"
#include "vpsettings.h"
#include "../vmisc/vabstractapplication.h"
#include "vpcommandline.h"

template <typename T> class QWeakPointer;
class VPMainWindow;
class QLocalServer;
class DialogPuzzlePreferences;

enum class SocketConnection : bool {Client = false, Server = true};

class VPApplication : public VAbstractApplication
{
    Q_OBJECT
public:
    VPApplication(int &argc, char **argv);
    virtual ~VPApplication() override;

    virtual bool notify(QObject * receiver, QEvent * event) override;

    virtual bool IsAppInGUIMode() const override;
    VPMainWindow *MainWindow();
    QList<VPMainWindow*> MainWindows();
    VPMainWindow *NewMainWindow();
    VPMainWindow *NewMainWindow(const VPCommandLinePtr &cmd);

    void InitOptions();

    virtual const VTranslateVars *TrVars() override;

    virtual void  OpenSettings() override;
    VPSettings *PuzzleSettings();
    void ActivateDarkMode();

    void ParseCommandLine(const SocketConnection &connection, const QStringList &arguments);
    void ProcessArguments(const VPCommandLinePtr &cmd);

    static VPCommandLinePtr CommandLine();
    static VPApplication *VApp();

    auto PreferencesDialog() const -> QSharedPointer<DialogPuzzlePreferences>;
    void SetPreferencesDialog(const QSharedPointer<DialogPuzzlePreferences> &newPreferencesDialog);

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
    Q_DISABLE_COPY(VPApplication)
    QList<QPointer<VPMainWindow> > mainWindows{};
    QLocalServer *localServer{nullptr};
    QWeakPointer<DialogPuzzlePreferences> m_preferencesDialog{};

    void Clean();
};

#endif // VPAPPLICATION_H
