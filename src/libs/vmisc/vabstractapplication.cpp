/************************************************************************
 **
 **  @file   vabstractapplication.cpp
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

#include "vabstractapplication.h"

#include <QDir>
#include <QLibraryInfo>
#include <QLoggingCategory>
#include <QMessageLogger>
#include <QTranslator>
#include <QUndoStack>
#include <Qt>
#include <QtDebug>
#include <QWidget>
#include <QStandardPaths>

#include "compatibility.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include "../vmisc/vtextcodec.h"
#else
#include <QTextCodec>
#endif

#ifdef Q_OS_UNIX
#  include <unistd.h>
#endif

#if defined(APPIMAGE) && defined(Q_OS_LINUX)
#   include "appimage.h"
#endif // defined(APPIMAGE) && defined(Q_OS_LINUX)

namespace
{
auto FilterLocales(const QStringList &locales) -> QStringList
{
    QStringList filtered;
    for (const auto &locale : locales)
    {
        if (not locale.startsWith(QLatin1String("ru")))
        {
            filtered.append(locale);
        }
    }

    return filtered;
}

//---------------------------------------------------------------------------------------------------------------------
auto LoadQM(QTranslator *translator, const QString &filename, const QString &locale, const QString &qmDir) -> bool
{
    QStringList languages;
    if (not locale.isEmpty())
    {
        languages.append(locale);
    }
    else
    {
        languages = QLocale().uiLanguages();
    }

    languages = FilterLocales(languages);

    for (auto &locale : languages)
    {
        const bool loaded = translator->load(filename + locale, qmDir);
        if (loaded)
        {
            return loaded;
        }
    }

    return false;
}
}  // namespace

const QString VAbstractApplication::warningMessageSignature = QStringLiteral("[PATTERN MESSAGE]");

//---------------------------------------------------------------------------------------------------------------------
VAbstractApplication::VAbstractApplication(int &argc, char **argv)
    :QApplication(argc, argv),
      undoStack(new QUndoStack(this))
{
    QString rules;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 1)
#if defined(V_NO_ASSERT)
    // Ignore SSL-related warnings
    // See issue #528: Error: QSslSocket: cannot resolve SSLv2_client_method.
    rules += QLatin1String("qt.network.ssl.warning=false\n");
    // See issue #568: Certificate checking on Mac OS X.
    rules += QLatin1String("qt.network.ssl.critical=false\n"
                           "qt.network.ssl.fatal=false\n");
#endif //defined(V_NO_ASSERT)
#endif // QT_VERSION >= QT_VERSION_CHECK(5, 4, 1)

#if defined(V_NO_ASSERT)
    // See issue #992: QXcbConnection: XCB Error.
    rules += QLatin1String("qt.qpa*=false\n");
    rules += QLatin1String("kf5.kio.core*=false\n");
    rules += QLatin1String("qt.gui.icc.warning=false\n");
#endif

    if (not rules.isEmpty())
    {
        QLoggingCategory::setFilterRules(rules);
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Enable support for HiDPI bitmap resources
    // The attribute is available since Qt 5.1, but by default disabled.
    // Because on Windows and Mac OS X we always use last version
    // and Linux users send bug reports probably they related to this attribute
    // better not enable it before Qt 5.6.
    //
    // Related issues:
    // Issue #584. frequent xcb errors and hangs
    // https://bitbucket.org/dismine/valentina/issues/584/frequent-xcb-errors-and-hangs
    // Issue #527. Error: Pasting a wrong formula : every dialog box is "glued" to the screen and can't close file
    // or Valentina.
    // https://bitbucket.org/dismine/valentina/issues/527/error-pasting-a-wrong-formula-every-dialog

    setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    connect(this, &QApplication::aboutToQuit, this, &VAbstractApplication::AboutToQuit);
}

//---------------------------------------------------------------------------------------------------------------------
VAbstractApplication::~VAbstractApplication()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QHashIterator<QStringConverter::Encoding, VTextCodec *> i(m_codecs);
    while (i.hasNext())
    {
        i.next();
        delete i.value();
    }
#endif
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief translationsPath return path to the root directory that contain QM files.
 * @param locale used only in Mac OS. If empty return path to the root directory. If not - return path to locale
 * subdirectory inside an app bundle.
 * @return path to a directory that contain QM files.
 */
