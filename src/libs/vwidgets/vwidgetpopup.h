/************************************************************************
 **
 **  @file   vwidgetpopup.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 2, 2015
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

#ifndef VWIDGETPOPUP_H
#define VWIDGETPOPUP_H

#include <QFrame>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../vmisc/defglobal.h"
#endif

/**
    \brief Class showing a widget as popup window.

    setWidget() function allows you to specify the widget to be popped up.
    After widget is set, you normally should call show() slot in order to pop the
    widget up at the specified global position.

    VWidgetPopup takes care about positioning of your widget on the screen so it will
    be always visible even if popped beside.
*/
class VWidgetPopup : public QFrame
{
    Q_OBJECT // NOLINT

public:
    /** Constructor.

        If \a parent not specified (default), then popup widget gets
        attribute Qt::WA_DeleteOnClose and will be deleted after close.
      */
    explicit VWidgetPopup(QWidget *parent = nullptr);

    /** Sets widget to be popped up to \a widget.
        If \a own is true then the widget will be reparented to the popup widget.
      */
    void SetWidget(QWidget *widget, bool own = true);

    /** Returns widget to be popped up. */
    auto Widget() const -> QWidget *;

    /** Returns true if widget is owned by this popup widget, false otherwise. */
    auto isOwned() const -> bool;

    auto GetLifeTime() const -> int;
    void SetLifeTime(int value);

    static void PopupMessage(QWidget *w, const QString &msg);

public slots:
    /** Pops up the widget at global coordinates \a coord. */
    void Show(QPoint coord);

protected:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VWidgetPopup) // NOLINT
    QWidget *mWidget;
    bool mOwn;
    QWidget *mOldParent;
    int lifeTime;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VWidgetPopup::Widget() const -> QWidget *
{
    return mWidget;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VWidgetPopup::isOwned() const -> bool
{
    return mOwn;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VWidgetPopup::GetLifeTime() const -> int
{
    return lifeTime;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VWidgetPopup::SetLifeTime(int value)
{
    lifeTime = value;
}

#endif // VWIDGETPOPUP_H
