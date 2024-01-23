/************************************************************************
 **
 **  @file   vfilepropertyeditor.cpp
 **  @author hedgeware <internal(at)hedgeware.net>
 **  @date
 **
 **  @brief
 **  @copyright
 **  All rights reserved. This program and the accompanying materials
 **  are made available under the terms of the GNU Lesser General Public License
 **  (LGPL) version 2.1 which accompanies this distribution, and is available at
 **  http://www.gnu.org/licenses/lgpl-2.1.html
 **
 **  This library is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 **  Lesser General Public License for more details.
 **
 *************************************************************************/

#include "vfilepropertyeditor.h"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QList>
#include <QMimeData>
#include <QRegularExpression>
#include <QSizePolicy>
#include <QToolButton>
#include <QUrl>

VPE::VFileEditWidget::VFileEditWidget(QWidget *parent, bool is_directory)
  : QWidget(parent),
    CurrentFilePath(),
    ToolButton(nullptr),
    FileLineEdit(nullptr),
    FileDialogFilter(),
    FilterList(),
    Directory(is_directory)
{
    // Create the tool button,ToolButton = new QToolButton(this);
    ToolButton = new QToolButton(this);
    ToolButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
    ToolButton->setText("...");
    ToolButton->setFixedWidth(20);
    ToolButton->installEventFilter(this);
    setFocusProxy(ToolButton); // Make the ToolButton the focus proxy
    setFocusPolicy(ToolButton->focusPolicy());
    connect(ToolButton, &QToolButton::clicked, this, &VFileEditWidget::onToolButtonClicked);

    // Create the line edit widget
    FileLineEdit = new QLineEdit(this);
    FileLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    FileLineEdit->setText(CurrentFilePath);
    FileLineEdit->installEventFilter(this);

    // The layout (a horizontal layout)
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(FileLineEdit);
    layout->addWidget(ToolButton);

    // Accept drops
    setAcceptDrops(true);
}

VPE::VFileEditWidget::~VFileEditWidget()
{
    // nothing needs to be done here
}

void VPE::VFileEditWidget::setFile(const QString &value, bool emit_signal)
{
    if (CurrentFilePath != value)
    {
        CurrentFilePath = value;
        FileLineEdit->setText(CurrentFilePath);

        if (emit_signal)
        {
            emit dataChangedByUser(CurrentFilePath, this);
            emit commitData(this);
        }
    }
}

void VPE::VFileEditWidget::setFilter(const QString &dialog_filter, const QStringList &filter_list)
{
    FileDialogFilter = dialog_filter;
    FilterList = filter_list;
}

void VPE::VFileEditWidget::setDirectory(bool dir)
{
    Directory = dir;
}

auto VPE::VFileEditWidget::getFile() const -> QString
{
    return CurrentFilePath;
}

void VPE::VFileEditWidget::onToolButtonClicked()
{
    QString filepath =
        (Directory ? QFileDialog::getExistingDirectory(nullptr, tr("Directory"), CurrentFilePath,
                                                       QFileDialog::ShowDirsOnly
#ifdef Q_OS_LINUX
                                                           | QFileDialog::DontUseNativeDialog
#endif
                                                       )
                   : QFileDialog::getOpenFileName(nullptr, tr("Open File"), CurrentFilePath, FileDialogFilter, nullptr
#ifdef Q_OS_LINUX
                                                  ,
                                                  QFileDialog::DontUseNativeDialog
#endif
                                                  ));
    if (filepath.isNull() == false)
    {
        setFile(filepath, true);
    }
}

auto VPE::VFileEditWidget::eventFilter(QObject *obj, QEvent *ev) -> bool
{
    if (ev->type() == QEvent::DragEnter || ev->type() == QEvent::Drop)
    {
        ev->ignore();
        if (ev->type() == QEvent::DragEnter)
        {
            dragEnterEvent(static_cast<QDragEnterEvent *>(ev));
        }
        else if (ev->type() == QEvent::Drop)
        {
            dropEvent(static_cast<QDropEvent *>(ev));
        }

        if (ev->isAccepted())
        {
            return true;
        }
        return QWidget::eventFilter(obj, ev);
    }
    else if (obj == ToolButton && ev->type() == QEvent::KeyPress)
    {
        // Ignore the event, so that eventually the delegate gets the event.
        ev->ignore();
        return true;
    }
    else if (obj == FileLineEdit)
    {
        if (ev->type() == QEvent::FocusOut)
        {
            setFile(FileLineEdit->text(), true);
            // We don't return true here because we still want the line edit to catch the event as well
        }
    }

    // forward the signal to the parent class
    return QWidget::eventFilter(obj, ev);
}

auto VPE::VFileEditWidget::isDirectory() -> bool
{
    return Directory;
}

void VPE::VFileEditWidget::dragEnterEvent(QDragEnterEvent *event)
{
    QString tmpFileName;
    if (checkMimeData(event->mimeData(), tmpFileName))
    {
        event->accept();
        event->acceptProposedAction();
    }
}

// cppcheck-suppress unusedFunction
void VPE::VFileEditWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

// cppcheck-suppress unusedFunction
void VPE::VFileEditWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void VPE::VFileEditWidget::dropEvent(QDropEvent *event)
{
    QString tmpFileName;
    if (checkMimeData(event->mimeData(), tmpFileName))
    {
        setFile(tmpFileName);
        emit dataChangedByUser(getFile(), this);
        emit commitData(this);
        event->accept();
        event->acceptProposedAction();
    }
}

auto VPE::VFileEditWidget::checkMimeData(const QMimeData *data, QString &file) const -> bool
{
    if (data->hasUrls())
    {
        QList<QUrl> tmpUrlList = data->urls();
        QFileInfo tmpFileInfo;

        auto tmpUrl = std::find_if(tmpUrlList.cbegin(), tmpUrlList.cend(),
                                   [](const QUrl &tmpUrl) { return QFile::exists(tmpUrl.toLocalFile()); });

        if (tmpUrl != tmpUrlList.cend())
        {
            tmpFileInfo = QFileInfo(tmpUrl->toLocalFile());
        }

        if (checkFileFilter(tmpFileInfo.fileName()))
        {
            file = tmpFileInfo.absoluteFilePath();
            return true;
        }
    }

    return false;
}

auto VPE::VFileEditWidget::checkFileFilter(const QString &file) const -> bool
{
    if (FilterList.isEmpty())
    {
        return true;
    }

    QFileInfo tmpFileInfo(file);

    if ((Directory && !tmpFileInfo.isDir()) || (!Directory && !tmpFileInfo.isFile()))
    {
        return false;
    }

    return std::any_of(FilterList.begin(), FilterList.end(),
                       [file](const QString &tmpFilter)
                       {
                           const QString wildcardFilter = QRegularExpression::wildcardToRegularExpression(tmpFilter);
                           QRegularExpression tmpRegExpFilter(QRegularExpression::anchoredPattern(wildcardFilter),
                                                              QRegularExpression::CaseInsensitiveOption);
                           return tmpRegExpFilter.match(file).hasMatch();
                       });
}
