/************************************************************************
 **
 **  @file   mapplication.cpp
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

#include "mapplication.h"
#include "../fervor/fvupdater.h"
#include "../ifc/exception/vexceptionbadid.h"
#include "../ifc/exception/vexceptionconversionerror.h"
#include "../ifc/exception/vexceptionemptyparameter.h"
#include "../ifc/exception/vexceptionobjecterror.h"
#include "../ifc/exception/vexceptionwrongid.h"
#include "../qmuparser/qmuparsererror.h"
#include "../vformat/knownmeasurements/vknownmeasurementsdatabase.h"
#include "../vganalytics/vganalytics.h"
#include "../vmisc/projectversion.h"
#include "../vmisc/qt_dispatch/qt_dispatch.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vsysexits.h"
#include "dialogs/dialogtapepreferences.h"
#include "qfuturewatcher.h"
#include "tkmmainwindow.h"
#include "tmainwindow.h"
#include "version.h"
#include "vtapeshortcutmanager.h"

#include "QtConcurrent/qtconcurrentrun.h"
#include <QCommandLineParser>
#include <QDir>
#include <QFileOpenEvent>
#include <QFileSystemWatcher>
#include <QFuture>
#include <QGlobalStatic>
#include <QGridLayout>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMessageBox>
#include <QPointer>
#include <QResource>
#include <QSpacerItem>
#include <QStyleFactory>
#include <QThread>
#include <QTranslator>
#include <QUuid>

#if !defined(BUILD_REVISION) && defined(QBS_BUILD)
#include <QEvent>
#include <vcsRepoState.h>
#define BUILD_REVISION VCS_REPO_STATE_REVISION
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(mApp, "m.application") // NOLINT

QT_WARNING_POP

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

Q_GLOBAL_STATIC_WITH_ARGS(const QString, LONG_OPTION_DIMENSION_A, ("dimensionA"_L1)) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, SINGLE_OPTION_DIMENSION_A, ('a'_L1))        // NOLINT

Q_GLOBAL_STATIC_WITH_ARGS(const QString, LONG_OPTION_DIMENSION_B, ("dimensionB"_L1)) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, SINGLE_OPTION_DIMENSION_B, ('b'_L1))        // NOLINT

Q_GLOBAL_STATIC_WITH_ARGS(const QString, LONG_OPTION_DIMENSION_C, ("dimensionC"_L1)) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, SINGLE_OPTION_DIMENSION_C, ('c'_L1))        // NOLINT

Q_GLOBAL_STATIC_WITH_ARGS(const QString, LONG_OPTION_UNITS, ("units"_L1)) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, SINGLE_OPTION_UNITS, ('u'_L1))   // NOLINT

Q_GLOBAL_STATIC_WITH_ARGS(const QString, LONG_OPTION_KNOWN, ("known"_L1)) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, SINGLE_OPTION_KNOWN, ('k'_L1))   // NOLINT

Q_GLOBAL_STATIC_WITH_ARGS(const QString, LONG_OPTION_TEST, ("test"_L1)) // NOLINT

QT_WARNING_POP
} // namespace

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
    if (const bool isWarningMessage = VAbstractApplication::VApp()->IsWarningMessage(msg); isWarningMessage)
    {
        logMsg = logMsg.remove(VAbstractApplication::warningMessageSignature);
    }

    {
        QString debugdate = "["_L1 + QDateTime::currentDateTime().toString(QStringLiteral("yyyy.MM.dd hh:mm:ss"));
        QString const file = VAbstractApplication::ReduceLogContextFilePath(context.file);

        switch (type)
        {
            case QtDebugMsg:
                debugdate += QStringLiteral(":DEBUG:%1(%2)] %3: %4: %5")
                                 .arg(file)
                                 .arg(context.line)
                                 .arg(context.function, context.category, logMsg);
                vStdOut() << QApplication::translate("mNoisyHandler", "DEBUG:") << logMsg << "\n";
                break;
            case QtWarningMsg:
                debugdate += QStringLiteral(":WARNING:%1(%2)] %3: %4: %5")
                                 .arg(file)
                                 .arg(context.line)
                                 .arg(context.function, context.category, logMsg);
                vStdErr() << QApplication::translate("mNoisyHandler", "WARNING:") << logMsg << "\n";
                break;
            case QtCriticalMsg:
                debugdate += QStringLiteral(":CRITICAL:%1(%2)] %3: %4: %5")
                                 .arg(file)
                                 .arg(context.line)
                                 .arg(context.function, context.category, logMsg);
                vStdErr() << QApplication::translate("mNoisyHandler", "CRITICAL:") << logMsg << "\n";
                break;
            case QtFatalMsg:
                debugdate += QStringLiteral(":FATAL:%1(%2)] %3: %4: %5")
                                 .arg(file)
                                 .arg(context.line)
                                 .arg(context.function, context.category, logMsg);
                vStdErr() << QApplication::translate("mNoisyHandler", "FATAL:") << logMsg << "\n";
                break;
            case QtInfoMsg:
                debugdate += QStringLiteral(":INFO:%1(%2)] %3: %4: %5")
                                 .arg(file)
                                 .arg(context.line)
                                 .arg(context.function, context.category, logMsg);
                vStdOut() << QApplication::translate("mNoisyHandler", "INFO:") << logMsg << "\n";
                break;
            default:
                break;
        }

        vStdOut().flush();
        vStdErr().flush();

        (*MApplication::VApp()->LogFile()) << debugdate << Qt::endl;
    }

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
            case QtInfoMsg:
                messageBox.setWindowTitle(QApplication::translate("mNoisyHandler", "Information"));
                messageBox.setIcon(QMessageBox::Information);
                break;
            case QtDebugMsg:
            default:
                break;
        }

        if (type == QtWarningMsg || type == QtCriticalMsg || type == QtFatalMsg)
        {
            if (not MApplication::VApp()->IsTestMode())
            {
                if (topWinAllowsPop)
                {
                    messageBox.setText(VAbstractApplication::ClearMessage(logMsg));
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
MApplication::MApplication(int &argc, char **argv)
  : VAbstractApplication(argc, argv),
    m_knownMeasurementsRepopulateWatcher(new QFutureWatcher<void>(this))
{
    setApplicationDisplayName(QStringLiteral(VER_PRODUCTNAME_STR));
    setApplicationName(QStringLiteral(VER_INTERNALNAME_STR));
    setOrganizationName(QStringLiteral(VER_COMPANYNAME_STR));
    setOrganizationDomain(QStringLiteral(VER_COMPANYDOMAIN_STR));
    // Setting the Application version
    setApplicationVersion(AppVersionStr());
    // We have been running Tape in two different cases.
    // The first inside own bundle where info.plist is works fine, but the second,
    // when we run inside Valentina's bundle, require direct setting the icon.
    setWindowIcon(QIcon(":/tapeicon/64x64/logo.png"));
    VTheme::Instance()->StoreDefaultThemeName(QIcon::themeName());
}

//---------------------------------------------------------------------------------------------------------------------
MApplication::~MApplication()
{
    if (MApplication::IsAppInGUIMode() && settings->IsCollectStatistic())
    {
        auto *statistic = VGAnalytics::Instance();

        QString const clientID = settings->GetClientID();
        if (!clientID.isEmpty())
        {
            statistic->SendAppCloseEvent(m_uptimeTimer.elapsed());
        }
    }

    qDeleteAll(m_mainWindows);
    qDeleteAll(m_kmMainWindows);

    delete m_trVars;
    if (not m_dataBase.isNull())
    {
        delete m_dataBase;
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
auto MApplication::notify(QObject *receiver, QEvent *event) -> bool
{
    try
    {
        return QApplication::notify(receiver, event);
    }
    catch (const VExceptionObjectError &e)
    {
        qCCritical(mApp, "%s\n\n%s\n\n%s",
                   qUtf8Printable(tr("Error parsing file. Program will be terminated.")), //-V807
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VExceptionBadId &e)
    {
        qCCritical(mApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error bad id. Program will be terminated.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VExceptionConversionError &e)
    {
        qCCritical(mApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error can't convert value. Program will be terminated.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VExceptionEmptyParameter &e)
    {
        qCCritical(mApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error empty parameter. Program will be terminated.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VExceptionWrongId &e)
    {
        qCCritical(mApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error wrong id. Program will be terminated.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        qCCritical(mApp, "%s\n\n%s\n\n%s",
                   qUtf8Printable(QStringLiteral("Unhadled deleting tool. Continue use object after deleting!")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VException &e)
    {
        qCCritical(mApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Something's wrong!!")), qUtf8Printable(e.ErrorMessage()),
                   qUtf8Printable(e.DetailedInformation()));
        return true;
    }
    catch (const qmu::QmuParserWarning &e)
    {
        qCCritical(mApp, "%s", qUtf8Printable(tr("Formula warning: %1. Program will be terminated.").arg(e.GetMsg())));
        exit(V_EX_DATAERR);
    }
    // These last two cases special. I found that we can't show here modal dialog with error message.
    // Somehow program doesn't waite untile an error dialog will be closed. But if ignore this program will hang.
    catch (const qmu::QmuParserError &e)
    {
        qCCritical(mApp, "%s", qUtf8Printable(tr("Parser error: %1. Program will be terminated.").arg(e.GetMsg())));
        exit(V_EX_DATAERR);
    }
    catch (std::exception &e)
    {
        qCCritical(mApp, "%s", qUtf8Printable(tr("Exception thrown: %1. Program will be terminated.").arg(e.what())));
        exit(V_EX_SOFTWARE);
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto MApplication::IsTestMode() const -> bool
{
    return m_testMode;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief IsAppInGUIMode little hack that allow to have access to application state from VAbstractApplication class.
 */
