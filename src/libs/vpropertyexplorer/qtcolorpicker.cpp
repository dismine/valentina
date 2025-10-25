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

#include <cmath>
#include <utility>
#include <QApplication>
#include <QColorDialog>
#include <QFocusEvent>
#include <QGridLayout>
#include <QGuiApplication>
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
#include <QTimer>
#include <QToolTip>
#include <QtCore/QMap>

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
    and its name.

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

        connect(colors, &QtColorPicker::colorChanged, &QtColorPicker::setCurrentColor);
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
    explicit ColorPickerItem(const QColor &color = Qt::white, QString text = QString(), QWidget *parent = nullptr);
    ~ColorPickerItem() override = default;

    auto color() const -> QColor;

    void setText(const QString &text);
    auto text() const -> QString;

    void setSelected(bool selected);
    auto isSelected() const -> bool;
    auto getCustom() const -> bool;
    void setCustom(bool newCustom);

    void setDefaultColor(const QColor &color);

signals:
    void clicked();
    void selected();

public slots:
    void setColor(const QColor &color, const QString &text = QString());

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *e) override;

private:
    Q_DISABLE_COPY_MOVE(ColorPickerItem) // NOLINT
    QColor c;
    QString t;
    bool sel{false};
    bool custom{false};
    QColor defaultColor{};
};

/*

*/
class ColorPickerPopup : public QFrame
{
    Q_OBJECT // NOLINT

public:
    ColorPickerPopup(int width, bool withColorDialog, QWidget *parent = nullptr);
    ~ColorPickerPopup() override = default;

    void insertColor(const QColor &col, const QString &text, vsizetype index, bool custom = false);
    void exec();

    void setExecFlag();

    auto lastSelected() const -> QColor;

    auto find(const QColor &col) const -> ColorPickerItem *;
    auto color(int index) const -> QColor;

    auto CustomItems() const -> QVector<ColorPickerItem *>;

    auto getUseNativeDialog() const -> bool;
    void setUseNativeDialog(bool newUseNativeDialog);

