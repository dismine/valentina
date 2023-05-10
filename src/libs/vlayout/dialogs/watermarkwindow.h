/************************************************************************
 **
 **  @file   watermarkwindow.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   24 12, 2019
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2019 Valentina project
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
#ifndef WATERMARKWINDOW_H
#define WATERMARKWINDOW_H

#include <QMainWindow>

#include "../ifc/ifcdef.h"
#include "../vmisc/vlockguard.h"

namespace Ui
{
class WatermarkWindow;
}

class WatermarkWindow : public QMainWindow
{
    Q_OBJECT // NOLINT

public:
    explicit WatermarkWindow(const QString &patternPath, QWidget *parent = nullptr);
    ~WatermarkWindow() override;

    auto CurrentFile() const -> QString;

    auto Open(QString path) -> bool;

signals:
    void New();
    void OpenAnother(const QString &fileName);

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void on_actionNew_triggered();
    bool on_actionSaveAs_triggered(); // NOLINT(modernize-use-trailing-return-type)
    bool on_actionSave_triggered();   // NOLINT(modernize-use-trailing-return-type)
    void on_actionOpen_triggered();
    void on_actionExit_triggered();

    void WatermarkChangesWereSaved(bool saved);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(WatermarkWindow) // NOLINT
    Ui::WatermarkWindow *ui;
    QString m_patternPath;
    QString m_curFile{};
    VWatermarkData m_data{};
    QColor m_okPathColor{};
    bool m_isInitialized{false};

    unsigned m_curFileFormatVersion{0x0};
    QString m_curFileFormatVersionStr{QLatin1String("0.0.0")};

    QSharedPointer<VLockGuard<char>> lock{};

    auto MaybeSave() -> bool;
    void UpdateWindowTitle();

    auto GetWatermarkFileName() -> QString;

    auto ContinueFormatRewrite(const QString &currentFormatVersion, const QString &maxFormatVersion) -> bool;
    auto SaveWatermark(const QString &fileName, QString &error) -> bool;
    void SetCurrentFile(const QString &fileName);
    auto OpenNewEditor(const QString &fileName = QString()) -> bool;
    void Clear();
    auto IgnoreLocking(int error, const QString &path) -> bool;
    void ShowWatermark();
    void ValidatePath();
    void ToolBarStyle(QToolBar *bar);
};

#endif // WATERMARKWINDOW_H
