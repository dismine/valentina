/************************************************************************
 **
 **  @file   dialoginsertnode.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 3, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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

#include "dialoginsertnode.h"
#include "../vmisc/theme/themeDef.h"
#include "../vpatterndb/vcontainer.h"
#include "../vtools/tools/toolsdef.h"
#include "../vtools/visualization/path/vistoolinsertnode.h"
#include "ui_dialoginsertnode.h"

#include <QMenu>

//---------------------------------------------------------------------------------------------------------------------
DialogInsertNode::DialogInsertNode(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent)
  : DialogTool(data, doc, toolId, parent),
    ui(new Ui::DialogInsertNode)
{
    ui->setupUi(this);
    InitOkCancel(ui);

    CheckPieces();

    connect(ui->comboBoxPiece, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { CheckPieces(); });

    connect(ui->listWidget, &QListWidget::customContextMenuRequested, this, &DialogInsertNode::ShowContextMenu);
    connect(ui->listWidget, &QListWidget::itemSelectionChanged, this, &DialogInsertNode::NodeSelected);
    connect(ui->spinBoxNodeNumber, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &DialogInsertNode::NodeNumberChanged);

    vis = new VisToolInsertNode(data);
}

//---------------------------------------------------------------------------------------------------------------------
DialogInsertNode::~DialogInsertNode()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInsertNode::SetPiecesList(const QVector<quint32> &list)
{
    FillComboBoxPiecesList(ui->comboBoxPiece, list);

    if (list.isEmpty())
    {
        qWarning() << tr("The list of pieces is empty. Please, first create at least one piece for current pattern "
                         "piece.");
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogInsertNode::GetPieceId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxPiece);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInsertNode::SetPieceId(quint32 id)
{
    if (ui->comboBoxPiece->count() <= 0)
    {
        ui->comboBoxPiece->addItem(data->GetPiece(id).GetName(), id);
    }
    else
    {
        const qint32 index = ui->comboBoxPiece->findData(id);
        if (index != -1)
        {
            ui->comboBoxPiece->setCurrentIndex(index);
        }
        else
        {
            ui->comboBoxPiece->setCurrentIndex(0);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogInsertNode::GetNodes() const -> QVector<VPieceNode>
{
    QVector<VPieceNode> nodes;
    for (qint32 i = 0; i < ui->listWidget->count(); ++i)
    {
        auto const node = qvariant_cast<VPieceNode>(ui->listWidget->item(i)->data(Qt::UserRole));
        for (int n = 1; n <= nodeNumbers.value(node.GetId(), 1); ++n)
        {
            nodes.append(node);
        }
    }
    return nodes;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInsertNode::ShowDialog(bool click)
{
    if (not click)
    {
        if (m_nodes.isEmpty())
        {
            return;
        }

        for (auto &node : m_nodes)
        {
            NewNodeItem(ui->listWidget, node, false, false);
        }

        m_nodes.clear();

        CheckNodes();

        prepare = true;
        setModal(true);
        emit ToolTip(QString());
        show();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInsertNode::CheckDependencyTreeComplete()
{
    // does nothing
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInsertNode::ChosenObject(quint32 id, const SceneObject &type)
{
    Q_UNUSED(type)

    if (prepare)
    {
        return;
    }

    if (auto nodeIterator = std::find_if(m_nodes.begin(),
                                         m_nodes.end(),
                                         [id](const VPieceNode &node) -> bool { return node.GetId() == id; });
        nodeIterator == m_nodes.cend())
    {
        GOType objectType = GOType::Unknown;
        try
        {
            objectType = data->GetGObject(id)->getType();
        }
        catch (const VExceptionBadId &)
        {
            qDebug() << "Cannot find an object with id" << id;
            return;
        }

        VPieceNode node;
        switch (objectType)
        {
            case GOType::Arc:
                node = VPieceNode(id, Tool::NodeArc);
                break;
            case GOType::EllipticalArc:
                node = VPieceNode(id, Tool::NodeElArc);
                break;
            case GOType::Point:
                node = VPieceNode(id, Tool::NodePoint);
                break;
            case GOType::Spline:
            case GOType::CubicBezier:
                node = VPieceNode(id, Tool::NodeSpline);
                break;
            case GOType::SplinePath:
            case GOType::CubicBezierPath:
                node = VPieceNode(id, Tool::NodeSplinePath);
                break;
            case GOType::Unknown:
            case GOType::PlaceLabel:
            default:
                qDebug() << "Got unexpected object type. Ignore.";
                return;
        }

        node.SetExcluded(true);
        m_nodes.append(node);
    }
    else
    {
        m_nodes.erase(nodeIterator);
    }

    auto *operation = qobject_cast<VisToolInsertNode *>(vis);
    SCASSERT(operation != nullptr)
    operation->SetNodes(NodesToObjects(m_nodes));
    if (!VAbstractValApplication::VApp()->getCurrentScene()->items().contains(operation))
    {
        operation->VisualMode(NULL_ID);
    }
    else
    {
        operation->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInsertNode::ShowContextMenu(const QPoint &pos)
{
    const int row = ui->listWidget->currentRow();
    if (ui->listWidget->count() == 0 || row == -1 || row >= ui->listWidget->count())
    {
        return;
    }

    QScopedPointer<QMenu> const menu(new QMenu());

    QListWidgetItem  const*rowItem = ui->listWidget->item(row);
    SCASSERT(rowItem != nullptr)

    const QAction *actionDelete = menu->addAction(FromTheme(VThemeIcon::EditDelete), tr("Delete"));

    QAction  const*selectedAction = menu->exec(ui->listWidget->viewport()->mapToGlobal(pos));
    if (selectedAction == actionDelete)
    {
        delete rowItem;
    }

    CheckNodes();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInsertNode::NodeSelected()
{
    QListWidgetItem  const*item = ui->listWidget->currentItem();

    if (item == nullptr)
    {
        ui->spinBoxNodeNumber->setDisabled(true);
        const QSignalBlocker blocker(ui->spinBoxNodeNumber);
        ui->spinBoxNodeNumber->setValue(1);
        return;
    }

    auto const node = qvariant_cast<VPieceNode>(item->data(Qt::UserRole));
    ui->spinBoxNodeNumber->setEnabled(true);
    const QSignalBlocker blocker(ui->spinBoxNodeNumber);
    ui->spinBoxNodeNumber->setValue(nodeNumbers.value(node.GetId(), 1));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInsertNode::NodeNumberChanged(int val)
{
    QListWidgetItem  const*item = ui->listWidget->currentItem();

    if (item == nullptr)
    {
        return;
    }

    auto const node = qvariant_cast<VPieceNode>(item->data(Qt::UserRole));
    nodeNumbers[node.GetId()] = val;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInsertNode::CheckPieces()
{
    QColor color;
    if (ui->comboBoxPiece->count() <= 0 || ui->comboBoxPiece->currentIndex() == -1)
    {
        m_flagError = false;
        color = errorColor;
    }
    else
    {
        m_flagError = true;
        color = OkColor(this);
    }
    ChangeColor(ui->labelPiece, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInsertNode::CheckNodes()
{
    m_flagNodes = ui->listWidget->count() > 0;
    CheckState();
}
