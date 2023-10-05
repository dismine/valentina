/************************************************************************
 **
 **  @file   mainwindowsnogui.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 5, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#ifndef MAINWINDOWSNOGUI_H
#define MAINWINDOWSNOGUI_H

#include <QLabel>
#include <QMainWindow>
#include <QPointer>
#include <QPrinter>

#include "../vlayout/vlayoutgenerator.h"
#include "../vlayout/vlayoutpiece.h"
#include "../vlayout/vprintlayout.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "dialogs/dialogsavelayout.h"
#include "xml/vpattern.h"

class QGraphicsScene;
struct PosterData;
class QGraphicsRectItem;
class VMeasurements;

#if defined(Q_OS_WIN) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
class QWinTaskbarButton;
class QWinTaskbarProgress;
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")

struct DetailForLayout
{
    DetailForLayout() = default;

    DetailForLayout(quint32 id, const VPiece &piece)
      : id(id),
        piece(piece)
    {
    }

    quint32 id{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)
    VPiece piece{};      // NOLINT(misc-non-private-member-variables-in-classes)
};

QT_WARNING_POP

enum class PreviewQuatilty : bool
{
    Fast = true,
    Slow = false
};

class MainWindowsNoGUI : public VAbstractMainWindow
{
    Q_OBJECT // NOLINT

public:
    explicit MainWindowsNoGUI(QWidget *parent = nullptr);
    ~MainWindowsNoGUI() override;

public slots:
    void ToolLayoutSettings(bool checked);
    void PrintPreviewOrigin();
    void PrintPreviewTiled();
    void PrintOrigin();
    void PrintTiled();
protected slots:
    void ExportFMeasurementsToCSV();

protected:
    QVector<VLayoutPiece> listDetails{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief currentScene pointer to current scene. */
    QGraphicsScene *currentScene{nullptr}; // NOLINT(misc-non-private-member-variables-in-classes)

    QGraphicsScene *tempSceneLayout{nullptr}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief pattern container with data (points, arcs, splines, spline paths, variables) */
    VContainer *pattern; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief doc dom document container */
    VPattern *doc{nullptr}; // NOLINT(misc-non-private-member-variables-in-classes)

    QList<QGraphicsItem *> gcontours{}; // NOLINT(misc-non-private-member-variables-in-classes)

    QVector<QVector<VLayoutPiece>> detailsOnLayout{}; // NOLINT(misc-non-private-member-variables-in-classes)

    QAction *undoAction{nullptr};                       // NOLINT(misc-non-private-member-variables-in-classes)
    QAction *redoAction{nullptr};                       // NOLINT(misc-non-private-member-variables-in-classes)
    QAction *actionDockWidgetToolOptions{nullptr};      // NOLINT(misc-non-private-member-variables-in-classes)
    QAction *actionDockWidgetGroups{nullptr};           // NOLINT(misc-non-private-member-variables-in-classes)
    QAction *actionDockWidgetBackgroundImages{nullptr}; // NOLINT(misc-non-private-member-variables-in-classes)

    bool isNoScaling{false};                                // NOLINT(misc-non-private-member-variables-in-classes)
    bool isNeedAutosave{false};                             // NOLINT(misc-non-private-member-variables-in-classes)
    VPrintLayout *m_layoutSettings{new VPrintLayout(this)}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief mouseCoordinate pointer to label who show mouse coordinate. */
    QPointer<QLabel> m_mouseCoordinate{nullptr};      // NOLINT(misc-non-private-member-variables-in-classes)
    QPointer<QLabel> m_unreadPatternMessage{nullptr}; // NOLINT(misc-non-private-member-variables-in-classes)

    QSharedPointer<DialogSaveLayout> m_dialogSaveLayout{}; // NOLINT(misc-non-private-member-variables-in-classes)

#if defined(Q_OS_WIN) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    QWinTaskbarButton *m_taskbarButton;              // NOLINT(misc-non-private-member-variables-in-classes)
    QWinTaskbarProgress *m_taskbarProgress{nullptr}; // NOLINT(misc-non-private-member-variables-in-classes)
#endif

    static auto PrepareDetailsForLayout(const QVector<DetailForLayout> &details) -> QVector<VLayoutPiece>;

    void ExportData(const QVector<VLayoutPiece> &listDetails);

    void InitTempLayoutScene();
    virtual void CleanLayout() = 0;
    virtual void PrepareSceneList(PreviewQuatilty quality) = 0;
    auto RecentFileList() const -> QStringList override;
    auto ScenePreview(int i, QSize iconSize, PreviewQuatilty quality) const -> QIcon;
    auto GenerateLayout(VLayoutGenerator &lGenerator) -> bool;
    auto FileName() const -> QString;

    auto ExportFMeasurementsToCSVData(const QString &fileName, bool withHeader, int mib, const QChar &separator) const
        -> bool;

    auto OpenMeasurementFile(const QString &path) const -> QSharedPointer<VMeasurements>;

    void CheckRequiredMeasurements(const VMeasurements *m) const;

private:
    Q_DISABLE_COPY_MOVE(MainWindowsNoGUI) // NOLINT

    static auto CreateShadows(const QList<QGraphicsItem *> &papers) -> QList<QGraphicsItem *>;
    static auto CreateScenes(const QList<QGraphicsItem *> &papers, const QList<QGraphicsItem *> &shadows,
                             const QList<QList<QGraphicsItem *>> &details) -> QList<QGraphicsScene *>;

    void PdfTiledFile(const QString &name);

    void ExportScene(const QList<QGraphicsScene *> &scenes, const QList<QGraphicsItem *> &papers,
                     const QList<QGraphicsItem *> &shadows, const QList<QList<QGraphicsItem *>> &details,
                     bool ignorePrinterFields, const QMarginsF &margins) const;

    void ExportApparelLayout(const QVector<VLayoutPiece> &details, const QString &name, const QSize &size) const;

    void ExportDetailsAsApparelLayout(QVector<VLayoutPiece> listDetails);

    void ExportFlatLayout(const QList<QGraphicsScene *> &scenes, const QList<QGraphicsItem *> &papers,
                          const QList<QGraphicsItem *> &shadows, const QList<QList<QGraphicsItem *>> &details,
                          bool ignorePrinterFields, const QMarginsF &margins);

    void ExportDetailsAsFlatLayout(const QVector<VLayoutPiece> &listDetails);

    void ShowLayoutError(const LayoutErrors &state);
};

#endif // MAINWINDOWSNOGUI_H
