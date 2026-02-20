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
#include "../../../toolcurve/vtoolabstractcurve.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "../vtools/undocommands/renameobject.h"
#include "../vtoolsinglepoint.h"

class VFormula;

struct VToolCutInitData : VToolSinglePointInitData
{
    using VToolSinglePointInitData::VToolSinglePointInitData;

    quint32 baseCurveId{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)
    QString formula{};            // NOLINT(misc-non-private-member-variables-in-classes)
    QString aliasSuffix1{};       // NOLINT(misc-non-private-member-variables-in-classes)
    QString aliasSuffix2{};       // NOLINT(misc-non-private-member-variables-in-classes)
    QString name1{};              // NOLINT(misc-non-private-member-variables-in-classes)
    QString name2{};              // NOLINT(misc-non-private-member-variables-in-classes)
};

enum class VToolCutNameField : quint8
{
    Name1,
    Name2,
    AliasSuffix1,
    AliasSuffix2
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

    auto GetName1() const -> QString;
    void SetName1(const QString &name);

    auto GetName2() const -> QString;
    void SetName2(const QString &name);

    auto GetAliasSuffix1() const -> QString;
    void SetAliasSuffix1(const QString &alias);

    auto GetAliasSuffix2() const -> QString;
    void SetAliasSuffix2(const QString &alias);

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

    QString m_name1;
    QString m_name2;

    QString m_aliasSuffix1{};
    QString m_aliasSuffix2{};

    struct ToolChanges
    {
        QString oldLabel{};
        QString newLabel{};
        QString oldName1{};
        QString newName1{};
        QString oldName2{};
        QString newName2{};
        QString oldAliasSuffix1{};
        QString newAliasSuffix1{};
        QString oldAliasSuffix2{};
        QString newAliasSuffix2{};

        auto HasChanges() const -> bool
        {
            return oldLabel != newLabel || oldName1 != newName1 || oldName2 != newName2
                   || oldAliasSuffix1 != newAliasSuffix1 || oldAliasSuffix2 != newAliasSuffix2;
        }

        auto LabelChanged() const -> bool { return oldLabel != newLabel; }
        auto Name1Changed() const -> bool { return oldName1 != newName1; }
        auto Name2Changed() const -> bool { return oldName2 != newName2; }
        auto AliasSuffix1Changed() const -> bool { return oldAliasSuffix1 != newAliasSuffix1; }
        auto AliasSuffix2Changed() const -> bool { return oldAliasSuffix2 != newAliasSuffix2; }
    };

    void ProcessToolCutOptions(const QDomElement &oldDomElement,
                               const QDomElement &newDomElement,
                               const ToolChanges &changes);

    void RefreshGeometry();
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    void ReadToolAttributes(const QDomElement &domElement) override;

    template <typename T> void ShowToolVisualization(bool show);

    template<typename T>
    static void FixSubCurveNames(VToolCutInitData &initData,
                                 const QSharedPointer<T> &baseCurve,
                                 const QSharedPointer<T> &leftSub,
                                 const QSharedPointer<T> &rightSub);

private:
    Q_DISABLE_COPY_MOVE(VToolCut) // NOLINT

    void UpdateNameField(VToolCutNameField field, const QString &value);
    auto HasConflict(const QString &value, VToolCutNameField currentField) const -> bool;
};

//---------------------------------------------------------------------------------------------------------------------
template<typename T>
inline void VToolCut::FixSubCurveNames(VToolCutInitData &initData,
                                       const QSharedPointer<T> &baseCurve,
                                       const QSharedPointer<T> &leftSub,
                                       const QSharedPointer<T> &rightSub)
{
    bool fixName1 = false;
    bool fixName2 = false;
    if (initData.name1.isEmpty())
    {
        if (!baseCurve->IsDerivative())
        {
            initData.name1 = leftSub->HeadlessName();
        }
        else
        {
            initData.name1 = GenerateUniqueCurveName(initData.data,
                                                     leftSub->GetTypeHead(),
                                                     QStringLiteral("LSubCurve"),
                                                     initData.name);
            fixName1 = true;
        }
    }

    if (initData.name2.isEmpty())
    {
        if (!baseCurve->IsDerivative())
        {
            initData.name2 = rightSub->HeadlessName();
        }
        else
        {
            initData.name2 = GenerateUniqueCurveName(initData.data,
                                                     rightSub->GetTypeHead(),
                                                     QStringLiteral("RSubCurve"),
                                                     initData.name);
            fixName2 = true;
        }
    }

    if (fixName1 || fixName2)
    {
        VAbstractApplication::VApp()->getUndoStack()->push(
            new RenameSegmentCurves(RenameAlias::CurveType(baseCurve->getType()),
                                    initData.name,
                                    fixName1 ? initData.name1 : QString(),
                                    fixName2 ? initData.name2 : QString(),
                                    initData.doc));

        QDomElement domElement = initData.doc->FindElementById(initData.id);
        if (!domElement.isElement())
        {
            qCDebug(vTool, "Can't find tool with id = %u", initData.id);
            return;
        }

        if (fixName1)
        {
            initData.doc->SetAttribute(domElement, AttrCurveName1, initData.name1);
        }

        if (fixName2)
        {
            initData.doc->SetAttribute(domElement, AttrCurveName2, initData.name2);
        }
    }
}

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
    if (auto *parentCurve = qobject_cast<VToolAbstractCurve *>(parent))
    {
        detailsMode ? parentCurve->ShowHandles(detailsMode) : parentCurve->ShowHandles(show);
    }
}

#endif // VTOOLCUT_H
