/************************************************************************
 **
 **  @file   dialoglayoutsettings.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   13 1, 2015
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

#ifndef DIALOGLAYOUTSETTINGS_H
#define DIALOGLAYOUTSETTINGS_H

#include <QCloseEvent>
#include <QMargins>

#include "../vlayout/dialogs/vabstractlayoutdialog.h"
#include "../vlayout/vlayoutdef.h"

namespace Ui
{
class DialogLayoutSettings;
}

class VLayoutGenerator;

class DialogLayoutSettings : public VAbstractLayoutDialog
{
    Q_OBJECT // NOLINT

public:
    explicit DialogLayoutSettings(VLayoutGenerator *generator, QWidget *parent = nullptr, bool disableSettings = false);
    ~DialogLayoutSettings() override;

    auto GetPaperHeight() const -> qreal;
    void SetPaperHeight(qreal value);

    auto GetPaperWidth() const -> qreal;
    void SetPaperWidth(qreal value);

    auto GetNestingTime() const -> int;
    void SetNestingTime(int value);

    auto GetEfficiencyCoefficient() const -> qreal;
    void SetEfficiencyCoefficient(qreal ration);

    auto GetLayoutWidth() const -> qreal;
    void SetLayoutWidth(qreal value);

    auto GetFields() const -> QMarginsF;
    void SetFields(const QMarginsF &value);

    auto GetGroup() const -> Cases;
    void SetGroup(const Cases &value);

    auto GetFollowGrainline() const -> bool;
    void SetFollowGrainline(bool state);

    auto GetManualPriority() const -> bool;
    void SetManualPriority(bool state);

    auto GetAutoCropLength() const -> bool;
    void SetAutoCropLength(bool autoCropLength);

    auto GetAutoCropWidth() const -> bool;
    void SetAutoCropWidth(bool autoCropWidth);

    auto IsSaveLength() const -> bool;
    void SetSaveLength(bool save);

    auto IsPreferOneSheetSolution() const -> bool;
    void SetPreferOneSheetSolution(bool prefer);

    auto IsUnitePages() const -> bool;
    void SetUnitePages(bool save);

    auto IsStripOptimization() const -> bool;
    void SetStripOptimization(bool save);

    auto GetMultiplier() const -> quint8;
    void SetMultiplier(const quint8 &value);

    auto IsIgnoreAllFields() const -> bool;
    void SetIgnoreAllFields(bool value);

    auto IsTextAsPaths() const -> bool;
    void SetTextAsPaths(bool value);

    auto IsNestQuantity() const -> bool;
    void SetNestQuantity(bool state);

    void SetBoundaryTogetherWithNotches(bool value);
    auto IsBoundaryTogetherWithNotches() const -> bool;

    void SetShowLayoutAllowance(bool value);
    auto IsShowLayoutAllowance() const -> bool;

    auto SelectedPrinter() const -> QString;

    void EnableLandscapeOrientation();

    // support functions for the command line parser which uses invisible dialog to properly build layout generator
    auto SelectTemplate(const PaperSizeTemplate &id) -> bool;
    static auto MakeHelpTemplateList() -> QString;
    static auto MakeHelpTiledPdfTemplateList() -> QString;
    auto SelectPaperUnit(const QString &units) -> bool;
    auto SelectLayoutUnit(const QString &units) -> bool;
    auto LayoutToPixels(qreal value) const -> qreal;
    auto PageToPixels(qreal value) const -> qreal;
    static auto MakeGroupsHelp() -> QString;

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    auto GetTemplateSize(const PaperSizeTemplate &tmpl, const Unit &unit) const -> QSizeF;

public slots:
    void DialogAccepted();
private slots:
    void ConvertPaperSize();
    void ConvertLayoutSize();
    void TemplateSelected();
    void FindTemplate();
    void PaperSizeChanged();
    void Swap(bool checked);
    void RestoreDefaults();
    void PrinterMargins();

    void CorrectMaxFileds();
    void IgnoreAllFields(int state);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(DialogLayoutSettings) // NOLINT

    bool m_disableSettings;

    Ui::DialogLayoutSettings *ui;
    Unit m_oldPaperUnit;
    Unit m_oldLayoutUnit;
    VLayoutGenerator *m_generator;
    bool m_isInitialized;

    void InitPaperUnits();
    void InitLayoutUnits();
    void InitPrinter();
    auto Template() -> QSizeF;

    auto MinPrinterFields() const -> QMarginsF;
    auto GetDefPrinterFields() const -> QMarginsF;

    auto PaperUnit() const -> Unit;
    auto LayoutUnit() const -> Unit;

    void CorrectPaperDecimals();
    void CorrectLayoutDecimals();

    void MinimumPaperSize();
    void MinimumLayoutSize();

    void ReadSettings();
    void WriteSettings() const;

    void SheetSize(const QSizeF &size);
    void SetAdditionalOptions(bool value);

    void SetMinMargins(const QMarginsF &fields, const QMarginsF &minFields);
};

#endif // DIALOGLAYOUTSETTINGS_H
