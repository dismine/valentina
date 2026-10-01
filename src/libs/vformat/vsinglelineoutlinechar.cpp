/************************************************************************
 **
 **  @file   vsinglelineoutlinechar.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   19 6, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#include "vsinglelineoutlinechar.h"
#include "../vmisc/compatibility.h"

#include <QCache>
#include <QDir>
#include <QFile>
#include <QFontMetrics>
#include <QGlobalStatic>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QPainterPath>
#include <QRawFont>
#include <QtDebug>

#include <memory>
#include <thread>

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
#include <QDirListing>
#endif

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

Q_GLOBAL_STATIC(QMutex, singleLineOutlineCharMutex)                         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(VOutlineCorrectionsCache, cachedCorrections, (5)) // NOLINT
// Bumped by every cache-clearing call. LoadCorrectionsAsync() captures the value before
// dispatching its worker and checks it again before committing the result, so a load that was
// already in flight when the corrections path changed (ClearAllCorrectionsCache()) or a single
// family was invalidated (ClearCorrectionsCache()) can't resurrect stale data afterward. Every
// access happens under singleLineOutlineCharMutex already (the cache ops right next to it need
// the same lock anyway), so a plain int guarded by that mutex is enough -- no atomic needed.
Q_GLOBAL_STATIC(int, correctionsGeneration)                       // NOLINT
Q_GLOBAL_STATIC(VOutlineCorrectionsNotifier, correctionsNotifier) // NOLINT

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
Q_REQUIRED_RESULT auto ParseCorrectiosn(const QJsonObject &correctionsObject) -> VOutlineCorrections *
{
    auto *corrections = new VOutlineCorrections;
    for (auto it = correctionsObject.constBegin(); it != correctionsObject.constEnd(); ++it)
    {
        QString glyph = it.key();
        if (glyph.isEmpty())
        {
            continue;
        }

        QHash<int, bool> segments;
        QJsonObject const segmentsObject = it.value().toObject();
        for (auto segmentsIt = segmentsObject.constBegin(); segmentsIt != segmentsObject.constEnd(); ++segmentsIt)
        {
            bool const correct = segmentsIt.value().toBool();
            if (!correct)
            {
                segments.insert(segmentsIt.key().toInt(), correct);
            }
        }

        if (!segments.isEmpty())
        {
            corrections->insert(glyph.front(), segments);
        }
    }

    return corrections;
}

//---------------------------------------------------------------------------------------------------------------------
auto CorrectPath(const QPainterPath &path, const QHash<int, bool> &segmentCorrections) -> QPainterPath
{
    const QList<QPolygonF> subpaths = path.toSubpathPolygons();

    QPainterPath outlinePath;
    for (int i = 0; i < subpaths.size(); ++i)
    {
        QPolygonF polygon = subpaths.at(i);
        if (segmentCorrections.value(i, true) && polygon.size() > 2)
        {
            polygon = First(polygon, polygon.size() - 1);
        }
        outlinePath.addPolygon(polygon);
    }

    return outlinePath;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FindAndParseCorrections locates and parses "<fontFamily>.json" under dirPath.
 *
 * dirPath is a user-configurable setting (Preferences -> Paths) and its default lives under the
 * platform "Documents" folder, which is commonly redirected to a network share or a cloud-sync
 * placeholder, so this can block for a long time. Returns nullptr if nothing usable was found, so
 * callers can cache that explicitly and never repeat the disk hit for this font family.
 */
