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

#include <QApplication>
#include <QColorDialog>
#include <QFocusEvent>
#include <QGridLayout>
#include <QHideEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QShowEvent>
#include <QStyle>
#include <QToolTip>
#include <QtCore/QMap>
#include <cmath>

#include "../vmisc/defglobal.h"
#include "qtcolorpicker.h"

/*! \class QtColorPicker

    \brief The QtColorPicker class provides a widget for selecting
    colors from a popup color grid.

    Users can invoke the color picker by clicking on it, or by
    navigating to it and pressing Space. They can use the mouse or
    arrow keys to navigate between colors on the grid, and select a
    color by clicking or by pressing Enter or Space. The
    colorChanged() signal is emitted whenever the color picker's color
    changes.

    The widget also supports negative selection: Users can click and
    hold the mouse button on the QtColorPicker widget, then move the
    mouse over the color grid and release the mouse button over the
    color they wish to select.

    The color grid shows a customized selection of colors. An optional
    ellipsis "..." button (signifying "more") can be added at the
    bottom of the grid; if the user presses this, a QColorDialog pops
    up and lets them choose any color they like. This button is made
    available by using setColorDialogEnabled().

    When a color is selected, the QtColorPicker widget shows the color
    and its name. If the name cannot be determined, the translatable
    name "Custom" is used.

    The QtColorPicker object is optionally initialized with the number
    of columns in the color grid. Colors are then added left to right,
    top to bottom using insertColor(). If the number of columns is not
    set, QtColorPicker calculates the number of columns and rows that
    will make the grid as square as possible.

    \code
    DrawWidget::DrawWidget(QWidget *parent, const char *name)
    {
        QtColorPicker *picker = new QtColorPicker(this);
        picker->insertColor(red, "Red"));
        picker->insertColor(QColor("green"), "Green"));
        picker->insertColor(QColor(0, 0, 255), "Blue"));
        picker->insertColor(white);

        connect(colors, SIGNAL(colorChanged(const QColor &)), SLOT(setCurrentColor(const QColor &)));
    }
    \endcode

    An alternative to adding colors manually is to initialize the grid
    with QColorDialog's standard colors using setStandardColors().

    QtColorPicker also provides a the static function getColor(),
    which pops up the grid of standard colors at any given point.

    \img colorpicker1.png
    \img colorpicker2.png

    \sa QColorDialog
*/

/*! \fn QtColorPicker::colorChanged(const QColor &color)

    This signal is emitted when the QtColorPicker's color is changed.
    \a color is the new color.

    To obtain the color's name, use text().
*/

/*
    A class  that acts very much  like a QPushButton. It's not styled,
    so we  can  expect  the  exact  same    look,  feel and   geometry
    everywhere.     Also,  this  button     always emits   clicked  on
    mouseRelease, even if the mouse button was  not pressed inside the
    widget.
*/
class ColorPickerButton : public QFrame
{
    Q_OBJECT // NOLINT

public:
    explicit ColorPickerButton(QWidget *parent);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
};

/*
    This class represents each "color" or item in the color grid.
*/
class ColorPickerItem : public QFrame
{
    Q_OBJECT // NOLINT

public:
    ColorPickerItem(const QColor &color = Qt::white, const QString &text = QString(), QWidget *parent = nullptr);
    ~ColorPickerItem();

    auto color() const -> QColor;
    auto text() const -> QString;

    void setSelected(bool);
    auto isSelected() const -> bool;
signals:
    void clicked();
    void selected();

public slots:
    void setColor(const QColor &color, const QString &text = QString());

protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *e);

private:
    QColor c;
    QString t;
    bool sel;
};

/*

*/
class ColorPickerPopup : public QFrame
{
    Q_OBJECT // NOLINT

public:
    ColorPickerPopup(int width, bool withColorDialog, QWidget *parent = nullptr);
    ~ColorPickerPopup();

    void insertColor(const QColor &col, const QString &text, vsizetype index);
    void exec();

    void setExecFlag();

    auto lastSelected() const -> QColor;

    auto find(const QColor &col) const -> ColorPickerItem *;
    auto color(int index) const -> QColor;

