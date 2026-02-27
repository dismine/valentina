/************************************************************************
 **
 **  @file   vabstracttool.h
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

#ifndef VABSTRACTTOOL_H
#define VABSTRACTTOOL_H

#include <QMap>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "../vtools/visualization/visualization.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "vdatatool.h"

class VGraphicsSimpleTextItem;
class VAbstractNode;

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_CLANG("-Wdeprecated")

struct VAbstractToolInitData
{
    VAbstractToolInitData()
      : id(NULL_ID),
        scene(nullptr),
        doc(nullptr),
        data(nullptr),
        parse(Document::FullParse),
        typeCreation(Source::FromFile)
    {
    }

    virtual ~VAbstractToolInitData() = default;

    VAbstractToolInitData(const VAbstractToolInitData &) = default;
    auto operator=(const VAbstractToolInitData &) -> VAbstractToolInitData & = default;

    VAbstractToolInitData(VAbstractToolInitData &&) = default;
    auto operator=(VAbstractToolInitData &&) -> VAbstractToolInitData & = default;

    /** @brief id tool id, 0 if tool doesn't exist yet.*/
    quint32 id;
    VMainGraphicsScene *scene;
    VAbstractPattern *doc;
    VContainer *data;
    Document parse;
    Source typeCreation;
};

QT_WARNING_POP

/**
 * @brief The VAbstractTool abstract class for all tools.
 */
class VAbstractTool : public VDataTool
{
    Q_OBJECT // NOLINT

public:
    VAbstractTool(VAbstractPattern *doc, VContainer *data, quint32 id, QObject *parent = nullptr);
    virtual ~VAbstractTool() override;
    auto getId() const -> quint32;

    static bool m_suppressContextMenu;
    static const QString AttrInUse;

    static auto CheckFormula(const quint32 &toolId, QString &formula, VContainer *data) -> qreal;

    static auto Colors() -> QStringList;
    static auto ColorsList() -> QMap<QString, QString>;

    static auto GetRecord(const quint32 id, const Tool &toolType, VAbstractPattern *doc) -> VToolRecord;
    static void RemoveRecord(const VToolRecord &record, VAbstractPattern *doc);
    static void AddRecord(const VToolRecord &record, VAbstractPattern *doc);
    static void AddRecord(const quint32 id, const Tool &toolType, VAbstractPattern *doc);
    static void AddNodes(VAbstractPattern *doc, QDomElement &domElement, const VPiecePath &path);
    static void AddNodes(VAbstractPattern *doc, QDomElement &domElement, const VPiece &piece);

    auto getData() const -> const VContainer *;

    auto PointsList() const -> QMap<QString, quint32>;
    virtual auto getTagName() const -> QString = 0;
    virtual void ShowVisualization(bool show) = 0;
    virtual void ChangeLabelPosition(quint32 id, const QPointF &pos);
    virtual void SetLabelVisible(quint32 id, bool visible);
public slots:
    /**
     * @brief FullUpdateFromFile update tool data form file.
     */
    virtual void FullUpdateFromFile() = 0;
    virtual void AllowHover(bool enabled) = 0;
    virtual void AllowSelecting(bool enabled) = 0;
    virtual void ToolSelectionType(const SelectionType &selectionType);
signals:
    /**
     * @brief toolhaveChange emit if tool create change that need save.
     */
    void toolhaveChange();
    /**
     * @brief ChoosedTool emit if object was clicked.
     * @param id object id in container.
     * @param type type of scene object.
     */
    void ChoosedTool(quint32 id, SceneObject type);
    /**
     * @brief FullUpdateTree emit if need reparse pattern file.
     */
    void LiteUpdateTree(const Document &parse);

    void ToolTip(const QString &toolTip);

protected:
    /** @brief doc dom document container */
    VAbstractPattern *doc;

    /** @brief id object id. */
    const quint32 m_id;

    QPointer<Visualization> vis;
    SelectionType m_selectionType;

    /**
     * @brief AddToFile add tag with informations about tool into file.
     */
    virtual void AddToFile() = 0;
    /**
     * @brief RefreshDataInFile refresh attributes in file. If attributes don't exist create them.
     */
    virtual void RefreshDataInFile();
    /**
     * @brief RemoveReferens decrement value of reference.
     */
    virtual void RemoveReferens() {}
    virtual void DeleteToolWithConfirm(bool ask = true);
    virtual void PerformDelete();

    template <typename T> static auto CreateNode(VContainer *data, quint32 id) -> quint32;
    static auto CreateNodeSpline(VContainer *data, quint32 id) -> quint32;
    static auto CreateNodeSplinePath(VContainer *data, quint32 id) -> quint32;
    static auto CreateNodePoint(VContainer *data, quint32 id, const QSharedPointer<VPointF> &point) -> quint32;

    template <typename T> void AddVisualization();

    virtual void SetVisualization() = 0;
    virtual void ToolCreation(const Source &typeCreation);

    static auto AddSANode(VAbstractPattern *doc, const QString &tagName, const VPieceNode &node) -> QDomElement;
    static void AddNode(VAbstractPattern *doc, QDomElement &domElement, const VPieceNode &node);

    static auto PrepareNodes(const VPiecePath &path, VMainGraphicsScene *scene, VAbstractPattern *doc, VContainer *data)
        -> QVector<VPieceNode>;
    static auto PrepareNode(const VPieceNode &node, VMainGraphicsScene *scene, VAbstractPattern *doc, VContainer *data)
        -> quint32;

private:
    Q_DISABLE_COPY_MOVE(VAbstractTool) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getId return object id.
 * @return id.
 */
inline auto VAbstractTool::getId() const -> quint32
{
    return m_id;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getData return pointer to data container.
 * @return container.
 */
inline auto VAbstractTool::getData() const -> const VContainer *
{
    return &data;
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> inline void VAbstractTool::AddVisualization()
{
    auto *visual = new T(getData());
    auto *scene = qobject_cast<VMainGraphicsScene *>(VAbstractValApplication::VApp()->getCurrentScene());
    // cppcheck-suppress unknownMacro
    SCASSERT(scene != nullptr)
    scene->addItem(visual);

    connect(visual, &Visualization::ToolTip, this, &VAbstractTool::ToolTip);
    vis = visual;
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
/**
 * @brief CreateNode create new node for detail.
 * @param data container.
 * @param id id parent object.
 * @return id for new object.
 */
auto VAbstractTool::CreateNode(VContainer *data, quint32 id) -> quint32
{
    // We can't use exist object. Need create new.
    auto *node = new T(*data->GeometricObject<T>(id).data());
    node->setMode(Draw::Modeling);
    node->setIdObject(id);
    return data->AddGObject(node);
}

#endif // VABSTRACTTOOL_H
