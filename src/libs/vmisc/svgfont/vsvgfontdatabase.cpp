/************************************************************************
 **
 **  @file   vsvgfontdatabase.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   31 5, 2023
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
#include "vsvgfontdatabase.h"

#include <QDir>
#include <QDirIterator>
#include <QMutex>
#include <QString>

#include "../vabstractapplication.h"
#include "../vcommonsettings.h"
#include "vsvgfont.h"
#include "vsvgfontengine.h"
#include "vsvgfontreader.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

Q_GLOBAL_STATIC(QMutex, svgFontDatabaseMutex) // NOLINT

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
auto GetStyleScore(SVGFontStyle desiredStyle, SVGFontStyle fontStyle) -> int
{
    // Assign a score based on the proximity of the desired style to the font style
    if (desiredStyle == fontStyle)
    {
        return 100; // Direct match
    }

    if (desiredStyle == SVGFontStyle::Italic && fontStyle == SVGFontStyle::Oblique)
    {
        return 75; // Close match between Italic and Oblique
    }

    if (desiredStyle == SVGFontStyle::Oblique && fontStyle == SVGFontStyle::Italic)
    {
        return 75; // Close match between Oblique and Italic
    }

    return 0; // No match
}

//---------------------------------------------------------------------------------------------------------------------
auto GetWeightScore(SVGFontWeight desiredWeight, SVGFontWeight fontWeight) -> int
{
    // Assign a score based on the proximity of the desired weight to the font weight
    int weightDifference = qAbs(static_cast<int>(desiredWeight) - static_cast<int>(fontWeight));
    if (weightDifference == 0)
    {
        return 100; // Direct match
    }

    int gradationThreshold = 200;
    if (weightDifference <= gradationThreshold)
    {
        // Calculate a score based on the proximity of the desired weight to the font weight
        int score = 100 - (weightDifference * 100 / gradationThreshold);
        return score;
    }

    return 0; // No match
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
void VSvgFontDatabase::PopulateFontDatabase(const QString &path)
{
    QMutexLocker locker(svgFontDatabaseMutex());

    m_fontCache.clear();
    m_fontEngineCache.clear();

    if (path.isEmpty())
    {
        VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
        QStringList directories = {SystemSVGFontPath(), settings->GetPathSVGFonts()};

        m_fontDB.clear();
        m_populated = false;

        for (int i = 0; i < directories.size(); ++i)
        {
            ParseDirectory(directories.at(i), i);
        }
    }
    else
    {
        int priority = (path == SystemSVGFontPath() ? 0 : 1);
        Invalidate(path);
        ParseDirectory(path, priority);
    }

    UpdateIndexes();
    m_populated = true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontDatabase::IsPopulated() const -> bool
{
    QMutexLocker locker(svgFontDatabaseMutex());
    return m_populated;
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFontDatabase::InvalidatePath(const QString &path)
{
    QMutexLocker locker(svgFontDatabaseMutex());

    Invalidate(path);
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontDatabase::Families() const -> QStringList
{
    QMutexLocker locker(svgFontDatabaseMutex());

    return m_indexFontFamily.keys();
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontDatabase::Font(const QString &family, SVGFontStyle style, SVGFontWeight weight) const -> VSvgFont
{
    QMutexLocker locker(svgFontDatabaseMutex());

    return QueryFont(family, style, weight);
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontDatabase::FontEngine(const VSvgFont &font) const -> VSvgFontEngine
{
    QMutexLocker locker(svgFontDatabaseMutex());

    return QueryFontEngine(font.FamilyName(), font.Style(), font.Weight(), font.PointSize());
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontDatabase::FontEngine(const QString &family, SVGFontStyle style, SVGFontWeight weight, int pointSize) const
    -> VSvgFontEngine
{
    QMutexLocker locker(svgFontDatabaseMutex());

    return QueryFontEngine(family, style, weight, pointSize);
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontDatabase::Bold(const QString &family, SVGFontStyle style) const -> bool
{
    QMutexLocker locker(svgFontDatabaseMutex());

    VSvgFont font = QueryFont(family, style, SVGFontWeight::Bold);
    int weightScore = GetWeightScore(SVGFontWeight::Bold, font.Weight());
    return weightScore > 0;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontDatabase::Italic(const QString &family, SVGFontStyle style) const -> bool
{
    QMutexLocker locker(svgFontDatabaseMutex());

    VSvgFont font = QueryFont(family, style, SVGFontWeight::Normal);
    int styleScore = GetStyleScore(style, font.Style());

    return styleScore > 0;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontDatabase::SystemSVGFontPath() -> QString
{
    const QString fontPath = QStringLiteral("/svgfonts");
#ifdef Q_OS_WIN
    return QCoreApplication::applicationDirPath() + fontPath;
#elif defined(Q_OS_MAC)
    QDir dirBundle(QCoreApplication::applicationDirPath() + "/../Resources"_L1 + fontPath);
    if (dirBundle.exists())
    {
        return dirBundle.absolutePath();
    }

    QDir appDir = QDir(QCoreApplication::applicationDirPath());
    appDir.cdUp();
    appDir.cdUp();
    appDir.cdUp();
    QDir dir(appDir.absolutePath() + fontPath);
    if (dir.exists())
    {
        return dir.absolutePath();
    }

    return PKGDATADIR + fontPath;
#else // Unix
    QDir dir(QCoreApplication::applicationDirPath() + fontPath);
    if (dir.exists())
    {
        return dir.absolutePath();
    }

#ifdef QBS_BUILD
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    dir.setPath(QCoreApplication::applicationDirPath() + "/../../.." + PKGDATADIR + fontPath);
#else
    dir = QDir(QCoreApplication::applicationDirPath() + "/../../.." + PKGDATADIR + fontPath);
#endif
    if (dir.exists())
    {
        return dir.absolutePath();
    }
#endif // QBS_BUILD

#if defined(APPIMAGE) && defined(Q_OS_LINUX)
    /* Fix path to trasnaltions when run inside AppImage. */
    return AppImageRoot() + PKGDATADIR + trPath;
