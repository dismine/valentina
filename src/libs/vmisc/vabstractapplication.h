/************************************************************************
 **
 **  @file   vabstractapplication.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   18 6, 2015
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

#ifndef VABSTRACTAPPLICATION_H
#define VABSTRACTAPPLICATION_H

#include <QApplication>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QLocale>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QTranslator>
#include <QtGlobal>

#include "../vpatterndb/vtranslatevars.h"
#include "def.h"
#include "vcommonsettings.h"

class QUndoStack;
class VAbstractApplication; // use in define
class VCommonSettings;
class VSvgFontDatabase;
class QFileSystemWatcher;
class VAbstractShortcutManager;
class VKnownMeasurementsDatabase;
class VTranslator;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
class VTextCodec;
#else
class QTextCodec;
using VTextCodec = QTextCodec;
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")
QT_WARNING_DISABLE_GCC("-Wsuggest-final-methods")

class VAbstractApplication : public QApplication
{
    Q_OBJECT // NOLINT

public:
    VAbstractApplication(int &argc, char **argv);
    ~VAbstractApplication() override;

    virtual auto TrVars() -> const VTranslateVars * = 0;

    static auto translationsPath(const QString &locale = QString()) -> QString;
    static auto QtTranslationsPath(const QString &locale = QString()) -> QString;

    static auto ReduceLogContextFilePath(QString path) -> QString;

    void LoadTranslation(QString locale);

    virtual void OpenSettings() = 0;
    auto Settings() -> VCommonSettings *;

    template <typename T> auto LocaleToString(const T &value) -> QString;

    auto getUndoStack() const -> QUndoStack *;

    virtual auto IsAppInGUIMode() const -> bool = 0;
    virtual auto IsPedantic() const -> bool;

    static auto ClearMessage(QString msg) -> QString;

    static const QString warningMessageSignature;
    auto IsWarningMessage(const QString &message) const -> bool;

    auto NativeFileDialog(QFileDialog::Options options = QFileDialog::Options()) const -> QFileDialog::Options;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    auto TextCodecCache(QStringConverter::Encoding encoding) const -> VTextCodec *;
    void CacheTextCodec(QStringConverter::Encoding encoding, VTextCodec *codec);
#endif

#if defined(Q_OS_WIN)
    static void WinAttachConsole();
#endif

    static auto VApp() -> VAbstractApplication *;

    auto SVGFontDatabase() -> VSvgFontDatabase *;

    virtual auto KnownMeasurementsDatabase() -> VKnownMeasurementsDatabase *;

    auto AppUptime() const -> qint64;

    auto GetShortcutManager() const -> VAbstractShortcutManager *;

    auto GetPlaceholderTranslator() -> QSharedPointer<VTranslator>;

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays)
    static auto IsOptionSet(int argc, char *argv[], const char *option) -> bool;
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays)
    static void InitHighDpiScaling(int argc, char *argv[]);

    static auto LogDirPath() -> QString;
    static auto CreateLogDir() -> bool;
    static void ClearOldLogs();

protected:
    QUndoStack *undoStack;

    /**
     * @brief settings pointer to settings. Help hide constructor creation settings. Make make code more readable.
     */
    VCommonSettings *settings{nullptr};

    QPointer<QTranslator> qtTranslator{nullptr};
#if (defined(Q_OS_WIN) && !defined(QBS_BUILD)) || defined(APPIMAGE)
    QPointer<QTranslator> qtxmlTranslator{nullptr};
    QPointer<QTranslator> qtBaseTranslator{nullptr};
#endif // defined(Q_OS_WIN) && !defined(QBS_BUILD)
    QPointer<QTranslator> appTranslator{nullptr};

    QElapsedTimer m_uptimeTimer{};

    VAbstractShortcutManager *m_shortcutManager{nullptr};

    virtual void InitTrVars() = 0;

    static void CheckSystemLocale();

protected slots:
    virtual void AboutToQuit() = 0;
    void SVGFontsPathChanged(const QString &oldPath, const QString &newPath);

private slots:
    void RepopulateFontDatabase(const QString &path);

private:
    Q_DISABLE_COPY_MOVE(VAbstractApplication) // NOLINT

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QHash<QStringConverter::Encoding, VTextCodec *> m_codecs{};
#endif

    VSvgFontDatabase *m_svgFontDatabase{nullptr};
    QFileSystemWatcher *m_svgFontDatabaseWatcher{nullptr};

    void ClearTranslation();

    void RestartSVGFontDatabaseWatcher();
};

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
template <typename T> inline auto VAbstractApplication::LocaleToString(const T &value) -> QString
{
    QLocale loc;
    VAbstractApplication::VApp()->Settings()->GetOsSeparator() ? loc = QLocale() : loc = QLocale::c();
    return loc.toString(value);
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractApplication::VApp() -> VAbstractApplication *
{
    return qobject_cast<VAbstractApplication *>(QCoreApplication::instance());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getSettings hide settings constructor.
 * @return pointer to class for acssesing to settings in ini file.
 */
inline auto VAbstractApplication::Settings() -> VCommonSettings *
{
    SCASSERT(settings != nullptr)
    return settings;
}

#endif // VABSTRACTAPPLICATION_H
