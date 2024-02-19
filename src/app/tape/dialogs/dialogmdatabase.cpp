/************************************************************************
 **
 **  @file   dialogmdatabase.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 7, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#include "dialogmdatabase.h"
#include "../mapplication.h"
#include "../vformat/knownmeasurements/vknownmeasurement.h"
#include "../vformat/knownmeasurements/vknownmeasurements.h"
#include "../vformat/knownmeasurements/vknownmeasurementsdatabase.h"
#include "../vmisc/def.h"
#include "ui_dialogmdatabase.h"

#include <QKeyEvent>
#include <QMenu>
#include <QSvgRenderer>
#include <QtSvg>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
DialogMDataBase::DialogMDataBase(const QUuid &id, const QStringList &usedMeasurements, QWidget *parent)
  : QDialog(parent),
    ui(new Ui::DialogMDataBase),
    m_selectMode(true),
    m_usedMeasurements(usedMeasurements),
    m_knownId(id)
{
    ui->setupUi(this);

#if defined(Q_OS_MAC)
    setWindowFlags(Qt::Window);
#endif

    InitDataBase(m_usedMeasurements);

    ui->treeWidget->installEventFilter(this);

    connect(ui->treeWidget, &QTreeWidget::itemChanged, this, &DialogMDataBase::UpdateChecks);
    connect(ui->treeWidget, &QTreeWidget::itemClicked, this, &DialogMDataBase::ShowDescription);
    connect(ui->treeWidget, &QTreeWidget::customContextMenuRequested, this, &DialogMDataBase::TreeMenu);
    connect(ui->lineEditSearch, &QLineEdit::textChanged, this, &DialogMDataBase::FilterMeasurements);

    ReadSettings();
}

//---------------------------------------------------------------------------------------------------------------------
DialogMDataBase::~DialogMDataBase()
{
    WriteSettings();
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMDataBase::GetNewNames() const -> QStringList
{
    if (m_selectMode)
    {
        QStringList newNames;
        QTreeWidgetItemIterator it(ui->treeWidget,
                                   QTreeWidgetItemIterator::NoChildren | QTreeWidgetItemIterator::Checked);
        while (*it)
        {
            const QString name = (*it)->data(0, Qt::UserRole).toString();
            if (not m_usedMeasurements.contains(name))
            {
                newNames.append(name);
            }
            ++it;
        }
        return newNames;
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMDataBase::ImgTag(const VPatternImage &image) -> QString
{
    if (!image.IsValid())
    {
        return QStringLiteral(R"(<img src="wrong.png" align="center" />)"); // In case of error
    }

    QString size;

    if (!VFuzzyComparePossibleNulls(image.GetSizeScale(), 100.0))
    {
        QSizeF const imaheSize = image.Size();
        size = QStringLiteral(R"(width="%1" height="%2")")
                   .arg(imaheSize.width() * image.GetSizeScale() / 100)
                   .arg(imaheSize.height() * image.GetSizeScale() / 100);
    }

    return QStringLiteral(R"(<img src="data:%1;base64,%2" align="center" %3% />)")
        .arg(image.ContentType(), QString(image.ContentData()), size);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
        if (m_generalGroup)
        {
            m_generalGroup->setText(0, tr("General", "Measurement section"));
        }
    }

    // remember to call base class implementation
    QDialog::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMDataBase::eventFilter(QObject *target, QEvent *event) -> bool
{
    if (target == ui->treeWidget && event->type() == QEvent::KeyPress)
    {
        auto *keyEvent = static_cast<QKeyEvent *>(event); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
        switch (keyEvent->key())
        {
            case Qt::Key_Up:
            {
                const QModelIndex model = ui->treeWidget->indexAbove(ui->treeWidget->currentIndex());
                QTreeWidgetItem *item = ui->treeWidget->itemAbove(ui->treeWidget->currentItem());
                ShowDescription(item, model.column());
                break;
            }
            case Qt::Key_Down:
            {
                const QModelIndex model = ui->treeWidget->indexBelow(ui->treeWidget->currentIndex());
                QTreeWidgetItem *item = ui->treeWidget->itemBelow(ui->treeWidget->currentItem());
                ShowDescription(item, model.column());
                break;
            }
            default:
                break;
        }
    }
    return QDialog::eventFilter(target, event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::UpdateChecks(QTreeWidgetItem *item, int column)
{
    bool diff = false;
    if (column != 0 && column != -1)
    {
        return;
    }

    if (item->childCount() != 0 && item->checkState(0) != Qt::PartiallyChecked && column != -1)
    {
        bool flag = false; // Check if we could change atleast one children
        Qt::CheckState const checkState = item->checkState(0);
        for (int i = 0; i < item->childCount(); ++i)
        {
            if (not m_usedMeasurements.contains(item->child(i)->data(0, Qt::UserRole).toString()))
            {
                item->child(i)->setCheckState(0, checkState);
                flag = true;
            }

            if (not flag) // All child in the list
            {
                item->setCheckState(0, Qt::Checked);
            }
        }
    }
    else if (item->childCount() == 0 || column == -1)
    {
        QTreeWidgetItem *parent = item->parent();
        if (parent == nullptr)
        {
            return;
        }
        for (int j = 0; j < parent->childCount(); ++j)
        {
            if (j != parent->indexOfChild(item) && item->checkState(0) != parent->child(j)->checkState(0))
            {
                diff = true;
            }
        }
        if (diff)
        {
            parent->setCheckState(0, Qt::PartiallyChecked);
        }
        else
        {
            parent->setCheckState(0, item->checkState(0));
        }

        UpdateChecks(parent, -1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::ShowDescription(QTreeWidgetItem *item, int column)
{
    if ((column != 0 && column != -1) || item == nullptr || item->childCount() != 0)
    {
        ui->textEdit->clear();
        return;
    }

    ui->textEdit->setHtml(ItemFullDescription(item));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::TreeMenu(const QPoint &pos)
{
    // Because item also will be selected need to show description
    const QModelIndex model = ui->treeWidget->currentIndex();
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    ShowDescription(item, model.column());

    auto *actionCollapseAll = new QAction(tr("Collapse All"), this);
    connect(actionCollapseAll, &QAction::triggered, ui->treeWidget, &QTreeWidget::collapseAll);

    auto *actionExpandeAll = new QAction(tr("Expand All"), this);
    connect(actionExpandeAll, &QAction::triggered, ui->treeWidget, &QTreeWidget::expandAll);

    QMenu menu(this);
    menu.addAction(actionCollapseAll);
    menu.addAction(actionExpandeAll);

    if (m_selectMode)
    {
        QString actionName;
        GlobalCheckState() == Qt::Checked ? actionName = tr("Check all") : actionName = tr("Uncheck all");

        auto *actionRecheck = new QAction(actionName, this);
        connect(actionRecheck, &QAction::triggered, this, &DialogMDataBase::Recheck);

        menu.addAction(actionRecheck);
    }
    menu.exec(ui->treeWidget->mapToGlobal(pos));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::Recheck()
{
    if (m_selectMode)
    {
        const Qt::CheckState check = GlobalCheckState();

        for (auto *group : m_groups)
        {
            ChangeCheckState(group, check);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::FilterMeasurements(const QString &search)
{
    for (auto *group : m_groups)
    {
        FilterGroup(group, search);
    }

    const QList<QTreeWidgetItem *> list = ui->treeWidget->selectedItems();
    list.isEmpty() ? ShowDescription(nullptr, -1) : ShowDescription(list.constFirst(), 0);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::InitDataBase(const QStringList &usedMeasurements)
{
    m_groups.clear();

    if (m_knownId.isNull())
    {
        return;
    }

    VKnownMeasurementsDatabase *db = MApplication::VApp()->KnownMeasurementsDatabase();
    VKnownMeasurements const knownDB = db->KnownMeasurements(m_knownId);

    QMap<int, VKnownMeasurement> const measurements = knownDB.OrderedGroupMeasurements(QString());
    if (!measurements.isEmpty())
    {
        m_generalGroup = InitGroup(tr("General", "Measurement section"), measurements, usedMeasurements);
        m_groups.append(m_generalGroup);
    }

    QStringList const groups = knownDB.Groups();
    for (const auto &group : groups)
    {
        QMap<int, VKnownMeasurement> const groupMeasurements = knownDB.OrderedGroupMeasurements(group);
        m_groups.append(InitGroup(group, groupMeasurements, usedMeasurements));
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMDataBase::InitGroup(const QString &groupName, const QMap<int, VKnownMeasurement> &mlist,
                                const QStringList &list) -> QTreeWidgetItem *
{
    QTreeWidgetItem *group = AddGroup(groupName);
    auto i = mlist.constBegin();
    while (i != mlist.constEnd())
    {
        AddMeasurement(group, i.value(), list);
        ++i;
    }

    return group;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::FilterGroup(QTreeWidgetItem *group, const QString &search) const
{
    SCASSERT(group != nullptr)

    bool match = false;
    for (int i = 0; i < group->childCount(); ++i)
    {
        QTreeWidgetItem *childItem = group->child(i);
        const QString description =
            QTextDocumentFragment::fromHtml(ItemFullDescription(childItem, false)).toPlainText();

        const bool hidden = not childItem->text(0).contains(search, Qt::CaseInsensitive) &&
                            not description.contains(search, Qt::CaseInsensitive);

        childItem->setHidden(hidden);
        if (not hidden)
        {
            match = true;
        }
    }

    group->setHidden(not group->text(0).contains(search, Qt::CaseInsensitive) && not match);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMDataBase::AddGroup(const QString &text) -> QTreeWidgetItem *
{
    auto *group = new QTreeWidgetItem(ui->treeWidget);
    group->setText(0, text);
    group->setExpanded(true);
    if (m_selectMode)
    {
        group->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        group->setCheckState(0, Qt::Unchecked);
        group->setBackground(0, QBrush(Qt::lightGray));
    }
    return group;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::AddMeasurement(QTreeWidgetItem *group, const VKnownMeasurement &measurement,
                                     const QStringList &list)
{
    SCASSERT(group != nullptr)

    auto *m = new QTreeWidgetItem(group);

    if (m_selectMode)
    {
        if (list.contains(measurement.name))
        {
            m->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            m->setCheckState(0, Qt::Checked);
            m->setBackground(0, QBrush(Qt::yellow));
        }
        else
        {
            m->setCheckState(0, Qt::Unchecked);
        }

        UpdateChecks(m, 0);
    }

    m->setText(0, measurement.name);
    m->setToolTip(0, measurement.fullName);
    m->setData(0, Qt::UserRole, measurement.name);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::ReadSettings()
{
    restoreGeometry(MApplication::VApp()->TapeSettings()->GetDataBaseGeometry());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::WriteSettings()
{
    MApplication::VApp()->TapeSettings()->SetDataBaseGeometry(saveGeometry());
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMDataBase::ItemFullDescription(QTreeWidgetItem *item, bool showImage) const -> QString
{
    if (item == nullptr || item->childCount() != 0)
    {
        return {};
    }

    const QString name = item->data(0, Qt::UserRole).toString();
    if (name.isEmpty())
    {
        return {};
    }

    VKnownMeasurementsDatabase *db = MApplication::VApp()->KnownMeasurementsDatabase();
    VKnownMeasurements const knownDB = db->KnownMeasurements(m_knownId);
    VKnownMeasurement const known = knownDB.Measurement(name);

    QString imgTag;
    if (showImage)
    {
        imgTag = ImgTag(knownDB.Image(known.diagram));
    }

    const QString text = u"<p align=\"center\" style=\"font-variant: normal; font-style: normal; font-weight: "
                         u"normal\"> %1 <br clear=\"left\"><b>%2</b></p>"
                         u"<p align=\"left\" style=\"font-variant: normal; font-style: normal; font-weight: "
                         u"normal\">%3</p>"_s.arg(imgTag, known.fullName, known.description);

    return text;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::ChangeCheckState(QTreeWidgetItem *group, Qt::CheckState check)
{
    SCASSERT(group != nullptr)
    group->setCheckState(0, check);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMDataBase::GlobalCheckState() const -> Qt::CheckState
{
    if (m_selectMode)
    {
        bool allUnchecked = true;
        bool allChecked = true;

        for (auto *group : m_groups)
        {
            if (group->checkState(0) == Qt::Checked)
            {
                allUnchecked = false;
            }
            if (group->checkState(0) != Qt::Checked)
            {
                allChecked = false;
            }
        }

        if (allUnchecked)
        {
            return Qt::Checked;
        }
        else if (allChecked)
        {
            return Qt::Unchecked;
        }

        return Qt::Checked;
    }

    return Qt::Unchecked;
}
