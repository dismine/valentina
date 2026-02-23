/************************************************************************
 **
 **  @file   vabstractpattern.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 6, 2015
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

#ifndef VABSTRACTPATTERN_H
#define VABSTRACTPATTERN_H

#include <QHash>
#include <QMap>
#include <QMetaObject>
#include <QMutex>
#include <QObject>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QUuid>
#include <QVector>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "vdomdocument.h"
#include "vtoolrecord.h"

class QDomElement;
class VPiecePath;
class VPieceNode;
class VPatternImage;
class VBackgroundPatternImage;
class VPatternBlockMapper;

enum class Document : qint8
{
    FullLiteParse,
    LiteParse,
    LitePPParse,
    FullParse
};
enum class LabelType : qint8
{
    NewPatternPiece,
    NewLabel
};

// Don't touch values!!!. Same values stored in xml.
enum class CrossCirclesPoint : qint8
{
    FirstPoint = 1,
    SecondPoint = 2
};
enum class VCrossCurvesPoint : qint8
{
    HighestPoint = 1,
    LowestPoint = 2
};
enum class HCrossCurvesPoint : qint8
{
    LeftmostPoint = 1,
    RightmostPoint = 2
};
enum class AxisType : qint8
{
    VerticalAxis = 1,
    HorizontalAxis = 2
};

class VContainer;
class VDataTool;
class VPatternGraph;

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")

struct VFormulaField
{
    QString expression{};  // NOLINT(misc-non-private-member-variables-in-classes)
    QDomElement element{}; // NOLINT(misc-non-private-member-variables-in-classes)
    QString attribute{};   // NOLINT(misc-non-private-member-variables-in-classes)
};

struct VFinalMeasurement
{
    QString name{};
    QString formula{};
    QString description{};
};

struct VGroupData
{
    QString name{};
    bool visible{true};
    QStringList tags{};
    vidtype tool{NULL_ID};
    QVector<QPair<vidtype, vidtype>> items{};
};

QT_WARNING_POP

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")

class VAbstractPattern : public VDomDocument
{
    Q_OBJECT // NOLINT

public:
    explicit VAbstractPattern(QObject *parent = nullptr);
    ~VAbstractPattern() override;

    auto RequiresMeasurements() const -> bool;
    auto ListMeasurements() const -> QStringList;
    auto ListExpressions() const -> QVector<VFormulaField>;
    auto ListIncrementExpressions() const -> QVector<VFormulaField>;

    virtual void CreateEmptyFile() = 0;

    auto CountPatternBlockTags() const -> int;

    auto GetActivNodeElement(const QString &name, QDomElement &element) const -> bool;

    void setXMLContent(const QString &fileName) override;

    virtual auto GenerateLabel(const LabelType &type, const QString &reservedName = QString()) const -> QString = 0;

    virtual void UpdateToolData(const quint32 &id, VContainer *data) = 0;

    virtual void Clear();

    static auto getTool(quint32 id) -> VDataTool *;
    static void AddTool(quint32 id, VDataTool *tool);
    static void RemoveTool(quint32 id);

    static auto ParsePieceNodes(const QDomElement &domElement) -> VPiecePath;
    static auto ParsePieceCSARecords(const QDomElement &domElement) -> QVector<CustomSARecord>;
    static auto ParsePieceInternalPaths(const QDomElement &domElement) -> QVector<quint32>;
    static auto ParsePiecePointRecords(const QDomElement &domElement) -> QVector<quint32>;

    void AddToolOnRemove(VDataTool *tool);

    auto getHistory() -> QVector<VToolRecord> *;
    auto getHistory() const -> const QVector<VToolRecord> *;
    auto GetLocalHistory(int blockIndex = -1) const -> QVector<VToolRecord>;

    auto MPath() const -> QString;
    void SetMPath(const QString &path);

    auto SiblingNodeId(const quint32 &nodeId) const -> quint32;

    auto GetDescription() const -> QString;
    void SetDescription(const QString &text);

    auto GetNotes() const -> QString;
    void SetNotes(const QString &text);

    auto GetPatternName() const -> QString;
    void SetPatternName(const QString &qsName);

    auto GetCompanyName() const -> QString;
    void SetCompanyName(const QString &qsName);

    auto GetPatternNumber() const -> QString;
    void SetPatternNumber(const QString &qsNum);

    auto GetCustomerName() const -> QString;
    void SetCustomerName(const QString &qsName);

    auto GetCustomerBirthDate() const -> QDate;
    void SetCustomerBirthDate(const QDate &date);

    auto GetCustomerEmail() const -> QString;
    void SetCustomerEmail(const QString &email);

    auto GetLabelDateFormat() const -> QString;
    void SetLabelDateFormat(const QString &format);

    auto GetLabelTimeFormat() const -> QString;
    void SetLabelTimeFormat(const QString &format);

    void SetPatternLabelTemplate(const QVector<VLabelTemplateLine> &lines);
    auto GetPatternLabelTemplate() const -> QVector<VLabelTemplateLine>;

    auto SetWatermarkPath(const QString &path) -> bool;
    auto GetWatermarkPath() const -> QString;

    void SetPatternMaterials(const QMap<int, QString> &materials);
    auto GetPatternMaterials() const -> QMap<int, QString>;

    auto GetFinalMeasurements() const -> QVector<VFinalMeasurement>;
    void SetFinalMeasurements(const QVector<VFinalMeasurement> &measurements);

    auto GetDefaultPieceLabelPath() const -> QString;
    void SetDefaultPieceLabelPath(const QString &path);

    void SetPatternWasChanged(bool changed);
    auto GetPatternWasChanged() const -> bool;

    auto GetPassmarkLengthVariable() const -> QString;
    void SetPassmarkLengthVariable(const QString &name);

    auto GetPassmarkWidthVariable() const -> QString;
    void SetPassmarkWidthVariable(const QString &name);

    auto GetImage() const -> VPatternImage;
    auto SetImage(const VPatternImage &image) -> bool;
    void DeleteImage();

    auto GetBackgroundImages() const -> QVector<VBackgroundPatternImage>;
    void SaveBackgroundImages(const QVector<VBackgroundPatternImage> &images);
    auto GetBackgroundImage(const QUuid &id) const -> VBackgroundPatternImage;
    void SaveBackgroundImage(const VBackgroundPatternImage &image);
    void DeleteBackgroundImage(const QUuid &id);

    auto GetVersion() const -> QString;
    void SetVersion();

    auto IsModified() const -> bool;
    void SetModified(bool modified);

    void ParseGroups(const QDomElement &domElement);
    auto CreateGroups(const QString &patternPieceName = QString()) -> QDomElement;
    auto CreateGroup(quint32 id, const QString &name, const QStringList &tags, const QMap<quint32, quint32> &groupData,
                     vidtype tool = null_id) -> QDomElement;
    auto GroupLinkedToTool(vidtype toolId) const -> vidtype;

    auto GetGroupName(quint32 id) -> QString;
    void SetGroupName(quint32 id, const QString &name);

    auto GetGroupTags(vidtype id) -> QStringList;
    void SetGroupTags(quint32 id, const QStringList &tags);

    auto GetDimensionAValue() -> double;
    void SetDimensionAValue(double value);

    auto GetDimensionBValue() -> double;
    void SetDimensionBValue(double value);

    auto GetDimensionCValue() -> double;
    void SetDimensionCValue(double value);

    auto GetGroupCategories() const -> QStringList;

    auto GetGroups(const QString &patternPieceName = QString()) -> QMap<quint32, VGroupData>;
    auto GetGroupsContainingItem(quint32 toolId, quint32 objectId, bool containItem) -> QMap<quint32, QString>;
    auto AddItemToGroup(quint32 toolId, quint32 objectId, quint32 groupId) -> QDomElement;
    auto RemoveItemFromGroup(quint32 toolId, quint32 objectId, quint32 groupId) -> QDomElement;
    auto GetGroupVisibility(quint32 id) -> bool;

    static auto FilterGroupTags(const QString &tags) -> QStringList;

    auto PieceDrawName(quint32 id) -> QString;

    virtual auto GetCompleteData() const -> VContainer;
    virtual auto GetCompletePPData(const QString &name) const -> VContainer;

    auto Units() const -> Unit;

    static const QString TagPattern;
    static const QString TagCalculation;
    static const QString TagModeling;
    static const QString TagDetails;
    static const QString TagDetail;
    static const QString TagDescription;
    static const QString TagImage;
    static const QString TagNotes;
    static const QString TagMeasurements;
    static const QString TagIncrements;
    static const QString TagPreviewCalculations;
    static const QString TagIncrement;
    static const QString TagDraw;
    static const QString TagGroups;
    static const QString TagGroup;
    static const QString TagGroupItem;
    static const QString TagPoint;
    static const QString TagSpline;
    static const QString TagArc;
    static const QString TagElArc;
    static const QString TagTools;
    static const QString TagOperation;
    static const QString TagData;
    static const QString TagPatternInfo;
    static const QString TagPatternName;
    static const QString TagPatternNum;
    static const QString TagCompanyName;
    static const QString TagCustomerName;
    static const QString TagCustomerBirthDate;
    static const QString TagCustomerEmail;
    static const QString TagPatternLabel;
    static const QString TagWatermark;
    static const QString TagPatternMaterials;
    static const QString TagFinalMeasurements;
    static const QString TagMaterial;
    static const QString TagFMeasurement;
    static const QString TagGrainline;
    static const QString TagPath;
    static const QString TagNodes;
    static const QString TagNode;
    static const QString TagBackgroundImages;
    static const QString TagBackgroundImage;
    static const QString TagPieceLabel;
    static const QString TagOffset;
    static const QString TagMirrorLine;

    static const QString AttrName;
    static const QString AttrEnabled;
    static const QString AttrVisible;
    static const QString AttrObject;
    static const QString AttrTool;
    static const QString AttrType;
    static const QString AttrLetter;
    static const QString AttrAnnotation;
    static const QString AttrOrientation;
    static const QString AttrRotationWay;
    static const QString AttrTilt;
    static const QString AttrFoldPosition;
    static const QString AttrQuantity;
    static const QString AttrOnFold;
    static const QString AttrDateFormat;
    static const QString AttrTimeFormat;
    static const QString AttrArrows;
    static const QString AttrNodeReverse;
    static const QString AttrNodeExcluded;
    static const QString AttrNodePassmark;
    static const QString AttrNodePassmarkLine;
    static const QString AttrNodePassmarkAngle;
    static const QString AttrNodeShowSecondPassmark;
    static const QString AttrNodePassmarkOpening;
    static const QString AttrNodeTurnPoint;
    static const QString AttrSABefore;
    static const QString AttrSAAfter;
    static const QString AttrStart;
    static const QString AttrPath;
    static const QString AttrEnd;
    static const QString AttrIncludeAs;
    static const QString AttrRotation;
    static const QString AttrNumber;
    static const QString AttrCheckUniqueness;
    static const QString AttrManualPassmarkLength;
    static const QString AttrPassmarkLength;
    static const QString AttrManualPassmarkWidth;
    static const QString AttrPassmarkWidth;
    static const QString AttrManualPassmarkAngle;
    static const QString AttrPassmarkAngle;
    static const QString AttrOpacity;
    static const QString AttrTags;
    static const QString AttrTransform;
    static const QString AttrHold;
    static const QString AttrZValue;
    static const QString AttrImageId;
    static const QString AttrDimensionA;
    static const QString AttrDimensionB;
    static const QString AttrDimensionC;
    static const QString AttrMirrorLineP1;
    static const QString AttrMirrorLineP2;
    static const QString AttrMirrorLineVisible;
    static const QString AttrFoldLineHeightFormula;
    static const QString AttrFoldLineWidthFormula;
    static const QString AttrFoldLineCenterFormula;
    static const QString AttrFoldLineManualHeight;
    static const QString AttrFoldLineManualWidth;
    static const QString AttrFoldLineManualCenter;
    static const QString AttrFoldLineType;
    static const QString AttrFoldLineFontSize;
    static const QString AttrFoldLineLabel;

    static const QString AttrContentType;

    static const QString AttrFormula;
    static const QString AttrSpecialUnits;

    static const QString NodeArc;
    static const QString NodeElArc;
    static const QString NodePoint;
    static const QString NodeSpline;
    static const QString NodeSplinePath;

    auto PatternGraph() const -> VPatternGraph *;

    void FindFormulaDependencies(const QString &formula, quint32 id, const QHash<QString, QList<quint32>> &variables);

    auto IsPatternGraphComplete() const -> bool;

    auto PatternBlockMapper() const -> VPatternBlockMapper *;

signals:
    /**
     * @brief FullUpdateFromFile update tool data form file.
     */
    void FullUpdateFromFile();
    /**
     * @brief patternChanged emit if we have unsaved change.
     */
    void patternChanged(bool saved);
    void UpdatePatternLabel();
    void ClearMainWindow();
    void UndoCommand();
    void SetEnabledGUI(bool enabled);
    void CheckLayout();
    void UpdateInLayoutList();
    void ShowDetail(quint32 id);
    void ShowPatternBlock(const QString &patterBlockName);
    void MadeProgress();
    /**
     * @brief UpdateGroups emit if the groups have been updated
     */
    void UpdateGroups();
    void UpdateToolTip();

    void BackgroundImageTransformationChanged(QUuid id);
    void BackgroundImagesHoldChanged();
    void BackgroundImageHoldChanged(const QUuid &id);
    void BackgroundImageVisibilityChanged(const QUuid &id);
    void BackgroundImagesVisibilityChanged();
    void BackgroundImageNameChanged(const QUuid &id);
    void BackgroundImagesZValueChanged();
    void BackgroundImagePositionChanged(const QUuid &id);
    void BackgroundImageOpacityChanged(const QUuid &id);

    void CancelLabelRendering();

    void PatternDependencyGraphCompleted();

