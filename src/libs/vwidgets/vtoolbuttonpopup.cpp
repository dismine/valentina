/************************************************************************
 **
 **  @file   vtoolbuttonpopup.cpp
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
#include "vtoolbuttonpopup.h"

#include <QAction>
#include <QEvent>
#include <QStyle>

//---------------------------------------------------------------------------------------------------------------------
VToolButtonPopup::VToolButtonPopup(QWidget *parent)
  : QToolButton(parent)
{
    setPopupMode(QToolButton::MenuButtonPopup);
    QObject::connect(this, &QToolButton::triggered, this, [this](QAction *action) { setDefaultAction(action); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolButtonPopup::SetToolGroupTooltip(const QString &toolGroupTooltip)
{
    m_toolGroupTooltip = toolGroupTooltip;
    CorrectToolTip();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolButtonPopup::AssignMenu(QMenu *menu)
{
    // Attach the menu normally.
    QToolButton::setMenu(menu);

    // --- WHY WE SET THIS PROPERTY ---
    // Qt's QSS system cannot detect whether a QToolButton has a menu.
    // The `hasMenu` property is NOT a built-in Qt property.
    // We create it manually so that stylesheet selectors like
    //     QToolButton[hasMenu="true"]
    // can match and apply custom styles (for example, extra padding or width).
    //
    // Without this, rules like [hasMenu="true"] will never match.
    setProperty("hasMenu", menu != nullptr);

    // --- WHY WE CALL unpolish()/polish() ---
    // Setting a dynamic property does NOT automatically reapply layout-related
    // QSS rules (like width, padding, margin). Qt only updates the style when
    // a widget is "polished" — i.e. when the style is (re)applied to the widget.
    //
    // To force Qt to re-evaluate and apply the QSS rules that depend on
    // `hasMenu`, we unpolish (remove) and then polish (reapply) the style.
    style()->unpolish(this);
    style()->polish(this);

    // --- WHY WE CALL updateGeometry() ---
    // Some geometry-related properties (width, margins, etc.) may affect layout.
    // Calling updateGeometry() notifies the layout system to recompute the size.
    updateGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolButtonPopup::event(QEvent *event) -> bool
{
    if (event->type() == QEvent::ToolTipChange && !handlingToolTipChange)
    {
        handlingToolTipChange = true;
        CorrectToolTip();
        handlingToolTipChange = false;
    }

    return QToolButton::event(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolButtonPopup::CorrectToolTip()
{
    if (!m_toolGroupTooltip.isEmpty())
    {
        setToolTip(QStringLiteral("<b><i>%1</i></b>:<br/>%2").arg(m_toolGroupTooltip, toolTip()));
    }
}
