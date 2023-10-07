/************************************************************************
 **
 **  @file
 **  @author Valentina Zhuravska <zhuravska19(at)gmail.com>
 **  @date   19 7, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#include "vabstractmainwindow.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/def.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vsysexits.h"
#include "../vpropertyexplorer/checkablemessagebox.h"
#include "dialogs/dialogexporttocsv.h"

#include <QAction>
#include <QFileDialog>
#include <QLockFile>
#include <QMessageBox>
#include <QStyle>
#include <QToolBar>

// #ifdef Q_OS_WIN
// extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
// #endif /*Q_OS_WIN*/

#if defined(Q_OS_MAC)
#include <QStyleFactory>
#endif

#include <QLoggingCategory>

using namespace Qt::Literals::StringLiterals;

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(abstactMainWindow, "abs.MainWindow") // NOLINT

QT_WARNING_POP

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto SelectNumber(QStringList path, int number) -> QStringList
{
    path = Reverse(path);
    QStringList subPath = path.mid(0, number);
    return Reverse(subPath);
}

//---------------------------------------------------------------------------------------------------------------------
auto RecentFiles(const QStringList &paths) -> QStringList
{
    QVector<QStringList> table;
    table.reserve(paths.size());

    for (const auto &path : paths)
    {
        table.append(SplitFilePaths(path));
    }

    auto CreateOptimized = [table](int tableRow)
    {
        QStringList optimized;
        const QStringList &path = table.at(tableRow);
        for (int count = 1; count <= path.size(); ++count)
        {
            bool isUnique = true;
            optimized = SelectNumber(path, count);

            for (int row = 0; row < table.size(); ++row)
            {
                if (row == tableRow)
                {
                    continue;
                }

                QStringList testSubPath = SelectNumber(table.at(row), count);
                if (optimized.join(QDir::separator()) == testSubPath.join(QDir::separator()))
                {
                    isUnique = false;
                    break;
                }
            }

            if (isUnique)
            {
                break;
            }
        }

        if (optimized.size() >= 3)
        {
            optimized = QStringList({optimized.constFirst(), QStringLiteral("…"), optimized.constLast()});
        }

        return optimized;
    };

    QVector<QStringList> optimizedPaths;
    optimizedPaths.reserve(paths.size());

    for (int row = 0; row < table.size(); ++row)
    {
        optimizedPaths.append(CreateOptimized(row));
    }

    QStringList recentFiles;
    recentFiles.reserve(paths.size());

    for (auto &path : optimizedPaths)
    {
        recentFiles.append(path.join(QDir::separator()));
    }

    return recentFiles;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VAbstractMainWindow::VAbstractMainWindow(QWidget *parent)
  : QMainWindow(parent),
    m_curFileFormatVersion(0x0),
    m_curFileFormatVersionStr("0.0.0"_L1)
{
    for (int i = 0; i < MaxRecentFiles; ++i)
    {
        m_recentFileActs[i] = nullptr;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractMainWindow::ShowToolTip(const QString &toolTip)
{
    Q_UNUSED(toolTip)
    // do nothing
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractMainWindow::ContinueFormatRewrite(const QString &currentFormatVersion, const QString &maxFormatVersion)
    -> bool
{
    if (VAbstractApplication::VApp()->Settings()->GetConfirmFormatRewriting())
    {
        Utils::CheckableMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Confirm format rewriting"));
        msgBox.setText(tr("This file is using previous format version v%1. The current is v%2. "
                          "Saving the file with this app version will update the format version for this "
                          "file. This may prevent you from be able to open the file with older app versions. "
                          "Do you really want to continue?")
                           .arg(currentFormatVersion, maxFormatVersion));
        msgBox.setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No);
        msgBox.setDefaultButton(QDialogButtonBox::No);
        msgBox.setIconPixmap(QApplication::style()->standardIcon(QStyle::SP_MessageBoxQuestion).pixmap(32, 32));

        int dialogResult = msgBox.exec();

        if (dialogResult == QDialog::Accepted)
        {
            VAbstractApplication::VApp()->Settings()->SetConfirmFormatRewriting(not msgBox.isChecked());
            return true;
        }
        else
        {
            return false;
        }
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractMainWindow::ToolBarStyle(QToolBar *bar) const
{
    SCASSERT(bar != nullptr)
    VAbstractApplication::VApp()->Settings()->GetToolBarStyle() ? bar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon)
                                                                : bar->setToolButtonStyle(Qt::ToolButtonIconOnly);
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractMainWindow::CSVFilePath() -> QString
{
    const QString filters = tr("Comma-Separated Values") + " (*.csv)"_L1;
    const QString suffix("csv"_L1);
    const QString path = QDir::homePath() + '/'_L1 + tr("values") + '.'_L1 + suffix;

    QString fileName = QFileDialog::getSaveFileName(this, tr("Export to CSV"), path, filters, nullptr,
                                                    VAbstractApplication::VApp()->NativeFileDialog());

    if (fileName.isEmpty())
    {
        return fileName;
    }

    QFileInfo f(fileName);
    if (f.suffix().isEmpty() && f.suffix() != suffix)
    {
        fileName += '.'_L1 + suffix;
    }

    return fileName;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractMainWindow::ExportToCSVData(const QString &fileName, bool withHeader, int mib, const QChar &separator)
{
    Q_UNUSED(fileName)
    Q_UNUSED(withHeader)
    Q_UNUSED(mib)
    Q_UNUSED(separator)
    // do nothing
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractMainWindow::UpdateRecentFileActions()
{
    const QStringList files = RecentFileList();
    const QStringList recentFiles = RecentFiles(files);
    const int numRecentFiles = qMin(static_cast<int>(files.size()), static_cast<int>(MaxRecentFiles));

    for (int i = 0; i < numRecentFiles; ++i)
    {
        QString recent = recentFiles.at(i);
        if (not recent.isEmpty())
        {
            const QString text = QStringLiteral("&%1. %2").arg(i + 1).arg(recentFiles.at(i));
            m_recentFileActs.at(i)->setText(text);
            m_recentFileActs.at(i)->setData(files.at(i));
            m_recentFileActs.at(i)->setVisible(true);
        }
    }

    m_separatorAct->setVisible(numRecentFiles > 0);
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractMainWindow::CheckFilePermissions(const QString &path, QWidget *messageBoxParent) -> bool
{
    QFileInfo info(path);

    // #ifdef Q_OS_WIN32
    //         qt_ntfs_permission_lookup++; // turn checking on
    // #endif /*Q_OS_WIN32*/

    if (not info.exists())
    {
        return true;
    }

    const bool isFileWritable = info.isWritable();

    // #ifdef Q_OS_WIN32
    //         qt_ntfs_permission_lookup--; // turn it off again
    // #endif /*Q_OS_WIN32*/

    if (not isFileWritable)
    {
        QMessageBox messageBox(messageBoxParent);
        messageBox.setIcon(QMessageBox::Question);
        messageBox.setText(tr("The file has no write permissions."));
        messageBox.setInformativeText(tr("Do you want to change the permissions?"));
        messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        messageBox.setDefaultButton(QMessageBox::Yes);

        if (messageBox.exec() == QMessageBox::Yes)
        {
            // #ifdef Q_OS_WIN32
            //             qt_ntfs_permission_lookup++; // turn checking on
            // #endif /*Q_OS_WIN32*/
            bool changed = QFile::setPermissions(path, QFileInfo(path).permissions() | QFileDevice::WriteUser);
            // #ifdef Q_OS_WIN32
            //             qt_ntfs_permission_lookup--; // turn it off again
            // #endif /*Q_OS_WIN32*/

            if (not changed)
            {
                messageBox.setIcon(QMessageBox::Warning);
                messageBox.setText(tr("Cannot set permissions for %1 to writable.").arg(path));
                messageBox.setInformativeText(tr("Could not save the file."));
                messageBox.setStandardButtons(QMessageBox::Ok);
                messageBox.setDefaultButton(QMessageBox::Ok);
                messageBox.exec();
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractMainWindow::IgnoreLocking(int error, const QString &path, bool guiMode) -> bool
{
    QMessageBox::StandardButton answer = QMessageBox::Abort;
    if (guiMode)
    {
        switch (error)
        {
            case QLockFile::LockFailedError:
                answer = QMessageBox::warning(this, tr("Locking file"),
                                              tr("This file already opened in another window. Ignore if you want "
                                                 "to continue (not recommended, can cause a data corruption)."),
                                              QMessageBox::Abort | QMessageBox::Ignore, QMessageBox::Abort);
                break;
            case QLockFile::PermissionError:
                answer = QMessageBox::question(this, tr("Locking file"),
                                               tr("The lock file could not be created, for lack of permissions. "
                                                  "Ignore if you want to continue (not recommended, can cause "
                                                  "a data corruption)."),
                                               QMessageBox::Abort | QMessageBox::Ignore, QMessageBox::Abort);
                break;
            case QLockFile::UnknownError:
                answer = QMessageBox::question(this, tr("Locking file"),
                                               tr("Unknown error happened, for instance a full partition "
                                                  "prevented writing out the lock file. Ignore if you want to "
                                                  "continue (not recommended, can cause a data corruption)."),
                                               QMessageBox::Abort | QMessageBox::Ignore, QMessageBox::Abort);
                break;
            default:
                answer = QMessageBox::Abort;
                break;
        }
    }

    if (answer == QMessageBox::Abort)
    {
        qCDebug(abstactMainWindow, "Failed to lock %s", qUtf8Printable(path));
        qCDebug(abstactMainWindow, "Error type: %d", error);
        if (not guiMode)
        {
            switch (error)
            {
                case QLockFile::LockFailedError:
                    qCCritical(abstactMainWindow, "%s",
                               qUtf8Printable(tr("This file already opened in another window.")));
                    break;
                case QLockFile::PermissionError:
                    qCCritical(abstactMainWindow, "%s",
                               qUtf8Printable(tr("The lock file could not be created, for lack of permissions.")));
                    break;
                case QLockFile::UnknownError:
                    qCCritical(abstactMainWindow, "%s",
                               qUtf8Printable(tr("Unknown error happened, for instance a full partition "
                                                 "prevented writing out the lock file.")));
                    break;
                default:
                    break;
            }

            QCoreApplication::exit(V_EX_NOINPUT);
        }
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractMainWindow::WindowsLocale()
{
    VAbstractApplication::VApp()->Settings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractMainWindow::ExportDataToCSV()
{
    QString fileName = CSVFilePath();

    if (fileName.isEmpty())
    {
        return;
    }

    DialogExportToCSV dialog(this);
    dialog.SetWithHeader(VAbstractApplication::VApp()->Settings()->GetCSVWithHeader());
    dialog.SetSelectedMib(VAbstractApplication::VApp()->Settings()->GetCSVCodec());
    dialog.SetSeparator(VAbstractApplication::VApp()->Settings()->GetCSVSeparator());

    if (dialog.exec() == QDialog::Accepted)
    {
        ExportToCSVData(fileName, dialog.IsWithHeader(), dialog.GetSelectedMib(), dialog.GetSeparator());

        VAbstractApplication::VApp()->Settings()->SetCSVSeparator(dialog.GetSeparator());
        VAbstractApplication::VApp()->Settings()->SetCSVCodec(dialog.GetSelectedMib());
        VAbstractApplication::VApp()->Settings()->SetCSVWithHeader(dialog.IsWithHeader());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractMainWindow::UpdateVisibilityGroups()
{
    // do nothing
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractMainWindow::UpdateDetailsList()
{
    // do nothing
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractMainWindow::ZoomFitBestCurrent()
{
    // do nothing
}
