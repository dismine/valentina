/************************************************************************
 **
 **  @file   vtoolbuttonpopup.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   8 7, 2023
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
#ifndef VTOOLBUTTONPOPUP_H
#define VTOOLBUTTONPOPUP_H

#include <QToolButton>

class VToolButtonPopup : public QToolButton
{
    Q_OBJECT // NOLINT

public:
    explicit VToolButtonPopup(QWidget *parent = nullptr);
    ~VToolButtonPopup() override = default;

    auto GetToolGroupTooltip() const -> QString;
    void SetToolGroupTooltip(const QString &toolGroupTooltip);

    void AssignMenu(QMenu *menu);

protected:
    auto event(QEvent *event) -> bool override;

private:
    Q_DISABLE_COPY_MOVE(VToolButtonPopup) // NOLINT

    QString m_toolGroupTooltip{};
    bool handlingToolTipChange{false};

    void CorrectToolTip();
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VToolButtonPopup::GetToolGroupTooltip() const -> QString
{
    return m_toolGroupTooltip;
}

#endif // VTOOLBUTTONPOPUP_H
