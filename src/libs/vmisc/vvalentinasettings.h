/************************************************************************
 **
 **  @file   vvalentinasettings.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 11, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#ifndef VVALENTINASETTINGS_H
#define VVALENTINASETTINGS_H

#include <QMetaObject>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QtGlobal>
#include <QMargins>

#include "vcommonsettings.h"
#include "../vlayout/vbank.h"

class VValentinaSettings : public VCommonSettings
{
    Q_OBJECT
public:
    VValentinaSettings(Format format, Scope scope, const QString &organization, const QString &application = QString(),
              QObject *parent = nullptr);
    VValentinaSettings(const QString &fileName, Format format, QObject *parent = nullptr);

    QString GetLabelLanguage() const;
    void SetLabelLanguage(const QString &value);

    static QString GetDefPathLayout();
    QString GetPathLayout() const;
    void SetPathLayout(const QString &value);

    // Layout settings
    qreal GetLayoutPaperHeight() const;
    void SetLayoutPaperHeight(qreal value);

    qreal GetLayoutPaperWidth() const;
    void SetLayoutPaperWidth(qreal value);

    qreal GetLayoutWidth() const;
    static qreal GetDefLayoutWidth();
    void SetLayoutWidth(qreal value);

    int  GetNestingTime() const;
    static int GetDefNestingTime(){return 1;}
    void SetNestingTime(int value);

    qreal GetEfficiencyCoefficient() const;
    static qreal GetDefEfficiencyCoefficient(){return 0.0;}
    void SetEfficiencyCoefficient(qreal value);

    QMarginsF GetFields(const QMarginsF &def = QMarginsF()) const;
    void SetFields(const QMarginsF &value);

    Cases GetLayoutGroup() const;
    static Cases GetDefLayoutGroup();
    void SetLayoutGroup(const Cases &value);

    bool GetLayoutFollowGrainline() const;
    static bool GetDefLayoutFollowGrainline();
    void SetLayoutFollowGrainline(bool value);

    bool GetLayoutManualPriority() const;
    static bool GetDefLayoutManualPriority();
    void SetLayoutManualPriority(bool value);

    bool GetLayoutNestQuantity() const;
    static bool GetDefLayoutNestQuantity();
    void SetLayoutNestQuantity(bool value);

    bool GetLayoutAutoCropLength() const;
    static bool GetDefLayoutAutoCropLength();
    void SetLayoutAutoCropLength(bool value);

    bool GetLayoutAutoCropWidth() const;
    static bool GetDefLayoutAutoCropWidth();
    void SetLayoutAutoCropWidth(bool value);

    bool GetLayoutSaveLength() const;
    static bool GetDefLayoutSaveLength();
    void SetLayoutSaveLength(bool value);

    bool GetLayoutPreferOneSheetSolution() const;
    static bool GetDefLayoutPreferOneSheetSolution();
    void SetLayoutPreferOneSheetSolution(bool value);

    bool GetLayoutUnitePages() const;
    static bool GetDefLayoutUnitePages();
    void SetLayoutUnitePages(bool value);

    bool GetIgnoreAllFields() const;
    static bool GetDefIgnoreAllFields();
    void SetIgnoreAllFields(bool value);

    bool GetStripOptimization() const;
    static bool GetDefStripOptimization();
    void SetStripOptimization(bool value);

    quint8 GetMultiplier() const;
    static quint8 GetDefMultiplier();
    void SetMultiplier(quint8 value);

    bool GetTextAsPaths() const;
    static bool GetDefTextAsPaths();
    void SetTextAsPaths(bool value);

    QStringList GetKnownMaterials() const;
    void        SetKnownMaterials(const QStringList &list);

    bool IsRememberPatternMaterials() const;
    void SetRememberPatternMaterials(bool value);

    // settings for the tiled PDFs
    qreal GetTiledPDFPaperHeight(const Unit &unit) const;
    void  SetTiledPDFPaperHeight(qreal value, const Unit &unit);

    qreal GetTiledPDFPaperWidth(const Unit &unit) const;
    void  SetTiledPDFPaperWidth(qreal value, const Unit &unit);

    bool IsDockWidgetGroupsActive() const;
    static bool GetDefDockWidgetGroupsActive();
    void SetDockWidgetGroupsActive(bool value);

    bool IsDockWidgetToolOptionsActive() const;
    static bool GetDefDockWidgetToolOptionsActive();
    void SetDockWidgetToolOptionsActive(bool value);

    bool IsDockWidgetPatternMessagesActive() const;
    static bool GetDefDockWidgetPatternMessagesActive();
    void SetDockWidgetPatternMessagesActive(bool value);

    int GetPatternMessageFontSize(int fontSizeDef) const;
    static int GetDefMinPatternMessageFontSize();
    static int GetDefMaxPatternMessageFontSize();
    void SetPatternMessageFontSize(int size);

    bool GetAutoRefreshPatternMessage() const;
    void SetAutoRefreshPatternMessage(bool value);

    QSize GetWatermarkEditorSize() const;
    void SetWatermarkEditorSize(const QSize& sz);

    bool GetToolPanelScaling() const;
    void SetToolPanelScaling(const bool &value);

    auto GetLayoutExportFormat() const -> qint8;
    void SetLayoutExportFormat(qint8 format);

private:
    Q_DISABLE_COPY(VValentinaSettings)

    template <typename T>
    T GetCachedValue(T &cache, const QString &setting, T defValue, T valueMin, T valueMax) const;  
};

#endif // VVALENTINASETTINGS_H
