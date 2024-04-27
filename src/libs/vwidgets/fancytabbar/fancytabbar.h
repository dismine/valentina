/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#ifndef FANCYTABWIDGET_H
#define FANCYTABWIDGET_H

#include <QIcon>
#include <QPropertyAnimation>
#include <QTimer>
#include <QWidget>

#include "../vmisc/defglobal.h"
#include "fancytab.h"

class QPainter;

class FancyTabBar : public QWidget
{
    Q_OBJECT // NOLINT

public:
    enum TabBarPosition
    {
        Above,
        Below,
        Left,
        Right
    };

    explicit FancyTabBar(TabBarPosition position, QWidget *parent = nullptr);
    ~FancyTabBar() override = default;

    auto sizeHint() const -> QSize override;
    auto minimumSizeHint() const -> QSize override;

    void SetOrientation(TabBarPosition p);

    void SetTabEnabled(int index, bool enable);
    auto IsTabEnabled(int index) const -> bool;

    void InsertTab(int index, const QIcon &icon, const QString &label);
    void RemoveTab(int index);

    void SetCurrentIndex(int index);
    auto CurrentIndex() const -> int;

    void SetTabToolTip(int index, const QString &toolTip);
    auto TabToolTip(int index) const -> QString;

    auto TabIcon(int index) const -> QIcon;

    auto TabText(int index) const -> QString;
    void SetTabText(int index, const QString &text);

    auto Count() const -> vsizetype;
    auto TabRect(int index) const -> QRect;

signals:
    void CurrentChanged(int);

protected:
    auto event(QEvent *event) -> bool override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *e) override;
#else
    void enterEvent(QEvent *e) override;
#endif
    void leaveEvent(QEvent *e) override;

private slots:
    void EmitCurrentIndex();

private:
    Q_DISABLE_COPY_MOVE(FancyTabBar) // NOLINT

    enum Corner
    {
        OutsideBeginning,
        OutsideEnd,
        InsideBeginning,
        InsideEnd
    };

    static const int m_rounding;

    TabBarPosition m_position;
    QRect m_hoverRect{};
    int m_hoverIndex{-1};
    int m_currentIndex{-1};
    QList<FancyTab *> m_attachedTabs{};
    QTimer m_timerTriggerChangedSignal{};

    auto GetCorner(const QRect &rect, Corner corner) const -> QPoint;

    auto AdjustRect(const QRect &rect, qint8 offsetOutside, qint8 offsetInside, qint8 offsetBeginning,
                    qint8 offsetEnd) const -> QRect;

    // Same with a point. + means towards Outside/End, - means towards Inside/Beginning
    auto AdjustPoint(const QPoint &point, qint8 offsetInsideOutside, qint8 offsetBeginningEnd) const -> QPoint;

    auto TabSizeHint(bool minimum = false) const -> QSize;
    void PaintTab(QPainter *painter, int tabIndex) const;
    auto ValidIndex(int index) const -> bool;
};

#endif // FANCYTABWIDGET_H
