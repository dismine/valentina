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

#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../../../../../visualization/visualization.h"
#include "../../../../vdatatool.h"
#include "../../../toolcurve/vabstractspline.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "../vtoolsinglepoint.h"

class VFormula;

struct VToolCutInitData : VToolSinglePointInitData
{
    using VToolSinglePointInitData::VToolSinglePointInitData;

    quint32 baseCurveId{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)
    QString formula{};            // NOLINT(misc-non-private-member-variables-in-classes)
    QString aliasSuffix1{};       // NOLINT(misc-non-private-member-variables-in-classes)
    QString aliasSuffix2{};       // NOLINT(misc-non-private-member-variables-in-classes)
};

class VToolCut : public VToolSinglePoint
{
    Q_OBJECT // NOLINT

public:
    explicit VToolCut(const VToolCutInitData &initData, QGraphicsItem *parent = nullptr);
    ~VToolCut() override = default;
    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Tool::Cut)
    };

    auto GetFormulaLength() const -> VFormula;
    void SetFormulaLength(const VFormula &value);

    auto GetAliasSuffix1() const -> QString;
    void SetAliasSuffix1(QString alias);

    auto GetAliasSuffix2() const -> QString;
    void SetAliasSuffix2(QString alias);

    auto CurveName() const -> QString;

    auto BaseCurveId() const -> quint32;

public slots:
    void SetDetailsMode(bool mode) override;
    void FullUpdateFromFile() override;

protected:
    /** @brief formula keep formula of length */
    QString formula;

    quint32 baseCurveId;
    bool detailsMode;

    QString m_aliasSuffix1{};
    QString m_aliasSuffix2{};

    void RefreshGeometry();
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    void ReadToolAttributes(const QDomElement &domElement) override;

    template <typename T> void ShowToolVisualization(bool show);

private:
    Q_DISABLE_COPY_MOVE(VToolCut) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VToolCut::BaseCurveId() const -> quint32
{
    return baseCurveId;
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> inline void VToolCut::ShowToolVisualization(bool show)
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
        delete vis.data();
    }

    VDataTool *parent = VAbstractPattern::getTool(VAbstractTool::data.GetGObject(baseCurveId)->getIdTool());
    if (auto *parentCurve = qobject_cast<VAbstractSpline *>(parent))
    {
        detailsMode ? parentCurve->ShowHandles(detailsMode) : parentCurve->ShowHandles(show);
    }
}

#endif // VTOOLCUT_H
