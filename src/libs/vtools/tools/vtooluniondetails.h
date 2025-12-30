/************************************************************************
 **
 **  @file   vtooluniondetails.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 12, 2013
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

#ifndef VTOOLUNIONDETAILS_H
#define VTOOLUNIONDETAILS_H

#include <QDomElement>
#include <QDomNode>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QVector>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vpatterndb/vpiece.h"
#include "vabstracttool.h"

class DialogTool;

constexpr int UNION_VERSSION = 2;

struct VToolUnionDetailsInitData : VAbstractToolInitData
{
    VToolUnionDetailsInitData() = default;

    quint32 d1id{NULL_ID};        // NOLINT(misc-non-private-member-variables-in-classes)
    quint32 d2id{NULL_ID};        // NOLINT(misc-non-private-member-variables-in-classes)
    quint32 indexD1{NULL_ID};     // NOLINT(misc-non-private-member-variables-in-classes)
    quint32 indexD2{NULL_ID};     // NOLINT(misc-non-private-member-variables-in-classes)
    bool retainPieces{false};     // NOLINT(misc-non-private-member-variables-in-classes)
    uint version{UNION_VERSSION}; // NOLINT(misc-non-private-member-variables-in-classes)
};

/**
 * @brief The VToolUnionDetails class tool union details.
 */
class VToolUnionDetails : public VAbstractTool
{
    Q_OBJECT // NOLINT

public:
    ~VToolUnionDetails() override = default;
    static auto Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolUnionDetails *;
    static auto Create(VToolUnionDetailsInitData initData) -> VToolUnionDetails *;

    static const QString ToolType;
    static const QString TagDetail;
    static const QString TagNode;
    static const QString TagChildren;
    static const QString TagChild;
    static const QString AttrIndexD1;
    static const QString AttrIndexD2;
    static const QString AttrIdObject;
    static const QString AttrNodeType;
    static const QString NodeTypeContour;
    static const QString NodeTypeModeling;

    static const quint8 unionVersion;

    auto getTagName() const -> QString override;
    void ShowVisualization(bool show) override;
    void GroupVisibility(quint32 object, bool visible) override;

    static auto CalcUnitedPath(const VPiecePath &d1Path, const VPiecePath &d2Path, quint32 indexD2, quint32 pRotate)
        -> QVector<QPair<bool, VPieceNode>>;
public slots:
    /**
     * @brief FullUpdateFromFile update tool data form file.
     */
    void FullUpdateFromFile() override {}
    void AllowHover(bool /*enabled*/) override {}
    void AllowSelecting(bool /*enabled*/) override {}

protected:
    void AddToFile() override;
    void SetVisualization() override {}

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VToolUnionDetails) // NOLINT
    /** @brief d1 first detail id. */
    quint32 d1id;

    /** @brief d2 second detail id. */
    quint32 d2id;

    /** @brief indexD1 index edge in first detail. */
    quint32 indexD1;

    /** @brief indexD2 index edge in second detail. */
    quint32 indexD2;

    uint version;

    explicit VToolUnionDetails(const VToolUnionDetailsInitData &initData, QObject *parent = nullptr);

    void AddDetail(QDomElement &domElement, const VPiece &d) const;
    void AddToModeling(const QDomElement &domElement);
    auto GetReferenceObjects() const -> QVector<quint32>;
    static auto ReferenceObjects(const QDomElement &root, const QString &tag, const QString &attribute)
        -> QVector<quint32>;
};

#endif // VTOOLUNIONDETAILS_H