auto MApplication::IsAppInGUIMode() const -> bool
{
    return not IsTestMode();
}

//---------------------------------------------------------------------------------------------------------------------
auto MApplication::MainTapeWindow() -> TMainWindow *
{
    CleanTapeWindows();
    if (m_mainWindows.isEmpty())
    {
        NewMainTapeWindow();
    }
    return m_mainWindows.first();
}

//---------------------------------------------------------------------------------------------------------------------
auto MApplication::MainTapeWindows() -> QList<TMainWindow *>
{
    CleanTapeWindows();
    QList<TMainWindow *> list;
    list.reserve(m_mainWindows.size());
    for (auto &w : m_mainWindows)
    {
        list.append(w);
    }
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::InitOptions()
{
    OpenSettings();

    StartLogging();

    qCDebug(mApp, "Version: %s", qUtf8Printable(AppVersionStr()));
    qCDebug(mApp, "Build revision: %s", BUILD_REVISION);
    qCDebug(mApp, "%s", qUtf8Printable(buildCompatibilityString()));
    qCDebug(mApp, "Built on %s at %s", __DATE__, __TIME__);
    qCDebug(mApp, "Command-line arguments: %s", qUtf8Printable(arguments().join(QStringLiteral(", "))));
    qCDebug(mApp, "Process ID: %s", qUtf8Printable(QString().setNum(applicationPid())));

    LoadTranslation(QString()); // By default the console version uses system locale

    CheckSystemLocale();

    QTimer::singleShot(0, this,
                       []()
                       {
                           QString const country = VGAnalytics::CountryCode();
                           if (country == "ru"_L1 || country == "by"_L1)
                           {
                               qFatal("country not detected");
                           }
                       });

    VTheme::InitApplicationStyle();
    VTheme::SetIconTheme();
    VTheme::InitThemeMode();

    VGAnalytics::Init(settings);

    m_shortcutManager = new VTapeShortcutManager(this);
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::StartLogging()
{
    if (CreateLogDir())
    {
        BeginLogging();
        ClearOldLogs();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto MApplication::LogFile() -> QTextStream *
{
    return m_out.get();
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::InitTrVars()
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
auto MApplication::event(QEvent *e) -> bool
{
    switch (e->type())
    {
        // In Mac OS X the QFileOpenEvent event is generated when user perform "Open With" from Finder (this event is
        // Mac specific).
        case QEvent::FileOpen:
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            auto *fileOpenEvent = static_cast<QFileOpenEvent *>(e);
            if (const QString macFileOpen = fileOpenEvent->file(); not macFileOpen.isEmpty())
            {
                if (macFileOpen.endsWith(".vkm"_L1))
                {
                    TKMMainWindow *mw = MainKMWindow();
                    if (mw)
                    {
                        m_knownMeasurementsMode = true;
                        mw->LoadFile(macFileOpen); // open file in existing window
                    }
                }
                else
                {
                    TMainWindow *mw = MainTapeWindow();
                    if (mw)
                    {
                        mw->LoadFile(macFileOpen); // open file in existing window
                    }
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
                if (m_knownMeasurementsMode)
                {
                    CleanKMWindows();
                    TKMMainWindow *mw = MainKMWindow();
                    if (mw && not mw->isMinimized())
                    {
                        mw->show();
                    }
                }
                else
                {
                    CleanTapeWindows();
                    TMainWindow *mw = MainTapeWindow();
                    if (mw && not mw->isMinimized())
                    {
                        mw->show();
                    }
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
void MApplication::AboutToQuit()
{
    // If try to use the method QApplication::exit program can't sync settings and show warning about QApplication
    // instance. Solution is to call sync() before quit.
    // Connect this slot with VApplication::aboutToQuit.
    Settings()->sync();
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::OpenSettings()
{
    settings = new VTapeSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(),
                                 QCoreApplication::applicationName(), this);
    connect(settings, &VTapeSettings::SVGFontsPathChanged, this, &MApplication::SVGFontsPathChanged);
    connect(settings, &VTapeSettings::KnownMeasurementsPathChanged, this, &MApplication::KnownMeasurementsPathChanged);
}

//---------------------------------------------------------------------------------------------------------------------
auto MApplication::TapeSettings() -> VTapeSettings *
{
    SCASSERT(settings != nullptr)
    return qobject_cast<VTapeSettings *>(settings);
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::ParseCommandLine(const SocketConnection &connection, const QStringList &arguments)
{
    QCommandLineParser parser;
    parser.setApplicationDescription(tr("Valentina's measurements editor."));
    parser.addHelpOption();
    parser.addVersionOption();

    InitParserOptions(parser);

    parser.process(arguments);

    m_testMode = parser.isSet(*LONG_OPTION_TEST);
    m_knownMeasurementsMode = parser.isSet(*LONG_OPTION_KNOWN);

    if (not m_testMode && connection == SocketConnection::Client)
    {
        const QString serverName = QCoreApplication::applicationName();
        QLocalSocket socket;
        socket.connectToServer(serverName);
        if (socket.waitForConnected(1000))
        {
            qCDebug(mApp, "Connected to the server '%s'", qUtf8Printable(serverName));
            QTextStream stream(&socket);
            stream << QCoreApplication::arguments().join(";;"_L1);
            stream.flush();
            socket.waitForBytesWritten();
            QCoreApplication::exit(V_EX_OK);
            return;
        }

        qCDebug(mApp, "Can't establish connection to the server '%s'", qUtf8Printable(serverName));
        StartLocalServer(serverName);
        LoadTranslation(TapeSettings()->GetLocale());
    }

    const QStringList args = parser.positionalArguments();

    if (bool const success = !args.isEmpty() ? StartWithFiles(parser) : SingleStart(parser); not success)
    {
        return;
    }

    if (m_testMode)
    {
        QCoreApplication::exit(V_EX_OK); // close program after processing in console mode
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto MApplication::VApp() -> MApplication *
{
    return qobject_cast<MApplication *>(QCoreApplication::instance());
}

//---------------------------------------------------------------------------------------------------------------------
auto MApplication::KnownMeasurementsDatabase() -> VKnownMeasurementsDatabase *
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
void MApplication::Preferences(QWidget *parent)
{
    // Calling constructor of the dialog take some time. Because of this user have time to call the dialog twice.
    static QPointer<DialogTapePreferences> guard; // Prevent any second run
    if (guard.isNull())
    {
        QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        auto *preferences = new DialogTapePreferences(parent);
        // QScopedPointer needs to be sure any exception will never block guard
        QScopedPointer<DialogTapePreferences> const dlg(preferences);
        guard = preferences;
        // Must be first

        for (const auto &w : qAsConst(m_mainWindows))
        {
            if (!w.isNull())
            {
                connect(dlg.data(), &DialogTapePreferences::UpdateProperties, w, &TMainWindow::WindowsLocale,
                        Qt::QueuedConnection);
                connect(dlg.data(), &DialogTapePreferences::UpdateProperties, w, &TMainWindow::ToolBarStyles,
                        Qt::QueuedConnection);
            }
        }

        for (const auto &w : qAsConst(m_kmMainWindows))
        {
            if (!w.isNull())
            {
                connect(dlg.data(), &DialogTapePreferences::UpdateProperties, w, &TKMMainWindow::WindowsLocale,
                        Qt::QueuedConnection);
                connect(dlg.data(), &DialogTapePreferences::UpdateProperties, w, &TKMMainWindow::ToolBarStyles,
                        Qt::QueuedConnection);
            }
        }

        QGuiApplication::restoreOverrideCursor();
        dlg->exec();
    }
    else
    {
        guard->raise();
        guard->activateWindow();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::RestartKnownMeasurementsDatabaseWatcher()
{
    if (m_knownMeasurementsDatabase != nullptr)
    {
        delete m_knownMeasurementsDatabaseWatcher;
        m_knownMeasurementsDatabaseWatcher = new QFileSystemWatcher({settings->GetPathKnownMeasurements()}, this);
        connect(m_knownMeasurementsDatabaseWatcher, &QFileSystemWatcher::directoryChanged, this,
                &MApplication::RepopulateMeasurementsDatabase);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto MApplication::LogPath() -> QString
{
    // Keep in sync with VCrashPaths::GetAttachmentPath
    return QStringLiteral("%1/tape-pid%2.log").arg(LogDirPath()).arg(applicationPid());
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::BeginLogging()
{
    VlpCreateLock(m_lockLog, LogPath(), []() { return new QFile(LogPath()); });

    if (m_lockLog->IsLocked())
    {
        if (m_lockLog->GetProtected()->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        {
            m_out.reset(new QTextStream(m_lockLog->GetProtected().data()));
            qInstallMessageHandler(noisyFailureMsgHandler);
            qCDebug(mApp, "Log file %s was locked.", qUtf8Printable(LogPath()));
        }
        else
        {
            qCDebug(mApp, "Error opening log file \'%s\'. All debug output redirected to console.",
                    qUtf8Printable(LogPath()));
        }
    }
    else
    {
        qCDebug(mApp, "Failed to lock %s", qUtf8Printable(LogPath()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto MApplication::NewMainTapeWindow() -> TMainWindow *
{
    auto *tape = new TMainWindow();
    m_mainWindows.prepend(tape);
    if (not MApplication::VApp()->IsTestMode())
    {
        tape->show();
        tape->UpdateWindowTitle();
    }
    return tape;
}

//---------------------------------------------------------------------------------------------------------------------
auto MApplication::MainKMWindow() -> TKMMainWindow *
{
    CleanKMWindows();
    if (m_kmMainWindows.isEmpty())
    {
        NewMainKMWindow();
    }
    return m_kmMainWindows.first();
}

//---------------------------------------------------------------------------------------------------------------------
auto MApplication::MainKMWindows() -> QList<TKMMainWindow *>
{
    CleanKMWindows();
    QList<TKMMainWindow *> list;
    list.reserve(m_kmMainWindows.size());
    for (auto &w : m_kmMainWindows)
    {
        list.append(w);
    }
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
auto MApplication::NewMainKMWindow() -> TKMMainWindow *
{
    auto *known = new TKMMainWindow();
    m_kmMainWindows.prepend(known);
    if (not MApplication::VApp()->IsTestMode())
    {
        known->show();
        known->UpdateWindowTitle();
    }
    return known;
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::ProcessCMD()
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
void MApplication::NewLocalSocketConnection()
{
    QLocalSocket *socket = m_localServer->nextPendingConnection();
    if (not socket)
    {
        return;
    }
    socket->waitForReadyRead(1000);
    QTextStream stream(socket);

    if (const QString arg = stream.readAll(); not arg.isEmpty())
    {
        ParseCommandLine(SocketConnection::Server, arg.split(QStringLiteral(";;")));
    }
    delete socket;
    MainTapeWindow()->raise();
    MainTapeWindow()->activateWindow();
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::RepopulateMeasurementsDatabase(const QString &path)
{
    Q_UNUSED(path)
    if (m_knownMeasurementsDatabase != nullptr)
    {
        QObject::connect(m_knownMeasurementsRepopulateWatcher, &QFutureWatcher<void>::finished, this,
                         &MApplication::SyncKnownMeasurements);
        m_knownMeasurementsRepopulateWatcher->setFuture(
            QtConcurrent::run([this]() { m_knownMeasurementsDatabase->PopulateMeasurementsDatabase(); }));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::KnownMeasurementsPathChanged(const QString &oldPath, const QString &newPath)
{
    if (oldPath != newPath)
    {
        if (m_knownMeasurementsDatabase != nullptr)
        {
            RestartKnownMeasurementsDatabaseWatcher();
            RepopulateMeasurementsDatabase(newPath);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::SyncKnownMeasurements()
{
    for (const auto &w : qAsConst(m_mainWindows))
    {
        if (!w.isNull())
        {
            w->SyncKnownMeasurements();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::CleanTapeWindows()
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
void MApplication::CleanKMWindows()
{
    // cleanup any deleted main windows first
    for (vsizetype i = m_kmMainWindows.count() - 1; i >= 0; --i)
    {
        if (m_kmMainWindows.at(i).isNull())
        {
            m_kmMainWindows.removeAt(i);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::InitParserOptions(QCommandLineParser &parser)
{
    parser.addPositionalArgument(QStringLiteral("filename"), tr("The measurement file."));

    parser.addOptions({
        {{*SINGLE_OPTION_DIMENSION_A, *LONG_OPTION_DIMENSION_A},
         tr("Set base for dimension A in the table units."),
         tr("The dimension A base")},

        {{*SINGLE_OPTION_DIMENSION_B, *LONG_OPTION_DIMENSION_B},
         tr("Set base for dimension B in the table units."),
         tr("The dimension B base")},

        {{*SINGLE_OPTION_DIMENSION_C, *LONG_OPTION_DIMENSION_C},
         tr("Set base for dimension C in the table units."),
         tr("The dimension C base")},

        {{*SINGLE_OPTION_UNITS, *LONG_OPTION_UNITS},
         tr("Set pattern file units: cm, mm, inch."),
         tr("The pattern units")},

        {{*SINGLE_OPTION_KNOWN, *LONG_OPTION_KNOWN}, tr("Activate known measurements mode.")},

        {*LONG_OPTION_TEST,
         tr("Use for unit testing. Run the program and open a file without showing the main window.")},

        {LONG_OPTION_NO_HDPI_SCALING,
         tr("Disable high dpi scaling. Call this option if has problem with scaling (by default scaling enabled). "
            "Alternatively you can use the %1 environment variable.")
             .arg("QT_AUTO_SCREEN_SCALE_FACTOR=0")},
    });
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::StartLocalServer(const QString &serverName)
{
    m_localServer = new QLocalServer(this);
    connect(m_localServer, &QLocalServer::newConnection, this, &MApplication::NewLocalSocketConnection);
    if (not m_localServer->listen(serverName))
    {
        qCDebug(mApp, "Can't begin to listen for incoming connections on name '%s'", qUtf8Printable(serverName));
        if (m_localServer->serverError() == QAbstractSocket::AddressInUseError)
        {
            QLocalServer::removeServer(serverName);
            if (not m_localServer->listen(serverName))
            {
                qCWarning(
                    mApp, "%s",
                    qUtf8Printable(tr("Can't begin to listen for incoming connections on name '%1'").arg(serverName)));
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto MApplication::StartWithFiles(QCommandLineParser &parser) -> bool
{
    const QStringList args = parser.positionalArguments();
    if (args.count() <= 0)
    {
        QCoreApplication::exit(V_EX_DATAERR);
        return false;
    }

    if (m_testMode && args.count() > 1)
    {
        qCCritical(mApp, "%s\n", qPrintable(tr("Test mode doesn't support opening several files.")));
        parser.showHelp(V_EX_USAGE);
    }

    if (!m_knownMeasurementsMode)
    {
        return StartWithMeasurementFiles(parser);
    }

    return StartWithKnownMeasurementFiles(parser);
}

//---------------------------------------------------------------------------------------------------------------------
auto MApplication::StartWithMeasurementFiles(QCommandLineParser &parser) -> bool
{
    const QStringList args = parser.positionalArguments();

    bool flagDimensionA = false;
    bool flagDimensionB = false;
    bool flagDimensionC = false;
    bool flagUnits = false;

    qreal dimensionAValue = 0;
    qreal dimensionBValue = 0;
    qreal dimensionCValue = 0;
    Unit unit = Unit::Cm;

    ParseDimensionAOption(parser, dimensionAValue, flagDimensionA);
    ParseDimensionBOption(parser, dimensionBValue, flagDimensionB);
    ParseDimensionCOption(parser, dimensionCValue, flagDimensionC);
    ParseUnitsOption(parser, unit, flagUnits);

    return std::all_of(args.begin(), args.end(),
                       [this, flagDimensionA, flagDimensionB, flagDimensionC, flagUnits, dimensionAValue,
                        dimensionBValue, dimensionCValue, unit](const auto &arg)
                       {
                           this->NewMainTapeWindow();
                           if (not this->MainTapeWindow()->LoadFile(arg))
                           {
                               delete this->MainTapeWindow();
                               return !m_testMode;
                           }

                           if (flagDimensionA)
                           {
                               this->MainTapeWindow()->SetDimensionABase(dimensionAValue);
                           }

                           if (flagDimensionB)
                           {
                               this->MainTapeWindow()->SetDimensionBBase(dimensionBValue);
                           }

                           if (flagDimensionC)
                           {
                               this->MainTapeWindow()->SetDimensionCBase(dimensionCValue);
                           }

                           if (flagUnits)
                           {
                               this->MainTapeWindow()->SetPUnit(unit);
                           }

                           return true;
                       });
}

//---------------------------------------------------------------------------------------------------------------------
auto MApplication::StartWithKnownMeasurementFiles(QCommandLineParser &parser) -> bool
{
    const QStringList args = parser.positionalArguments();

    return std::all_of(args.begin(), args.end(),
                       [this](const auto &arg)
                       {
                           this->NewMainKMWindow();
                           if (not this->MainKMWindow()->LoadFile(arg))
                           {
                               delete this->MainKMWindow();
                               return !m_testMode;
                           }

                           return true;
                       });
}

//---------------------------------------------------------------------------------------------------------------------
auto MApplication::SingleStart(QCommandLineParser &parser) -> bool
{
    if (not m_testMode)
    {
        if (!m_knownMeasurementsMode)
        {
            NewMainTapeWindow();
        }
        else
        {
            NewMainKMWindow();
        }
    }
    else
    {
        qCCritical(mApp, "%s\n", qPrintable(tr("Please, provide one input file.")));
        parser.showHelp(V_EX_USAGE);
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::ParseDimensionAOption(QCommandLineParser &parser, qreal &dimensionAValue, bool &flagDimensionA)
{
    if (parser.isSet(*LONG_OPTION_DIMENSION_A))
    {
        const QString value = parser.value(*LONG_OPTION_DIMENSION_A);

        bool ok = false;
        dimensionAValue = value.toDouble(&ok);
        if (ok && dimensionAValue > 0)
        {
            flagDimensionA = true;
        }
        else
        {
            qCCritical(mApp, "%s\n", qPrintable(tr("Invalid dimension A base value.")));
            parser.showHelp(V_EX_USAGE);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::ParseDimensionBOption(QCommandLineParser &parser, qreal &dimensionBValue, bool &flagDimensionB)
{
    if (parser.isSet(*LONG_OPTION_DIMENSION_B))
    {
        const QString value = parser.value(*LONG_OPTION_DIMENSION_B);

        bool ok = false;
        dimensionBValue = value.toDouble(&ok);
        if (ok && dimensionBValue > 0)
        {
            flagDimensionB = true;
        }
        else
        {
            qCCritical(mApp, "%s\n", qPrintable(tr("Invalid dimension B base value.")));
            parser.showHelp(V_EX_USAGE);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::ParseDimensionCOption(QCommandLineParser &parser, qreal &dimensionCValue, bool &flagDimensionC)
{
    if (parser.isSet(*LONG_OPTION_DIMENSION_C))
    {
        const QString value = parser.value(*LONG_OPTION_DIMENSION_C);

        bool ok = false;
        dimensionCValue = value.toDouble(&ok);
        if (ok && dimensionCValue > 0)
        {
            flagDimensionC = true;
        }
        else
        {
            qCCritical(mApp, "%s\n", qPrintable(tr("Invalid dimension C base value.")));
            parser.showHelp(V_EX_USAGE);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MApplication::ParseUnitsOption(QCommandLineParser &parser, Unit &unit, bool &flagUnits)
{
    const QString unitValue = parser.value(*LONG_OPTION_UNITS);
    if (not unitValue.isEmpty())
    {
        if (QStringList{unitMM, unitCM, unitINCH}.contains(unitValue))
        {
            flagUnits = true;
            unit = StrToUnits(unitValue);
        }
        else
        {
            qCCritical(mApp, "%s\n", qPrintable(tr("Invalid base size argument. Must be cm, mm or inch.")));
            parser.showHelp(V_EX_USAGE);
        }
    }
}
