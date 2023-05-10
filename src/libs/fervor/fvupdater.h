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

#ifndef FVUPDATER_H
#define FVUPDATER_H

#include <QMetaObject>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QUrl>
#include <QXmlStreamReader>
#include <QtGlobal>

#include "fvavailableupdate.h"
#include "fvupdatewindow.h"

class FvUpdater final : public QObject
{
    Q_OBJECT // NOLINT

public:
    // Singleton
    static auto sharedUpdater() -> FvUpdater *;
    static void drop();
    static auto CurrentFeedURL() -> QString;
    static auto IsTestBuild() -> bool;

    // Set / get feed URL
    void SetFeedURL(const QUrl &feedURL);
    void SetFeedURL(const QString &feedURL);
    auto GetFeedURL() const -> QString;

    auto IsDropOnFinnish() const -> bool;
    void SetDropOnFinnish(bool value);

public slots:
    // Check for updates
    bool CheckForUpdates(bool silentAsMuchAsItCouldGet = true); // NOLINT(modernize-use-trailing-return-type)

    // Aliases
    bool CheckForUpdatesSilent();    // NOLINT(modernize-use-trailing-return-type)
    bool CheckForUpdatesNotSilent(); // NOLINT(modernize-use-trailing-return-type)

protected:
    friend class FvUpdateWindow; // Uses GetProposedUpdate() and others
    auto GetProposedUpdate() -> QPointer<FvAvailableUpdate>;

protected slots:
    // Update window button slots
    void InstallUpdate();
    void SkipUpdate();
    void RemindMeLater();

    // Update confirmation dialog button slots
    void UpdateInstallationConfirmed();

private slots:
    void httpFeedDownloadFinished();

private:
    //
    // Singleton business
    //
    Q_DISABLE_COPY_MOVE(FvUpdater) // NOLINT
    FvUpdater();		  // Hide main constructor
    virtual ~FvUpdater(); // Hide main destructor

    static QPointer<FvUpdater> m_Instance; // Singleton instance

    QPointer<FvUpdateWindow> m_updaterWindow; // Updater window (NULL if not shown)

    // Available update (NULL if not fetched)
    QPointer<FvAvailableUpdate> m_proposedUpdate;

    // If true, don't show the error dialogs and the "no updates." dialog
    // (silentAsMuchAsItCouldGet from CheckForUpdates() goes here)
    // Useful for automatic update checking upon application startup.
    bool m_silentAsMuchAsItCouldGet;

    //
    // HTTP feed fetcher infrastructure
    //
    QUrl                    m_feedURL;		    // Feed URL that will be fetched
    QNetworkAccessManager   m_qnam;
    QPointer<QNetworkReply> m_reply;
    bool                    m_httpRequestAborted;
    bool                    m_dropOnFinnish;

    QXmlStreamReader m_xml;	// XML data collector and parser

    void showUpdaterWindowUpdatedWithCurrentUpdateProposal();		// Show updater window
    void hideUpdaterWindow();										// Hide + destroy m_updaterWindow

    void startDownloadFeed(const QUrl &url); // Start downloading feed
    void cancelDownloadFeed();			     // Stop downloading the current feed

    // Dialogs (notifications)
    // Show an error message
    void showErrorDialog(const QString &message, bool showEvenInSilentMode = false);
    // Show an informational message
    void showInformationDialog(const QString &message, bool showEvenInSilentMode = false);

    auto xmlParseFeed() -> bool; // Parse feed in m_xml
    auto searchDownloadedFeedForUpdates(const QString &xmlEnclosureUrl, const QString &xmlEnclosureVersion,
                                        const QString &xmlEnclosurePlatform) -> bool;

    static auto VersionIsIgnored(const QString &version) -> bool;
    static void IgnoreVersion(const QString &version);
    static auto CurrentlyRunningOnPlatform(const QString &platform) -> bool;
};

#endif // FVUPDATER_H
