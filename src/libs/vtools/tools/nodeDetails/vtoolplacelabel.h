/************************************************************************
 **
 **  @file   vtoolplacelabel.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 10, 2017
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
#ifndef VTOOLPLACELABEL_H
#define VTOOLPLACELABEL_H

#include <QtGlobal>

#include "vabstractnode.h"
#include "../vgeometry/vplacelabelitem.h"

class DialogTool;

struct VToolPlaceLabelInitData : VAbstractNodeInitData
{
    VToolPlaceLabelInitData()
    {}

    QString width{'0'};
    QString height{'0'};
    QString angle{'0'};
    PlaceLabelType type{PlaceLabelType::Button};
    quint32 centerPoint{NULL_ID};
    QString visibilityTrigger{'1'};
};

class VToolPlaceLabel : public VAbstractNode
{
    Q_OBJECT // NOLINT
public:
    static auto Create(const QPointer<DialogTool> &dialog, VAbstractPattern *doc, VContainer *data)
        -> VToolPlaceLabel *;
    static auto Create(VToolPlaceLabelInitData &initData) -> VToolPlaceLabel *;

    static const QString ToolType;
    virtual auto getTagName() const -> QString override;

    static void AddAttributes(VAbstractPattern *doc, QDomElement &domElement, quint32 id, const VPlaceLabelItem &label);
public slots:
    virtual void FullUpdateFromFile () override {}
    virtual void AllowHover(bool enabled) override;
    virtual void AllowSelecting(bool enabled) override;
protected:
    virtual void AddToFile() override;
    virtual void ShowNode() override {}
    virtual void HideNode() override {}
private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VToolPlaceLabel) // NOLINT

    quint32 m_pieceId;

    VToolPlaceLabel(const VToolPlaceLabelInitData &initData, QObject *qoParent = nullptr);
};

#endif // VTOOLPLACELABEL_H
