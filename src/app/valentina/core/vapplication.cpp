/************************************************************************
 **
 **  @file   vapplication.cpp
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

#include "vapplication.h"
#include "../ifc/exception/vexceptionbadid.h"
#include "../ifc/exception/vexceptionconversionerror.h"
#include "../ifc/exception/vexceptionemptyparameter.h"
#include "../ifc/exception/vexceptioninvalidnotch.h"
#include "../ifc/exception/vexceptionobjecterror.h"
#include "../ifc/exception/vexceptionwrongid.h"
#include "../mainwindow.h"
#include "../qmuparser/qmuparsererror.h"
#include "../version.h"
#include "../vformat/knownmeasurements/vknownmeasurementsdatabase.h"
#include "../vganalytics/vganalytics.h"
#include "../vmisc/qt_dispatch/qt_dispatch.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vsysexits.h"
#include "../vmisc/vvalentinasettings.h"
#include "vvalentinashortcutmanager.h"

#include "QtConcurrent/qtconcurrentrun.h"
#include <vcsRepoState.h>
#include <QDateTime>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QFileSystemWatcher>
#include <QFuture>
#include <QIcon>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QObject>
#include <QProcess>
#include <QScopeGuard>
#include <QStandardPaths>
#include <QStyleFactory>
#include <QTemporaryFile>
#include <QThread>
#include <QTimer>
#include <QUuid>
#include <QtDebug>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QtXmlPatterns>
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

#if defined(APPIMAGE) && defined(Q_OS_LINUX)
#include "../vmisc/appimage.h"
#endif // defined(APPIMAGE) && defined(Q_OS_LINUX)

using namespace Qt::Literals::StringLiterals;

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(vApp, "v.application") // NOLINT

QT_WARNING_POP

namespace
{
auto AppFilePath(const QString &appName) -> QString
{
#ifdef Q_OS_WIN
    const auto executableSuffix = QStringLiteral(".exe");
#else
    const QString executableSuffix;
#endif

#if defined(APPIMAGE) && defined(Q_OS_LINUX)
    return AppImageRoot() + BINDIR + '/'_L1 + appName;
#else
    if (QFileInfo const canonicalFile(
            QStringLiteral("%1/%2").arg(QCoreApplication::applicationDirPath(), appName + executableSuffix));
        canonicalFile.exists())
    {
        return canonicalFile.absoluteFilePath();
    }
#endif // defined(APPIMAGE) && defined(Q_OS_LINUX)

#if defined(Q_OS_MACOS) && defined(MULTI_BUNDLE)
    QFileInfo multiBundleFile(
        QStringLiteral("%1/../../../%2.app/Contents/MacOS/%2").arg(QCoreApplication::applicationDirPath(), appName));
    if (multiBundleFile.exists())
    {
        return multiBundleFile.absoluteFilePath();
    }
#endif

#if !defined(Q_OS_MACOS)
    return appName + executableSuffix;
#else
    return appName + QStringLiteral(".app");
#endif
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
inline void LogMessageDetails(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString debugdate = "["_L1 + QDateTime::currentDateTime().toString(QStringLiteral("yyyy.MM.dd hh:mm:ss"));
    QString const file = VAbstractApplication::ReduceLogContextFilePath(context.file);
    const bool isPatternMessage = VAbstractApplication::VApp()->IsWarningMessage(msg);

    switch (type)
    {
        case QtDebugMsg:
            debugdate += QStringLiteral(":DEBUG:%1(%2)] %3: %4: %5")
                             .arg(file)
                             .arg(context.line)
                             .arg(context.function, context.category, msg);
            vStdOut() << QApplication::translate("vNoisyHandler", "DEBUG:") << msg << "\n";
            break;
        case QtWarningMsg:
            if (isPatternMessage)
            {
                VAbstractValApplication::VApp()->PostWarningMessage(msg, type);
            }
            debugdate += QStringLiteral(":WARNING:%1(%2)] %3: %4: %5")
                             .arg(file)
                             .arg(context.line)
                             .arg(context.function, context.category, msg);
            vStdErr() << QApplication::translate("vNoisyHandler", "WARNING:") << msg << "\n";
            break;
        case QtCriticalMsg:
            if (isPatternMessage)
            {
                VAbstractValApplication::VApp()->PostWarningMessage(msg, type);
            }
            debugdate += QStringLiteral(":CRITICAL:%1(%2)] %3: %4: %5")
                             .arg(file)
                             .arg(context.line)
                             .arg(context.function, context.category, msg);
            vStdErr() << QApplication::translate("vNoisyHandler", "CRITICAL:") << msg << "\n";
            break;
        case QtFatalMsg:
            debugdate += QStringLiteral(":FATAL:%1(%2)] %3: %4: %5")
                             .arg(file)
                             .arg(context.line)
                             .arg(context.function, context.category, msg);
            vStdErr() << QApplication::translate("vNoisyHandler", "FATAL:") << msg << "\n";
            break;
        case QtInfoMsg:
            if (isPatternMessage)
            {
                VAbstractValApplication::VApp()->PostWarningMessage(msg, type);
            }
            debugdate += QStringLiteral(":INFO:%1(%2)] %3: %4: %5")
                             .arg(file)
                             .arg(context.line)
                             .arg(context.function, context.category, msg);
            vStdOut() << QApplication::translate("vNoisyHandler", "INFO:") << msg << "\n";
            break;
        default:
            break;
    }
    vStdOut().flush();
    vStdErr().flush();

    (*VApplication::VApp()->LogFile()) << debugdate << Qt::endl;
}

//---------------------------------------------------------------------------------------------------------------------
inline void noisyFailureMsgHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // only the GUI thread should display message boxes.  If you are
    // writing a multithreaded application and the error happens on
    // a non-GUI thread, you'll have to queue the message to the GUI
    QCoreApplication *instance = QCoreApplication::instance();
    const bool isGuiThread = instance && (QThread::currentThread() == instance->thread());

    if (not isGuiThread)
    {
        auto Handler = [](QtMsgType type, const QMessageLogContext &context, const QString &msg)
        { noisyFailureMsgHandler(type, context, msg); };

        q_dispatch_async_main(Handler, type, context, msg);
        return;
    }

    // Why on earth didn't Qt want to make failed signal/slot connections qWarning?
    if ((type == QtDebugMsg) && msg.contains(QStringLiteral("::connect")))
    {
        type = QtWarningMsg;
    }

#if defined(V_NO_ASSERT)
    // I have decided to hide this annoing message for release builds.
    if ((type == QtWarningMsg) && msg.contains(QStringLiteral("QSslSocket: cannot resolve")))
    {
        type = QtDebugMsg;
    }

    if ((type == QtWarningMsg) && msg.contains(QStringLiteral("setGeometry: Unable to set geometry")))
    {
        type = QtDebugMsg;
    }
#endif // defined(V_NO_ASSERT)

#if defined(Q_OS_MAC)
    // Hide anything that starts with QMacCGContext
    if ((type == QtWarningMsg) && msg.contains(QStringLiteral("QMacCGContext::")))
    {
        type = QtDebugMsg;
    }

    // See issue #568
    if (msg.contains(QStringLiteral("Error receiving trust for a CA certificate")))
    {
        type = QtDebugMsg;
    }
#endif

    // this is another one that doesn't make sense as just a debug message.  pretty serious
    // sign of a problem
    // http://www.developer.nokia.com/Community/Wiki/QPainter::begin:Paint_device_returned_engine_%3D%3D_0_(Known_Issue)
    if ((type == QtDebugMsg) && msg.contains(QStringLiteral("QPainter::begin")) &&
        msg.contains(QStringLiteral("Paint device returned engine")))
    {
        type = QtWarningMsg;
    }

    // This qWarning about "Cowardly refusing to send clipboard message to hung application..."
    // is something that can easily happen if you are debugging and the application is paused.
    // As it is so common, not worth popping up a dialog.
    if ((type == QtWarningMsg) && msg.contains(QStringLiteral("QClipboard::event")) &&
        msg.contains(QStringLiteral("Cowardly refusing")))
    {
        type = QtDebugMsg;
    }

    // Annoying warning that we can ignore
    if ((type == QtWarningMsg) && (msg.contains(QStringLiteral("OpenType support missing for")) ||
                                   msg.contains(QStringLiteral("DirectWrite: CreateFontFaceFromHDC() failed (Indicates "
                                                               "an error in an input file such as a font file.)"))))
    {
        type = QtDebugMsg;
    }

    QString logMsg = msg;
    const bool isPatternMessage = VAbstractApplication::VApp()->IsWarningMessage(msg);
    if (isPatternMessage)
    {
        logMsg = logMsg.remove(VAbstractValApplication::warningMessageSignature);
    }

    LogMessageDetails(type, context, msg);

    if (isGuiThread)
    {
        if ((type == QtWarningMsg || type == QtCriticalMsg || type == QtFatalMsg) && VApplication::IsGUIMode())
        {
            // fixme: trying to make sure there are no save/load dialogs are opened, because error message during
            //  them will lead to crash
            const bool topWinAllowsPop = (QApplication::activeModalWidget() == nullptr) ||
                                         !QApplication::activeModalWidget()->inherits("QFileDialog");

            if (topWinAllowsPop && (not isPatternMessage || (type == QtCriticalMsg || type == QtFatalMsg)))
            {
                QMessageBox messageBox;
                switch (type)
                {
                    case QtWarningMsg:
                        messageBox.setWindowTitle(QApplication::translate("vNoisyHandler", "Warning"));
                        messageBox.setIcon(QMessageBox::Warning);
                        break;
                    case QtCriticalMsg:
                        messageBox.setWindowTitle(QApplication::translate("vNoisyHandler", "Critical error"));
                        messageBox.setIcon(QMessageBox::Critical);
                        break;
                    case QtFatalMsg:
                        messageBox.setWindowTitle(QApplication::translate("vNoisyHandler", "Fatal error"));
                        messageBox.setIcon(QMessageBox::Critical);
                        break;
                    case QtInfoMsg:
                        messageBox.setWindowTitle(QApplication::translate("vNoisyHandler", "Information"));
                        messageBox.setIcon(QMessageBox::Information);
                        break;
                    case QtDebugMsg:
                    default:
                        break;
                }

                messageBox.setText(VAbstractValApplication::ClearMessage(logMsg));
                messageBox.setStandardButtons(QMessageBox::Ok);
                messageBox.setWindowModality(Qt::ApplicationModal);
                messageBox.setModal(true);
#ifndef QT_NO_CURSOR
                QGuiApplication::setOverrideCursor(Qt::ArrowCursor);
#endif
                messageBox.exec();
#ifndef QT_NO_CURSOR
                QGuiApplication::restoreOverrideCursor();
#endif
            }
        }

        if (QtFatalMsg == type)
        {
            abort();
        }
    }
    else
    {
        if (type != QtDebugMsg)
        {
            abort(); // be NOISY unless overridden!
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VApplication constructor.
 * @param argc number arguments.
 * @param argv command line.
 */