    auto CustomItems() const -> QVector<ColorPickerItem *>;

signals:
    void selected(const QColor &);
    void hid();

public slots:
    void getColorFromDialog();

protected slots:
    void updateSelected();

protected:
    void keyPressEvent(QKeyEvent *e);
    void showEvent(QShowEvent *e);
    void hideEvent(QHideEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

    void regenerateGrid();

private:
    Q_DISABLE_COPY_MOVE(ColorPickerPopup) // NOLINT
    QMap<int, QMap<int, QWidget *>> widgetAt{};
    QList<ColorPickerItem *> items{};
    QGridLayout *grid{nullptr};
    ColorPickerButton *moreButton{nullptr};
    QEventLoop *eventLoop{nullptr};

    int lastPos{0};
    int cols;
    QColor lastSel{};
};

/*!
    Constructs a QtColorPicker widget. The popup will display a grid
    with \a cols columns, or if \a cols is -1, the number of columns
    will be calculated automatically.

    If \a enableColorDialog is true, the popup will also have a "More"
    button (signified by an ellipsis "...") that presents a
    QColorDialog when clicked.

    After constructing a QtColorPicker, call insertColor() to add
    individual colors to the popup grid, or call setStandardColors()
    to add all the standard colors in one go.

    The \a parent argument is passed to QFrame's constructor.

    \sa QFrame
*/
QtColorPicker::QtColorPicker(QWidget *parent, int columns, bool enableColorDialog)
  : QPushButton(parent),
    withColorDialog(enableColorDialog)
{
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    setAutoDefault(false);
    setAutoFillBackground(true);
    setCheckable(true);

    // Set text
    setText(tr("Black"));
    firstInserted = false;

    // Create and set icon
    col = Qt::black;
    dirty = true;

    // Create color grid popup and connect to it.
    popup = new ColorPickerPopup(columns, withColorDialog, this);
    connect(popup, SIGNAL(selected(const QColor &)), SLOT(setCurrentColor(const QColor &)));
    connect(popup, SIGNAL(hid()), SLOT(popupClosed()));

    // Connect this push button's pressed() signal.
    connect(this, SIGNAL(toggled(bool)), SLOT(buttonPressed(bool)));
}

/*! \internal

    Pops up the color grid, and makes sure the status of
    QtColorPicker's button is right.
*/
void QtColorPicker::buttonPressed(bool toggled)
{
    if (!toggled)
        return;

    const QRect desktop = this->parentWidget()->geometry();
    // Make sure the popup is inside the desktop.
    QPoint pos = rect().bottomLeft();
    if (pos.x() < desktop.left())
        pos.setX(desktop.left());
    if (pos.y() < desktop.top())
        pos.setY(desktop.top());

    if ((pos.x() + popup->sizeHint().width()) > desktop.width())
        pos.setX(desktop.width() - popup->sizeHint().width());
    if ((pos.y() + popup->sizeHint().height()) > desktop.bottom())
        pos.setY(desktop.bottom() - popup->sizeHint().height());
    popup->move(mapToGlobal(pos));

    if (ColorPickerItem *item = popup->find(col))
        item->setSelected(true);

    // Remove focus from this widget, preventing the focus rect
    // from showing when the popup is shown. Order an update to
    // make sure the focus rect is cleared.
    clearFocus();
    update();

    // Allow keyboard navigation as soon as the popup shows.
    popup->setFocus();

    // Execute the popup. The popup will enter the event loop.
    popup->show();
}

/*!
    \internal
*/
void QtColorPicker::paintEvent(QPaintEvent *e)
{
    if (dirty)
    {
        int const iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize);
        QPixmap pix(iconSize, iconSize);
        pix.fill(palette().button().color());

        QPainter p(&pix);

        int const w = pix.width();  // width of cell in pixels
        int const h = pix.height(); // height of cell in pixels
        p.setPen(QPen(Qt::gray));
        p.setBrush(col);
        p.drawRect(2, 2, w - 5, h - 5);
        setIcon(QIcon(pix));

        dirty = false;
    }
    QPushButton::paintEvent(e);
}

