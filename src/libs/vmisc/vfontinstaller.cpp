/************************************************************************
 **
 **  @file   vfontinstaller.cpp
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
#include "vfontinstaller.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFontDatabase>
#include <QMessageBox>
#include <QProcess>
#include <QStandardPaths>

#ifdef Q_OS_WIN
#include <qt_windows.h> // For Q_OS_WIN and HIWORD/LOWORD
#include <windows.h>
#include <winreg.h> // For Registry definitions
#endif

#include "vabstractapplication.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
/**
 * @brief Performs Windows-specific font registration in the HKEY_CURRENT_USER registry hive.
 * This is necessary for older Windows versions (pre-10.0.17083) to recognize the font.
 * @param fontFileName The filename of the copied font (e.g., "MyFont.ttf").
 * @param destinationFilePath The full path to the font file (e.g., "C:\...\Fonts\MyFont.ttf").
 * @return True if registration was successful or unnecessary, false otherwise.
 */
#ifdef Q_OS_WIN
auto RegisterFontInRegistry(const QString &fontFileName, const QString &destinationFilePath) -> bool
{
    // 1. Determine the correct display suffix based on file extension
    QString fileExtension = QFileInfo(fontFileName).suffix().toLower();
    QString displaySuffix;

    if (fileExtension == "ttf"_L1 || fileExtension == "ttc"_L1)
    {
        displaySuffix = " (TrueType)"_L1;
    }
    else if (fileExtension == "otf"_L1)
    {
        // OpenType fonts should be registered with the (OpenType) suffix on older Windows.
        displaySuffix = " (OpenType)"_L1;
    }
    else
    {
        // Fallback for uncommon or unsupported formats.
        qWarning() << "Attempting to register font with unsupported extension in registry:" << fileExtension;
        displaySuffix = QString(); // Use an empty suffix as a last resort.
    }

    // The key for per-user fonts that does NOT require admin rights.
    const QString registryPath = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";
    QSettings settings(registryPath, QSettings::NativeFormat);

    // The registry key name is the font's display name, and the value is the full path to the file.
    QString keyName = QFileInfo(fontFileName).baseName() + displaySuffix;

    // The value must be the full, native path of the font file.
    QString value = QDir::toNativeSeparators(destinationFilePath);

    settings.setValue(keyName, value);

    if (settings.status() != QSettings::NoError)
    {
        qCritical() << "Failed to write font registry entry for key:" << keyName << ". Error:" << settings.status();
        return false;
    }

    // 2. Broadcast the change to running applications (WM_FONTCHANGE)
    qInfo() << "Broadcasting WM_FONTCHANGE message...";

    // Send a broadcast message to all top-level windows to notify them of the font change.
    DWORD_PTR result;
    LRESULT lresult = SendMessageTimeoutW(HWND_BROADCAST,
                                          WM_FONTCHANGE,
                                          0,
                                          0,
                                          SMTO_ABORTIFHUNG,
                                          5000, // Timeout in milliseconds (5 seconds)
                                          &result);

    if (lresult == 0 && GetLastError() != 0)
    {
        // Log error only if SendMessageTimeout fails with a non-zero error code.
        qWarning() << "WM_FONTCHANGE broadcast failed or timed out. Error code:" << GetLastError();
    }
    else
    {
        qInfo() << "WM_FONTCHANGE broadcast completed.";
    }

    return true;
}
#endif // Q_OS_WIN

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Retrieves the cross-platform path where user outline fonts are installed.
 * @return The absolute path to the user's outline font directory.
 */
