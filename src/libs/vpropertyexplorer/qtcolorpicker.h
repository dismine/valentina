/****************************************************************************
**
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of a Qt Solutions component.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Solutions Commercial License Agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.1, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** Please note Third Party Software included with Qt Solutions may impose
** additional restrictions and it is the user's responsibility to ensure
** that they have met the licensing requirements of the GPL, LGPL, or Qt
** Solutions Commercial license and the relevant license of the Third
** Party Software they are using.
**
** If you are unsure which license is appropriate for your use, please
** contact Nokia at qt-info@nokia.com.
**
****************************************************************************/

#ifndef QTCOLORPICKER_H
#define QTCOLORPICKER_H

#include <QColor>
#include <QFocusEvent>
#include <QLabel>
#include <QPushButton>
#include <QtCore/QEvent>
#include <QtCore/QString>

#include "vpropertyexplorer_global.h"

class ColorPickerPopup;
class ColorPickerItem;

namespace VPE
{
class VPROPERTYEXPLORERSHARED_EXPORT QtColorPicker : public QPushButton
{
    Q_OBJECT // NOLINT

    Q_PROPERTY(bool colorDialog READ colorDialogEnabled WRITE setColorDialogEnabled)

public:
    explicit QtColorPicker(QWidget *parent = nullptr,
                           int columns = -1,
                           bool enableColorDialog = true,
                           bool useNativeDialog = true);

    ~QtColorPicker() override = default;

    void insertColor(const QColor &color, const QString &text = QString(), int index = -1);
    void insertCustomColor(const QColor &color, int index = -1);

    auto currentColor() const -> QColor;

    auto color(int index) const -> QColor;

    void setColorDialogEnabled(bool enabled);
    auto colorDialogEnabled() const -> bool;

    void setStandardColors();

    auto CustomColors() const -> QVector<QColor>;

    static auto getColor(const QPoint &point, bool allowCustomColors = true) -> QColor;

    auto getUseNativeDialog() const -> bool;
    void setUseNativeDialog(bool newUseNativeDialog);

    void setDefaultColor(const QColor &color) const;

    void makeDirty();

public slots:
    void setCurrentColor(const QColor &color);

signals:
    void colorChanged(const QColor &);

protected:
    void paintEvent(QPaintEvent *e) override;

private slots:
    void buttonPressed(bool toggled);
    void popupClosed();

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(QtColorPicker) // NOLINT
    ColorPickerPopup *popup{nullptr};
    QColor col{Qt::black};
    bool withColorDialog{false};
    bool dirty{true};
    bool firstInserted{false};
};

} // namespace VPE

#endif