    void setDefaultColor(const QColor &color) const;

signals:
    void selected(const QColor &color);
    void hid();

public slots:
    void getColorFromDialog();

protected slots:
    void updateSelected();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void showEvent(QShowEvent *e) override;
    void hideEvent(QHideEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

    void regenerateGrid();

private:
    Q_DISABLE_COPY_MOVE(ColorPickerPopup) // NOLINT
    QMap<int, QMap<int, QWidget *>> widgetAt{};
    QList<ColorPickerItem *> items{};
    QGridLayout *grid{nullptr};
    ColorPickerButton *moreButton{nullptr};

    int lastPos{0};
    int cols;
    QColor lastSel{};

    bool useNativeDialog{true};

    auto findFocusedWidget(int &row, int &col) -> bool;

    void navigateLeft(int &row, int &col) const;
    void navigateRight(int &row, int &col);
    static void navigateUp(int &row, int &col);
    void navigateDown(int &row, int &col);

    void handleSelection(int row, int col);
    void selectColor(ColorPickerItem *item);

    void setFocusOnWidget(int row, int col);
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
VPE::QtColorPicker::QtColorPicker(QWidget *parent, int columns, bool enableColorDialog, bool useNativeDialog)
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

    // Create color grid popup and connect to it.
    popup = new ColorPickerPopup(columns, withColorDialog, this);
    popup->setUseNativeDialog(useNativeDialog);
    connect(popup, &ColorPickerPopup::selected, this, &QtColorPicker::setCurrentColor);
    connect(popup, &ColorPickerPopup::hid, this, &QtColorPicker::popupClosed);

    // Connect this push button's pressed() signal.
    connect(this, &QAbstractButton::toggled, this, &QtColorPicker::buttonPressed);
}

/*! \internal

    Pops up the color grid, and makes sure the status of
    QtColorPicker's button is right.
*/
void VPE::QtColorPicker::buttonPressed(bool toggled)
{
    if (!toggled)
    {
        return;
    }

    // base position: just below the button, left aligned
    QPoint desired = mapToGlobal(QPoint(0, height()));

    // Ensure popup has a reasonable size
    popup->adjustSize();
    QSize psize = popup->sizeHint();
    if (psize.isEmpty())
    {
        popup->resize(200, 150); // fallback sensible default
        psize = popup->size();
    }

    // Find the screen that contains the button
    QScreen *screen = this->screen();
    if (screen == nullptr)
    {
        screen = QGuiApplication::primaryScreen();
    }

    QRect const avail = screen->availableGeometry();

    // adjust if it'll go off-screen to the right/bottom
    if (desired.x() + psize.width() > avail.right())
    {
        desired.setX(avail.right() - psize.width());
    }
    if (desired.y() + psize.height() > avail.bottom())
    {
        desired.setY(desired.y() - psize.height() - height()); // try above the button
    }

    // make sure coordinates are valid
    if (desired.x() < avail.left())
    {
        desired.setX(avail.left());
    }
    if (desired.y() < avail.top())
    {
        desired.setY(avail.top());
    }

    popup->move(desired);

    // Show popup after current event finishes to avoid immediate close by popup mouse-grab logic
    QTimer::singleShot(0,
                       popup,
                       [this]()
                       {
                           popup->show();
                           popup->raise();
                           popup->activateWindow();
                           popup->setFocus(Qt::PopupFocusReason);
                       });
}

/*!
    \internal
*/
void VPE::QtColorPicker::paintEvent(QPaintEvent *e)
{
    if (dirty)
    {
        QPalette const palette = this->palette();

        int const iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize);
        QPixmap pix(iconSize, iconSize);
        pix.fill(palette.button().color());

        QPainter p(&pix);

        int const w = pix.width();  // width of cell in pixels
        int const h = pix.height(); // height of cell in pixels
        p.setPen(QPen(palette.color(QPalette::Text)));

        if (QRect const cell(2, 2, w - 5, h - 5); !col.isValid())
        {
            p.fillRect(cell, QBrush(Qt::white));

            QBrush hatchBrush(Qt::DiagCrossPattern);
            hatchBrush.setColor(Qt::red);
            p.fillRect(cell, hatchBrush);

            p.setPen(QPen(palette.color(QPalette::Text)));
            p.drawRect(cell);
        }
        else
        {
            p.setBrush(col);
            p.drawRect(cell);
        }
        setIcon(QIcon(pix));

        dirty = false;
    }
    QPushButton::paintEvent(e);
}

/*! \internal

    Makes sure the button isn't pressed when the popup hides.
*/
void VPE::QtColorPicker::popupClosed()
{
    setChecked(false);
    setFocus();
}

auto VPE::QtColorPicker::getUseNativeDialog() const -> bool
{
    if (popup != nullptr)
    {
        return popup->getUseNativeDialog();
    }

    return true;
}

void VPE::QtColorPicker::setUseNativeDialog(bool useNativeDialog)
{
    if (popup != nullptr)
    {
        popup->setUseNativeDialog(useNativeDialog);
    }
}

void VPE::QtColorPicker::setDefaultColor(const QColor &color) const
{
    popup->setDefaultColor(color);
}

void VPE::QtColorPicker::makeDirty()
{
    dirty = true;
}

/*!
    Returns the currently selected color.

    \sa text()
*/
auto VPE::QtColorPicker::currentColor() const -> QColor
{
    return col;
}

/*!
    Returns the color at position \a index.
*/
auto VPE::QtColorPicker::color(int index) const -> QColor
{
    return popup->color(index);
}

/*!
    Adds the 17 predefined colors from the Qt namespace.

    (The names given to the colors, "Black", "White", "Red", etc., are
    all translatable.)

    \sa insertColor()
*/
void VPE::QtColorPicker::setStandardColors()
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

auto VPE::QtColorPicker::CustomColors() const -> QVector<QColor>
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
    is inserted.

    This function emits the colorChanged() signal if the new color is
    different from the old one.
