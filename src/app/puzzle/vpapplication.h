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
    Q_OBJECT // NOLINT
public:
    VPApplication(int &argc, char **argv);
    ~VPApplication() override;

    auto notify(QObject * receiver, QEvent * event) -> bool override;

    auto IsAppInGUIMode() const -> bool override;
    auto MainWindow() -> VPMainWindow *;
    auto MainWindows() -> QList<VPMainWindow*>;
    auto NewMainWindow() -> VPMainWindow *;
    auto NewMainWindow(const VPCommandLinePtr &cmd) -> VPMainWindow *;

    void InitOptions();

    auto TrVars() -> const VTranslateVars * override;

    void OpenSettings() override;
    auto PuzzleSettings() -> VPSettings *;
    void ActivateDarkMode();

    void ParseCommandLine(const SocketConnection &connection, const QStringList &arguments);
    void ProcessArguments(const VPCommandLinePtr &cmd);

    static auto CommandLine() -> VPCommandLinePtr;
    static auto VApp() -> VPApplication *;

    auto PreferencesDialog() const -> QSharedPointer<DialogPuzzlePreferences>;
    void SetPreferencesDialog(const QSharedPointer<DialogPuzzlePreferences> &newPreferencesDialog);

public slots:
    void ProcessCMD();

protected:
    void InitTrVars() override;
    auto event(QEvent *e) -> bool override;

protected slots:
    void AboutToQuit() override;

private slots:
    void NewLocalSocketConnection();

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VPApplication) //NOLINT
    QList<QPointer<VPMainWindow> > m_mainWindows{};
    QLocalServer *m_localServer{nullptr};
    QWeakPointer<DialogPuzzlePreferences> m_preferencesDialog{};

    void Clean();

    void StartLocalServer(const QString &serverName);

    void StartWithFiles(const VPCommandLinePtr &cmd, const QStringList &rawLayouts);
    void SingleStart(const VPCommandLinePtr &cmd, const QStringList &rawLayouts);
};

#endif // VPAPPLICATION_H