/*! \internal

    Makes sure the button isn't pressed when the popup hides.
*/
void QtColorPicker::popupClosed()
{
    setChecked(false);
    setFocus();
}

/*!
    Returns the currently selected color.

    \sa text()
*/
auto QtColorPicker::currentColor() const -> QColor
{
    return col;
}

/*!
    Returns the color at position \a index.
*/
auto QtColorPicker::color(int index) const -> QColor
{
    return popup->color(index);
}

/*!
    Adds the 17 predefined colors from the Qt namespace.

    (The names given to the colors, "Black", "White", "Red", etc., are
    all translatable.)

    \sa insertColor()
*/
void QtColorPicker::setStandardColors()
{
    insertColor(Qt::black, tr("Black"));
    insertColor(Qt::white, tr("White"));
    insertColor(Qt::red, tr("Red"));
    insertColor(Qt::darkRed, tr("Dark red"));
    insertColor(Qt::green, tr("Green"));
    insertColor(Qt::darkGreen, tr("Dark green"));
    insertColor(Qt::blue, tr("Blue"));
    insertColor(Qt::darkBlue, tr("Dark blue"));
    insertColor(Qt::cyan, tr("Cyan"));
    insertColor(Qt::darkCyan, tr("Dark cyan"));
    insertColor(Qt::magenta, tr("Magenta"));
    insertColor(Qt::darkMagenta, tr("Dark magenta"));
    insertColor(Qt::yellow, tr("Yellow"));
    insertColor(Qt::darkYellow, tr("Dark yellow"));
    insertColor(Qt::gray, tr("Gray"));
    insertColor(Qt::darkGray, tr("Dark gray"));
    insertColor(Qt::lightGray, tr("Light gray"));
}

auto QtColorPicker::CustomColors() const -> QVector<QColor>
{
    QVector<QColor> customColor;
    if (popup != nullptr)
    {
        QVector<ColorPickerItem *> const items = popup->CustomItems();
        customColor.reserve(items.size());

        for (auto *item : items)
        {
            if (item != nullptr)
            {
                customColor.append(item->color());
            }
        }
    }
    return customColor;
}

/*!
    Makes \a color current. If \a color is not already in the color grid, it
    is inserted with the text "Custom".

    This function emits the colorChanged() signal if the new color is
    valid, and different from the old one.
*/
void QtColorPicker::setCurrentColor(const QColor &color)
{
    if (col == color || !color.isValid())
        return;

    ColorPickerItem *item = popup->find(color);
    if (!item)
    {
        insertColor(color, tr("Custom"));
        item = popup->find(color);
    }

    col = color;
    setText(item->text());

    dirty = true;

    popup->hide();
    repaint();

    item->setSelected(true);
    emit colorChanged(color);
}

/*!
    Adds the color \a color with the name \a text to the color grid,
    at position \a index. If index is -1, the color is assigned
    automatically assigned a position, starting from left to right,
    top to bottom.
*/
void QtColorPicker::insertColor(const QColor &color, const QString &text, int index)
{
    popup->insertColor(color, text, index);
    if (!firstInserted)
    {
        col = color;
        setText(text);
        firstInserted = true;
    }
}

/*! \property QtColorPicker::colorDialog
    \brief Whether the ellipsis "..." (more) button is available.

    If this property is set to TRUE, the color grid popup will include
    a "More" button (signified by an ellipsis, "...") which pops up a
    QColorDialog when clicked. The user will then be able to select
    any custom color they like.
*/
void QtColorPicker::setColorDialogEnabled(bool enabled)
{
    withColorDialog = enabled;
}
auto QtColorPicker::colorDialogEnabled() const -> bool
{
    return withColorDialog;
}

