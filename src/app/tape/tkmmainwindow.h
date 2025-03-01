/************************************************************************
 **
 **  @file   tkmmainwindow.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   31 10, 2023
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
#ifndef TKMMAINWINDOW_H
#define TKMMAINWINDOW_H

#include "../vformat/knownmeasurements/vknownmeasurements.h"
#include "../vmisc/vabstractshortcutmanager.h"
#include "../vmisc/vlockguard.h"
#include "../vmisc/vtablesearch.h"
#include "../vwidgets/vabstractmainwindow.h"

#include <QPointer>
#include <QTemporaryFile>

namespace Ui
{
class TKMMainWindow;
}

class VKnownMeasurementsDocument;
class VPatternImage;
class QCompleter;
class QxtCsvModel;
class QListWidget;

class TKMMainWindow final : public VAbstractMainWindow
{
    Q_OBJECT // NOLINT

public:
    explicit TKMMainWindow(QWidget *parent = nullptr);
    ~TKMMainWindow() override;

    auto CurrentFile() const -> QString;

    auto LoadFile(const QString &path) -> bool;

    void UpdateWindowTitle();

    auto IsUntitled() const -> bool;

    auto GetUntitledIndex() const -> int;
    void SetUntitledIndex(int newUntitledIndex);

public slots:
    void ToolBarStyles();

private slots:
    void FileNew();
    void OpenKnownMeasurements();

    bool FileSave();   // NOLINT(modernize-use-trailing-return-type)
    bool FileSaveAs(); // NOLINT(modernize-use-trailing-return-type)
    void AboutToShowWindowMenu();
    void ShowWindow() const;
    void ImportDataFromCSV();

#if defined(Q_OS_MAC)
    void OpenAt(QAction *where);
#endif // defined(Q_OS_MAC)

    void SaveKnownMeasurementsName();
    void SaveKnownMeasurementsDescription();

    void RemoveMeasurement();
    void MoveTop();
    void MoveUp();
    void MoveDown();
    void MoveBottom();

    void AddImage();
    void RemoveImage();
    void SaveImage();
    void ShowImage();

    void AddKnown();

    void ShowMData();
    void ShowImageData();

    void DeployFormula();

    void SaveMName();
    void SaveMFormula();
    void SaveMDescription();
    void SaveMFullName();
    void SaveMUnits();
    void SaveMGroup();
    void SaveMDiagram();

    void SaveImageTitle();
    void SaveImageSizeScale();

    void AskDefaultSettings();

    void UpdateShortcuts();

    void RefreshMeasurementImagePreview(int index);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(TKMMainWindow) // NOLINT
    Ui::TKMMainWindow *ui;
    QString m_curFile{};
    bool m_isInitialized{false};
    bool m_mIsReadOnly{false};
    QMenu *m_searchHistory;
    QSharedPointer<VLockGuard<char>> m_lock{nullptr};
    QSharedPointer<VTableSearch> m_search{};
    VKnownMeasurementsDocument *m_m{nullptr};
    int m_formulaBaseHeight{0};

    QMultiHash<VShortcutAction, QAction *> m_actionShortcuts{};
    QMultiHash<VShortcutAction, QAbstractButton *> m_buttonShortcuts{};
    QHash<QAbstractButton *, QString> m_serachButtonTooltips{};

    QPointer<QTemporaryFile> m_tmpImage{};

    VKnownMeasurements m_known{};

    QCompleter *m_groupCompleter{};

    int untitledIndex{0};

    int lastSelectedTab{0};

    void SetupMenu();
    void InitWindow();
    void InitSearch();

    void MeasurementsWereSaved(bool saved);
    void SetCurrentFile(const QString &fileName);
    auto SaveKnownMeasurements(const QString &fileName, QString &error) -> bool;

    auto MaybeSave() -> bool;
    void UpdatePadlock(bool ro);
    auto AddCell(const QString &text, int row, int column, int aligment) -> QTableWidgetItem *;

    void ReadSettings();
    void WriteSettings();

    void InitIcons();

    void InitSearchHistory();
    void SaveSearchRequest();
    void UpdateSearchControlsTooltips();

    static auto UnknownMeasurementImage() -> QString;

    void CreateWindowMenu(QMenu *menu);

    void RefreshTable();
    void RefreshImages();

    void ShowMDiagram(const VPatternImage &image);

    auto Open(const QString &pathTo, const QString &filter) -> QString;
    void Controls();
    void MFields(bool enabled);
    void ImageFields(bool enabled);

    auto GenerateMeasurementName() const -> QString;

    void InitMeasurementUnits();
    void InitMeasurementDiagramList();

    void ShowError(const QString &text);
    void RefreshDataAfterImport();
    auto CheckMName(const QString &name, const QSet<QString> &importedNames) const -> QString;

    auto CSVColumnHeader(int column) const -> QString;
    auto ReadCSV(const QxtCsvModel &csv, const QVector<int> &map, bool withHeader) -> QVector<VKnownMeasurement>;
    void ImportKnownMeasurements(const QxtCsvModel &csv, const QVector<int> &map, bool withHeader);

    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    auto eventFilter(QObject *object, QEvent *event) -> bool override;
    void ExportToCSVData(const QString &fileName, bool withHeader, int mib, const QChar &separator) override;
    auto RecentFileList() const -> QStringList override;

    void AddMeasurementImages(const QStringList &imagePaths);

    auto HandleKeyPress(QWidget *widget, QEvent *event) const -> bool;
    auto HandleListWidgetEvent(const QListWidget *listWidget, QEvent *event) -> bool;
    auto HandleDragEnterMove(QDragEnterEvent *dragEvent) -> bool;
    auto HandleDrop(QDropEvent *dropEvent) -> bool;
};

#endif // TKMMAINWINDOW_H
