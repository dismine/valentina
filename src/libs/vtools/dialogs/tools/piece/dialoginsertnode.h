/************************************************************************
 **
 **  @file   dialoginsertnode.h
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

#ifndef DIALOGINSERTNODE_H
#define DIALOGINSERTNODE_H

#include "../dialogtool.h"
#include "../vpatterndb/vpiecenode.h"

namespace Ui
{
    class DialogInsertNode;
}

class DialogInsertNode : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    explicit DialogInsertNode(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent = nullptr);
    virtual ~DialogInsertNode();

    virtual void SetPiecesList(const QVector<quint32> &list) override;

    auto GetPieceId() const -> quint32;
    void    SetPieceId(quint32 id);

    auto GetNodes() const -> QVector<VPieceNode>;

    virtual void ShowDialog(bool click) override;

public slots:
    virtual void SelectedObject(bool selected, quint32 object, quint32 tool) override;

protected:
    virtual auto IsValid() const -> bool final;

private slots:
    void ShowContextMenu(const QPoint &pos);
    void NodeSelected();
    void NodeNumberChanged(int val);

private:
    Q_DISABLE_COPY_MOVE(DialogInsertNode) // NOLINT
    Ui::DialogInsertNode *ui;

    QVector<VPieceNode> m_nodes{};
    bool m_flagNodes{false};
    bool m_flagError{false};

    QMap<quint32, int> nodeNumbers{};

    void CheckPieces();
    void CheckNodes();
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogInsertNode::IsValid() const -> bool
{
    return m_flagNodes && m_flagError;
}

#endif // DIALOGINSERTNODE_H