/*!
    Pops up a color grid with Qt default colors at \a point, using
    global coordinates. If \a allowCustomColors is true, there will
    also be a button on the popup that invokes QColorDialog.

    For example:

    \code
        void Drawer::mouseReleaseEvent(QMouseEvent *e)
        {
        if (e->button() & RightButton) {
                QColor color = QtColorPicker::getColor(mapToGlobal(e->pos()));
            }
        }
    \endcode
*/
auto QtColorPicker::getColor(const QPoint &point, bool allowCustomColors) -> QColor
{
    ColorPickerPopup popup(-1, allowCustomColors);

    popup.insertColor(Qt::black, tr("Black"), 0);
    popup.insertColor(Qt::white, tr("White"), 1);
    popup.insertColor(Qt::red, tr("Red"), 2);
    popup.insertColor(Qt::darkRed, tr("Dark red"), 3);
    popup.insertColor(Qt::green, tr("Green"), 4);
    popup.insertColor(Qt::darkGreen, tr("Dark green"), 5);
    popup.insertColor(Qt::blue, tr("Blue"), 6);
    popup.insertColor(Qt::darkBlue, tr("Dark blue"), 7);
    popup.insertColor(Qt::cyan, tr("Cyan"), 8);
    popup.insertColor(Qt::darkCyan, tr("Dark cyan"), 9);
    popup.insertColor(Qt::magenta, tr("Magenta"), 10);
    popup.insertColor(Qt::darkMagenta, tr("Dark magenta"), 11);
    popup.insertColor(Qt::yellow, tr("Yellow"), 12);
    popup.insertColor(Qt::darkYellow, tr("Dark yellow"), 13);
    popup.insertColor(Qt::gray, tr("Gray"), 14);
    popup.insertColor(Qt::darkGray, tr("Dark gray"), 15);
    popup.insertColor(Qt::lightGray, tr("Light gray"), 16);

    popup.move(point);
    popup.exec();
    return popup.lastSelected();
}

/*! \internal

    Constructs the popup widget.
*/
ColorPickerPopup::ColorPickerPopup(int width, bool withColorDialog, QWidget *parent)
  : QFrame(parent, Qt::Popup),
    cols(width)
{
    setFrameStyle(QFrame::StyledPanel);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    if (withColorDialog)
    {
        moreButton = new ColorPickerButton(this);
        moreButton->setFixedWidth(24);
        moreButton->setFixedHeight(21);
        moreButton->setFrameRect(QRect(2, 2, 20, 17));
        connect(moreButton, SIGNAL(clicked()), SLOT(getColorFromDialog()));
    }
    else
    {
        moreButton = nullptr;
    }

    eventLoop = nullptr;
    grid = nullptr;
    regenerateGrid();
}

/*! \internal

    Destructs the popup widget.
*/
ColorPickerPopup::~ColorPickerPopup()
{
    if (eventLoop != nullptr)
    {
        eventLoop->exit();
    }
}

/*! \internal

    If there is an item whole color is equal to \a col, returns a
    pointer to this item; otherwise returns 0.
*/
auto ColorPickerPopup::find(const QColor &col) const -> ColorPickerItem *
{
    auto item =
        std::find_if(items.begin(), items.end(), [col](ColorPickerItem *item) { return item && item->color() == col; });

    if (item != items.end())
    {
        return *item;
    }

    return nullptr;
}

/*! \internal

    Adds \a item to the grid. The items are added from top-left to
    bottom-right.
*/
void ColorPickerPopup::insertColor(const QColor &col, const QString &text, vsizetype index)
{
    // Don't add colors that we have already.
    ColorPickerItem *existingItem = find(col);
    ColorPickerItem *lastSelectedItem = find(lastSelected());

    if (existingItem != nullptr)
    {
        if (lastSelectedItem && existingItem != lastSelectedItem)
        {
            lastSelectedItem->setSelected(false);
        }
        existingItem->setFocus();
        existingItem->setSelected(true);
        return;
    }

    auto *item = new ColorPickerItem(col, text, this);

    if (lastSelectedItem)
    {
        lastSelectedItem->setSelected(false);
    }
    else
    {
        item->setSelected(true);
        lastSel = col;
    }
    item->setFocus();

    connect(item, SIGNAL(selected()), SLOT(updateSelected()));

    if (index == -1)
    {
        index = items.count();
    }

    items.insert(index, item);
    regenerateGrid();

    update();
}

/*! \internal

*/
auto ColorPickerPopup::color(int index) const -> QColor
{
    if (index < 0 || index > static_cast<int>(items.count() - 1))
    {
        return QColor();
    }

    auto *that = const_cast<ColorPickerPopup *>(this);
    return that->items.at(index)->color();
}

