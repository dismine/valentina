/***************************************************************************************************
 **
 **  Copyright (c) 2012 Linas Valiukas and others.
 **
 **  Permission is hereby granted, free of charge, to any person obtaining a copy of this
 **  software and associated documentation files (the "Software"), to deal in the Software
 **  without restriction, including without limitation the rights to use, copy, modify,
 **  merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 **  permit persons to whom the Software is furnished to do so, subject to the following conditions:
 **
 **  The above copyright notice and this permission notice shall be included in all copies or
 **  substantial portions of the Software.
 **
 **  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 **  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 **  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 **  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 **  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 **
 ******************************************************************************************************/

#include "fvupdater.h"

#include <QApplication>
#include <QByteArray>
#include <QDate>
#include <QDesktopServices>
#include <QDir>
#include <QGlobalStatic>
#include <QLatin1String>
#include <QMessageBox>
#include <QMessageLogger>
#include <QMutex>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslConfiguration>
#include <QStringList>
#include <QVariant>
#include <QXmlStreamAttributes>
#include <QtDebug>
#include <qsystemdetection.h>
#include <qxmlstream.h>

#include "../ifc/exception/vexception.h"
#include "../ifc/xml/vabstractconverter.h"
#include "../vmisc/projectversion.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "fvavailableupdate.h"
#include "fvupdatewindow.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, defaultFeedURL, ("https://valentinaproject.bitbucket.io/Appcast.xml"_L1))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, testFeedURL, ("https://valentinaproject.bitbucket.io/Appcast_testing.xml"_L1))

QT_WARNING_POP
} // namespace

QPointer<FvUpdater> FvUpdater::m_Instance;

