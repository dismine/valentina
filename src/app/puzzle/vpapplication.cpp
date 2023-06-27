/************************************************************************
 **
 **  @file   vpapplication.cpp
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

#include "vpapplication.h"
#include "../ifc/exception/vexceptionbadid.h"
#include "../ifc/exception/vexceptionconversionerror.h"
#include "../ifc/exception/vexceptionemptyparameter.h"
#include "../ifc/exception/vexceptionobjecterror.h"
#include "../ifc/exception/vexceptionwrongid.h"
#include "../vganalytics/def.h"
#include "../vganalytics/vganalytics.h"
#include "../vmisc/vsysexits.h"
#include "version.h"
#include "vpmainwindow.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../vmisc/diagnostic.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 5, 0)

#include "../fervor/fvupdater.h"
#include "../vmisc/qt_dispatch/qt_dispatch.h"

#include <QLoggingCategory>
#include <QMessageBox>

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(pApp, "p.application") // NOLINT

QT_WARNING_POP

#include <QCommandLineParser>
#include <QFileOpenEvent>
#include <QLocalServer>
#include <QLocalSocket>
#include <QPixmapCache>

#if !defined(BUILD_REVISION) && defined(QBS_BUILD)
#include <vcsRepoState.h>
#define BUILD_REVISION VCS_REPO_STATE_REVISION
#endif

//---------------------------------------------------------------------------------------------------------------------
inline void noisyFailureMsgHandler(QtMsgType type, const QMessageLogContext &context,
                                   const QString &msg) // NOLINT(readability-function-cognitive-complexity)
{
    // only the GUI thread should display message boxes.  If you are
    // writing a multithreaded application and the error happens on
    // a non-GUI thread, you'll have to queue the message to the GUI
    QCoreApplication *instance = QCoreApplication::instance();
    const bool isGuiThread = (instance != nullptr) && (QThread::currentThread() == instance->thread());

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
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0) && QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
    // Try hide very annoying, Qt related, warnings in Mac OS X
    // QNSView mouseDragged: Internal mouse button tracking invalid (missing Qt::LeftButton)
    // https://bugreports.qt.io/browse/QTBUG-42846
    if ((type == QtWarningMsg) && msg.contains(QStringLiteral("QNSView")))
    {
        type = QtDebugMsg;
    }
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 9, 0)
    // Hide Qt bug 'Assertion when reading an icns file'
    // https://bugreports.qt.io/browse/QTBUG-45537
    // Remove after Qt fix will be released
    if ((type == QtWarningMsg) && msg.contains(QStringLiteral("QICNSHandler::read()")))
    {
        type = QtDebugMsg;
    }
#endif

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

    switch (type)
    {
        case QtDebugMsg:
            vStdOut() << QApplication::translate("mNoisyHandler", "DEBUG:") << msg << "\n";
            return;
        case QtWarningMsg:
            vStdErr() << QApplication::translate("mNoisyHandler", "WARNING:") << msg << "\n";
            break;
        case QtCriticalMsg:
            vStdErr() << QApplication::translate("mNoisyHandler", "CRITICAL:") << msg << "\n";
            break;
        case QtFatalMsg:
            vStdErr() << QApplication::translate("mNoisyHandler", "FATAL:") << msg << "\n";
            break;
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
        case QtInfoMsg:
            vStdOut() << QApplication::translate("mNoisyHandler", "INFO:") << msg << "\n";
            break;
#endif
        default:
            break;
    }

    vStdOut().flush();
    vStdErr().flush();

    if (isGuiThread)
    {
        // fixme: trying to make sure there are no save/load dialogs are opened, because error message during them will
        // lead to crash
        const bool topWinAllowsPop = (QApplication::activeModalWidget() == nullptr) ||
                                     !QApplication::activeModalWidget()->inherits("QFileDialog");
        QMessageBox messageBox;
        switch (type)
        {
            case QtWarningMsg:
                messageBox.setWindowTitle(QApplication::translate("mNoisyHandler", "Warning"));
                messageBox.setIcon(QMessageBox::Warning);
                break;
            case QtCriticalMsg:
                messageBox.setWindowTitle(QApplication::translate("mNoisyHandler", "Critical error"));
                messageBox.setIcon(QMessageBox::Critical);
                break;
            case QtFatalMsg:
                messageBox.setWindowTitle(QApplication::translate("mNoisyHandler", "Fatal error"));
                messageBox.setIcon(QMessageBox::Critical);
                break;
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
            case QtInfoMsg:
                messageBox.setWindowTitle(QApplication::translate("mNoisyHandler", "Information"));
                messageBox.setIcon(QMessageBox::Information);
                break;
#endif
            case QtDebugMsg:
                Q_UNREACHABLE(); //-V501
                break;
            default:
                break;
        }

        if (type == QtWarningMsg || type == QtCriticalMsg || type == QtFatalMsg)
        {
            if (VPApplication::VApp()->IsAppInGUIMode())
            {
                if (topWinAllowsPop)
                {
                    messageBox.setText(msg);
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
VPApplication::VPApplication(int &argc, char **argv)
  : VAbstractApplication(argc, argv)
{
    setApplicationDisplayName(QStringLiteral(VER_PRODUCTNAME_STR));
    setApplicationName(QStringLiteral(VER_INTERNALNAME_STR));
    setOrganizationName(QStringLiteral(VER_COMPANYNAME_STR));
    setOrganizationDomain(QStringLiteral(VER_COMPANYDOMAIN_STR));
    // Setting the Application version
    setApplicationVersion(APP_VERSION_STR);
    // We have been running Puzzle in two different cases.
    // The first inside own bundle where info.plist is works fine, but the second,
    // when we run inside Valentina's bundle, require direct setting the icon.
    setWindowIcon(QIcon(":/puzzleicon/64x64/logo.png"));
}

//---------------------------------------------------------------------------------------------------------------------
VPApplication::~VPApplication()
{
    if (settings->IsCollectStatistic())
    {
        auto *statistic = VGAnalytics::Instance();

        QString clientID = settings->GetClientID();
        if (!clientID.isEmpty())
        {
            statistic->SendAppCloseEvent(m_uptimeTimer.elapsed());
        }
    }

    qDeleteAll(m_mainWindows);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief notify Reimplemented from QApplication::notify().
 * @param receiver receiver.
 * @param event event.
 * @return value that is returned from the receiver's event handler.
 */
