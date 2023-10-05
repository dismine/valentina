/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   6 11, 2016
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

#ifndef VTOOLSEAMALLOWANCE_H
#define VTOOLSEAMALLOWANCE_H

#include <QGraphicsPathItem>
#include <QObject>
#include <QtGlobal>

#include "../vwidgets/vgrainlineitem.h"
#include "../vwidgets/vtextgraphicsitem.h"
#include "vinteractivetool.h"

class DialogTool;
class VNoBrushScalePathItem;

struct VToolSeamAllowanceInitData : VAbstractToolInitData
{
    VToolSeamAllowanceInitData() = default;

    VPiece detail{};    // NOLINT(misc-non-private-member-variables-in-classes)
    QString width{'0'}; // NOLINT(misc-non-private-member-variables-in-classes)
    QString drawName{}; // NOLINT(misc-non-private-member-variables-in-classes)
};

class VToolSeamAllowance : public VInteractiveTool, public QGraphicsPathItem
{
    Q_OBJECT // NOLINT

public:
    ~VToolSeamAllowance() override = default;

    static auto Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolSeamAllowance *;
    static auto Create(VToolSeamAllowanceInitData &initData) -> VToolSeamAllowance *;
    static auto Duplicate(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc)
        -> VToolSeamAllowance *;
    static auto Duplicate(VToolSeamAllowanceInitData &initData) -> VToolSeamAllowance *;

    static const quint8 pieceVersion;

    static const QString TagCSA;
    static const QString TagRecord;
    static const QString TagIPaths;
    static const QString TagPins;
    static const QString TagPlaceLabels;

    static const QString AttrSeamAllowance;
    static const QString AttrHideMainPath;
    static const QString AttrSeamAllowanceBuiltIn;
    static const QString AttrUnited;
    static const QString AttrFont;
    static const QString AttrTopLeftPin;
    static const QString AttrBottomRightPin;
    static const QString AttrCenterPin;
    static const QString AttrTopPin;
    static const QString AttrBottomPin;
    static const QString AttrPiecePriority;

    void RemoveWithConfirm(bool ask);

    static void InsertNodes(const QVector<VPieceNode> &nodes, quint32 pieceId, VMainGraphicsScene *scene,
                            VContainer *data, VAbstractPattern *doc);

    static void AddAttributes(VAbstractPattern *doc, QDomElement &domElement, quint32 id, const VPiece &piece);
    static void AddCSARecord(VAbstractPattern *doc, QDomElement &domElement, CustomSARecord record);
    static void AddCSARecords(VAbstractPattern *doc, QDomElement &domElement, const QVector<CustomSARecord> &records);
    static void AddInternalPaths(VAbstractPattern *doc, QDomElement &domElement, const QVector<quint32> &paths);
    static void AddPins(VAbstractPattern *doc, QDomElement &domElement, const QVector<quint32> &pins);
    static void AddPlaceLabels(VAbstractPattern *doc, QDomElement &domElement, const QVector<quint32> &placeLabels);
    static void AddPatternPieceData(VAbstractPattern *doc, QDomElement &domElement, const VPiece &piece);
    static void AddPatternInfo(VAbstractPattern *doc, QDomElement &domElement, const VPiece &piece);
    static void AddGrainline(VAbstractPattern *doc, QDomElement &domElement, const VPiece &piece);

    void Move(qreal x, qreal y);
    void Update(const VPiece &piece);
    void DisconnectOutsideSignals();
    void ConnectOutsideSignals();
    void ReinitInternals(const VPiece &detail, VMainGraphicsScene *scene);
    void RefreshGeometry(bool updateChildren = true);

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Tool::Piece)
    };

    auto getTagName() const -> QString override;
    void ShowVisualization(bool show) override;
    void GroupVisibility(quint32 object, bool visible) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    auto boundingRect() const -> QRectF override;
    auto shape() const -> QPainterPath override;
public slots:
    void FullUpdateFromGuiApply() override;
    void FullUpdateFromFile() override;
    void EnableToolMove(bool move);
    void AllowHover(bool enabled) override;
    void AllowSelecting(bool enabled) override;
    virtual void ResetChildren(QGraphicsItem *pItem);
    virtual void UpdateAll();
    virtual void retranslateUi();
    void Highlight(quint32 id);
    void UpdateDetailLabel();
    void UpdatePatternInfo();
    void UpdatePassmarks();
    void ShowOptions();
    void DeleteFromMenu();
protected slots:
    void UpdateGrainline();
    void SaveMoveDetail(const QPointF &ptPos);
    void SaveResizeDetail(qreal dLabelW);
    void SaveRotationDetail(qreal dRot);
    void SaveMovePattern(const QPointF &ptPos);
    void SaveResizePattern(qreal dLabelW);
    void SaveRotationPattern(qreal dRot);
    void SaveMoveGrainline(const QPointF &ptPos);
    void SaveResizeGrainline(qreal dLength);
    void SaveRotateGrainline(qreal dRot, const QPointF &ptPos);

