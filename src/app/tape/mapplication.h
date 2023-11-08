/************************************************************************
 **
 **  @file   mapplication.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   8 7, 2015
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

#ifndef MAPPLICATION_H
#define MAPPLICATION_H

#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vtranslatevars.h"
#include "dialogs/dialogmdatabase.h"
#include "vtapesettings.h"

#include <QFutureWatcher>

class TMainWindow;
class TKMMainWindow;
class QLocalServer;
class QCommandLineParser;
class VKnownMeasurementsDatabase;

enum class SocketConnection : bool
{
    Client = false,
    Server = true
};

class MApplication : public VAbstractApplication
{
    Q_OBJECT // NOLINT

public:
    MApplication(int &argc, char **argv);
    ~MApplication() override;

    auto notify(QObject *receiver, QEvent *event) -> bool override;

    auto IsTestMode() const -> bool;
    auto IsAppInGUIMode() const -> bool override;

    auto MainTapeWindow() -> TMainWindow *;
    auto MainTapeWindows() -> QList<TMainWindow *>;
    auto NewMainTapeWindow() -> TMainWindow *;

    auto MainKMWindow() -> TKMMainWindow *;
    auto MainKMWindows() -> QList<TKMMainWindow *>;
    auto NewMainKMWindow() -> TKMMainWindow *;

    void InitOptions();

    auto TrVars() -> const VTranslateVars * override;

    void OpenSettings() override;
    auto TapeSettings() -> VTapeSettings *;

    void ParseCommandLine(const SocketConnection &connection, const QStringList &arguments);

    static auto VApp() -> MApplication *;

    auto KnownMeasurementsDatabase() -> VKnownMeasurementsDatabase * override;

    void Preferences(QWidget *parent = nullptr);

public slots:
    void ProcessCMD();

protected:
    void InitTrVars() override;
    auto event(QEvent *e) -> bool override;

protected slots:
    void AboutToQuit() override;

private slots:
    void NewLocalSocketConnection();
    void RepopulateMeasurementsDatabase(const QString &path);
    void KnownMeasurementsPathChanged(const QString &oldPath, const QString &newPath);
    void SyncKnownMeasurements();

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(MApplication) // NOLINT
    QList<QPointer<TMainWindow>> m_mainWindows{};
    QList<QPointer<TKMMainWindow>> m_kmMainWindows{};
    QLocalServer *m_localServer{nullptr};
    VTranslateVars *m_trVars{nullptr};
    QPointer<DialogMDataBase> m_dataBase{};
    bool m_testMode{false};
    bool m_knownMeasurementsMode{false};
    VKnownMeasurementsDatabase *m_knownMeasurementsDatabase{nullptr};
    QFileSystemWatcher *m_knownMeasurementsDatabaseWatcher{nullptr};
    QFutureWatcher<void> *m_knownMeasurementsRepopulateWatcher;

    void CleanTapeWindows();
    void CleanKMWindows();

    static void InitParserOptions(QCommandLineParser &parser);
    void StartLocalServer(const QString &serverName);

    auto StartWithFiles(QCommandLineParser &parser) -> bool;
    auto StartWithMeasurementFiles(QCommandLineParser &parser) -> bool;
    auto StartWithKnownMeasurementFiles(QCommandLineParser &parser) -> bool;
    auto SingleStart(QCommandLineParser &parser) -> bool;

    static void ParseDimensionAOption(QCommandLineParser &parser, qreal &dimensionAValue, bool &flagDimensionA);
    static void ParseDimensionBOption(QCommandLineParser &parser, qreal &dimensionBValue, bool &flagDimensionB);
    static void ParseDimensionCOption(QCommandLineParser &parser, qreal &dimensionCValue, bool &flagDimensionC);
    static void ParseUnitsOption(QCommandLineParser &parser, Unit &unit, bool &flagUnits);

    void RestartKnownMeasurementsDatabaseWatcher();
};

//---------------------------------------------------------------------------------------------------------------------
inline auto MApplication::TrVars() -> const VTranslateVars *
{
    return m_trVars;
}

#endif // MAPPLICATION_H
