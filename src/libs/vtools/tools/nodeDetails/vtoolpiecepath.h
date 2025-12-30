/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   24 11, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#ifndef VTOOLPIECEPATH_H
#define VTOOLPIECEPATH_H

#include <QtGlobal>

#include "vabstractnode.h"

class DialogTool;

struct VToolPiecePathInitData : VAbstractNodeInitData
{
    VToolPiecePathInitData() = default;

    VPiecePath path{};
};

class VToolPiecePath : public VAbstractNode, public QGraphicsPathItem
{
    Q_OBJECT // NOLINT
public:
    ~VToolPiecePath() override = default;

    static auto Create(const QPointer<DialogTool> &dialog,
                       VMainGraphicsScene *scene,
                       VAbstractPattern *doc,
                       VContainer *data) -> VToolPiecePath *;
    static auto Create(VToolPiecePathInitData initData) -> VToolPiecePath *;

    auto type() const -> int override { return Type; }
    enum { Type = UserType + static_cast<int>(Tool::PiecePath)};
    auto getTagName() const -> QString override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    void RefreshGeometry();

    static void AddAttributes(VAbstractPattern *doc, QDomElement &domElement, quint32 id, const VPiecePath &path);
public slots:
    void FullUpdateFromFile() override;
    void AllowHover(bool enabled) override;
    void AllowSelecting(bool enabled) override;

protected:
    void AddToFile() override;
    void ToolCreation(const Source &typeCreation) override;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VToolPiecePath) // NOLINT

    quint32 m_pieceId;

    explicit VToolPiecePath(const VToolPiecePathInitData &initData,
                            QObject *qoParent = nullptr,
                            QGraphicsItem *parent = nullptr);

    void IncrementNodes(const VPiecePath &path) const;
    void DecrementNodes(const VPiecePath &path) const;
};

#endif // VTOOLPIECEPATH_H
