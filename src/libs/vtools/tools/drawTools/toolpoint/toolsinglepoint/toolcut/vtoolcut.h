/************************************************************************
 **
 **  @file   vtoolcut.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   25 6, 2014
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

#ifndef VTOOLCUT_H
#define VTOOLCUT_H

#include <qcompilerdetection.h>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../../../toolcurve/vabstractspline.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vtoolsinglepoint.h"
#include "../vmisc/def.h"
#include "../../../../vdatatool.h"
#include "../../../../../visualization/visualization.h"

class VFormula;

struct VToolCutInitData : VToolSinglePointInitData
{
    VToolCutInitData()
        : VToolSinglePointInitData()
    {}

    quint32 baseCurveId{NULL_ID};
    QString formula{};
    QString aliasSuffix1{};
    QString aliasSuffix2{};
};

class VToolCut : public VToolSinglePoint
{
    Q_OBJECT // NOLINT
public:
    explicit VToolCut(const VToolCutInitData &initData, QGraphicsItem * parent = nullptr);
    virtual int   type() const override {return Type;}
    enum { Type = UserType + static_cast<int>(Tool::Cut)};

    VFormula GetFormulaLength() const;
    void     SetFormulaLength(const VFormula &value);

    QString GetAliasSuffix1() const;
    void    SetAliasSuffix1(QString alias);

    QString GetAliasSuffix2() const;
    void    SetAliasSuffix2(QString alias);

    QString CurveName() const;

public slots:
    virtual void    Disable(bool disable, const QString &namePP) override;
    virtual void    DetailsMode(bool mode) override;
    virtual void    FullUpdateFromFile() override;
protected:
    /** @brief formula keep formula of length */
    QString       formula;

    quint32       baseCurveId;
    bool          detailsMode;

    QString m_aliasSuffix1{};
    QString m_aliasSuffix2{};

    void          RefreshGeometry();
    virtual void  RemoveReferens() override;
    virtual void  SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    virtual void  ReadToolAttributes(const QDomElement &domElement) override;

    template <typename T>
    void ShowToolVisualization(bool show);

private:
    Q_DISABLE_COPY_MOVE(VToolCut) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline void VToolCut::ShowToolVisualization(bool show)
{
    if (show)
    {
        if (vis.isNull())
        {
            AddVisualization<T>();
            SetVisualization();
        }
        else
        {
            if (T *visual = qobject_cast<T *>(vis))
            {
                visual->show();
            }
        }
    }
    else
    {
        delete vis;
    }

    VDataTool *parent = VAbstractPattern::getTool(VAbstractTool::data.GetGObject(baseCurveId)->getIdTool());
    if (VAbstractSpline *parentCurve = qobject_cast<VAbstractSpline *>(parent))
    {
        detailsMode ? parentCurve->ShowHandles(detailsMode) : parentCurve->ShowHandles(show);
    }
}

#endif // VTOOLCUT_H
