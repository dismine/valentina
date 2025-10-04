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

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
#include <QDirListing>
#endif

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

Q_GLOBAL_STATIC(QMutex, singleLineOutlineCharMutex)                         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(VOutlineCorrectionsCache, cachedCorrections, (5)) // NOLINT

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
} // namespace

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
void VSingleLineOutlineChar::LoadCorrections(const QString &dirPath) const
{
    auto const fileName = QStringLiteral("%1.json").arg(m_font.family());

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
        return; // No matching files found
    }
#else
    QDir directory(dirPath);
    directory.setNameFilters(QStringList(fileName));
    QStringList const matchingFiles = directory.entryList();
    if (matchingFiles.isEmpty())
    {
        return; // No matching files found
    }
    QString const filePath = directory.absoluteFilePath(matchingFiles.constFirst());
#endif

    QFile jsonFile(filePath);
    if (!jsonFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open file for reading.";
        return;
    }

    // Read the JSON data from the file
    QByteArray const jsonData = jsonFile.readAll();

    // Create a JSON document from the JSON data
    QJsonDocument const jsonDocument = QJsonDocument::fromJson(jsonData);

    if (jsonDocument.isNull())
    {
        qDebug() << "Failed to parse JSON document.";
        return;
    }

    QMutexLocker const locker(singleLineOutlineCharMutex());
    cachedCorrections()->insert(m_font.family(), ParseCorrectiosn(jsonDocument.object()));
}

//---------------------------------------------------------------------------------------------------------------------
void VSingleLineOutlineChar::ClearCorrectionsCache()
{
    QMutexLocker const locker(singleLineOutlineCharMutex());
    cachedCorrections()->remove(m_font.family());
}

//---------------------------------------------------------------------------------------------------------------------
auto VSingleLineOutlineChar::DrawChar(qreal x, qreal y, QChar c) const -> QPainterPath
{
    if (c == QChar(0x042B) || c == QChar(0x044B) || c == QChar(0x042A) || c == QChar(0x044A) || c == QChar(0x0401) ||
        c == QChar(0x0451) || c == QChar(0x042D) || c == QChar(0x044D))
    {
        c = QChar(0xFFFD);
    }

    QPainterPath path;
    path.addText(x, y, m_font, c);

    QMutexLocker locker(singleLineOutlineCharMutex());
    QHash<int, bool> segmentCorrections;

    if (cachedCorrections()->contains(m_font.family()))
    {
        segmentCorrections = cachedCorrections()->object(m_font.family())->value(c);
    }
    locker.unlock();

    return CorrectPath(path, segmentCorrections);
}

//---------------------------------------------------------------------------------------------------------------------
auto VSingleLineOutlineChar::IsPopulated() const -> bool
{
    return cachedCorrections()->contains(m_font.family());
}
