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
#include "../vpatterndb/measurements.h"
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
DialogMDataBase::DialogMDataBase(const QStringList &list, QWidget *parent)
  : QDialog(parent),
    ui(new Ui::DialogMDataBase),
    m_selectMode(true),
    m_list(list)
{
    ui->setupUi(this);

#if defined(Q_OS_MAC)
    setWindowFlags(Qt::Window);
#endif

    InitDataBase(m_list);

    ui->treeWidget->installEventFilter(this);

    connect(ui->treeWidget, &QTreeWidget::itemChanged, this, &DialogMDataBase::UpdateChecks);
    connect(ui->treeWidget, &QTreeWidget::itemClicked, this, &DialogMDataBase::ShowDescription);
    connect(ui->treeWidget, &QTreeWidget::customContextMenuRequested, this, &DialogMDataBase::TreeMenu);
    connect(ui->lineEditSearch, &QLineEdit::textChanged, this, &DialogMDataBase::FilterMeasurements);

    ReadSettings();
}

//---------------------------------------------------------------------------------------------------------------------
DialogMDataBase::DialogMDataBase(QWidget *parent)
  : QDialog(parent),
    ui(new Ui::DialogMDataBase),
    m_selectMode(false)
{
    ui->setupUi(this);

#if defined(Q_OS_MAC)
    setWindowFlags(Qt::Window);
#endif

    InitDataBase();

    ui->treeWidget->installEventFilter(this);

    connect(ui->treeWidget, &QTreeWidget::itemClicked, this, &DialogMDataBase::ShowDescription);
    connect(ui->treeWidget, &QTreeWidget::customContextMenuRequested, this, &DialogMDataBase::TreeMenu);
    connect(ui->treeWidget, &QTreeWidget::itemActivated, this, &DialogMDataBase::ShowDescription);
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
            if (not m_list.contains(name))
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
void DialogMDataBase::RetranslateGroups()
{
    RetranslateGroup(m_groupA, "A. " + tr("Direct Height", "Measurement section"), ListGroupA());
    RetranslateGroup(m_groupB, "B. " + tr("Direct Width", "Measurement section"), ListGroupB());
    RetranslateGroup(m_groupC, "C. " + tr("Indentation", "Measurement section"), ListGroupC());
    RetranslateGroup(m_groupD, "D. " + tr("Hand", "Measurement section"), ListGroupD());
    RetranslateGroup(m_groupE, "E. " + tr("Foot", "Measurement section"), ListGroupE());
    RetranslateGroup(m_groupF, "F. " + tr("Head", "Measurement section"), ListGroupF());
    RetranslateGroup(m_groupG, "G. " + tr("Circumference and Arc", "Measurement section"), ListGroupG());
    RetranslateGroup(m_groupH, "H. " + tr("Vertical", "Measurement section"), ListGroupH());
    RetranslateGroup(m_groupI, "I. " + tr("Horizontal", "Measurement section"), ListGroupI());
    RetranslateGroup(m_groupJ, "J. " + tr("Bust", "Measurement section"), ListGroupJ());
    RetranslateGroup(m_groupK, "K. " + tr("Balance", "Measurement section"), ListGroupK());
    RetranslateGroup(m_groupL, "L. " + tr("Arm", "Measurement section"), ListGroupL());
    RetranslateGroup(m_groupM, "M. " + tr("Leg", "Measurement section"), ListGroupM());
    RetranslateGroup(m_groupN, "N. " + tr("Crotch and Rise", "Measurement section"), ListGroupN());
    RetranslateGroup(m_groupO, "O. " + tr("Men & Tailoring", "Measurement section"), ListGroupO());
    RetranslateGroup(m_groupP, "P. " + tr("Historical & Specialty", "Measurement section"), ListGroupP());
    RetranslateGroup(m_groupQ, "Q. " + tr("Patternmaking measurements", "Measurement section"), ListGroupQ());

    ShowDescription(ui->treeWidget->currentItem(), 0);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMDataBase::ImgTag(const QString &number) -> QString
{
    QString imgUrl(QStringLiteral("<img src=\"wrong.png\" align=\"center\"/>")); // In case of error
    const QString filePath =
        QStringLiteral("://diagrams/%1.svg").arg(MapDiagrams(VAbstractApplication::VApp()->TrVars(), number));
    if (QFileInfo::exists(filePath))
    {
        // Load your SVG
        QSvgRenderer renderer;
        const bool ok = renderer.load(filePath);
        if (ok)
        {
            const QScreen *screen = QGuiApplication::screens().at(0);
            if (screen)
            {
                const QSize defSize = renderer.defaultSize();

                // Prepare a QImage with desired characteritisc
                QImage image(defSize, QImage::Format_RGB32);
                image.fill(Qt::white);

                const QRect geometry = screen->geometry();
                const int baseHeight = 1440;
                const int imgHeight = geometry.height() * defSize.height() / baseHeight;

                QImage scaledImg = image.scaledToHeight(imgHeight);

                // Get QPainter that paints to the image
                QPainter painter(&scaledImg);
                renderer.render(&painter);

                QByteArray byteArray;
                QBuffer buffer(&byteArray);
                scaledImg.save(&buffer, "PNG");
                imgUrl = QStringLiteral("<img src=\"data:image/png;base64,") + byteArray.toBase64() +
                         QStringLiteral("\" align=\"center\"/>");
            }
        }
    }

    return imgUrl;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
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
        Qt::CheckState checkState = item->checkState(0);
        for (int i = 0; i < item->childCount(); ++i)
        {
            if (not m_list.contains(item->child(i)->data(0, Qt::UserRole).toString()))
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

        ChangeCheckState(m_groupA, check);
        ChangeCheckState(m_groupB, check);
        ChangeCheckState(m_groupC, check);
        ChangeCheckState(m_groupD, check);
        ChangeCheckState(m_groupE, check);
        ChangeCheckState(m_groupF, check);
        ChangeCheckState(m_groupG, check);
        ChangeCheckState(m_groupH, check);
        ChangeCheckState(m_groupI, check);
        ChangeCheckState(m_groupJ, check);
        ChangeCheckState(m_groupK, check);
        ChangeCheckState(m_groupL, check);
        ChangeCheckState(m_groupM, check);
        ChangeCheckState(m_groupN, check);
        ChangeCheckState(m_groupO, check);
        ChangeCheckState(m_groupP, check);
        ChangeCheckState(m_groupQ, check);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::FilterMeasurements(const QString &search)
{
    FilterGroup(m_groupA, search);
    FilterGroup(m_groupB, search);
    FilterGroup(m_groupC, search);
    FilterGroup(m_groupD, search);
    FilterGroup(m_groupE, search);
    FilterGroup(m_groupF, search);
    FilterGroup(m_groupG, search);
    FilterGroup(m_groupH, search);
    FilterGroup(m_groupI, search);
    FilterGroup(m_groupJ, search);
    FilterGroup(m_groupK, search);
    FilterGroup(m_groupL, search);
    FilterGroup(m_groupM, search);
    FilterGroup(m_groupN, search);
    FilterGroup(m_groupO, search);
    FilterGroup(m_groupP, search);
    FilterGroup(m_groupQ, search);

    const QList<QTreeWidgetItem *> list = ui->treeWidget->selectedItems();
    list.isEmpty() ? ShowDescription(nullptr, -1) : ShowDescription(list.constFirst(), 0);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::InitDataBase(const QStringList &list)
{
    InitGroup(&m_groupA, "A. " + tr("Direct Height", "Measurement section"), ListGroupA(), list);
    InitGroup(&m_groupB, "B. " + tr("Direct Width", "Measurement section"), ListGroupB(), list);
    InitGroup(&m_groupC, "C. " + tr("Indentation", "Measurement section"), ListGroupC(), list);
    InitGroup(&m_groupD, "D. " + tr("Hand", "Measurement section"), ListGroupD(), list);
    InitGroup(&m_groupE, "E. " + tr("Foot", "Measurement section"), ListGroupE(), list);
    InitGroup(&m_groupF, "F. " + tr("Head", "Measurement section"), ListGroupF(), list);
    InitGroup(&m_groupG, "G. " + tr("Circumference and Arc", "Measurement section"), ListGroupG(), list);
    InitGroup(&m_groupH, "H. " + tr("Vertical", "Measurement section"), ListGroupH(), list);
    InitGroup(&m_groupI, "I. " + tr("Horizontal", "Measurement section"), ListGroupI(), list);
    InitGroup(&m_groupJ, "J. " + tr("Bust", "Measurement section"), ListGroupJ(), list);
    InitGroup(&m_groupK, "K. " + tr("Balance", "Measurement section"), ListGroupK(), list);
    InitGroup(&m_groupL, "L. " + tr("Arm", "Measurement section"), ListGroupL(), list);
    InitGroup(&m_groupM, "M. " + tr("Leg", "Measurement section"), ListGroupM(), list);
    InitGroup(&m_groupN, "N. " + tr("Crotch and Rise", "Measurement section"), ListGroupN(), list);
    InitGroup(&m_groupO, "O. " + tr("Men & Tailoring", "Measurement section"), ListGroupO(), list);
    InitGroup(&m_groupP, "P. " + tr("Historical & Specialty", "Measurement section"), ListGroupP(), list);
    InitGroup(&m_groupQ, "Q. " + tr("Patternmaking measurements", "Measurement section"), ListGroupQ(), list);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::InitGroup(QTreeWidgetItem **group, const QString &groupName, const QStringList &mList,
                                const QStringList &list)
{
    *group = AddGroup(groupName);
    for (const auto &m : mList)
    {
        AddMeasurement(*group, m, list);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::FilterGroup(QTreeWidgetItem *group, const QString &search)
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
    group->setToolTip(0, text);
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
void DialogMDataBase::AddMeasurement(QTreeWidgetItem *group, const QString &name, const QStringList &list)
{
    SCASSERT(group != nullptr)

    auto *m = new QTreeWidgetItem(group);

    if (m_selectMode)
    {
        if (list.contains(name))
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

    const QString text = VAbstractApplication::VApp()->TrVars()->MNumber(name) + ". " +
                         VAbstractApplication::VApp()->TrVars()->MToUser(name);
    m->setText(0, text);
    m->setToolTip(0, text);
    m->setData(0, Qt::UserRole, name);
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
auto DialogMDataBase::ItemFullDescription(QTreeWidgetItem *item, bool showImage) -> QString
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

    const QString number = VAbstractApplication::VApp()->TrVars()->MNumber(name);

    QString imgTag;
    if (showImage)
    {
        imgTag = ImgTag(number);
    }

    const QString text = u"<p align=\"center\" style=\"font-variant: normal; font-style: normal; font-weight: "
                         u"normal\"> %1 <br clear=\"left\"><b>%2</b>. <i>%3</i></p>"
                         u"<p align=\"left\" style=\"font-variant: normal; font-style: normal; font-weight: "
                         u"normal\">%4</p>"_s.arg(imgTag, number, VAbstractApplication::VApp()->TrVars()->GuiText(name),
                                                  VAbstractApplication::VApp()->TrVars()->Description(name));

    return text;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::RetranslateGroup(QTreeWidgetItem *group, const QString &groupText, const QStringList &list)
{
    group->setText(0, groupText);
    group->setToolTip(0, groupText);

    for (int i = 0; i < list.size(); ++i)
    {
        RetranslateMeasurement(group, i, list.at(i));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::RetranslateMeasurement(QTreeWidgetItem *group, int index, const QString &name)
{
    const QString text = VAbstractApplication::VApp()->TrVars()->MNumber(name) + ". " +
                         VAbstractApplication::VApp()->TrVars()->MToUser(name);

    QTreeWidgetItem *m = group->child(index);
    m->setText(0, text);
    m->setToolTip(0, text);
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
    SCASSERT(m_groupA != nullptr)
    SCASSERT(m_groupB != nullptr)
    SCASSERT(m_groupC != nullptr)
    SCASSERT(m_groupD != nullptr)
    SCASSERT(m_groupE != nullptr)
    SCASSERT(m_groupF != nullptr)
    SCASSERT(m_groupG != nullptr)
    SCASSERT(m_groupH != nullptr)
    SCASSERT(m_groupI != nullptr)
    SCASSERT(m_groupJ != nullptr)
    SCASSERT(m_groupK != nullptr)
    SCASSERT(m_groupL != nullptr)
    SCASSERT(m_groupM != nullptr)
    SCASSERT(m_groupN != nullptr)
    SCASSERT(m_groupO != nullptr)
    SCASSERT(m_groupP != nullptr)
    SCASSERT(m_groupQ != nullptr)

    if (m_selectMode)
    {
        if (m_groupA->checkState(0) == Qt::Unchecked && m_groupB->checkState(0) == Qt::Unchecked &&
            m_groupC->checkState(0) == Qt::Unchecked && m_groupD->checkState(0) == Qt::Unchecked &&
            m_groupE->checkState(0) == Qt::Unchecked && m_groupF->checkState(0) == Qt::Unchecked &&
            m_groupG->checkState(0) == Qt::Unchecked && m_groupH->checkState(0) == Qt::Unchecked &&
            m_groupI->checkState(0) == Qt::Unchecked && m_groupJ->checkState(0) == Qt::Unchecked &&
            m_groupK->checkState(0) == Qt::Unchecked && m_groupL->checkState(0) == Qt::Unchecked &&
            m_groupM->checkState(0) == Qt::Unchecked && m_groupN->checkState(0) == Qt::Unchecked &&
            m_groupO->checkState(0) == Qt::Unchecked && m_groupP->checkState(0) == Qt::Unchecked &&
            m_groupQ->checkState(0) == Qt::Unchecked)
        {
            return Qt::Checked;
        }

        if (m_groupA->checkState(0) == Qt::Checked && m_groupB->checkState(0) == Qt::Checked &&
            m_groupC->checkState(0) == Qt::Checked && m_groupD->checkState(0) == Qt::Checked &&
            m_groupE->checkState(0) == Qt::Checked && m_groupF->checkState(0) == Qt::Checked &&
            m_groupG->checkState(0) == Qt::Checked && m_groupH->checkState(0) == Qt::Checked &&
            m_groupI->checkState(0) == Qt::Checked && m_groupJ->checkState(0) == Qt::Checked &&
            m_groupK->checkState(0) == Qt::Checked && m_groupL->checkState(0) == Qt::Checked &&
            m_groupM->checkState(0) == Qt::Checked && m_groupN->checkState(0) == Qt::Checked &&
            m_groupO->checkState(0) == Qt::Checked && m_groupP->checkState(0) == Qt::Checked &&
            m_groupQ->checkState(0) == Qt::Checked)
        {
            return Qt::Unchecked;
        }
        return Qt::Checked;
    }

    return Qt::Unchecked;
}