// reimplemented from QApplication so we can throw exceptions in slots
auto VPApplication::notify(QObject *receiver, QEvent *event) -> bool
{
    try
    {
        return QApplication::notify(receiver, event);
    }
    catch (const VExceptionObjectError &e)
    {
        qCCritical(pApp, "%s\n\n%s\n\n%s",
                   qUtf8Printable(tr("Error parsing file. Program will be terminated.")), //-V807
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VExceptionBadId &e)
    {
        qCCritical(pApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error bad id. Program will be terminated.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VExceptionConversionError &e)
    {
        qCCritical(pApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error can't convert value. Program will be terminated.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VExceptionEmptyParameter &e)
    {
        qCCritical(pApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error empty parameter. Program will be terminated.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VExceptionWrongId &e)
    {
        qCCritical(pApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error wrong id. Program will be terminated.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        qCCritical(pApp, "%s\n\n%s\n\n%s",
                   qUtf8Printable(QStringLiteral("Unhadled deleting tool. Continue use object after deleting!")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VException &e)
    {
        qCCritical(pApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Something's wrong!!")), qUtf8Printable(e.ErrorMessage()),
                   qUtf8Printable(e.DetailedInformation()));
        return true;
    }
    catch (std::exception &e)
    {
        qCCritical(pApp, "%s", qUtf8Printable(tr("Exception thrown: %1. Program will be terminated.").arg(e.what())));
        exit(V_EX_SOFTWARE);
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief IsAppInGUIMode little hack that allow to have access to application state from VAbstractApplication class.
 */
auto VPApplication::IsAppInGUIMode() const -> bool
{
    return CommandLine()->IsGuiEnabled();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPApplication::MainWindow() -> VPMainWindow *
{
    Clean();
    if (m_mainWindows.isEmpty())
    {
        NewMainWindow();
    }
    return m_mainWindows[0];
}

//---------------------------------------------------------------------------------------------------------------------
auto VPApplication::MainWindows() -> QList<VPMainWindow *>
{
    Clean();
    QList<VPMainWindow *> list;
    list.reserve(m_mainWindows.size());
    for (auto &w : m_mainWindows)
    {
        list.append(w);
    }
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPApplication::NewMainWindow() -> VPMainWindow *
{
    VPCommandLinePtr cmd;
    VPCommandLine::ProcessInstance(cmd, {ConstFirst<QString>(VPApplication::arguments())});
    return NewMainWindow(cmd);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPApplication::NewMainWindow(const VPCommandLinePtr &cmd) -> VPMainWindow *
{
    auto *puzzle = new VPMainWindow(cmd);
    m_mainWindows.prepend(puzzle);
    puzzle->show();
    puzzle->UpdateWindowTitle();
    puzzle->InitZoom();
    return puzzle;
}

//---------------------------------------------------------------------------------------------------------------------
void VPApplication::InitOptions()
{
    qInstallMessageHandler(noisyFailureMsgHandler);

    OpenSettings();

    qCDebug(pApp, "Version: %s", qUtf8Printable(APP_VERSION_STR));
    qCDebug(pApp, "Build revision: %s", BUILD_REVISION);
    qCDebug(pApp, "%s", qUtf8Printable(buildCompatibilityString()));
    qCDebug(pApp, "Built on %s at %s", __DATE__, __TIME__);
    qCDebug(pApp, "Command-line arguments: %s", qUtf8Printable(arguments().join(QStringLiteral(", "))));
    qCDebug(pApp, "Process ID: %s", qUtf8Printable(QString().setNum(applicationPid())));

    QPixmapCache::setCacheLimit(50 * 1024 /* 50 MB */);

    LoadTranslation(QString()); // By default the console version uses system locale

    VPCommandLine::Instance();

    CheckSystemLocale();

    static const char *GENERIC_ICON_TO_CHECK = "document-open";
    if (not QIcon::hasThemeIcon(GENERIC_ICON_TO_CHECK))
    {
        // If there is no default working icon theme then we should
        // use an icon theme that we provide via a .qrc file
        // This case happens under Windows and Mac OS X
        // This does not happen under GNOME or KDE
        QIcon::setThemeName(QStringLiteral("win.icon.theme"));
    }
    ActivateDarkMode();

    auto *statistic = VGAnalytics::Instance();
    QString clientID = settings->GetClientID();
    if (clientID.isEmpty())
    {
        clientID = QUuid::createUuid().toString();
        settings->SetClientID(clientID);
    }
    statistic->SetClientID(clientID);
    statistic->SetGUILanguage(settings->GetLocale());
    statistic->SetMeasurementId(GA_MEASUREMENT_ID);
    statistic->SetApiSecret(GA_API_SECRET);
    statistic->Enable(settings->IsCollectStatistic());
}

//---------------------------------------------------------------------------------------------------------------------
auto VPApplication::TrVars() -> const VTranslateVars *
{
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
void VPApplication::OpenSettings()
{
    settings = new VPSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(),
                              QCoreApplication::applicationName(), this);
    connect(settings, &VPSettings::SVGFontsPathChanged, this, &VPApplication::SVGFontsPathChanged);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPApplication::PuzzleSettings() -> VPSettings *
{
    SCASSERT(settings != nullptr)
    return qobject_cast<VPSettings *>(settings);
}

//---------------------------------------------------------------------------------------------------------------------
void VPApplication::ActivateDarkMode()
{
    VPSettings *settings = PuzzleSettings();
    if (settings->GetDarkMode())
    {
        QFile f(QStringLiteral(":qdarkstyle/style.qss"));
        if (!f.exists())
        {
            qDebug() << "Unable to set stylesheet, file not found\n";
        }
        else
        {
            f.open(QFile::ReadOnly | QFile::Text);
            QTextStream ts(&f);
            VPApplication::VApp()->setStyleSheet(ts.readAll());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPApplication::ParseCommandLine(const SocketConnection &connection, const QStringList &arguments)
{
    VPCommandLinePtr cmd;
    VPCommandLine::ProcessInstance(cmd, arguments);

    if (cmd->IsGuiEnabled() && connection == SocketConnection::Client)
    {
        const QString serverName = QCoreApplication::applicationName();
        QLocalSocket socket;
        socket.connectToServer(serverName);
        if (socket.waitForConnected(1000))
        {
            qCDebug(pApp, "Connected to the server '%s'", qUtf8Printable(serverName));
            QTextStream stream(&socket);
            stream << arguments.join(QStringLiteral(";;"));
            stream.flush();
            socket.waitForBytesWritten();
            QCoreApplication::exit(V_EX_OK);
            return;
        }

        qCDebug(pApp, "Can't establish connection to the server '%s'", qUtf8Printable(serverName));
        StartLocalServer(serverName);
        LoadTranslation(PuzzleSettings()->GetLocale());
    }

    ProcessArguments(cmd);
}

//---------------------------------------------------------------------------------------------------------------------
void VPApplication::ProcessArguments(const VPCommandLinePtr &cmd)
{
    const QStringList rawLayouts = cmd->OptionRawLayouts();
    const QStringList args = cmd->OptionFileNames();
    bool success = args.count() > 0 ? StartWithFiles(cmd, rawLayouts) : SingleStart(cmd, rawLayouts);

    if (not success)
    {
        return;
    }

    if (not cmd->IsGuiEnabled())
    {
        QCoreApplication::exit(V_EX_OK); // close program after processing in console mode
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPApplication::ProcessCMD()
{
    ParseCommandLine(SocketConnection::Client, arguments());

    if (IsAppInGUIMode() && Settings()->IsAutomaticallyCheckUpdates())
    {
        // Set feed URL before doing anything else
        FvUpdater::sharedUpdater()->SetFeedURL(FvUpdater::CurrentFeedURL());

        // Check for updates automatically
        FvUpdater::sharedUpdater()->CheckForUpdatesSilent();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPApplication::event(QEvent *e) -> bool
{
    switch (e->type())
    {
        // In Mac OS X the QFileOpenEvent event is generated when user perform "Open With" from Finder (this event is
        // Mac specific).
        case QEvent::FileOpen:
        {
            auto *fileOpenEvent =
                static_cast<QFileOpenEvent *>(e); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
            const QString macFileOpen = fileOpenEvent->file();
            if (not macFileOpen.isEmpty())
            {
                VPMainWindow *mw = MainWindow();
                if (mw != nullptr)
                {
                    mw->LoadFile(macFileOpen); // open file in existing window
                }
                return true;
            }
            break;
        }
#if defined(Q_OS_MAC)
        case QEvent::ApplicationActivate:
        {
            Clean();
            VPMainWindow *mw = MainWindow();
            if (mw && not mw->isMinimized())
            {
                mw->show();
            }
            return true;
        }
#endif // defined(Q_OS_MAC)
        default:
            return VAbstractApplication::event(e);
    }
    return VAbstractApplication::event(e);
}

//---------------------------------------------------------------------------------------------------------------------
void VPApplication::InitTrVars()
{
    // do nothing
}

//---------------------------------------------------------------------------------------------------------------------
void VPApplication::AboutToQuit()
{
    // If try to use the method QApplication::exit program can't sync settings and show warning about QApplication
    // instance. Solution is to call sync() before quit.
    // Connect this slot with VApplication::aboutToQuit.
    Settings()->sync();
}

//---------------------------------------------------------------------------------------------------------------------
void VPApplication::NewLocalSocketConnection()
{
    QScopedPointer<QLocalSocket> socket(m_localServer->nextPendingConnection());
    if (socket.isNull())
    {
        return;
    }
    socket->waitForReadyRead(1000);
    QTextStream stream(socket.data());
    const QString arg = stream.readAll();
    if (not arg.isEmpty())
    {
        ParseCommandLine(SocketConnection::Server, arg.split(QStringLiteral(";;")));
    }
    MainWindow()->raise();
    MainWindow()->activateWindow();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPApplication::PreferencesDialog() const -> QSharedPointer<DialogPuzzlePreferences>
{
    return m_preferencesDialog;
}

//---------------------------------------------------------------------------------------------------------------------
void VPApplication::SetPreferencesDialog(const QSharedPointer<DialogPuzzlePreferences> &newPreferencesDialog)
{
    m_preferencesDialog = newPreferencesDialog;
}

//---------------------------------------------------------------------------------------------------------------------
void VPApplication::Clean()
{
    // cleanup any deleted main windows first
    for (vsizetype i = m_mainWindows.count() - 1; i >= 0; --i)
    {
        if (m_mainWindows.at(i).isNull())
        {
            m_mainWindows.removeAt(i);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPApplication::StartLocalServer(const QString &serverName)
{
    m_localServer = new QLocalServer(this);
    connect(m_localServer, &QLocalServer::newConnection, this, &VPApplication::NewLocalSocketConnection);
    if (not m_localServer->listen(serverName))
    {
        qCDebug(pApp, "Can't begin to listen for incoming connections on name '%s'", qUtf8Printable(serverName));
        if (m_localServer->serverError() == QAbstractSocket::AddressInUseError)
        {
            QLocalServer::removeServer(serverName);
            if (not m_localServer->listen(serverName))
            {
                qCWarning(
                    pApp, "%s",
                    qUtf8Printable(tr("Can't begin to listen for incoming connections on name '%1'").arg(serverName)));
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPApplication::StartWithFiles(const VPCommandLinePtr &cmd, const QStringList &rawLayouts) -> bool
{
    const QStringList args = cmd->OptionFileNames();

    if (args.count() <= 0)
    {
        QCoreApplication::exit(V_EX_DATAERR);
        return false;
    }

    if (not cmd->IsGuiEnabled() && args.count() > 1)
    {
        qCCritical(pApp, "%s\n", qPrintable(tr("Export mode doesn't support opening several files.")));
        cmd.get()->parser.showHelp(V_EX_USAGE);
    }

    if (args.count() > 1 && not rawLayouts.isEmpty())
    {
        qCCritical(pApp, "%s\n",
                   qPrintable(tr("Import raw layout data does not support opening several layout files.")));
        cmd.get()->parser.showHelp(V_EX_USAGE);
    }

    for (const auto &arg : args) // NOLINT(readability-use-anyofallof)
    {
        NewMainWindow(cmd);
        if (not MainWindow()->LoadFile(arg))
        {
            if (not cmd->IsGuiEnabled())
            {
                return false; // process only one input file
            }
            delete MainWindow();
            continue;
        }

        if (not rawLayouts.isEmpty())
        {
            MainWindow()->ImportRawLayouts(rawLayouts);
        }
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPApplication::SingleStart(const VPCommandLinePtr &cmd, const QStringList &rawLayouts) -> bool
{
    NewMainWindow(cmd);
    if (not rawLayouts.isEmpty())
    {
        MainWindow()->ImportRawLayouts(rawLayouts);
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPApplication::CommandLine() -> VPCommandLinePtr
{
    return VPCommandLine::instance;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPApplication::VApp() -> VPApplication *
{
    return qobject_cast<VPApplication *>(QCoreApplication::instance());
}
