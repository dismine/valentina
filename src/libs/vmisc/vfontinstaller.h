/************************************************************************
 **
 **  @file   vfontinstaller.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 10, 2025
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2025 Valentina project
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
#ifndef VFONTINSTALLER_H
#define VFONTINSTALLER_H

#include <qtclasshelpermacros.h>
#include <QObject>
#include <QtGlobal>

class QString;
class QDir;

/**
 * @brief The FontInstaller class handles cross-platform font installation.
 * * It uses QStandardPaths::FontsLocation to find the user's writable for outline font
 * directory on Windows, macOS, and Linux, and includes necessary post-copy
 * operations (like font cache rebuilding on Linux).
 */

class VFontInstaller : public QObject
{
    Q_OBJECT // NOLINT

public:
    explicit VFontInstaller(QObject *parent = nullptr);
    ~VFontInstaller() override = default;

    enum class InstallError : quint8
    {
        NoError,
        SourceFileNotFound,
        DestinationPathNotFound,
        CopyFailed,
        FontCacheRebuildFailed, // Specific to Linux
        RegistrationFailed      // Added to handle Windows registry errors
    };

    /**
     * @brief Installs a single font file to the user's font directory.
     * @param sourceFilePath The absolute path to the font file (e.g., ".ttf", ".otf" or ".svg").
     * @return An InstallError code indicating success or failure reason.
     */
    auto InstallFont(const QString &sourceFilePath) -> InstallError;

    auto ErrorMessage() const -> QString;

private:
    Q_DISABLE_COPY_MOVE(VFontInstaller)

    /**
     * @brief m_errorMessage If installation fails, this string will contain a detailed error.
     */
    QString m_errorMessage{};

    /**
     * @brief Installs a single outline font file to the user's system font directory.
     * @param sourceFilePath The absolute path to the outline font file (e.g., ".ttf" or ".otf").
     * @return An InstallError code indicating success or failure reason.
     */
    auto InstallOutLineFont(const QString &sourceFilePath) -> InstallError;

    /**
     * @brief Installs a single SVG font file to the user's font directory.
     * @param sourceFilePath The absolute path to the SVG font file (e.g., ".svg").
     * @return An InstallError code indicating success or failure reason.
     */
    auto InstallSVGFont(const QString &sourceFilePath) -> InstallError;

    /**
     * @brief Searches for and installs an optional companion JSON correction file for the font.
     * * The correction file is expected to be in the same source directory as the font file,
     * named after the font's official family name (e.g., "Inter.json").
     * @param sourceDir The source directory containing the font and potentially the JSON file.
     * @param fontId The ID of the font as loaded by QFontDatabase.
     */
    void InstallFontCorrections(const QDir &sourceDir, int fontId);
};

#endif // VFONTINSTALLER_H