#else
    return PKGDATADIR + fontPath;
#endif // defined(APPIMAGE) && defined(Q_OS_LINUX)
#endif // Unix
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFontDatabase::UpdateIndexes()
{
    m_indexFontPath.clear();
    m_indexFontFamily.clear();

    auto i = m_fontDB.constBegin();
    while (i != m_fontDB.constEnd())
    {
        m_indexFontPath.insert(i.key(), i.value().Path());
        m_indexFontFamily[i.value().FamilyName()].append(i.key());
        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFontDatabase::ParseDirectory(const QString &path, int priority)
{
    VSvgFontReader reader;
    QDirIterator it(path, {"*.svg"}, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        QString fontFilePath = it.next();
        QFile fontFile(fontFilePath);

        if (fontFile.exists() && fontFile.open(QIODevice::ReadOnly))
        {
            VSvgFont font = reader.ReadSvgFontHeader(&fontFile);
            if (not font.IsValid())
            {
                continue;
            }

            font.SetPriority(priority);
            font.SetPath(fontFilePath);
            if (m_fontDB.contains(font.Id()))
            {
                VSvgFont duplicate = m_fontDB.value(font.Id());
                if (font.Priority() >= duplicate.Priority())
                {
                    m_fontDB.insert(font.Id(), font);
                }
            }
            else
            {
                m_fontDB.insert(font.Id(), font);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFontDatabase::Invalidate(const QString &path)
{
    const QString absolutePath = QFileInfo(path).absoluteDir().absolutePath();

    auto i = m_indexFontPath.constBegin();
    while (i != m_indexFontPath.constEnd())
    {
        if (QFileInfo(i.key()).absoluteDir().absolutePath().startsWith(absolutePath))
        {
            m_fontDB.remove(i.value());
        }
        ++i;
    }

    m_fontCache.clear();
    m_fontEngineCache.clear();
    UpdateIndexes();
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontDatabase::QueryFont(const QString &family, SVGFontStyle style, SVGFontWeight weight) const -> VSvgFont
{
    QByteArray hash = VSvgFont::Hash(family, style, weight);
    if (m_fontCache.contains(hash))
    {
        return {*m_fontCache.object(hash)};
    }

    if (m_indexFontFamily.contains(family))
    {
        const QList<QString> fonts = m_indexFontFamily.value(family);
        VSvgFont bestMatch;
        int bestMatchScore = 0;
        for (const auto &fontId : fonts)
        {
            if (m_fontDB.contains(fontId))
            {
                VSvgFont f = m_fontDB.value(fontId);

                int styleScore = GetStyleScore(style, f.Style());
                int weightScore = GetWeightScore(weight, f.Weight());

                // Calculate the total match score
                int familyScore = 100;
                int totalScore = familyScore + styleScore + weightScore;

                // Update the best match if the current font has a higher score
                if (totalScore > bestMatchScore)
                {
                    bestMatch = f;
                    bestMatchScore = totalScore;
                }
            }
        }

        m_fontCache.insert(bestMatch.Hash(), new VSvgFont(bestMatch));
        return bestMatch;
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontDatabase::QueryFontEngine(const QString &family, SVGFontStyle style, SVGFontWeight weight,
                                       int pointSize) const -> VSvgFontEngine
{
    VSvgFont font = QueryFont(family, style, weight);
    QByteArray hash = font.Hash();
    if (m_fontEngineCache.contains(hash))
    {
        VSvgFontEngine engine = *m_fontEngineCache.object(hash);
        VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
        engine.SetFontPointSize(pointSize > 0 ? pointSize : settings->GetPatternLabelFontSize());

        return engine;
    }

    if (font.IsValid())
    {
        VSvgFontReader reader;
        QFile fontFile(font.Path());

        if (fontFile.exists() && fontFile.open(QIODevice::ReadOnly))
        {
            VSvgFontEngine engine = reader.ReadSvgFont(&fontFile);
            VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
            engine.SetFontPointSize(pointSize > 0 ? pointSize : settings->GetPatternLabelFontSize());

            m_fontEngineCache.insert(engine.Font().Hash(), new VSvgFontEngine(engine));
            return engine;
        }
    }

    return {};
}