VApplication::VApplication(int &argc, char **argv)
  : VAbstractValApplication(argc, argv)
{
    setApplicationDisplayName(QStringLiteral(VER_PRODUCTNAME_STR));
    setApplicationName(QStringLiteral(VER_INTERNALNAME_STR));
    setOrganizationName(QStringLiteral(VER_COMPANYNAME_STR));
    setOrganizationDomain(QStringLiteral(VER_COMPANYDOMAIN_STR));
    // Setting the Application version
    setApplicationVersion(AppVersionStr());
    // making sure will create new instance...just in case we will ever do 2 objects of VApplication
    VCommandLine::Reset();
    VTheme::Instance()->StoreDefaultThemeName(QIcon::themeName());
}

//---------------------------------------------------------------------------------------------------------------------
VApplication::~VApplication()
{
    qCDebug(vApp, "Application closing.");

    if (IsGUIMode() && settings->IsCollectStatistic())
    {
        auto *statistic = VGAnalytics::Instance();

        QString const clientID = settings->GetClientID();
        if (!clientID.isEmpty())
        {
            statistic->SendAppCloseEvent(m_uptimeTimer.elapsed());
        }
    }

    qInstallMessageHandler(nullptr); // Resore the message handler
    delete m_trVars;
    VCommandLine::Reset();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief NewValentina start Valentina in new process, send path to pattern file in argument.
 * @param fileName path to pattern file.
 */
void VApplication::NewValentina(const QString &fileName)
{
    qCDebug(vApp, "Open new detached process.");
    if (fileName.isEmpty())
    {
        qCDebug(vApp, "New process without arguments. program = %s",
                qUtf8Printable(QCoreApplication::applicationFilePath()));
        // Path can contain spaces.
        if (QProcess::startDetached(QCoreApplication::applicationFilePath(), QStringList()))
        {
            qCDebug(vApp, "The process was started successfully.");
        }
        else
        {
            qCWarning(vApp, "Could not run process. The operation timed out or an error occurred.");
        }
    }
    else
    {
        const auto run = QStringLiteral("\"%1\" \"%2\"").arg(QCoreApplication::applicationFilePath(), fileName);
        qCDebug(vApp, "New process with arguments. program = %s", qUtf8Printable(run));
        if (QProcess::startDetached(QCoreApplication::applicationFilePath(), QStringList{fileName}))
        {
            qCDebug(vApp, "The process was started successfully.");
        }
        else
        {
            qCWarning(vApp, "Could not run process. The operation timed out or an error occurred.");
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief notify Reimplemented from QApplication::notify().
 * @param receiver receiver.
 * @param event event.
 * @return value that is returned from the receiver's event handler.
 */
// reimplemented from QApplication so we can throw exceptions in slots
auto VApplication::notify(QObject *receiver, QEvent *event) -> bool
{
    try
    {
        return QApplication::notify(receiver, event);
    }
    catch (const VExceptionObjectError &e)
    {
        qCCritical(vApp, "%s\n\n%s\n\n%s",
                   qUtf8Printable(tr("Error parsing file. Program will be terminated.")), //-V807
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VExceptionBadId &e)
    {
        qCCritical(vApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error bad id. Program will be terminated.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VExceptionConversionError &e)
    {
        qCCritical(vApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error can't convert value. Program will be terminated.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VExceptionEmptyParameter &e)
    {
        qCCritical(vApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error empty parameter. Program will be terminated.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VExceptionWrongId &e)
    {
        qCCritical(vApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error wrong id. Program will be terminated.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        qCCritical(vApp, "%s\n\n%s\n\n%s",
                   qUtf8Printable(QStringLiteral("Unhadled deleting tool. Continue use object after deleting")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VExceptionInvalidNotch &e)
    {
        qCCritical(vApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Invalid notch.")), qUtf8Printable(e.ErrorMessage()),
                   qUtf8Printable(e.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VException &e)
    {
        qCCritical(vApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Something's wrong!!")), qUtf8Printable(e.ErrorMessage()),
                   qUtf8Printable(e.DetailedInformation()));
        return true;
    }
    catch (const qmu::QmuParserWarning &e)
    {
        qCCritical(vApp, "%s", qUtf8Printable(tr("Formula warning: %1. Program will be terminated.").arg(e.GetMsg())));
        exit(V_EX_DATAERR);
    }
    // These last two cases are special. I found that we can't show here a modal dialog with an error message.
    // Somehow program doesn't wait until an error dialog will be closed. But if ignore the exception the program will
    // hang.
    catch (const qmu::QmuParserError &e)
    {
        qCCritical(vApp, "%s", qUtf8Printable(tr("Parser error: %1. Program will be terminated.").arg(e.GetMsg())));
        exit(V_EX_DATAERR);
    }
    catch (std::exception &e)
    {
        qCCritical(vApp, "%s", qUtf8Printable(tr("Exception thrown: %1. Program will be terminated.").arg(e.what())));
        exit(V_EX_SOFTWARE);
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto VApplication::TapeFilePath() -> QString
{
#ifdef Q_OS_MACOS
    const auto appName = QStringLiteral("Tape");
#else
    const auto appName = QStringLiteral("tape");
#endif
    return AppFilePath(appName);
}

//---------------------------------------------------------------------------------------------------------------------
auto VApplication::PuzzleFilePath() -> QString
{
#ifdef Q_OS_MACOS
    const auto appName = QStringLiteral("Puzzle");
#else
    const auto appName = QStringLiteral("puzzle");
#endif
    return AppFilePath(appName);
}

//---------------------------------------------------------------------------------------------------------------------
auto VApplication::LogPath() -> QString
{
    // Keep in sync with VCrashPaths::GetAttachmentPath
    return QStringLiteral("%1/valentina-pid%2.log").arg(LogDirPath()).arg(applicationPid());
}

//---------------------------------------------------------------------------------------------------------------------
void VApplication::BeginLogging()
{
    VlpCreateLock(m_lockLog, LogPath(), []() { return new QFile(LogPath()); });

    if (m_lockLog->IsLocked())
    {
        if (m_lockLog->GetProtected()->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        {
            m_out.reset(new QTextStream(m_lockLog->GetProtected().data()));
            qInstallMessageHandler(noisyFailureMsgHandler);
            qCDebug(vApp, "Log file %s was locked.", qUtf8Printable(LogPath()));
        }
        else
        {
            qCDebug(vApp, "Error opening log file \'%s\'. All debug output redirected to console.",
                    qUtf8Printable(LogPath()));
        }
    }
    else
    {
        qCDebug(vApp, "Failed to lock %s", qUtf8Printable(LogPath()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VApplication::InitOptions()
{
    OpenSettings();

    StartLogging();

    qDebug() << "Version:" << AppVersionStr();
    qDebug() << "Build revision:" << VCS_REPO_STATE_REVISION;
    qDebug() << buildCompatibilityString();
    qDebug() << "Built on" << __DATE__ << "at" << __TIME__;
    qDebug() << "Command-line arguments:" << arguments();
    qDebug() << "Process ID:" << applicationPid();

    LoadTranslation(QString()); // By default the console version uses system locale

    // Create command line parser after loading translations to show localized version.
    VCommandLine::Get(*this);

    CheckSystemLocale();

    QTimer::singleShot(0, this,
                       []()
                       {
                           QString const country = VGAnalytics::CountryCode();
                           if (country == "ru"_L1 || country == "by"_L1 || country == "ir"_L1)
                           {
                               QCoreApplication::exit();
                           }
                       });

    if (VApplication::IsGUIMode()) // By default console version uses system locale
    {
        LoadTranslation(ValentinaSettings()->GetLocale());

        VTheme::InitApplicationStyle();
        VTheme::SetIconTheme();
        VTheme::InitThemeMode();

        m_shortcutManager = new VValentinaShortcutManager(this);
    }

    VGAnalytics::Init(settings);
}

//---------------------------------------------------------------------------------------------------------------------
void VApplication::StartDetachedProcess(const QString &program, const QStringList &arguments)
{
#if !defined(Q_OS_MACOS)
    const QString workingDirectory = QFileInfo(program).absoluteDir().absolutePath();
    QProcess::startDetached(program, arguments, workingDirectory);
#else
    if (not program.endsWith(".app"_L1))
    {
        const QString workingDirectory = QFileInfo(program).absoluteDir().absolutePath();
        QProcess::startDetached(program, arguments, workingDirectory);
    }
    else
    {
        QStringList openArguments{"-n", QStringLiteral("/Applications/%1").arg(program)};
        if (not arguments.isEmpty())
        {
            openArguments.append("--args"_L1);
            openArguments += arguments;
        }

        QProcess::startDetached("open"_L1, openArguments);
    }
#endif
}

//---------------------------------------------------------------------------------------------------------------------
auto VApplication::LabelLanguages() -> QStringList
{
    QStringList list{
        "de", // German
        "en", // English
        "fr", // French
        "ru", // Russian
        "uk", // Ukrainian
        "hr", // Croatian
        "sr", // Serbian
        "bs", // Bosnian
        "cs"  // Czech
    };
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
void VApplication::StartLogging()
{
    if (CreateLogDir())
    {
        BeginLogging();
        ClearOldLogs();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VApplication::LogFile() -> QTextStream *
{
    return m_out.get();
}

//---------------------------------------------------------------------------------------------------------------------
auto VApplication::KnownMeasurementsDatabase() -> VKnownMeasurementsDatabase *
{
    if (m_knownMeasurementsDatabase == nullptr)
    {
        m_knownMeasurementsDatabase = new VKnownMeasurementsDatabase();

        RestartKnownMeasurementsDatabaseWatcher();
    }

    if (!m_knownMeasurementsDatabase->IsPopulated())
    {
        m_knownMeasurementsDatabase->PopulateMeasurementsDatabase();
    }

    return m_knownMeasurementsDatabase;
}

//---------------------------------------------------------------------------------------------------------------------
void VApplication::RestartKnownMeasurementsDatabaseWatcher()
{
    if (m_knownMeasurementsDatabase != nullptr)
    {
        delete m_knownMeasurementsDatabaseWatcher;
        m_knownMeasurementsDatabaseWatcher = new QFileSystemWatcher({settings->GetPathKnownMeasurements()}, this);
        connect(m_knownMeasurementsDatabaseWatcher, &QFileSystemWatcher::directoryChanged, this,
                &VApplication::RepopulateMeasurementsDatabase);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VApplication::TrVars() -> const VTranslateVars *
{
    return m_trVars;
}

//---------------------------------------------------------------------------------------------------------------------
void VApplication::InitTrVars()
{
    if (m_trVars != nullptr)
    {
        m_trVars->Retranslate();
    }
    else
    {
        m_trVars = new VTranslateVars();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VApplication::event(QEvent *e) -> bool
{
    switch (e->type())
    {
        // In Mac OS X the QFileOpenEvent event is generated when user perform "Open With" from Finder (this event is
        // Mac specific).
        case QEvent::FileOpen:
        {
            auto *fileOpenEvent = dynamic_cast<QFileOpenEvent *>(e);
            if (const QString macFileOpen = fileOpenEvent->file(); not macFileOpen.isEmpty())
            {
                if (auto *window = qobject_cast<MainWindow *>(mainWindow); window != nullptr)
                {
                    window->LoadPattern(macFileOpen); // open file in existing window
                }
                return true;
            }
            break;
        }
#if defined(Q_OS_MAC)
        case QEvent::ApplicationStateChange:
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            if (static_cast<QApplicationStateChangeEvent *>(e)->applicationState() == Qt::ApplicationActive)
            {
                if (mainWindow && not mainWindow->isMinimized())
                {
                    mainWindow->show();
                }
                return true;
            }
            break;
#endif // defined(Q_OS_MAC)
        default:
            return VAbstractApplication::event(e);
    }
    return VAbstractApplication::event(e);
}

//---------------------------------------------------------------------------------------------------------------------
void VApplication::AboutToQuit()
{
    // If try to use the method QApplication::exit program can't sync settings and show warning about QApplication
    // instance. Solution is to call sync() before quit.
    // Connect this slot with VApplication::aboutToQuit.
    Settings()->sync();
}

//---------------------------------------------------------------------------------------------------------------------
void VApplication::RepopulateMeasurementsDatabase(const QString &path)
{
    Q_UNUSED(path)
    if (m_knownMeasurementsDatabase != nullptr)
    {
        QFuture<void> const future =
            QtConcurrent::run([this]() { m_knownMeasurementsDatabase->PopulateMeasurementsDatabase(); });
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VApplication::KnownMeasurementsPathChanged(const QString &oldPath, const QString &newPath)
{
    if (oldPath != newPath && m_knownMeasurementsDatabase != nullptr)
    {
        RestartKnownMeasurementsDatabaseWatcher();
        RepopulateMeasurementsDatabase(newPath);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VApplication::IsGUIMode() -> bool
{
    return (VCommandLine::instance != nullptr) && VCommandLine::instance->IsGuiEnabled();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief IsAppInGUIMode little hack that allow to have access to application state from VAbstractApplication class.
 */
auto VApplication::IsAppInGUIMode() const -> bool
{
    return IsGUIMode();
}

//---------------------------------------------------------------------------------------------------------------------
auto VApplication::IsPedantic() const -> bool
{
    return (VCommandLine::instance != nullptr) && VCommandLine::instance->IsPedantic();
}

//---------------------------------------------------------------------------------------------------------------------
void VApplication::OpenSettings()
{
    VAbstractValApplication::OpenSettings();
    connect(settings, &VValentinaSettings::KnownMeasurementsPathChanged, this,
            &VApplication::KnownMeasurementsPathChanged);
}

//---------------------------------------------------------------------------------------------------------------------
auto VApplication::VApp() -> VApplication *
{
    return qobject_cast<VApplication *>(QCoreApplication::instance());
}

//---------------------------------------------------------------------------------------------------------------------
auto VApplication::CommandLine() -> VCommandLinePtr
{
    return VCommandLine::instance;
}