public slots:
    virtual void LiteParseTree(const Document &parse) = 0;
    void haveLiteChange();
    void NeedFullParsing();
    void ClearScene();
    void CheckInLayoutList();
    void SelectedDetail(quint32 id);
    void UpdateVisiblityGroups();

protected slots:
    void CancelFormulaDependencyChecks();

protected:
    QVector<VDataTool *> toolsOnRemove;

    /** @brief history history records. */
    QVector<VToolRecord> history;

    /** @brief modified keep state of the document for cases that do not cover QUndoStack*/
    mutable bool modified;

    Unit m_units{Unit::LAST_UNIT_DO_NOT_USE};
    QString m_patternNumber{};
    QString m_labelDateFormat{};
    QString m_patternName{};
    QString m_MPath{};
    QString m_watermarkPath{};
    QString m_companyName{};

    bool m_fileParsingCompleted{true};

    /** @brief tools list with pointer on tools. */
    static QHash<quint32, VDataTool *> tools;
    /** @brief patternLabelLines list to speed up reading a template by many pieces. */
    static QVector<VLabelTemplateLine> patternLabelLines;
    /** @brief patternMaterials list to speed up reading materials by many pieces. */
    static QMap<int, QString> patternMaterials;
    static bool patternLabelWasChanged;

    static void ToolExists(const quint32 &id);
    static auto ParsePathNodes(const QDomElement &domElement) -> VPiecePath;
    static auto ParseSANode(const QDomElement &domElement) -> VPieceNode;

    auto CheckTagExists(const QString &tag) -> QDomElement;
    void InsertTag(const QStringList &tags, const QDomElement &element);

    void SetChildTag(const QString &qsParent, const QString &qsChild, const QString &qsValue);

    auto GroupHasItem(const QDomElement &groupDomElement, quint32 toolId, quint32 objectId) -> bool;

    auto ReadUnits() const -> Unit;
    auto ReadPatternNumber() const -> QString;
    auto ReadLabelDateFormat() const -> QString;
    auto ReadPatternName() const -> QString;
    auto ReadMPath() const -> QString;
    auto ReadWatermarkPath() const -> QString;
    auto ReadCompanyName() const -> QString;