*/
void VPE::QtColorPicker::setCurrentColor(const QColor &color)
{
    if (col == color)
    {
        return;
    }

    ColorPickerItem *item = popup->find(color);
    if (item == nullptr)
    {
        insertCustomColor(color);
        item = popup->find(color);
    }

    if (ColorPickerItem *previusItem = popup->find(col); previusItem != nullptr)
    {
        previusItem->setSelected(false);
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
void VPE::QtColorPicker::insertColor(const QColor &color, const QString &text, int index)
{
    popup->insertColor(color, text, index);
    if (!firstInserted)
    {
        col = color;
        setText(text);
        firstInserted = true;
    }
}

/*!
    Adds the color \a color as custom color to the color grid,
    at position \a index. If index is -1, the color is assigned
    automatically assigned a position, starting from left to right,
    top to bottom.
*/
void VPE::QtColorPicker::insertCustomColor(const QColor &color, int index)
{
    popup->insertColor(color, color.name(), index, true);
    if (!firstInserted)
    {
        col = color;
        setText(color.name());
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
void VPE::QtColorPicker::setColorDialogEnabled(bool enabled)
{
    withColorDialog = enabled;
}
auto VPE::QtColorPicker::colorDialogEnabled() const -> bool
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
auto VPE::QtColorPicker::getColor(const QPoint &point, bool allowCustomColors) -> QColor
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
        connect(moreButton, &ColorPickerButton::clicked, this, &ColorPickerPopup::getColorFromDialog);
    }
    else
    {
        moreButton = nullptr;
    }

    regenerateGrid();
}

/*! \internal

    If there is an item whole color is equal to \a col, returns a
    pointer to this item; otherwise returns 0.
*/
auto ColorPickerPopup::find(const QColor &col) const -> ColorPickerItem *
{
    if (auto item = std::find_if(items.begin(),
                                 items.end(),
                                 [col](const ColorPickerItem *currentItem)
                                 { return (currentItem != nullptr) && currentItem->color() == col; });
        item != items.end())
    {
        return *item;
    }

    return nullptr;
}

/*! \internal

    Adds \a item to the grid. The items are added from top-left to
    bottom-right.
*/
void ColorPickerPopup::insertColor(const QColor &col, const QString &text, vsizetype index, bool custom)
{
    // Don't add colors that we have already.
    ColorPickerItem *existingItem = find(col);
    ColorPickerItem *lastSelectedItem = find(lastSelected());

    if (existingItem != nullptr)
    {
        if ((lastSelectedItem != nullptr) && existingItem != lastSelectedItem)
        {
            lastSelectedItem->setSelected(false);
        }
        existingItem->setText(text);
        existingItem->setFocus();
        existingItem->setSelected(true);
        return;
    }

    auto *item = new ColorPickerItem(col, text, this);
    item->setCustom(custom);

    if (lastSelectedItem != nullptr)
    {
        lastSelectedItem->setSelected(false);
    }
    else
    {
        item->setSelected(true);
        lastSel = col;
    }
    item->setFocus();

    connect(item, &ColorPickerItem::selected, this, &ColorPickerPopup::updateSelected);

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
        return {};
    }

    return items.at(index)->color();
}

auto ColorPickerPopup::CustomItems() const -> QVector<ColorPickerItem *>
{
    QVector<ColorPickerItem *> customItems;
    customItems.reserve(items.size());

    for (auto *item : items)
    {
        if (item != nullptr && item->getCustom())
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

    QEventLoop eventLoop;
    connect(this, &ColorPickerPopup::hid, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#define V_CONST const
#else
#define V_CONST
#endif

/*! \internal

*/
void ColorPickerPopup::updateSelected()
{
    // The CONST macro is used to handle differences in Qt 5 and Qt 6:
    // - In Qt 6, QLayoutItem::widget() is declared as a const method, requiring a const-qualified QLayoutItem pointer.
    // - In Qt 5, QLayoutItem::widget() is not const, so the const qualifier is omitted to avoid compiler errors.
    V_CONST QLayoutItem *layoutItem = nullptr;
    int i = 0;
    while ((layoutItem = grid->itemAt(i)) != nullptr)
    {
        if (V_CONST QWidget *w = layoutItem->widget(); (w != nullptr) && w->inherits("ColorPickerItem"))
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            if (auto *litem = reinterpret_cast<ColorPickerItem *>(layoutItem->widget()); litem != sender())
            {
                litem->setSelected(false);
            }
        }
        ++i;
    }

    if ((sender() != nullptr) && sender()->inherits("ColorPickerItem"))
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
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

    if (!findFocusedWidget(curRow, curCol))
    {
        e->ignore();
        return;
    }

    switch (e->key())
    {
        case Qt::Key_Left:
            navigateLeft(curRow, curCol);
            break;
        case Qt::Key_Right:
            navigateRight(curRow, curCol);
            break;
        case Qt::Key_Up:
            navigateUp(curRow, curCol);
            break;
        case Qt::Key_Down:
            navigateDown(curRow, curCol);
            break;
        case Qt::Key_Space:
        case Qt::Key_Return:
        case Qt::Key_Enter:
            handleSelection(curRow, curCol);
            break;
        case Qt::Key_Escape:
            hide();
            break;
        default:
            e->ignore();
            break;
    }

    setFocusOnWidget(curRow, curCol);
}

/*! \internal

*/
void ColorPickerPopup::hideEvent(QHideEvent *e)
{
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
void ColorPickerPopup::showEvent(QShowEvent *e)
{
    Q_UNUSED(e)
    bool foundSelected = false;
    for (int i = 0; i < grid->columnCount(); ++i)
    {
        for (int j = 0; j < grid->rowCount(); ++j)
        {
            QWidget *w = widgetAt[j][i];
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            if ((w != nullptr) && w->inherits("ColorPickerItem") && static_cast<ColorPickerItem *>(w)->isSelected())
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
        columns = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(items.count()))));
    }

    // When the number of columns grows, the number of rows will
    // fall. There's no way to shrink a grid, so we create a new
    // one.
    delete grid;

    grid = new QGridLayout(this);
    grid->setContentsMargins(1, 1, 1, 1);
    grid->setSpacing(0);

    int ccol = 0;
    int crow = 0;
    for (auto *item : items)
    {
        if (item != nullptr)
        {
            widgetAt[crow][ccol] = item;
            grid->addWidget(item, crow, ccol);
            ++ccol;
            if (ccol == columns)
            {
                ++crow;
                ccol = 0;
            }
        }
    }

    if (moreButton != nullptr)
    {
        grid->addWidget(moreButton, crow, ccol);
        widgetAt[crow][ccol] = moreButton;
    }
    updateGeometry();
}

auto ColorPickerPopup::getUseNativeDialog() const -> bool
{
    return useNativeDialog;
}

void ColorPickerPopup::setUseNativeDialog(bool newUseNativeDialog)
{
    useNativeDialog = newUseNativeDialog;
}

void ColorPickerPopup::setDefaultColor(const QColor &color) const
{
    if (ColorPickerItem *existingItem = find(color); existingItem != nullptr)
    {
        existingItem->setDefaultColor(color);
    }
}

/*! \internal
    \fn bool ColorPickerPopup::findFocusedWidget(int &row, int &col)
    \brief Locates the currently focused widget in the grid.

    This function iterates through the grid to find the widget that has
    focus, updating the row and column indices accordingly.

    \param row Reference to the variable to store the focused widget's row index.
    \param col Reference to the variable to store the focused widget's column index.
    \return True if a focused widget is found, otherwise false.
*/
auto ColorPickerPopup::findFocusedWidget(int &row, int &col) -> bool
{
    for (int j = 0; j < grid->rowCount(); ++j)
    {
        for (int i = 0; i < grid->columnCount(); ++i)
        {
            if (widgetAt[j][i] != nullptr && widgetAt[j][i]->hasFocus())
            {
                row = j;
                col = i;
                return true;
            }
        }
    }
    return false;
}

/*! \internal
    \fn void ColorPickerPopup::navigateLeft(int &row, int &col)
    \brief Moves the focus to the left in the grid.

    This function updates the current row and column indices to navigate
    left. If the column index goes out of bounds, it wraps to the previous row.

    \param row Reference to the current row index, updated during navigation.
    \param col Reference to the current column index, updated during navigation.
*/
void ColorPickerPopup::navigateLeft(int &row, int &col) const
{
    if (col > 0)
    {
        --col;
    }
    else if (row > 0)
    {
        --row;
        col = grid->columnCount() - 1;
    }
}

/*! \internal
    \fn void ColorPickerPopup::navigateRight(int &row, int &col)
    \brief Moves the focus to the right in the grid.

    This function updates the current row and column indices to navigate
    right. If the column index goes out of bounds, it wraps to the next row.

    \param row Reference to the current row index, updated during navigation.
    \param col Reference to the current column index, updated during navigation.
*/
void ColorPickerPopup::navigateRight(int &row, int &col)
{
    if (col < grid->columnCount() - 1 && widgetAt[row][col + 1] != nullptr)
    {
        ++col;
    }
    else if (row < grid->rowCount() - 1)
    {
        ++row;
        col = 0;
    }
}

/*! \internal
    \fn void ColorPickerPopup::navigateUp(int &row, int &col)
    \brief Moves the focus upward in the grid.

    This function updates the current row and column indices to navigate
    upwards. If the row index goes out of bounds, it resets to the first column.

    \param row Reference to the current row index, updated during navigation.
    \param col Reference to the current column index, updated during navigation.
*/
void ColorPickerPopup::navigateUp(int &row, int &col)
{
    if (row > 0)
    {
        --row;
    }
    else
    {
        col = 0;
    }
}

/*! \internal
    \fn void ColorPickerPopup::navigateDown(int &row, int &col)
    \brief Moves the focus downward in the grid.

    This function updates the current row and column indices to navigate
    downwards, accounting for missing widgets in lower rows.

    \param row Reference to the current row index, updated during navigation.
    \param col Reference to the current column index, updated during navigation.
*/
void ColorPickerPopup::navigateDown(int &row, int &col)
{
    if (row < grid->rowCount() - 1)
    {
        if (const QWidget *w = widgetAt[row + 1][col]; w != nullptr)
        {
            ++row;
        }
        else
        {
            for (int i = 1; i < grid->columnCount(); ++i)
            {
                if (widgetAt[row + 1][i] == nullptr)
                {
                    col = i - 1;
                    ++row;
                    break;
                }
            }
        }
    }
}

/*! \internal
    \fn void ColorPickerPopup::handleSelection(int row, int col)
    \brief Processes the selection of the widget at the given grid position.

    This function handles interactions with widgets at the specified position,
    such as `ColorPickerItem` or `QPushButton`.

    \param row The row index of the widget to select.
    \param col The column index of the widget to select.
*/
void ColorPickerPopup::handleSelection(int row, int col)
{
    QWidget *widget = widgetAt[row][col];
    if (widget == nullptr)
    {
        return;
    }

    if (widget->inherits("ColorPickerItem") || widget->inherits("QPushButton"))
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        selectColor(reinterpret_cast<ColorPickerItem *>(widget));
    }
}

/*! \internal
    \fn void ColorPickerPopup::selectColor(ColorPickerItem *item)
    \brief Marks a `ColorPickerItem` as selected and deselects all others.

    This function highlights the specified `ColorPickerItem` and emits the
    `selected` signal with the corresponding color. The popup is then hidden.

    \param item Pointer to the `ColorPickerItem` to select.
*/
void ColorPickerPopup::selectColor(ColorPickerItem *item)
{
    item->setSelected(true);

    // The CONST macro is used to handle differences in Qt 5 and Qt 6:
    // - In Qt 6, QLayoutItem::widget() is declared as a const method, requiring a const-qualified QLayoutItem pointer.
    // - In Qt 5, QLayoutItem::widget() is not const, so the const qualifier is omitted to avoid compiler errors.

    V_CONST QLayoutItem *layoutItem = nullptr;
    int i = 0;
    while ((layoutItem = grid->itemAt(i)) != nullptr)
    {
        if (V_CONST QWidget *wl = layoutItem->widget(); (wl != nullptr) && wl->inherits("ColorPickerItem"))
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            if (auto *litem = reinterpret_cast<ColorPickerItem *>(layoutItem->widget()); litem != item)
            {
                litem->setSelected(false);
            }
        }
        ++i;
    }

    lastSel = item->color();
    emit selected(item->color());
    hide();
}

