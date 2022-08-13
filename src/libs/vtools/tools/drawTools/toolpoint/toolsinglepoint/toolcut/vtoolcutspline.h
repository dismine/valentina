/************************************************************************
 **
 **  @file   vtoolcutspline.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 12, 2013
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

#ifndef VTOOLCUTSPLINE_H
#define VTOOLCUTSPLINE_H

#include <qcompilerdetection.h>
#include <QDomElement>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "vtoolcut.h"

template <class T> class QSharedPointer;

/**
 * @brief The VToolCutSpline class for tool CutSpline. This tool find point on spline and cut spline on two.
 */
class VToolCutSpline : public VToolCut
{
    Q_OBJECT // NOLINT
public:
    ~VToolCutSpline() override =default;
    void setDialog() override;
    static auto Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolCutSpline *;
    static auto Create(VToolCutInitData &initData) -> VToolCutSpline *;
    static const QString ToolType;
    static const QString AttrSpline;
    auto type() const -> int override {return Type;}
    enum { Type = UserType + static_cast<int>(Tool::CutSpline)};
    void  ShowVisualization(bool show) override;
protected slots:
    void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id=NULL_ID) override;
protected:
    void SaveDialog(QDomElement &domElement, QList<quint32> &oldDependencies, QList<quint32> &newDependencies) override;
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    void ReadToolAttributes(const QDomElement &domElement) override;
    void SetVisualization() override;
    auto MakeToolTip() const -> QString override;
private:
    Q_DISABLE_COPY_MOVE(VToolCutSpline) // NOLINT

    explicit VToolCutSpline(const VToolCutInitData &initData, QGraphicsItem * parent = nullptr);
};

#endif // VTOOLCUTSPLINE_H
