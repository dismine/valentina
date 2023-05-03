/************************************************************************
 **
 **  @file   vtooltruedarts.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   23 6, 2015
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

#ifndef VTOOLTRUEDARTS_H
#define VTOOLTRUEDARTS_H

#include <qcompilerdetection.h>
#include <QDomElement>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "vtooldoublepoint.h"

template <class T> class QSharedPointer;

struct VToolTrueDartsInitData : VDrawToolInitData
{
    VToolTrueDartsInitData()
        : VDrawToolInitData(),
          p1id(NULL_ID),
          p2id(NULL_ID),
          baseLineP1Id(NULL_ID),
          baseLineP2Id(NULL_ID),
          dartP1Id(NULL_ID),
          dartP2Id(NULL_ID),
          dartP3Id(NULL_ID),
          name1(),
          mx1(labelMX),
          my1(labelMY),
          showLabel1(true),
          name2(),
          mx2(10),
          my2(15),
          showLabel2(true)
    {}

    quint32 p1id;
    quint32 p2id;
    quint32 baseLineP1Id;
    quint32 baseLineP2Id;
    quint32 dartP1Id;
    quint32 dartP2Id;
    quint32 dartP3Id;
    QString name1;
    qreal   mx1;
    qreal   my1;
    bool    showLabel1;
    QString name2;
    qreal   mx2;
    qreal   my2;
    bool    showLabel2;
};

class VToolTrueDarts : public VToolDoublePoint
{
    Q_OBJECT // NOLINT
public:
    static void    FindPoint(const QPointF &baseLineP1, const QPointF &baseLineP2, const QPointF &dartP1,
                             const QPointF &dartP2, const QPointF &dartP3, QPointF &p1, QPointF &p2);
    virtual void   SetDialog() override;
    static auto Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolTrueDarts *;
    static auto Create(VToolTrueDartsInitData initData) -> VToolTrueDarts *;
    static const QString ToolType;
    virtual auto type() const -> int override { return Type; }
    enum { Type = UserType + static_cast<int>(Tool::TrueDarts)};

    virtual void   ShowVisualization(bool show) override;

    auto BaseLineP1Name() const -> QString;
    auto BaseLineP2Name() const -> QString;
    auto DartP1Name() const -> QString;
    auto DartP2Name() const -> QString;
    auto DartP3Name() const -> QString;

protected slots:
    virtual void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id=NULL_ID) override;
protected:
    virtual void RemoveReferens() override;
    virtual void SaveDialog(QDomElement &domElement, QList<quint32> &oldDependencies,
                            QList<quint32> &newDependencies) override;
    virtual void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    virtual void ReadToolAttributes(const QDomElement &domElement) override;
    virtual void SetVisualization() override;

private:
    Q_DISABLE_COPY_MOVE(VToolTrueDarts) // NOLINT
    quint32 baseLineP1Id;
    quint32 baseLineP2Id;
    quint32 dartP1Id;
    quint32 dartP2Id;
    quint32 dartP3Id;

    VToolTrueDarts(const VToolTrueDartsInitData &initData, QGraphicsItem *parent = nullptr);
};

#endif // VTOOLTRUEDARTS_H
