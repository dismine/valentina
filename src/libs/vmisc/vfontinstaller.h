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

#include <QObject>
#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <qtclasshelpermacros.h>
#endif

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
        RegistrationFailed,     // Added to handle Windows registry errors
        UserCancelled
    };

    /**
     * @brief Installs a single font file to the user's font directory.
     * @param sourceFilePath The absolute path to the font file (e.g., ".ttf", ".otf" or ".svg").
     * @param window Parent window
     * @return An InstallError code indicating success or failure reason.
     */
    auto InstallFont(const QString &sourceFilePath, QWidget *window) -> InstallError;

    auto ErrorMessage() const -> QString;

    /**
     * @brief Resets the overwrite choice. Call this before starting
     * a new batch of font installations.
     */
    void ResetOverwriteMode();

private:
    Q_DISABLE_COPY_MOVE(VFontInstaller)

    // Define the states for the overwrite decision
    enum class OverwriteMode
    {
        Ask,      // Ask the user for each font
        YesToAll, // Overwrite all subsequent fonts
        NoToAll   // Skip all subsequent fonts
    };

    // Add this member variable to your class
    OverwriteMode m_overwriteMode = OverwriteMode::Ask;

    /**
     * @brief m_errorMessage If installation fails, this string will contain a detailed error.
     */
    QString m_errorMessage{};

    /**
     * @brief Installs a single outline font file to the user's system font directory.
     * @param sourceFilePath The absolute path to the outline font file (e.g., ".ttf" or ".otf").
     * @param window Parent window
     * @return An InstallError code indicating success or failure reason.
     */
    auto InstallOutLineFont(const QString &sourceFilePath, QWidget *window) -> InstallError;

    /**
     * @brief Installs a single SVG font file to the user's font directory.
     * @param sourceFilePath The absolute path to the SVG font file (e.g., ".svg").
     * @param window Parent window
     * @return An InstallError code indicating success or failure reason.
     */
    auto InstallSVGFont(const QString &sourceFilePath, QWidget *window) -> InstallError;

    /**
     * @brief Searches for and installs an optional companion JSON correction file for the font.
     * * The correction file is expected to be in the same source directory as the font file,
     * named after the font's official family name (e.g., "Inter.json").
     * @param sourceDir The source directory containing the font and potentially the JSON file.
     * @param fontId The ID of the font as loaded by QFontDatabase.
     */
    void InstallFontCorrections(const QDir &sourceDir, int fontId);

    auto ValidateInstallPath(const QString &installPath, const QString &sourceFilePath, QString &outDestinationFilePath)
        -> InstallError;
    auto HandleFileOverwriteAndCopy(const QString &sourceFilePath, const QString &destinationFilePath, QWidget *window)
        -> InstallError;
};

//---------------------------------------------------------------------------------------------------------------------
inline void VFontInstaller::ResetOverwriteMode()
{
    m_overwriteMode = OverwriteMode::Ask;
}

#endif // VFONTINSTALLER_H
