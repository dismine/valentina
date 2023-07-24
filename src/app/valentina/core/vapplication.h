/************************************************************************
 **
 **  @file   vapplication.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#ifndef VAPPLICATION_H
#define VAPPLICATION_H

#include "../vmisc/vabstractvalapplication.h"
#include "../vmisc/vlockguard.h"
#include "../vpatterndb/vtranslatevars.h"
#include "vcmdexport.h"

class VApplication; // use in define

/**
 * @brief The VApplication class reimplamentation QApplication class.
 */
class VApplication : public VAbstractValApplication
{
    Q_OBJECT // NOLINT

public:
    VApplication(int &argc, char **argv);
    ~VApplication() override;

    static void NewValentina(const QString &fileName = QString());
    auto notify(QObject *receiver, QEvent *event) -> bool override;

    void InitOptions();

    static void StartDetachedProcess(const QString &program, const QStringList &arguments);

    static auto TapeFilePath() -> QString;
    static auto PuzzleFilePath() -> QString;

    auto getAutoSaveTimer() const -> QTimer *;
    void setAutoSaveTimer(QTimer *value);

    static auto LabelLanguages() -> QStringList;

    void StartLogging();
    auto LogFile() -> QTextStream *;

    auto TrVars() -> const VTranslateVars * override;

    auto static IsGUIMode() -> bool;
    auto IsAppInGUIMode() const -> bool override;
    auto IsPedantic() const -> bool override;

    static auto VApp() -> VApplication *;
    static auto CommandLine() -> VCommandLinePtr;

protected:
    void InitTrVars() override;
    auto event(QEvent *e) -> bool override;

protected slots:
    void AboutToQuit() override;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VApplication) // NOLINT
    VTranslateVars *m_trVars{nullptr};
    QTimer *m_autoSaveTimer{nullptr};

    QSharedPointer<VLockGuard<QFile>> m_lockLog{};
    std::shared_ptr<QTextStream> m_out{nullptr};

    static auto LogDirPath() -> QString;
    static auto LogPath() -> QString;
    static auto CreateLogDir() -> bool;
    void BeginLogging();
    static void ClearOldLogs();
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VApplication::getAutoSaveTimer() const -> QTimer *
{
    return m_autoSaveTimer;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VApplication::setAutoSaveTimer(QTimer *value)
{
    m_autoSaveTimer = value;
}

#endif // VAPPLICATION_H
