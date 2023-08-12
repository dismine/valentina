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

#ifndef VABSTRACTMAINWINDOW_H
#define VABSTRACTMAINWINDOW_H

#include <QMainWindow>
#include <QMetaObject>
#include <QObject>
#include <QString>

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../vmisc/defglobal.h"
#endif

struct VFinalMeasurement;

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")
QT_WARNING_DISABLE_GCC("-Wsuggest-final-methods")

class VAbstractMainWindow : public QMainWindow
{
    Q_OBJECT // NOLINT

public:
    explicit VAbstractMainWindow(QWidget *parent = nullptr);
    ~VAbstractMainWindow() override = default;

public slots:
    virtual void ShowToolTip(const QString &toolTip);
    virtual void UpdateVisibilityGroups();
    virtual void UpdateDetailsList();
    virtual void ZoomFitBestCurrent();

protected slots:
    void WindowsLocale();
    void ExportDataToCSV();

protected:
    unsigned m_curFileFormatVersion;
    QString m_curFileFormatVersionStr;
    enum
    {
        MaxRecentFiles = 5
    };
    QVector<QAction *> m_recentFileActs{QVector<QAction *>(MaxRecentFiles)};
    QAction *m_separatorAct{nullptr};

    auto ContinueFormatRewrite(const QString &currentFormatVersion, const QString &maxFormatVersion) -> bool;
    virtual void ToolBarStyle(QToolBar *bar) const;

    auto CSVFilePath() -> QString;

    virtual void ExportToCSVData(const QString &fileName, bool withHeader, int mib, const QChar &separator);

    virtual auto RecentFileList() const -> QStringList = 0;
    void UpdateRecentFileActions();

    static auto CheckFilePermissions(const QString &path, QWidget *messageBoxParent = nullptr) -> bool;

    auto IgnoreLocking(int error, const QString &path, bool guiMode) -> bool;

private:
    Q_DISABLE_COPY_MOVE(VAbstractMainWindow) // NOLINT
};

QT_WARNING_POP

#endif // VABSTRACTMAINWINDOW_H