auto ColorPickerPopup::CustomItems() const -> QVector<ColorPickerItem *>
{
    QVector<ColorPickerItem *> customItems;
    customItems.reserve(items.size());

    for (auto *item : items)
    {
        if (item != nullptr && item->text() == QCoreApplication::translate("ColorPickerPopup", "Custom"))
        {
            customItems.append(item);
        }
    }

    return customItems;
}

/*! \internal

*/
void ColorPickerPopup::exec()
{
    show();

    QEventLoop e;
    eventLoop = &e;
    (void)e.exec();
    eventLoop = nullptr;
}

/*! \internal

*/
void ColorPickerPopup::updateSelected()
{
    QLayoutItem *layoutItem;
    int i = 0;
    while ((layoutItem = grid->itemAt(i)) != nullptr)
    {
        QWidget *w = layoutItem->widget();
        if (w && w->inherits("ColorPickerItem"))
        {
            auto *litem = reinterpret_cast<ColorPickerItem *>(layoutItem->widget());
            if (litem != sender())
            {
                litem->setSelected(false);
            }
        }
        ++i;
    }

    if (sender() && sender()->inherits("ColorPickerItem"))
    {
        auto *item = static_cast<ColorPickerItem *>(sender());
        lastSel = item->color();
        emit selected(item->color());
    }

    hide();
}

/*! \internal

*/
void ColorPickerPopup::mouseReleaseEvent(QMouseEvent *e)
{
    if (!rect().contains(e->pos()))
    {
        hide();
    }
}

/*! \internal

    Controls keyboard navigation and selection on the color grid.
*/
void ColorPickerPopup::keyPressEvent(QKeyEvent *e)
{
    int curRow = 0;
    int curCol = 0;

    bool foundFocus = false;
    for (int j = 0; !foundFocus && j < grid->rowCount(); ++j)
    {
        for (int i = 0; !foundFocus && i < grid->columnCount(); ++i)
        {
            if ((widgetAt[j][i] != nullptr) && widgetAt[j][i]->hasFocus())
            {
                curRow = j;
                curCol = i;
                foundFocus = true;
                break;
            }
        }
    }

    switch (e->key())
    {
        case Qt::Key_Left:
            if (curCol > 0)
            {
                --curCol;
            }
            else if (curRow > 0)
            {
                --curRow;
                curCol = grid->columnCount() - 1;
            }
            break;
        case Qt::Key_Right:
            if (curCol < grid->columnCount() - 1 && widgetAt[curRow][curCol + 1])
            {
                ++curCol;
            }
            else if (curRow < grid->rowCount() - 1)
            {
                ++curRow;
                curCol = 0;
            }
            break;
        case Qt::Key_Up:
            if (curRow > 0)
            {
                --curRow;
            }
            else
                curCol = 0;
            break;
        case Qt::Key_Down:
            if (curRow < grid->rowCount() - 1)
            {
                QWidget *w = widgetAt[curRow + 1][curCol];
                if (w)
                {
                    ++curRow;
                }
                else
                    for (int i = 1; i < grid->columnCount(); ++i)
                    {
                        if (!widgetAt[curRow + 1][i])
                        {
                            curCol = i - 1;
                            ++curRow;
                            break;
                        }
                    }
            }
            break;
        case Qt::Key_Space:
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            QWidget *w = widgetAt[curRow][curCol];
            if ((w != nullptr) && w->inherits("ColorPickerItem"))
            {
                auto *wi = reinterpret_cast<ColorPickerItem *>(w);
                wi->setSelected(true);

                QLayoutItem *layoutItem;
                int i = 0;
                while ((layoutItem = grid->itemAt(i)) != nullptr)
                {
                    QWidget *wl = layoutItem->widget();
                    if (wl && wl->inherits("ColorPickerItem"))
                    {
                        auto *litem = reinterpret_cast<ColorPickerItem *>(layoutItem->widget());
                        if (litem != wi)
                        {
                            litem->setSelected(false);
                        }
                    }
                    ++i;
                }

                lastSel = wi->color();
                emit selected(wi->color());
                hide();
            }
            else if (w && w->inherits("QPushButton"))
            {
                auto *wi = reinterpret_cast<ColorPickerItem *>(w);
                wi->setSelected(true);

                QLayoutItem *layoutItem;
                int i = 0;
                while ((layoutItem = grid->itemAt(i)) != nullptr)
                {
                    QWidget *wl = layoutItem->widget();
                    if (wl && wl->inherits("ColorPickerItem"))
                    {
                        auto *litem = reinterpret_cast<ColorPickerItem *>(layoutItem->widget());
                        if (litem != wi)
                        {
                            litem->setSelected(false);
                        }
                    }
                    ++i;
                }

                lastSel = wi->color();
                emit selected(wi->color());
                hide();
            }
            break;
        }
        case Qt::Key_Escape:
            hide();
            break;
        default:
            e->ignore();
            break;
    }

    widgetAt[curRow][curCol]->setFocus();
}

