/************************************************************************
 **
 **  @file   dialogsavemanuallayout.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 9, 2021
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2021 Valentina project
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
#ifndef DIALOGSAVEMANUALLAYOUT_H
#define DIALOGSAVEMANUALLAYOUT_H

#include <memory>

#include "../vdxf/dxfdef.h"
#include "../vlayout/dialogs/vabstractlayoutdialog.h"
#include "../vlayout/vlayoutdef.h"
#include "../vmisc/defglobal.h"

namespace Ui
{
class DialogSaveManualLayout;
}

class DialogSaveManualLayout : public VAbstractLayoutDialog
{
    Q_OBJECT // NOLINT

public:
    explicit DialogSaveManualLayout(vsizetype count, bool consoleExport, const QString &fileName = QString(),
                                    QWidget *parent = nullptr);
    ~DialogSaveManualLayout() override;

    auto Path() const -> QString;
    auto FileName() const -> QString;

    auto Format() const -> LayoutExportFormats;
    void SelectFormat(LayoutExportFormats format);

    void SetBinaryDXFFormat(bool binary);
    auto IsBinaryDXFFormat() const -> bool;

    auto DxfCompatibility() const -> DXFApparelCompatibility;
    void SetDxfCompatibility(DXFApparelCompatibility type);

    void SetShowGrainline(bool show);
    auto IsShowGrainline() const -> bool;

    void SetDestinationPath(const QString &cmdDestinationPath);

    auto IsTextAsPaths() const -> bool;
    void SetTextAsPaths(bool textAsPaths);

    void SetExportUnified(bool value);
    auto IsExportUnified() const -> bool;

    void SetTilesScheme(bool value);
    auto IsTilesScheme() const -> bool;

    void SetHideRuler(bool value);
    auto IsHideRuler() const -> bool;

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void Save();
    void PathChanged(const QString &text);
    void ShowExample();

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(DialogSaveManualLayout) // NOLINT
    std::unique_ptr<Ui::DialogSaveManualLayout> ui;
    vsizetype m_count;
    bool m_isInitialized{false};
    bool m_scaleConnected{true};
    bool m_consoleExport;

    void ReadSettings();
    void WriteSettings() const;

    void InitDxfCompatibility();
    void InitFileFormats();
    void InitFileFormatTypes(LayoutExportFileFormat format);
};

#endif // DIALOGSAVEMANUALLAYOUT_H