auto VAbstractApplication::translationsPath(const QString &locale) -> QString
{
    const QString trPath = QStringLiteral("/translations");
#ifdef Q_OS_WIN
    Q_UNUSED(locale)
    return QCoreApplication::applicationDirPath() + trPath;
#elif defined(Q_OS_MAC)
    QString mainPath;
    if (locale.isEmpty())
    {
        mainPath = QCoreApplication::applicationDirPath() + QLatin1String("/../Resources") + trPath;
    }
    else
    {
        mainPath = QCoreApplication::applicationDirPath() + QLatin1String("/../Resources") + trPath + QLatin1String("/")
                + locale + QLatin1String(".lproj");
    }
    QDir dirBundle(mainPath);
    if (dirBundle.exists())
    {
        return dirBundle.absolutePath();
    }
    else
    {
        QDir appDir = QDir(qApp->applicationDirPath());
        appDir.cdUp();
        appDir.cdUp();
        appDir.cdUp();
        QDir dir(appDir.absolutePath() + trPath);
        if (dir.exists())
        {
            return dir.absolutePath();
        }
        else
        {
            return PKGDATADIR + trPath;
        }
    }
#else // Unix
    Q_UNUSED(locale)
    QDir dir(QCoreApplication::applicationDirPath() + trPath);
    if (dir.exists())
    {
        return dir.absolutePath();
    }

#if defined(APPIMAGE) && defined(Q_OS_LINUX)
    /* Fix path to trasnaltions when run inside AppImage. */
    return AppImageRoot() + PKGDATADIR + trPath;
#else
    return PKGDATADIR + trPath;
#endif // defined(APPIMAGE) && defined(Q_OS_LINUX)
#endif // Unix
}

//---------------------------------------------------------------------------------------------------------------------
QUndoStack *VAbstractApplication::getUndoStack() const
{
    return undoStack;
}

