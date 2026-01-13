/************************************************************************
 **
 **  @file   dialoglayoutscale.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 3, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2020 Valentina project
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
#ifndef DIALOGLAYOUTSCALE_H
#define DIALOGLAYOUTSCALE_H

#include <QDialog>

namespace Ui
{
class DialogLayoutScale;
}

class DialogLayoutScale final : public QDialog
{
    Q_OBJECT // NOLINT

public:
    explicit DialogLayoutScale(bool printTiled, QWidget *parent = nullptr);
    ~DialogLayoutScale() override;

    void SetTiledMargins(QMarginsF margins);
    auto GetTiledMargins() const -> QMarginsF;

    void SetXScale(qreal scale);
    auto GetXScale() const -> qreal;

    void SetYScale(qreal scale);
    auto GetYScale() const -> qreal;

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void Save();
    void ToggleScaleConnection();
    void HorizontalScaleChanged(double d);
    void VerticalScaleChanged(double d);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(DialogLayoutScale) // NOLINT
    Ui::DialogLayoutScale *ui;
    bool isInitialized{false};
    bool m_scaleConnected{true};

    void ReadSettings();
    void WriteSettings() const;
};

#endif // DIALOGLAYOUTSCALE_H