Q_REQUIRED_RESULT auto FindAndParseCorrections(const QString &dirPath, const QString &fontFamily)
    -> VOutlineCorrections *
{
    auto const fileName = QStringLiteral("%1.json").arg(fontFamily);

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    using F = QDirListing::IteratorFlag;

    QDirListing dirListing(dirPath, QStringList(fileName), F::FilesOnly);
    QString filePath;
    for (const auto &entry : dirListing)
    {
        if (entry.fileName() == fileName)
        {
            filePath = entry.absoluteFilePath();
            break; // Exit after finding the first match
        }
    }

    if (filePath.isEmpty())
    {
        return nullptr; // No matching files found
    }
#else
    QDir directory(dirPath);
    directory.setNameFilters(QStringList(fileName));
    QStringList const matchingFiles = directory.entryList();
    if (matchingFiles.isEmpty())
    {
        return nullptr; // No matching files found
    }
    QString const filePath = directory.absoluteFilePath(matchingFiles.constFirst());
#endif

    QFile jsonFile(filePath);
    if (!jsonFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open file for reading.";
        return nullptr;
    }

    // Read the JSON data from the file
    QByteArray const jsonData = jsonFile.readAll();

    // Create a JSON document from the JSON data
    QJsonDocument const jsonDocument = QJsonDocument::fromJson(jsonData);

    if (jsonDocument.isNull())
    {
        qDebug() << "Failed to parse JSON document.";
        return nullptr;
    }

    return ParseCorrectiosn(jsonDocument.object());
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
auto GetOutlineCorrectionsNotifier() -> VOutlineCorrectionsNotifier *
{
    return correctionsNotifier();
}

//---------------------------------------------------------------------------------------------------------------------
VSingleLineOutlineChar::VSingleLineOutlineChar(const QFont &font)
  : m_font(font)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VSingleLineOutlineChar::ExportCorrections(const QString &dirPath) const
{
    QRawFont const rawFont = QRawFont::fromFont(m_font);
    QJsonObject correctionsObject;

    for (char32_t unicode = 0; unicode <= 0x10FFFF; ++unicode)
    {
        // Check if the glyph is available for the font
        if (rawFont.supportsCharacter(unicode))
        {
            QString const str = QString::fromUcs4(&unicode, 1);

            QPainterPath path;
            path.addText(0, 0, m_font, str);

            const QList<QPolygonF> subpaths = path.toSubpathPolygons();
            if (subpaths.isEmpty())
            {
                continue;
            }

            QJsonObject segments;
            for (int i = 0; i < subpaths.size(); ++i)
            {
                segments[QString::number(i)] = true;
            }

            correctionsObject[str] = segments;
        }
    }

    auto const filename = QStringLiteral("%1/%2.json").arg(dirPath, m_font.family());
    QFile jsonFile(filename);
    if (!jsonFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qCritical() << tr("Failed to open file for writing.");
        return;
    }

    QJsonDocument const jsonDocument(correctionsObject);

    // Write the JSON string to the file
    QTextStream out(&jsonFile);
    out << jsonDocument.toJson(QJsonDocument::Indented);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief LoadCorrections looks for corrections for the current font and blocks until it knows.
 *
 * Callers that consume the result right after calling this (export, printing/layout, the
 * preferences preview) need the correct data, not "eventually correct", so this stays
 * synchronous. It does cache a miss (no file, unreadable, unparsable) the same as a hit, so a
 * missing or permanently unreachable corrections directory is only ever hit once per font family
 * per run rather than on every call. Painting on the UI thread should use LoadCorrectionsAsync()
 * instead.
 */
void VSingleLineOutlineChar::LoadCorrections(const QString &dirPath) const
{
    std::unique_ptr<VOutlineCorrections> corrections(FindAndParseCorrections(dirPath, m_font.family()));
    QMutexLocker const locker(singleLineOutlineCharMutex());
    if (VOutlineCorrectionsCache *cache = cachedCorrections())
    {
        cache->insert(m_font.family(), corrections ? corrections.release() : new VOutlineCorrections);
    }
    // else: the cache is already gone (shutting down); corrections, if any, frees itself.
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief LoadCorrectionsAsync is LoadCorrections() for call sites inside paint().
 *
 * dirPath is a user-configurable setting (Preferences -> Paths) whose default lives under the
 * platform "Documents" folder, which is commonly redirected to a network share or a cloud-sync
 * placeholder, so finding/reading the corrections file there can block for a long time -- painting
 * on the main thread must never do that. This marks the font family as loading (an empty
 * placeholder, so IsPopulated() stops retrying and paint() draws uncorrected glyphs for now) and
 * runs the actual disk access on a detached worker thread, which emits
 * GetOutlineCorrectionsNotifier()'s CorrectionsLoaded signal once it commits a real result, so a
 * caller that painted uncorrected glyphs while the load was in flight can connect to it and
 * repaint itself instead of waiting on an unrelated redraw.
 *
 * A raw detached std::thread, not QThreadPool, is deliberate: QThreadPool::~QThreadPool()
 * (including the global instance, destroyed at process exit) blocks until every runnable has
 * finished, so a worker stuck on the same kind of slow/unreachable path this function exists to
 * route around would turn "painting hangs" into "closing the app hangs" instead. A detached
 * thread is never joined by anyone, so it cannot block shutdown; if it is still running when the
 * process exits, the OS simply reclaims it. That also means it can still be running after this
 * translation unit's Q_GLOBAL_STATICs are destroyed, hence the generation check and null guards
 * below instead of a bare cachedCorrections()->insert(...).
 */
void VSingleLineOutlineChar::LoadCorrectionsAsync(const QString &dirPath) const
{
    int startGeneration = 0;
    {
        QMutexLocker const locker(singleLineOutlineCharMutex());
        VOutlineCorrectionsCache *cache = cachedCorrections();
        if (cache == nullptr || cache->contains(m_font.family()))
        {
            return; // Shutting down, already loaded, or a background load is already in progress.
        }
        cache->insert(m_font.family(), new VOutlineCorrections);
        if (const int *generation = correctionsGeneration())
        {
            startGeneration = *generation;
        }
    }

    const QFont font = m_font;
    std::thread(
        [font, dirPath, startGeneration]()
        {
            std::unique_ptr<VOutlineCorrections> corrections(FindAndParseCorrections(dirPath, font.family()));
            if (!corrections)
            {
                return;
            }

            {
                QMutexLocker const locker(singleLineOutlineCharMutex());
                VOutlineCorrectionsCache *cache = cachedCorrections();
                const int *generation = correctionsGeneration();
                if (cache == nullptr || generation == nullptr || *generation != startGeneration)
                {
                    // The cache was cleared (corrections path changed via
                    // ClearAllCorrectionsCache(), this family invalidated via
                    // ClearCorrectionsCache(), or the process is shutting down) while this load
                    // was in flight -- that result is for a state that no longer exists, so drop
                    // it instead of resurrecting stale data (corrections frees itself on return).
                    return;
                }
                cache->insert(font.family(), corrections.release());
            }

            // Whatever painted this family's glyphs uncorrected while this load was running (see
            // LoadCorrectionsAsync()'s doc comment) can now redraw itself with the real result.
            // Emitted outside the lock above, and safe from a worker thread: Qt queues delivery
            // to whichever thread each connected receiver actually lives on.
            if (VOutlineCorrectionsNotifier *notifier = GetOutlineCorrectionsNotifier())
            {
                emit notifier->CorrectionsLoaded(font.family());
            }
        })
        .detach();
}

//---------------------------------------------------------------------------------------------------------------------
void VSingleLineOutlineChar::ClearCorrectionsCache()
{
    QMutexLocker const locker(singleLineOutlineCharMutex());
    if (int *generation = correctionsGeneration())
    {
        ++(*generation);
    }
    if (VOutlineCorrectionsCache *cache = cachedCorrections())
    {
        cache->remove(m_font.family());
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ClearAllCorrectionsCache invalidates every font family's cached corrections at once.
 *
 * Call this whenever the corrections directory itself changes (Preferences -> Paths): a path
 * change can affect every font family that has already been resolved, not just one, so the
 * per-instance ClearCorrectionsCache() is not enough.
 */
void VSingleLineOutlineChar::ClearAllCorrectionsCache()
{
    QMutexLocker const locker(singleLineOutlineCharMutex());
    if (int *generation = correctionsGeneration())
    {
        ++(*generation);
    }
    if (VOutlineCorrectionsCache *cache = cachedCorrections())
    {
        cache->clear();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VSingleLineOutlineChar::DrawChar(qreal x, qreal y, QChar c) const -> QPainterPath
{
    if (c == QChar(0x042B) || c == QChar(0x044B) || c == QChar(0x042A) || c == QChar(0x044A) || c == QChar(0x0401)
        || c == QChar(0x0451) || c == QChar(0x042D) || c == QChar(0x044D))
    {
        c = QChar(0xFFFD);
    }

    QPainterPath path;
    path.addText(x, y, m_font, c);

    QHash<int, bool> segmentCorrections;
    {
        QMutexLocker const locker(singleLineOutlineCharMutex());
        if (VOutlineCorrectionsCache *cache = cachedCorrections(); cache != nullptr && cache->contains(m_font.family()))
        {
            segmentCorrections = cache->object(m_font.family())->value(c);
        }
    }

    return CorrectPath(path, segmentCorrections);
}

//---------------------------------------------------------------------------------------------------------------------
auto VSingleLineOutlineChar::IsPopulated() const -> bool
{
    QMutexLocker const locker(singleLineOutlineCharMutex());
    VOutlineCorrectionsCache *cache = cachedCorrections();
    return cache != nullptr && cache->contains(m_font.family());
}
