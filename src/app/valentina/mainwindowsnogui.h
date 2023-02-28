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

#include "../vlayout/vlayoutpiece.h"
#include "xml/vpattern.h"
#include "dialogs/dialogsavelayout.h"
#include "../vlayout/vlayoutgenerator.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "../vlayout/vtextmanager.h"
#include "../vlayout/vprintlayout.h"

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

struct DetailForLayout {
    DetailForLayout() = default;

    DetailForLayout(quint32 id, const VPiece &piece)
        : id(id), piece(piece)
    {}

    quint32 id{NULL_ID};
    VPiece piece;
};

QT_WARNING_POP

enum class PreviewQuatilty : bool {Fast = true, Slow = false};

class MainWindowsNoGUI : public VAbstractMainWindow
{
    Q_OBJECT // NOLINT
public:
    explicit MainWindowsNoGUI(QWidget *parent = nullptr);
    virtual ~MainWindowsNoGUI() override;

public slots:
    void ToolLayoutSettings(bool checked);
    void PrintPreviewOrigin();
    void PrintPreviewTiled();
    void PrintOrigin();
    void PrintTiled();
protected slots:
    void ExportFMeasurementsToCSV();
protected:
    QVector<VLayoutPiece> listDetails{};

    /** @brief currentScene pointer to current scene. */
    QGraphicsScene *currentScene{nullptr};

    QGraphicsScene *tempSceneLayout{nullptr};

    /** @brief pattern container with data (points, arcs, splines, spline paths, variables) */
    VContainer         *pattern;

    /** @brief doc dom document container */
    VPattern           *doc{nullptr};

    QList<QGraphicsItem *> gcontours{};

    QVector<QVector<VLayoutPiece> > detailsOnLayout{};

    QAction *undoAction{nullptr};
    QAction *redoAction{nullptr};
    QAction *actionDockWidgetToolOptions{nullptr};
    QAction *actionDockWidgetGroups{nullptr};
    QAction *actionDockWidgetBackgroundImages{nullptr};

    bool isNoScaling{false};
    bool isNeedAutosave{false};
    VPrintLayout *m_layoutSettings{new VPrintLayout(this)};

    /** @brief mouseCoordinate pointer to label who show mouse coordinate. */
    QPointer<QLabel> m_mouseCoordinate{nullptr};
    QPointer<QLabel> m_unreadPatternMessage{nullptr};

    QSharedPointer<DialogSaveLayout> m_dialogSaveLayout{};

#if defined(Q_OS_WIN) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    QWinTaskbarButton *m_taskbarButton;
    QWinTaskbarProgress *m_taskbarProgress{nullptr};
#endif

    static QVector<VLayoutPiece> PrepareDetailsForLayout(const QVector<DetailForLayout> &details);

    void ExportData(const QVector<VLayoutPiece> &listDetails);

    void InitTempLayoutScene();
    virtual void CleanLayout()=0;
    virtual void PrepareSceneList(PreviewQuatilty quality)=0;
    virtual QStringList RecentFileList() const override;
    QIcon ScenePreview(int i, QSize iconSize, PreviewQuatilty quality) const;
    bool GenerateLayout(VLayoutGenerator& lGenerator);
    QString FileName() const;

    bool ExportFMeasurementsToCSVData(const QString &fileName,
                                      bool withHeader, int mib, const QChar &separator) const;

    QSharedPointer<VMeasurements> OpenMeasurementFile(const QString &path) const;

    void CheckRequiredMeasurements(const VMeasurements *m) const;

private:
    Q_DISABLE_COPY_MOVE(MainWindowsNoGUI) // NOLINT

    static QList<QGraphicsItem *> CreateShadows(const QList<QGraphicsItem *> &papers);
    static QList<QGraphicsScene *> CreateScenes(const QList<QGraphicsItem *> &papers,
                                                const QList<QGraphicsItem *> &shadows,
                                                const QList<QList<QGraphicsItem *> > &details);

    void PdfTiledFile(const QString &name);

    void ExportScene(const QList<QGraphicsScene *> &scenes,
                     const QList<QGraphicsItem *> &papers,
                     const QList<QGraphicsItem *> &shadows,
                     const QList<QList<QGraphicsItem *> > &details,
                     bool ignorePrinterFields, const QMarginsF &margins) const;

    void ExportApparelLayout(const QVector<VLayoutPiece> &details, const QString &name, const QSize &size) const;

    void ExportDetailsAsApparelLayout(QVector<VLayoutPiece> listDetails);

    void ExportFlatLayout(const QList<QGraphicsScene *> &scenes,
                          const QList<QGraphicsItem *> &papers,
                          const QList<QGraphicsItem *> &shadows,
                          const QList<QList<QGraphicsItem *> > &details,
                          bool ignorePrinterFields, const QMarginsF &margins);

    void ExportDetailsAsFlatLayout(const QVector<VLayoutPiece> &listDetails);

    void ShowLayoutError(const LayoutErrors &state);
};

#endif // MAINWINDOWSNOGUI_H