//---------------------------------------------------------------------------------------------------------------------
bool VAbstractApplication::IsPedantic() const
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
#if defined(Q_OS_WIN)
void VAbstractApplication::WinAttachConsole()
{
    /* Windows does not really support dual mode applications.
     * To see console output we need to attach console.
     * For case of using pipeline we check std output handler.
     * Original idea: https://stackoverflow.com/a/41701133/3045403
     */
    auto stdout_type = GetFileType(GetStdHandle(STD_OUTPUT_HANDLE));
    if (stdout_type == FILE_TYPE_UNKNOWN && AttachConsole(ATTACH_PARENT_PROCESS))
    {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
}
#endif

//---------------------------------------------------------------------------------------------------------------------
void VAbstractApplication::LoadTranslation(QString locale)
{
    if (locale.startsWith(QLatin1String("ru")))
    {
        locale = QString();
    }

    if (locale.isEmpty())
    {
        qDebug()<<"Default locale";
    }
    else
    {
        qDebug()<<"Checked locale:"<<locale;
    }

    ClearTranslation();

    const QString appQmDir = VAbstractApplication::translationsPath(locale);

    qtTranslator = new QTranslator(this);
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
    const QString qtQmDir = appQmDir;
#else
    const QString qtQmDir = QLibraryPath(QLibraryInfo::TranslationsPath);
#endif
    LoadQM(qtTranslator, QStringLiteral("qt_"), locale, qtQmDir);
    installTranslator(qtTranslator);

#if defined(Q_OS_WIN) && !defined(QBS_BUILD)
    qtxmlTranslator = new QTranslator(this);
    LoadQM(qtxmlTranslator, QStringLiteral("qtxmlpatterns_"), locale, qtQmDir);
    installTranslator(qtxmlTranslator);

    qtBaseTranslator = new QTranslator(this);
    LoadQM(qtBaseTranslator, QStringLiteral("qtbase_"), locale, qtQmDir);
    installTranslator(qtBaseTranslator);
#endif // defined(Q_OS_WIN) && !defined(QBS_BUILD)

    appTranslator = new QTranslator(this);
    LoadQM(appTranslator, QStringLiteral("valentina_"), locale, appQmDir);
    installTranslator(appTranslator);

    pmsTranslator = new QTranslator(this);
    LoadQM(pmsTranslator, QStringLiteral("measurements_") + Settings()->GetPMSystemCode() + '_', locale, appQmDir);
    installTranslator(pmsTranslator);

    InitTrVars();//Very important do it after load QM files.
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractApplication::ClearTranslation()
{
    if (not qtTranslator.isNull())
    {
        removeTranslator(qtTranslator);
        delete qtTranslator;
    }

#if defined(Q_OS_WIN) && !defined(QBS_BUILD)
    if (not qtxmlTranslator.isNull())
    {
        removeTranslator(qtxmlTranslator);
        delete qtxmlTranslator;
    }

    if (not qtBaseTranslator.isNull())
    {
        removeTranslator(qtBaseTranslator);
        delete qtBaseTranslator;
    }
#endif // defined(Q_OS_WIN) && !defined(QBS_BUILD)

    if (not appTranslator.isNull())
    {
        removeTranslator(appTranslator);
        delete appTranslator;
    }

    if (not pmsTranslator.isNull())
    {
        removeTranslator(pmsTranslator);
        delete pmsTranslator;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ClearMessage helps to clear a message string from standard Qt function.
 * @param msg the message that contains '"' at the start and at the end
 * @return cleared string
 */
QString VAbstractApplication::ClearMessage(QString msg)
{
    if (msg.startsWith('"') && msg.endsWith('"'))
    {
        msg.remove(0, 1);
        msg.chop(1);
    }

    msg.replace("\\\"", "\"");

    return msg;
}

//---------------------------------------------------------------------------------------------------------------------
bool VAbstractApplication::IsWarningMessage(const QString &message) const
{
    return VAbstractApplication::ClearMessage(message).startsWith(warningMessageSignature);
}

//---------------------------------------------------------------------------------------------------------------------
QFileDialog::Options VAbstractApplication::NativeFileDialog(QFileDialog::Options options) const
{
    if (settings->IsDontUseNativeDialog())
    {
        options |= QFileDialog::DontUseNativeDialog;
    }

    return options;
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
//---------------------------------------------------------------------------------------------------------------------
VTextCodec *VAbstractApplication::TextCodecCache(QStringConverter::Encoding encoding) const
{
    if (m_codecs.contains(encoding))
    {
        return m_codecs.value(encoding);
    }

    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractApplication::CacheTextCodec(QStringConverter::Encoding encoding, VTextCodec *codec)
{
    if (not m_codecs.contains(encoding))
    {
        m_codecs.insert(encoding, codec);
    }
}
#endif

//---------------------------------------------------------------------------------------------------------------------
void VAbstractApplication::CheckSystemLocale()
{
    const QString defLocale = QLocale::system().name();
    if (defLocale.startsWith(QLatin1String("ru")))
    {
        qFatal("Incompatible locale \"%s\"", qPrintable(defLocale));
    }

    auto CheckLanguage =[](QStandardPaths::StandardLocation type, const QStringList &test)
    {
        const QString path = QStandardPaths::locate(type, QString(), QStandardPaths::LocateDirectory);
        return std::any_of(test.begin(), test.end(), [path](const QString &t) { return path.contains(t); });
    };

    int match = 0;
    match += CheckLanguage(QStandardPaths::DesktopLocation, {"Рабочий стол"});
    match += CheckLanguage(QStandardPaths::DocumentsLocation, {"Мои документы", "Документы"});
    match += CheckLanguage(QStandardPaths::MusicLocation, {"Моя музыка", "Музыка"});
    match += CheckLanguage(QStandardPaths::MoviesLocation, {"Мои видео", "Видео"});
    match += CheckLanguage(QStandardPaths::PicturesLocation, {"Мои рисунки", "Изображения", "Картинки"});
    match += CheckLanguage(QStandardPaths::DownloadLocation, {"Мои документы", "Загрузки"});

    if (match >= 4)
    {
        qFatal("russian language detected");
    }
}
