/************************************************************************
 **
 **  @file   dialoguniondetails.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   23 12, 2013
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

#include "dialoguniondetails.h"

#include <QCheckBox>
#include <QVector>

#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpiece.h"
#include "../vpatterndb/vpiecenode.h"
#include "dialogtool.h"
#include "ui_dialoguniondetails.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogUnionDetails create dialog
 * @param data container with data
 * @param parent parent widget
 */
DialogUnionDetails::DialogUnionDetails(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent)
  : DialogTool(data, doc, toolId, parent),
    ui(new Ui::DialogUnionDetails),
    indexD1(0),
    indexD2(0),
    d1(NULL_ID),
    d2(NULL_ID),
    numberD(0),
    numberP(0),
    p1(NULL_ID),
    p2(NULL_ID)
{
    ui->setupUi(this);
    InitOkCancel(ui);
}

//---------------------------------------------------------------------------------------------------------------------
DialogUnionDetails::~DialogUnionDetails()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogUnionDetails::RetainPieces() const -> bool
{
    return ui->checkBox->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChoosedObject gets id and type of selected object. Save correct data and ignore wrong.
 * @param id id of point or detail
 * @param type type of object
 */
void DialogUnionDetails::ChosenObject(quint32 id, const SceneObject &type)
{
    if (numberD == 0)
    {
        ChoosedDetail(id, type, d1, indexD1);
    }
    else
    {
        if (d1 == id)
        {
            emit ToolTip(tr("Select unique detail"));
            return;
        }
        ChoosedDetail(id, type, d2, indexD2);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief CheckObject check if detail contain this id
 * @param id id of item
 * @param idDetail detail id
 * @return true if contain
 */
auto DialogUnionDetails::CheckObject(const quint32 &id, const quint32 &idDetail) const -> bool
{
    if (idDetail == NULL_ID)
    {
        return false;
    }
    const VPiece det = data->GetPiece(idDetail);
    return det.GetPath().Contains(id);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogUnionDetails::CheckDetail(const quint32 &idDetail) const -> bool
{
    if (idDetail == NULL_ID)
    {
        return false;
    }

    const VPiece det = data->GetPiece(idDetail);
    return det.GetPath().CountNodes() >= 3 && det.GetPath().ListNodePoint().size() >= 2;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChoosedDetail help save information about detail and points on detail
 * @param id id selected object
 * @param type type selected object
 * @param idDetail id detail
 * @param index index of edge
 */
void DialogUnionDetails::ChoosedDetail(const quint32 &id, const SceneObject &type, quint32 &idDetail, vsizetype &index)
{
    if (idDetail == NULL_ID && type == SceneObject::Detail)
    {
        if (CheckDetail(id))
        {
            idDetail = id;
            emit ToolTip(tr("Select a first point"));
            return;
        }

        emit ToolTip(tr("Workpiece should have at least two points and three objects"));
        return;
    }

    if (not CheckObject(id, idDetail))
    {
        return;
    }

    if (type == SceneObject::Point)
    {
        if (numberP == 0)
        {
            p1 = id;
            ++numberP;
            emit ToolTip(tr("Select a second point"));
            return;
        }

        if (numberP == 1)
        {
            if (id == p1)
            {
                emit ToolTip(tr("Select a unique point"));
                return;
            }

            VPiece d = data->GetPiece(idDetail);
            if (d.GetPath().OnEdge(p1, id))
            {
                p2 = id;
                index = d.GetPath().Edge(p1, p2);
                ++numberD;
                if (numberD > 1)
                {
                    ++numberP;
                    emit ToolTip(QString());
                    this->setModal(true);
                    this->show();
                }
                else
                {
                    numberP = 0;
                    p1 = 0;
                    p2 = 0;
                    emit ToolTip(tr("Select a detail"));
                }
            }
            else
            {
                emit ToolTip(tr("Select a point on edge"));
            }
        }
    }
}