protected:
    void AddToFile() override;
    void RefreshDataInFile() override;
    auto itemChange(GraphicsItemChange change, const QVariant &value) -> QVariant override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void SetVisualization() override {}
    void DeleteToolWithConfirm(bool ask = true) override;
    void ToolCreation(const Source &typeCreation) override;
    void SetDialog() final;
    void SaveDialogChange(const QString &undoText = QString()) final;

private slots:
    void ToggleInLayout(bool checked);
    void ToggleHideMainPath(bool checked);
    void ToggleForbidFlipping(bool checked);
    void ToggleForceFlipping(bool checked);
    void ToggleExcludeState(quint32 id);
    void ToggleTurnPointState(quint32 id);
    void ToggleNodePointAngleType(quint32 id, PieceNodeAngle type);
    void ToggleNodePointPassmark(quint32 id, bool toggle);
    void TogglePassmarkAngleType(quint32 id, PassmarkAngleType type);
    void TogglePassmarkLineType(quint32 id, PassmarkLineType type);
    void ResetPieceLabelTemplate();

private:
    Q_DISABLE_COPY_MOVE(VToolSeamAllowance) // NOLINT

    QPainterPath m_mainPath{}; // Must be first to prevent crash
    QRectF m_pieceBoundingRect{};

    /** @brief sceneDetails pointer to the scene. */
    VMainGraphicsScene *m_sceneDetails;
    QString m_drawName;

    VNoBrushScalePathItem *m_seamAllowance;
    VTextGraphicsItem *m_dataLabel;
    VTextGraphicsItem *m_patternInfo;
    VGrainlineItem *m_grainLine;
    QGraphicsPathItem *m_passmarks;
    QGraphicsPathItem *m_placeLabels;

    bool m_acceptHoverEvents{true};

    /** @brief m_geometryIsReady is true when a piece's geometry is ready and checks for validity can be enabled. */
    bool m_geometryIsReady{false};

    explicit VToolSeamAllowance(const VToolSeamAllowanceInitData &initData, QGraphicsItem *parent = nullptr);

    void UpdateExcludeState();
    void UpdateInternalPaths();

    auto FindLabelGeometry(const VPatternLabelData &labelData, const QVector<quint32> &pins, qreal &rotationAngle,
                           qreal &labelWidth, qreal &labelHeight, QPointF &pos) -> VPieceItem::MoveTypes;
    auto FindGrainlineGeometry(const VGrainlineData &geom, const QVector<quint32> &pins, qreal &length,
                               qreal &rotationAngle, QPointF &pos) -> VPieceItem::MoveTypes;

    void InitNodes(const VPiece &detail, VMainGraphicsScene *scene);
    static void InitNode(const VPieceNode &node, VMainGraphicsScene *scene, VToolSeamAllowance *parent);
    void InitCSAPaths(const VPiece &detail) const;
    void InitInternalPaths(const VPiece &detail);
    void InitSpecialPoints(const QVector<quint32> &points) const;

    auto PrepareLabelData(const VPatternLabelData &labelData, const QVector<quint32> &pins,
                          VTextGraphicsItem *labelItem, QPointF &pos, qreal &labelAngle) -> bool;

    auto SelectedTools() const -> QList<VToolSeamAllowance *>;

    auto IsGrainlinePositionValid() const -> bool;

    static void AddPointRecords(VAbstractPattern *doc, QDomElement &domElement, const QVector<quint32> &records,
                                const QString &tag);

    static auto DuplicateNodes(const VPiecePath &path, const VToolSeamAllowanceInitData &initData,
                               QMap<quint32, quint32> &replacements) -> QVector<VPieceNode>;
    static auto DuplicateNode(const VPieceNode &node, const VToolSeamAllowanceInitData &initData) -> quint32;

    static auto DuplicatePiecePath(quint32 id, const VToolSeamAllowanceInitData &initData) -> quint32;

    static auto DuplicateCustomSARecords(const QVector<CustomSARecord> &records,
                                         const VToolSeamAllowanceInitData &initData,
                                         const QMap<quint32, quint32> &replacements) -> QVector<CustomSARecord>;

    static auto DuplicateInternalPaths(const QVector<quint32> &iPaths, const VToolSeamAllowanceInitData &initData)
        -> QVector<quint32>;
    static auto DuplicatePlaceLabels(const QVector<quint32> &placeLabels, const VToolSeamAllowanceInitData &initData)
        -> QVector<quint32>;
};

#endif // VTOOLSEAMALLOWANCE_H