//---------------------------------------------------------------------------------------------------------------------
auto FvUpdater::sharedUpdater() -> FvUpdater *
{
    static QMutex mutex;
    if (m_Instance.isNull())
    {
        mutex.lock();
        m_Instance = new FvUpdater;
        mutex.unlock();
    }

    return m_Instance.data();
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::drop()
{
    static QMutex mutex;
    mutex.lock();
    delete m_Instance;
    mutex.unlock();
}

//---------------------------------------------------------------------------------------------------------------------
auto FvUpdater::CurrentFeedURL() -> QString
{
    return FvUpdater::IsTestBuild() ? *testFeedURL : *defaultFeedURL;
}

//---------------------------------------------------------------------------------------------------------------------
auto FvUpdater::IsTestBuild() -> bool
{
    return (MAJOR_VERSION * 1000 + MINOR_VERSION) % 2 != 0;
}

//---------------------------------------------------------------------------------------------------------------------
FvUpdater::FvUpdater()
  : QObject(nullptr),
    m_updaterWindow(nullptr),
    m_proposedUpdate(nullptr),
    m_silentAsMuchAsItCouldGet(true),
    m_feedURL(),
    m_qnam(),
    m_reply(nullptr),
    m_httpRequestAborted(false),
    m_dropOnFinnish(true),
    m_xml()
{
    // noop
}

//---------------------------------------------------------------------------------------------------------------------
FvUpdater::~FvUpdater()
{
    hideUpdaterWindow();
    delete m_reply;
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::showUpdaterWindowUpdatedWithCurrentUpdateProposal()
{
    // Destroy window if already exists
    hideUpdaterWindow();

    // Create a new window
    m_updaterWindow = new FvUpdateWindow(VAbstractValApplication::VApp()->getMainWindow());
    m_updaterWindow->UpdateWindowWithCurrentProposedUpdate();
    if (m_updaterWindow != nullptr)
    {
        m_updaterWindow->exec();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::hideUpdaterWindow()
{
    if (m_updaterWindow != nullptr)
    {
        m_updaterWindow->close();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::SetFeedURL(const QUrl &feedURL)
{
    m_feedURL = feedURL;
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::SetFeedURL(const QString &feedURL)
{
    SetFeedURL(QUrl(feedURL));
}

//---------------------------------------------------------------------------------------------------------------------
auto FvUpdater::GetFeedURL() const -> QString
{
    return m_feedURL.toString();
}

//---------------------------------------------------------------------------------------------------------------------
auto FvUpdater::IsDropOnFinnish() const -> bool
{
    return m_dropOnFinnish;
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::SetDropOnFinnish(bool value)
{
    m_dropOnFinnish = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto FvUpdater::GetProposedUpdate() -> QPointer<FvAvailableUpdate>
{
    return m_proposedUpdate;
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::InstallUpdate()
{
    qDebug() << "Install update";

    UpdateInstallationConfirmed();
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::SkipUpdate()
{
    qDebug() << "Skip update";

    QPointer<FvAvailableUpdate> proposedUpdate = GetProposedUpdate();
    if (proposedUpdate.isNull())
    {
        qWarning() << "Proposed update is NULL (shouldn't be at this point)";
        return;
    }

    // Start ignoring this particular version
    IgnoreVersion(proposedUpdate->GetEnclosureVersion());

    hideUpdaterWindow();
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::RemindMeLater()
{
    qDebug() << "Remind me later";

    VAbstractApplication::VApp()->Settings()->SetDateOfLastRemind(QDate::currentDate());

    hideUpdaterWindow();
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::UpdateInstallationConfirmed()
{
    qDebug() << "Confirm update installation";

    QPointer<FvAvailableUpdate> proposedUpdate = GetProposedUpdate();
    if (proposedUpdate.isNull())
    {
        qWarning() << "Proposed update is NULL (shouldn't be at this point)";
        return;
    }

    // Open a link
    if (not QDesktopServices::openUrl(proposedUpdate->GetEnclosureUrl()))
    {
        showErrorDialog(tr("Cannot open your default browser."), true);
        return;
    }

    hideUpdaterWindow();
}

//---------------------------------------------------------------------------------------------------------------------
auto FvUpdater::CheckForUpdates(bool silentAsMuchAsItCouldGet) -> bool
{
    if (m_feedURL.isEmpty())
    {
        qCritical() << "Please set feed URL via setFeedURL() before calling CheckForUpdates().";
        return false;
    }

    m_silentAsMuchAsItCouldGet = silentAsMuchAsItCouldGet;

    // Check if application's organization name and domain are set, fail otherwise
    // (nowhere to store QSettings to)
    if (QCoreApplication::organizationName().isEmpty())
    {
        qCritical() << "QApplication::organizationName is not set. Please do that.";
        return false;
    }
    if (QCoreApplication::organizationDomain().isEmpty())
    {
        qCritical() << "QApplication::organizationDomain is not set. Please do that.";
        return false;
    }

    // Set application name / version is not set yet
    if (QCoreApplication::applicationName().isEmpty())
    {
        qCritical() << "QApplication::applicationName is not set. Please do that.";
        return false;
    }

    if (QCoreApplication::applicationVersion().isEmpty())
    {
        qCritical() << "QApplication::applicationVersion is not set. Please do that.";
        return false;
    }

    cancelDownloadFeed();
    m_httpRequestAborted = false;
    startDownloadFeed(m_feedURL);

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto FvUpdater::CheckForUpdatesSilent() -> bool
{
    if (VAbstractApplication::VApp()->Settings()->GetDateOfLastRemind().daysTo(QDate::currentDate()) >= 1)
    {
        const bool success = CheckForUpdates(true);
        if (m_dropOnFinnish && not success)
        {
            drop();
        }
        return success;
    }

    if (m_dropOnFinnish)
    {
        drop();
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto FvUpdater::CheckForUpdatesNotSilent() -> bool
{
    const bool success = CheckForUpdates(false);
    if (m_dropOnFinnish && not success)
    {
        drop();
    }
    return success;
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::startDownloadFeed(const QUrl &url)
{
    m_xml.clear();

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/xml"));
    request.setHeader(QNetworkRequest::UserAgentHeader, QCoreApplication::applicationName());
    request.setUrl(url);
#ifndef QT_NO_SSL
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
#endif

    m_reply = m_qnam.get(request);

    connect(m_reply.data(), &QNetworkReply::readyRead, this,
            [this]()
            {
                // this slot gets called every time the QNetworkReply has new data.
                // We read all of its new data and write it into the file.
                // That way we use less RAM than when reading it at the finished()
                // signal of the QNetworkReply
                m_xml.addData(m_reply->readAll());
            });
    connect(m_reply.data(), &QNetworkReply::downloadProgress, this,
            [this](qint64 bytesRead, qint64 totalBytes)
            {
                Q_UNUSED(bytesRead)
                Q_UNUSED(totalBytes)

                if (m_httpRequestAborted)
                {
                    return;
                }
            });
    connect(m_reply.data(), &QNetworkReply::finished, this, &FvUpdater::httpFeedDownloadFinished);
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::cancelDownloadFeed()
{
    if (m_reply != nullptr)
    {
        m_httpRequestAborted = true;
        m_reply->abort();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::httpFeedDownloadFinished()
{
    if (m_httpRequestAborted)
    {
        m_reply->deleteLater();
        return;
    }

    const QVariant redirectionTarget = m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (m_reply->error() != QNetworkReply::NoError)
    {
        // Error.
        showErrorDialog(tr("Feed download failed: %1.").arg(m_reply->errorString()), false);
    }
    else if (not redirectionTarget.isNull())
    {
        const QUrl newUrl = m_feedURL.resolved(redirectionTarget.toUrl());

        m_feedURL = newUrl;
        m_reply->deleteLater();

        startDownloadFeed(m_feedURL);
        return;
    }
    else
    {
        // Done.
        xmlParseFeed();
    }

    m_reply->deleteLater();

    if (m_dropOnFinnish)
    {
        drop();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto FvUpdater::xmlParseFeed() -> bool
{
    QString xmlEnclosureUrl, xmlEnclosureVersion, xmlEnclosurePlatform;

    // Parse
    while (not m_xml.atEnd())
    {
        m_xml.readNext();

        if (m_xml.isStartElement())
        {
            if (m_xml.name() == "item"_L1)
            {
                xmlEnclosureUrl.clear();
                xmlEnclosureVersion.clear();
                xmlEnclosurePlatform.clear();
            }
            else if (m_xml.name() == "enclosure"_L1)
            {
                const QXmlStreamAttributes attribs = m_xml.attributes();
                const QString fervorPlatform = QStringLiteral("fervor:platform");

                if (attribs.hasAttribute(fervorPlatform))
                {
                    if (CurrentlyRunningOnPlatform(attribs.value(fervorPlatform).toString().trimmed()))
                    {
                        xmlEnclosurePlatform = attribs.value(fervorPlatform).toString().trimmed();

                        const QString attributeUrl = QStringLiteral("url");
                        if (attribs.hasAttribute(attributeUrl))
                        {
                            xmlEnclosureUrl = attribs.value(attributeUrl).toString().trimmed();
                        }
                        else
                        {
                            xmlEnclosureUrl.clear();
                        }

                        const QString fervorVersion = QStringLiteral("fervor:version");
                        if (attribs.hasAttribute(fervorVersion))
                        {
                            const QString candidateVersion = attribs.value(fervorVersion).toString().trimmed();
                            if (not candidateVersion.isEmpty())
                            {
                                xmlEnclosureVersion = candidateVersion;
                            }
                        }
                    }
                }
            }
        }
        else if (m_xml.isEndElement())
        {
            if (m_xml.name() == "item"_L1)
            {
                // That's it - we have analyzed a single <item> and we'll stop
                // here (because the topmost is the most recent one, and thus
                // the newest version.

                return searchDownloadedFeedForUpdates(xmlEnclosureUrl, xmlEnclosureVersion, xmlEnclosurePlatform);
            }
        }

        if (m_xml.error() && m_xml.error() != QXmlStreamReader::PrematureEndOfDocumentError)
        {
            showErrorDialog(
                tr("Feed parsing failed: %1 %2.").arg(QString::number(m_xml.lineNumber()), m_xml.errorString()), false);
            return false;
        }
    }

    // No updates were found if we're at this point
    // (not a single <item> element found)
    showInformationDialog(tr("No updates were found."), false);

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto FvUpdater::searchDownloadedFeedForUpdates(const QString &xmlEnclosureUrl, const QString &xmlEnclosureVersion,
                                               const QString &xmlEnclosurePlatform) -> bool
{
    qDebug() << "Enclosure URL:" << xmlEnclosureUrl;
    qDebug() << "Enclosure version:" << xmlEnclosureVersion;
    qDebug() << "Enclosure platform:" << xmlEnclosurePlatform;

    // Validate
    if (xmlEnclosureUrl.isEmpty() || xmlEnclosureVersion.isEmpty() || xmlEnclosurePlatform.isEmpty())
    {
        showErrorDialog(tr("Feed error: invalid \"enclosure\" with the download link"), false);
        return false;
    }

    // Relevant version?
    if (VersionIsIgnored(xmlEnclosureVersion))
    {
        qDebug() << "Version '" << xmlEnclosureVersion << "' is ignored, too old or something like that.";

        showInformationDialog(tr("No updates were found."), false);

        return true; // Things have succeeded when you think of it.
    }

    //
    // Success! At this point, we have found an update that can be proposed
    // to the user.
    //

    delete m_proposedUpdate;
    m_proposedUpdate = new FvAvailableUpdate(this);
    m_proposedUpdate->SetEnclosureUrl(xmlEnclosureUrl);
    m_proposedUpdate->SetEnclosureVersion(xmlEnclosureVersion);
    m_proposedUpdate->SetEnclosurePlatform(xmlEnclosurePlatform);

    // Show "look, there's an update" window
    showUpdaterWindowUpdatedWithCurrentUpdateProposal();

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto FvUpdater::VersionIsIgnored(const QString &version) -> bool
{
    // We assume that variable 'version' contains either:
    //	1) The current version of the application (ignore)
    //	2) The version that was skipped before and thus stored in QSettings (ignore)
    //	3) A newer version (don't ignore)
    // 'version' is not likely to contain an older version in any case.

    unsigned decVersion = 0x0;
    try
    {
        decVersion = VAbstractConverter::GetFormatVersion(version);
    }
    catch (const VException &e)
    {
        Q_UNUSED(e)
        return true; // Ignore invalid version
    }

    if (decVersion == AppVersion())
    {
        return true;
    }

    const unsigned lastSkippedVersion = VAbstractApplication::VApp()->Settings()->GetLatestSkippedVersion();
    if (lastSkippedVersion != 0x0)
    {
        if (decVersion == lastSkippedVersion)
        {
            // Implicitly skipped version - skip
            return true;
        }
    }

    if (decVersion > AppVersion())
    {
        // Newer version - do not skip
        return false;
    }

    // Fallback - skip
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::IgnoreVersion(const QString &version)
{
    unsigned decVersion = 0x0;
    try
    {
        decVersion = VAbstractConverter::GetFormatVersion(version);
    }
    catch (const VException &e)
    {
        Q_UNUSED(e)
        return; // Ignore invalid version
    }

    if (decVersion == AppVersion())
    {
        // Don't ignore the current version
        return;
    }

    VAbstractApplication::VApp()->Settings()->SetLatestSkippedVersion(decVersion);
}

//---------------------------------------------------------------------------------------------------------------------
auto FvUpdater::CurrentlyRunningOnPlatform(const QString &platform) -> bool
{
    const QStringList platforms = QStringList() << "Q_OS_LINUX"
                                                << "Q_OS_MAC"
                                                << "Q_OS_WIN32";

    switch (platforms.indexOf(platform.toUpper().trimmed()))
    {
        case 0:   // Q_OS_LINUX
#ifdef Q_OS_LINUX // Defined on Linux.
            return true;
#else
            return false;
#endif
        case 1: // Q_OS_MAC
#ifdef Q_OS_MAC // Defined on MAC OS (synonym for Darwin).
            return true;
#else
            return false;
#endif
        case 2:   // Q_OS_WIN32
#ifdef Q_OS_WIN32 // Defined on all supported versions of Windows.
            return true;
#else
            return false;
#endif
        default:
            break;
    }

    // Fallback
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::showErrorDialog(const QString &message, bool showEvenInSilentMode)
{
    if (m_silentAsMuchAsItCouldGet)
    {
        if (not showEvenInSilentMode)
        {
            // Don't show errors in the silent mode
            return;
        }
    }

    QMessageBox dlFailedMsgBox;
    dlFailedMsgBox.setIcon(QMessageBox::Critical);
    dlFailedMsgBox.setText(message);
    dlFailedMsgBox.exec();
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::showInformationDialog(const QString &message, bool showEvenInSilentMode)
{
    if (m_silentAsMuchAsItCouldGet)
    {
        if (not showEvenInSilentMode)
        {
            // Don't show information dialogs in the silent mode
            return;
        }
    }

    QMessageBox dlInformationMsgBox;
    dlInformationMsgBox.setIcon(QMessageBox::Information);
    dlInformationMsgBox.setText(message);
    dlInformationMsgBox.exec();
}
