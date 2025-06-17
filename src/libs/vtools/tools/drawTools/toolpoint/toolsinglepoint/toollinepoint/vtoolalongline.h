/************************************************************************
 **
 **  @file   vtoolalongline.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
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

#ifndef VTOOLALONGLINE_H
#define VTOOLALONGLINE_H


#include <QDomElement>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "vtoollinepoint.h"

template <class T> class QSharedPointer;

struct VToolAlongLineInitData : VToolLinePointInitData
{
    VToolAlongLineInitData() = default;

    QString formula{QStringLiteral("100.0")}; // NOLINT(misc-non-private-member-variables-in-classes)
    quint32 firstPointId{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)
    quint32 secondPointId{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)
};

/**
 * @brief The VToolAlongLine class tool for creation point along line.
 */
class VToolAlongLine : public VToolLinePoint
{
    Q_OBJECT // NOLINT
public:
    ~VToolAlongLine() override = default;
    
    void SetDialog() override;
    static auto Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene,
                       VAbstractPattern *doc, VContainer *data) -> VToolAlongLine*;
    static auto Create(VToolAlongLineInitData &initData) -> VToolAlongLine*;
    static const QString ToolType;
    auto type() const -> int override {return Type;}
    enum { Type = UserType + static_cast<int>(Tool::AlongLine)};

    auto SecondPointName() const -> QString;

    void ShowVisualization(bool show) override;
protected slots:
    void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id=NULL_ID) override;
protected:
    void RemoveReferens() override;
    void SaveDialog(QDomElement &domElement, QList<quint32> &oldDependencies, QList<quint32> &newDependencies) override;
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    void ReadToolAttributes(const QDomElement &domElement) override;
    void SetVisualization() override;
    auto MakeToolTip() const -> QString override;
private:
    Q_DISABLE_COPY_MOVE(VToolAlongLine) // NOLINT

    /** @brief secondPointId id second point of line. */
    quint32 m_secondPointId;

    explicit VToolAlongLine(const VToolAlongLineInitData &initData, QGraphicsItem *parent = nullptr);
};

#endif // VTOOLALONGLINE_H