/*! \internal

*/
void ColorPickerPopup::hideEvent(QHideEvent *e)
{
    if (eventLoop)
    {
        eventLoop->exit();
    }

    setFocus();

    emit hid();
    QFrame::hideEvent(e);
}

/*! \internal

*/
auto ColorPickerPopup::lastSelected() const -> QColor
{
    return lastSel;
}

/*! \internal

    Sets focus on the popup to enable keyboard navigation. Draws
    focusRect and selection rect.
*/
void ColorPickerPopup::showEvent(QShowEvent *)
{
    bool foundSelected = false;
    for (int i = 0; i < grid->columnCount(); ++i)
    {
        for (int j = 0; j < grid->rowCount(); ++j)
        {
            QWidget *w = widgetAt[j][i];
            if (w && w->inherits("ColorPickerItem") && static_cast<ColorPickerItem *>(w)->isSelected())
            {
                w->setFocus();
                foundSelected = true;
                break;
            }
        }
    }

    if (!foundSelected)
    {
        if (items.isEmpty())
        {
            setFocus();
        }
        else
        {
            widgetAt[0][0]->setFocus();
        }
    }
}

/*!

*/
void ColorPickerPopup::regenerateGrid()
{
    widgetAt.clear();

    int columns = cols;
    if (columns == -1)
    {
        columns = static_cast<int>(ceil(sqrt(static_cast<float>(items.count()))));
    }

    // When the number of columns grows, the number of rows will
    // fall. There's no way to shrink a grid, so we create a new
    // one.
    if (grid)
        delete grid;
    grid = new QGridLayout(this);
    grid->setContentsMargins(1, 1, 1, 1);
    grid->setSpacing(0);

    int ccol = 0, crow = 0;
    for (int i = 0; i < items.size(); ++i)
    {
        if (items.at(i))
        {
            widgetAt[crow][ccol] = items.at(i);
            grid->addWidget(items.at(i), crow, ccol++);
            if (ccol == columns)
            {
                ++crow;
                ccol = 0;
            }
        }
    }

    if (moreButton)
    {
        grid->addWidget(moreButton, crow, ccol);
        widgetAt[crow][ccol] = moreButton;
    }
    updateGeometry();
}

/*! \internal

    Copies the color dialog's currently selected item and emits
    itemSelected().
*/
void ColorPickerPopup::getColorFromDialog()
{
    QColor const col = QColorDialog::getColor(lastSel, parentWidget());
    if (!col.isValid())
    {
        return;
    }

    insertColor(col, tr("Custom"), -1);
    lastSel = col;
    emit selected(col);
}

/*!
    Constructs a ColorPickerItem whose color is set to \a color, and
    whose name is set to \a text.
*/
ColorPickerItem::ColorPickerItem(const QColor &color, const QString &text, QWidget *parent)
  : QFrame(parent),
    c(color),
    t(text),
    sel(false)
{
    setToolTip(t);
    setFixedWidth(24);
    setFixedHeight(21);
}

/*!
    Destructs a ColorPickerItem.
 */
ColorPickerItem::~ColorPickerItem()
{
}

/*!
    Returns the item's color.

    \sa text()
*/
auto ColorPickerItem::color() const -> QColor
{
    return c;
}