/*! \internal
    \fn void ColorPickerPopup::setFocusOnWidget(int row, int col)
    \brief Sets focus on the widget at the specified grid position.

    This function focuses the widget at the given row and column indices, if available.

    \param row The row index of the widget to focus.
    \param col The column index of the widget to focus.
*/
void ColorPickerPopup::setFocusOnWidget(int row, int col)
{
    if (widgetAt[row][col] != nullptr)
    {
        widgetAt[row][col]->setFocus();
    }
}

/*! \internal

    Copies the color dialog's currently selected item and emits
    itemSelected().
*/
void ColorPickerPopup::getColorFromDialog()
{
    auto options = QColorDialog::ColorDialogOptions();
    if (!useNativeDialog)
    {
        options = QColorDialog::DontUseNativeDialog;
    }

    QColor const col = QColorDialog::getColor(lastSel, parentWidget(), QString(), options);
    if (!col.isValid())
    {
        return;
    }

    insertColor(col, col.name(), -1, true);
    lastSel = col;
    emit selected(col);
}

/*!
    Constructs a ColorPickerItem whose color is set to \a color, and
    whose name is set to \a text.
*/
ColorPickerItem::ColorPickerItem(const QColor &color, QString text, QWidget *parent)
  : QFrame(parent),
    c(color),
    t(std::move(text))
{
    setToolTip(t);
    setFixedWidth(24);
    setFixedHeight(21);
}