private slots:
    void CleanDependenciesWatcher();

private:
    Q_DISABLE_COPY_MOVE(VAbstractPattern) // NOLINT

    VPatternGraph *m_patternGraph;

    VPatternBlockMapper *m_patternBlockMapper;

    QList<QFutureWatcher<void> *> m_formulaDependenciesWatchers{};
    mutable QMutex m_watchersMutex{};

    auto ListIncrements() const -> QStringList;
    auto ListPointExpressions() const -> QVector<VFormulaField>;
    auto ListArcExpressions() const -> QVector<VFormulaField>;
    auto ListElArcExpressions() const -> QVector<VFormulaField>;
    auto ListSplineExpressions() const -> QVector<VFormulaField>;
    auto ListOperationExpressions() const -> QVector<VFormulaField>;
    auto ListNodesExpressions(const QDomElement &nodes) const -> QVector<VFormulaField>;
    auto ListPathExpressions() const -> QVector<VFormulaField>;
    auto ListGrainlineExpressions(const QDomElement &element) const -> QVector<VFormulaField>;
    auto ListPieceExpressions() const -> QVector<VFormulaField>;
    auto ListFinalMeasurementsExpressions() const -> QVector<VFormulaField>;

    static auto IsVariable(const QString &token) -> bool;
    static auto IsFunction(const QString &token) -> bool;

    auto ParseItemElement(const QDomElement &domElement) -> QPair<bool, QMap<quint32, quint32>>;

    auto GetMaterials(const QDomElement &element) const -> QMap<int, QString>;
    void SetMaterials(QDomElement &element, const QMap<int, QString> &materials);

    auto GetFMeasurements(const QDomElement &element) const -> QVector<VFinalMeasurement>;
    void SetFMeasurements(QDomElement &element, const QVector<VFinalMeasurement> &measurements);

    auto GetBackgroundPatternImage(const QDomElement &element) const -> VBackgroundPatternImage;
    auto GetBackgroundImageElement(const QUuid &id) const -> QDomElement;
    void WriteBackgroundImage(QDomElement &element, const VBackgroundPatternImage &image);

    auto GetPatternVariable(const QString &attribute) const -> QString;
    void SePatternVariable(const QString &attribute, const QString &name);
};

QT_WARNING_POP

#endif // VABSTRACTPATTERN_H