/*!
    Returns the item's text.

    \sa color()
*/
auto ColorPickerItem::text() const -> QString
{
    return t;
}

/*!

*/
auto ColorPickerItem::isSelected() const -> bool
{
    return sel;
}

/*!

*/
void ColorPickerItem::setSelected(bool selected)
{
    sel = selected;
    update();
}

/*!
    Sets the item's color to \a color, and its name to \a text.
*/
void ColorPickerItem::setColor(const QColor &color, const QString &text)
{
    c = color;
    t = text;
    setToolTip(t);
    update();
}

/*!

*/
void ColorPickerItem::mouseMoveEvent(QMouseEvent *)
{
    setFocus();
    update();
}

/*!

*/
void ColorPickerItem::mouseReleaseEvent(QMouseEvent *)
{
    sel = true;
    emit selected();
}

/*!

*/
void ColorPickerItem::mousePressEvent(QMouseEvent *)
{
    setFocus();
    update();
}

/*!

*/
void ColorPickerItem::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    int const w = width();  // width of cell in pixels
    int const h = height(); // height of cell in pixels

    p.setPen(QPen(Qt::gray, 0, Qt::SolidLine));

    if (sel)
    {
        p.drawRect(1, 1, w - 3, h - 3);
    }

    p.setPen(QPen(Qt::black, 0, Qt::SolidLine));
    p.drawRect(3, 3, w - 7, h - 7);
    p.fillRect(QRect(4, 4, w - 8, h - 8), QBrush(c));

    if (hasFocus())
    {
        p.drawRect(0, 0, w - 1, h - 1);
    }
}

/*!

*/
ColorPickerButton::ColorPickerButton(QWidget *parent)
  : QFrame(parent)
{
    setFrameStyle(StyledPanel);
}

/*!

*/
void ColorPickerButton::mousePressEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
    setFrameShadow(Sunken);
    update();
}

/*!

*/
void ColorPickerButton::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
    setFocus();
    update();
}

/*!

*/
void ColorPickerButton::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
    setFrameShadow(Raised);
    repaint();
    emit clicked();
}

/*!

*/
void ColorPickerButton::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down || e->key() == Qt::Key_Left || e->key() == Qt::Key_Right)
    {
        QCoreApplication::sendEvent(parent(), e);
    }
    else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Space || e->key() == Qt::Key_Return)
    {
        setFrameShadow(Sunken);
        update();
    }
    else
    {
        QFrame::keyPressEvent(e);
    }
}

/*!

*/
void ColorPickerButton::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down || e->key() == Qt::Key_Left || e->key() == Qt::Key_Right)
    {
        QCoreApplication::sendEvent(parent(), e);
    }
    else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Space || e->key() == Qt::Key_Return)
    {
        setFrameShadow(Raised);
        repaint();
        emit clicked();
    }
    else
    {
        QFrame::keyReleaseEvent(e);
    }
}

/*!

*/
void ColorPickerButton::focusInEvent(QFocusEvent *e)
{
    setFrameShadow(Raised);
    update();
    QFrame::focusInEvent(e);
}

/*!

*/
void ColorPickerButton::focusOutEvent(QFocusEvent *e)
{
    setFrameShadow(Raised);
    update();
    QFrame::focusOutEvent(e);
}

/*!

*/
void ColorPickerButton::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);

    QPainter p(this);
    p.fillRect(contentsRect(), palette().button());

    QRect const r = rect();

    int const offset = frameShadow() == Sunken ? 1 : 0;

    QPen const pen(palette().buttonText(), 1);
    p.setPen(pen);

    p.drawRect(r.center().x() + offset - 4, r.center().y() + offset, 1, 1);
    p.drawRect(r.center().x() + offset, r.center().y() + offset, 1, 1);
    p.drawRect(r.center().x() + offset + 4, r.center().y() + offset, 1, 1);
    if (hasFocus())
    {
        p.setPen(QPen(Qt::black, 0, Qt::SolidLine));
        p.drawRect(0, 0, width() - 1, height() - 1);
    }

    p.end();
}

#include "qtcolorpicker.moc"