/*!
    Returns the item's color.

    \sa text()
*/
auto ColorPickerItem::color() const -> QColor
{
    return c;
}

void ColorPickerItem::setText(const QString &text)
{
    t = text;
    setToolTip(t);
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
    setText(text);
    update();
}

/*!

*/
void ColorPickerItem::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
    setFocus();
    update();
}

/*!

*/
void ColorPickerItem::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
    sel = true;
    emit selected();
}

/*!

*/
void ColorPickerItem::mousePressEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
    setFocus();
    update();
}

/*!

*/
void ColorPickerItem::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)
    QPainter p(this);
    int const w = width();  // width of cell in pixels
    int const h = height(); // height of cell in pixels

    QPalette const palette = this->palette();

    if (hasFocus())
    {
        p.drawRect(0, 0, w - 1, h - 1);
        p.fillRect(QRect(0, 0, w - 1, h - 1), QBrush(palette.color(QPalette::Highlight)));

        if (sel)
        {
            p.setPen(QPen(palette.color(QPalette::HighlightedText), 0, Qt::SolidLine));
            p.drawRect(1, 1, w - 3, h - 3);
        }
    }
    else if (sel)
    {
        p.setPen(QPen(palette.color(QPalette::Text), 0, Qt::SolidLine));
        p.drawRect(1, 1, w - 3, h - 3);
    }

    p.setPen(QPen(palette.color(QPalette::Text), 0, Qt::SolidLine));
    p.drawRect(3, 3, w - 7, h - 7);

    if (!c.isValid())
    {
        QColor const fillColor = defaultColor.isValid() ? defaultColor : Qt::white;
        p.fillRect(QRect(4, 4, w - 8, h - 8), fillColor);
        QBrush hatchBrush(Qt::DiagCrossPattern);

        if (!defaultColor.isValid())
        {
            hatchBrush.setColor(Qt::red);
        }
        else
        {
            int const luminance = qGray(fillColor.red(), fillColor.green(), fillColor.blue());
            hatchBrush.setColor((luminance > 128) ? Qt::black : Qt::white);
        }

        p.fillRect(QRect(4, 4, w - 8, h - 8), hatchBrush);
    }
    else
    {
        p.fillRect(QRect(4, 4, w - 8, h - 8), QBrush(c));
    }
}

auto ColorPickerItem::getCustom() const -> bool
{
    return custom;
}

void ColorPickerItem::setCustom(bool newCustom)
{
    custom = newCustom;
}

void ColorPickerItem::setDefaultColor(const QColor &color)
{
    defaultColor = color;
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

    QPalette const palette = this->palette();

    QPainter p(this);
    p.fillRect(contentsRect(), palette.button());

    QRect const r = rect();

    int const offset = frameShadow() == Sunken ? 1 : 0;

    QPen const pen(palette.buttonText(), 1);
    p.setPen(pen);

    p.drawRect(r.center().x() + offset - 4, r.center().y() + offset, 1, 1);
    p.drawRect(r.center().x() + offset, r.center().y() + offset, 1, 1);
    p.drawRect(r.center().x() + offset + 4, r.center().y() + offset, 1, 1);

    if (hasFocus())
    {
        p.setPen(QPen(palette.color(QPalette::Text), 0, Qt::SolidLine));
        p.drawRect(0, 0, width() - 1, height() - 1);
    }

    p.end();
}

#include "qtcolorpicker.moc"