auto GetOutlineFontInstallPath() -> QString
{
    QString installPath;

#ifdef Q_OS_WIN
    // Explicitly targeting the canonical user font directory: %LOCALAPPDATA%\Microsoft\Windows\Fonts
    // This path is used for both the file copy and is referenced in the HKCU Registry entry.
    // We construct this path by finding the user's home directory and appending the standard path,
    // ensuring it is always a writable, non-admin location.

    // QStandardPaths::HomeLocation resolves to C:/Users/<USER>
    QString const userHome = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    if (!userHome.isEmpty())
    {
        // Append the standard relative path to the user's local AppData font folder.
        // This is the correct location for per-user font installation on modern and older Windows.
        installPath = userHome + "/AppData/Local/Microsoft/Windows/Fonts";
    }
    else
    {
        // Fallback, though this may lead to C:\Windows\Fonts if no user home is found.
        installPath = QStandardPaths::writableLocation(QStandardPaths::FontsLocation);
    }
#else
    // For Linux and macOS, QStandardPaths::FontsLocation is reliable for user-specific fonts.
    // On Linux, this is typically ~/.local/share/fonts
    // On macOS, this is typically ~/Library/Fonts
    installPath = QStandardPaths::writableLocation(QStandardPaths::FontsLocation);
#endif

    // Convert to native separators (e.g., C:\Users\...) for the final path on Windows
    return QDir::toNativeSeparators(installPath);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Executes the font cache rebuilding command on Linux.
 * * This is crucial for Fontconfig to recognize the new font immediately.
 * On Windows and macOS, the OS usually handles this automatically upon file copy/registration.
 * @return True if successful, false otherwise.
 */
#ifdef Q_OS_LINUX
auto RebuildFontCache() -> bool
{
    qInfo() << "Attempting to rebuild font cache using 'fc-cache -f -v'...";

    QProcess process;
    // -f forces a rebuild, -v provides verbose output (for debugging)
    process.start("fc-cache", QStringList() << "-f" << "-v");

    // Wait for the process to finish, timeout after 5 seconds
    if (!process.waitForFinished(5000))
    {
        qCritical() << "Font cache rebuild process timed out.";
        process.close();
        return false;
    }

    if (process.exitCode() != 0)
    {
        qCritical() << "Font cache rebuild failed. Error:" << process.readAllStandardError();
        return false;
    }
    qInfo() << "Font cache successfully rebuilt.";
    return true;
}
#endif // Q_OS_LINUX
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VFontInstaller::VFontInstaller(QObject *parent)
  : QObject(parent)
{
    // Ensure the Qt application name and organization are set for QStandardPaths to work reliably.
    // If you haven't set these globally in your main() function, it's highly recommended.
    if (QCoreApplication::applicationName().isEmpty())
    {
        qWarning() << "Warning: QCoreApplication::setApplicationName() should be called before using QStandardPaths "
                      "for best results.";
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VFontInstaller::ErrorMessage() const -> QString
{
    return m_errorMessage;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Installs a single font file to the user's font directory.
 * @param sourceFilePath The absolute path to the font file.
 * @param window Parent window
 * @return An InstallError code indicating success or failure reason.
 */
auto VFontInstaller::InstallFont(const QString &sourceFilePath, QWidget *window) -> InstallError
{
    m_errorMessage.clear();

    QFileInfo const sourceInfo(sourceFilePath);

    // 1. Validate source file
    if (!sourceInfo.exists() || !sourceInfo.isFile())
    {
        m_errorMessage = QStringLiteral("Source file not found or is not a file: %1").arg(sourceFilePath);
        return InstallError::SourceFileNotFound;
    }

    if (sourceInfo.suffix() == "svg"_L1)
    {
        return InstallSVGFont(sourceFilePath, window);
    }

    return InstallOutLineFont(sourceFilePath, window);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Installs a single font file to the user's system outline font directory.
 * @param sourceFilePath The absolute path to the outline font file.
 * @param window Parent window
 * @return An InstallError code indicating success or failure reason.
 */
auto VFontInstaller::InstallOutLineFont(const QString &sourceFilePath, QWidget *window) -> InstallError
{
    // 1. & 2. Validate path and get destination
    QString destinationFilePath;
    if (InstallError const pathError = ValidateInstallPath(GetOutlineFontInstallPath(),
                                                           sourceFilePath,
                                                           destinationFilePath);
        pathError != InstallError::NoError)
    {
        return pathError;
    }

    // 3. & 4. Handle copy and overwrite logic
    if (InstallError const copyError = HandleFileOverwriteAndCopy(sourceFilePath, destinationFilePath, window);
        copyError != InstallError::NoError)
    {
        return copyError; // Returns CopyFailed, UserCancelled, etc.
    }

    // 5. Perform platform-specific post-copy and registration actions
#ifdef Q_OS_LINUX
    if (!RebuildFontCache())
    {
        m_errorMessage = tr(
            "Font copy succeeded, but Linux font cache rebuild failed. Font may not be immediately visible.");
        return InstallError::FontCacheRebuildFailed;
    }
#elif defined(Q_OS_WIN)
    // For all Windows versions, register the font in HKCU and broadcast the change.
    if (!RegisterFontInRegistry(QFileInfo(sourceFilePath).fileName(), destinationFilePath))
    {
        m_errorMessage = tr("Font copy succeeded, but Windows Registry registration failed.");
        // Registration failure should still be returned as an error state.
        return InstallError::RegistrationFailed;
    }
#endif

    // 6. Explicitly inform Qt's running application about the new font.
    // This is the key step to avoid restarting the application.
    if (int const fontId = QFontDatabase::addApplicationFont(destinationFilePath); fontId == -1)
    {
        qWarning() << "Could not load font into Qt's application font database:" << destinationFilePath
                   << ". It might already be loaded.";

        InstallFontCorrections(QFileInfo(sourceFilePath).dir(), fontId);
    }
    else
    {
        qInfo() << "Successfully loaded font into Qt's application font database (ID:" << fontId << ")";
    }

    // For macOS, the copy to ~/Library/Fonts is usually sufficient for automatic system registration.

    m_errorMessage.clear(); // Clear error message on success
    return InstallError::NoError;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Installs a single SVG font file to the user's font directory.
 * @param sourceFilePath The absolute path to the SVG font file.
 * @param window Parent window
 * @return An InstallError code indicating success or failure reason.
 */
auto VFontInstaller::InstallSVGFont(const QString &sourceFilePath, QWidget *window) -> InstallError
{
    // 1. & 2. Validate path and get destination
    QString destinationFilePath;
    if (InstallError const pathError = ValidateInstallPath(VAbstractApplication::VApp()->Settings()->GetPathSVGFonts(),
                                                           sourceFilePath,
                                                           destinationFilePath);
        pathError != InstallError::NoError)
    {
        return pathError;
    }

    // 3. & 4. Handle copy and overwrite logic
    if (InstallError const copyError = HandleFileOverwriteAndCopy(sourceFilePath, destinationFilePath, window);
        copyError != InstallError::NoError)
    {
        return copyError; // Returns CopyFailed, UserCancelled, etc.
    }

    // SVG fonts require no post-install steps

    m_errorMessage.clear(); // Clear error message on success
    return InstallError::NoError;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Searches for and installs an optional companion JSON correction file for the font.
 * * The correction file is expected to be in the same source directory as the font file,
 * named after the font's official family name (e.g., "Inter.json").
 * * @param sourceDir The source directory containing the font and potentially the JSON file.
 * @param fontId The ID of the font as loaded by QFontDatabase.
 */
//NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void VFontInstaller::InstallFontCorrections(const QDir &sourceDir, int fontId)
{
    if (fontId == -1)
    {
        return;
    }

    QString fontFamilyName;
    QStringList families = QFontDatabase::applicationFontFamilies(fontId);
    if (!families.isEmpty())
    {
        fontFamilyName = families.first();
        qInfo() << "Searching font correction for family name:" << fontFamilyName;
    }
    else
    {
        qWarning() << tr("Font loaded, but could not determine font family name to install font corrections.");
        return;
    }

    // Search for companion JSON file (using the official family name)
    QString const jsonFileName = QStringLiteral("%1.json").arg(fontFamilyName);
    QString const jsonFilePath = sourceDir.filePath(jsonFileName);

    if (!QFileInfo::exists(jsonFilePath))
    {
        qInfo() << "No font corrections file found for font family:" << fontFamilyName;
        return;
    }

    qInfo() << "Font corrections file found at:" << jsonFilePath;

    QString const installPath = VAbstractApplication::VApp()->Settings()->GetPathFontCorrections();
    QString const destinationFilePath = QDir(installPath).filePath(jsonFileName);

    // QFile::copy fails if the destination already exists and will not overwrite by default.
    if (!QFile::copy(jsonFilePath, destinationFilePath))
    {
        // If copy fails, check if the files are the same (already installed)
        if (QFile::exists(destinationFilePath))
        {
            // Font correction file already exists.
            qWarning() << tr("Font correction file already exists at destination: %1.").arg(destinationFilePath);
        }
        else
        {
            qCritical()
                << tr("Failed to copy font correction file from %1 to %2. Possible reasons: permission denied, file "
                      "locked, or general I/O error.")
                       .arg(jsonFilePath, destinationFilePath);
            return;
        }
    }
    else
    {
        qInfo() << "Successfully copied font correction file to:" << destinationFilePath;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the file copy, including the "overwrite" dialog logic.
 * @param sourceFilePath The full path to the source file.
 * @param destinationFilePath The full path to the destination.
 * @param window The parent widget for the dialog.
 * @return InstallError::NoError on success, or an error code on failure.
 */
auto VFontInstaller::HandleFileOverwriteAndCopy(const QString &sourceFilePath,
                                                const QString &destinationFilePath,
                                                QWidget *window) -> VFontInstaller::InstallError
{
    // 3. Check for existing file and ask user to replace
    if (QFile::exists(destinationFilePath))
    {
        bool doOverwrite = (m_overwriteMode == OverwriteMode::YesToAll);

        if (m_overwriteMode == OverwriteMode::NoToAll)
        {
            // Instantly skip if "No to All" was previously selected
            m_errorMessage = tr("Skipped existing font (User choice: No to All): %1")
                                 .arg(QFileInfo(sourceFilePath).fileName());
            return InstallError::UserCancelled;
        }

        // If we don't have an "All" answer yet, ask the user
        if (m_overwriteMode == OverwriteMode::Ask)
        {
            QMessageBox msgBox(window);
            msgBox.setWindowTitle(tr("Font Already Exists"));
            msgBox.setText(tr("The font '%1' is already installed.").arg(QFileInfo(sourceFilePath).fileName()));
            msgBox.setInformativeText(tr("Do you want to replace it?"));
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll | QMessageBox::NoToAll);
            msgBox.setDefaultButton(QMessageBox::No);

            int const reply = msgBox.exec();

            switch (reply)
            {
                case QMessageBox::YesToAll:
                    m_overwriteMode = OverwriteMode::YesToAll;
                    doOverwrite = true;
                    break;
                case QMessageBox::Yes:
                    doOverwrite = true;
                    break;
                case QMessageBox::NoToAll:
                    m_overwriteMode = OverwriteMode::NoToAll;
                    doOverwrite = false;
                    break;
                case QMessageBox::No:
                default:
                    doOverwrite = false;
                    break;
            }
        }

        // If the user decided NOT to overwrite, we must return now.
        if (!doOverwrite)
        {
            m_errorMessage = tr("Installation cancelled by user. Font not replaced.");
            return InstallError::UserCancelled;
        }

        // --- User wants to overwrite ---
        if (!QFile::remove(destinationFilePath))
        {
            m_errorMessage = tr("Failed to remove existing file: %1. Check permissions.").arg(destinationFilePath);
            return InstallError::CopyFailed;
        }
        qInfo() << "Existing font file removed, proceeding with overwrite.";
    }

    // 4. Copy the new file
    if (!QFile::copy(sourceFilePath, destinationFilePath))
    {
        m_errorMessage = tr("Failed to copy file from %1 to %2. Possible reasons: permission denied, file "
                            "locked, or general I/O error.")
                             .arg(sourceFilePath, destinationFilePath);
        return InstallError::CopyFailed;
    }

    qInfo() << "Successfully copied font file to:" << destinationFilePath;
    return InstallError::NoError;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Validates an installation path, creates it, and calculates the final destination file path.
 * @param installPath The base directory to install to (e.g., "/usr/share/fonts").
 * @param sourceFilePath The file to be installed (e.g., "/tmp/myfont.ttf").
 * @param outDestinationFilePath Output parameter for the calculated full destination path.
 * @return InstallError::NoError on success, or an error code on failure.
 */
auto VFontInstaller::ValidateInstallPath(const QString &installPath,
                                         const QString &sourceFilePath,
                                         QString &outDestinationFilePath) -> VFontInstaller::InstallError
{
    // 1. Validate installation path
    if (installPath.isEmpty())
    {
        m_errorMessage = tr("Could not determine the system font installation path.");
        return InstallError::DestinationPathNotFound;
    }

    if (QDir const dir; !dir.mkpath(installPath))
    {
        m_errorMessage = tr("Failed to create destination directory: %1").arg(installPath);
        return InstallError::DestinationPathNotFound;
    }

    // 2. Define destination path
    QFileInfo const sourceInfo(sourceFilePath);
    outDestinationFilePath = QDir(installPath).filePath(sourceInfo.fileName());
    return InstallError::NoError;
}
