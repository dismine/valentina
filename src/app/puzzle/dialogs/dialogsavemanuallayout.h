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

#include "../vlayout/dialogs/vabstractlayoutdialog.h"
#include "../vlayout/vlayoutdef.h"

namespace Ui
{
class DialogSaveManualLayout;
}

class DialogSaveManualLayout : public VAbstractLayoutDialog
{
    Q_OBJECT

public:
    explicit DialogSaveManualLayout(int count, bool consoleExport, const QString &fileName = QString(),
                                    QWidget *parent = nullptr);
    virtual ~DialogSaveManualLayout();

    auto Path() const -> QString;
    auto FileName() const -> QString;

    auto Format() const -> LayoutExportFormats;
    void SelectFormat(LayoutExportFormats format);

    void SetBinaryDXFFormat(bool binary);
    bool IsBinaryDXFFormat() const;

    void SetDestinationPath(const QString& cmdDestinationPath);

    bool IsTextAsPaths() const;
    void SetTextAsPaths(bool textAsPaths);

    void  SetXScale(qreal scale);
    qreal GetXScale() const;

    void  SetYScale(qreal scale);
    qreal GetYScale() const;

    void SetExportUnified(bool value);
    bool IsExportUnified() const;

protected:
    virtual void showEvent(QShowEvent *event) override;

private slots:
    void Save();
    void PathChanged(const QString &text);
    void ShowExample();
    void ToggleScaleConnection();
    void HorizontalScaleChanged(double d);
    void VerticalScaleChanged(double d);

private:
    Q_DISABLE_COPY(DialogSaveManualLayout)
    Ui::DialogSaveManualLayout *ui;
    int  m_count;
    bool m_isInitialized{false};
    bool m_scaleConnected{true};
    bool m_consoleExport;

    static bool havePdf;
    static bool tested;
    static bool SupportPSTest();
    static QVector<std::pair<QString, LayoutExportFormats> > InitFormats();

    void RemoveFormatFromList(LayoutExportFormats format);

    void ReadSettings();
    void WriteSettings() const;
};

#endif // DIALOGSAVEMANUALLAYOUT_H
