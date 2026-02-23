/************************************************************************
 **
 **  @file   vpattern.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   24 2, 2014
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

#include "vpattern.h"
#include "../core/vapplication.h"
#include "../ifc/exception/vexceptionconversionerror.h"
#include "../ifc/exception/vexceptionemptyparameter.h"
#include "../ifc/exception/vexceptionobjecterror.h"
#include "../ifc/exception/vexceptionundo.h"
#include "../ifc/exception/vexceptionwrongid.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatternconverter.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../qmuparser/qmuparsererror.h"
#include "../qmuparser/qmutokenparser.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vcubicbezier.h"
#include "../vgeometry/vcubicbezierpath.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vsplinepath.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/customevents.h"
#include "../vmisc/def.h"
#include "../vmisc/projectversion.h"
#include "../vmisc/vsysexits.h"
#include "../vmisc/vvalentinasettings.h"
#include "../vpatterndb/calculator.h"
#include "../vpatterndb/floatItemData/vgrainlinedata.h"
#include "../vpatterndb/floatItemData/vpatternlabeldata.h"
#include "../vpatterndb/floatItemData/vpiecelabeldata.h"
#include "../vpatterndb/variables/vincrement.h"
#include "../vpatterndb/vnodedetail.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vpatterndb/vpiecepath.h"
#include "../vtools/tools/drawTools/operation/flipping/vtoolflippingbyaxis.h"
#include "../vtools/tools/drawTools/operation/flipping/vtoolflippingbyline.h"
#include "../vtools/tools/drawTools/operation/vtoolmove.h"
#include "../vtools/tools/drawTools/operation/vtoolrotation.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolarc.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolarcwithlength.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolcubicbezier.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolcubicbezierpath.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolellipticalarc.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolellipticalarcwithlength.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolspline.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolsplinepath.h"
#include "../vtools/tools/drawTools/toolpoint/tooldoublepoint/vtooltruedarts.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcutarc.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcutspline.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcutsplinepath.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolalongline.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolbisector.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolcurveintersectaxis.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolendline.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolheight.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoollineintersectaxis.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolnormal.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolshoulderpoint.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolbasepoint.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoollineintersect.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointfromarcandtangent.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointfromcircleandtangent.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofcontact.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofintersection.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofintersectionarcs.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofintersectioncircles.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofintersectioncurves.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtooltriangle.h"
#include "../vtools/tools/drawTools/vtoolline.h"
#include "../vtools/tools/nodeDetails/vnodearc.h"
#include "../vtools/tools/nodeDetails/vnodeellipticalarc.h"
#include "../vtools/tools/nodeDetails/vnodepoint.h"
#include "../vtools/tools/nodeDetails/vnodespline.h"
#include "../vtools/tools/nodeDetails/vnodesplinepath.h"
#include "../vtools/tools/nodeDetails/vtoolpiecepath.h"
#include "../vtools/tools/nodeDetails/vtoolpin.h"
#include "../vtools/tools/nodeDetails/vtoolplacelabel.h"
#include "../vtools/tools/vdatatool.h"
#include "../vtools/tools/vtoolseamallowance.h"
#include "../vtools/tools/vtooluniondetails.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "tools/drawTools/toolcurve/vtoolgraduatedcurve.h"
#include "tools/drawTools/toolcurve/vtoolparallelcurve.h"
#include "typedef.h"

#include <chrono>
#include <functional>
#include <qstringliteral.h>
#include <QDebug>
#include <QFileInfo>
#include <QFuture>
#include <QFutureWatcher>
#include <QMessageBox>
#include <QScopeGuard>
#include <QTimer>
#include <QUndoStack>
#include <QtConcurrentMap>
#include <QtConcurrentRun>
#include <QtNumeric>
#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif /*Q_OS_WIN*/

using namespace std::chrono_literals;

using namespace Qt::Literals::StringLiterals;

const QString VPattern::AttrReadOnly = QStringLiteral("readOnly");       // NOLINT(cert-err58-cpp)
const QString VPattern::AttrLabelPrefix = QStringLiteral("labelPrefix"); // NOLINT(cert-err58-cpp)

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto FileComment() -> QString
{
    return QStringLiteral("Pattern created with Valentina v%1 (https://smart-pattern.com.ua/).").arg(AppVersionStr());
}

//---------------------------------------------------------------------------------------------------------------------
void GatherCount(int &count, const int nodes)
{
    count += nodes;
}

//---------------------------------------------------------------------------------------------------------------------
auto DefLabelLanguage() -> QString
{
    QString def = VAbstractValApplication::VApp()->ValentinaSettings()->GetLabelLanguage();
    if (not ConvertToSet<QString>(VApplication::LabelLanguages()).contains(def))
    {
        def = QStringLiteral("en");
    }
    return def;
}

struct VPieceFoldLineData
{
    quint32 p1{NULL_ID};
    quint32 p2{NULL_ID};
    bool mirrorLineVisible{true};
    QString heightFormula{};
    QString widthFormula{};
    QString centerFormula{};
    bool manualHeight{false};
    bool manualWidth{false};
    bool manualCenter{false};
    FoldLineType type{FoldLineType::TwoArrowsTextAbove};
    unsigned int fontSize{defFoldLineFontSize};
    bool italic{false};
    bool bold{false};
    QString label{};
    int alignment{Qt::AlignHCenter};
};

//---------------------------------------------------------------------------------------------------------------------
auto ParsePieceMirrorLine(const QDomElement &domElement) -> VPieceFoldLineData
{
    return {
        .p1 = VDomDocument::GetParametrUInt(domElement, VAbstractPattern::AttrMirrorLineP1, NULL_ID_STR),
        .p2 = VDomDocument::GetParametrUInt(domElement, VAbstractPattern::AttrMirrorLineP2, NULL_ID_STR),
        .mirrorLineVisible = VDomDocument::GetParametrBool(domElement, VAbstractPattern::AttrMirrorLineVisible, trueStr),
        .heightFormula = VDomDocument::GetParametrEmptyString(domElement, VAbstractPattern::AttrFoldLineHeightFormula),
        .widthFormula = VDomDocument::GetParametrEmptyString(domElement, VAbstractPattern::AttrFoldLineWidthFormula),
        .centerFormula = VDomDocument::GetParametrEmptyString(domElement, VAbstractPattern::AttrFoldLineCenterFormula),
        .manualHeight = VDomDocument::GetParametrBool(domElement, VAbstractPattern::AttrFoldLineManualHeight, falseStr),
        .manualWidth = VDomDocument::GetParametrBool(domElement, VAbstractPattern::AttrFoldLineManualWidth, falseStr),
        .manualCenter = VDomDocument::GetParametrBool(domElement, VAbstractPattern::AttrFoldLineManualCenter, falseStr),
        .type = StringToFoldLineType(
            VDomDocument::GetParametrString(domElement,
                                            VAbstractPattern::AttrFoldLineType,
                                            FoldLineTypeToString(FoldLineType::TwoArrowsTextAbove))),
        .fontSize = qMax(static_cast<unsigned int>(VCommonSettings::MinPieceLabelFontPointSize()),
                         VDomDocument::GetParametrUInt(domElement,
                                                       VAbstractPattern::AttrFoldLineFontSize,
                                                       QString::number(defFoldLineFontSize))),
        .italic = VDomDocument::GetParametrBool(domElement, VDomDocument::AttrItalic, falseStr),
        .bold = VDomDocument::GetParametrBool(domElement, VDomDocument::AttrBold, falseStr),
        .label = VDomDocument::GetParametrEmptyString(domElement, VAbstractPattern::AttrFoldLineLabel),
        .alignment = VDomDocument::GetParametrInt(domElement,
                                                  VDomDocument::AttrAlignment,
                                                  QString::number(Qt::AlignHCenter))};
}
} // anonymous namespace

//---------------------------------------------------------------------------------------------------------------------
VPattern::VPattern(VContainer *data, VMainGraphicsScene *sceneDraw, VMainGraphicsScene *sceneDetail, QObject *parent)
  : VAbstractPattern(parent),
    data(data),
    sceneDraw(sceneDraw),
    sceneDetail(sceneDetail),
    m_refreshPieceGeometryWatcher(new QFutureWatcher<void>(this))
{
    SCASSERT(sceneDraw != nullptr)
    SCASSERT(sceneDetail != nullptr)

    connect(qApp,
            &QCoreApplication::aboutToQuit,
            m_refreshPieceGeometryWatcher,
            [this]() -> void
            {
                m_refreshPieceGeometryWatcher->cancel();
                m_refreshPieceGeometryWatcher->waitForFinished();
            });

    connect(this, &VAbstractPattern::PatternDependencyGraphCompleted, this, &VPattern::CollectGarbage);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief CreateEmptyFile create minimal empty file.
 */
void VPattern::CreateEmptyFile()
{
    this->clear();
    QDomElement patternElement = this->createElement(TagPattern);
    SetAttribute(patternElement, AttrLabelPrefix, DefLabelLanguage());

    patternElement.appendChild(createComment(FileComment()));
    patternElement.appendChild(CreateElementWithText(TagVersion, VPatternConverter::PatternMaxVerStr));
    patternElement.appendChild(
        CreateElementWithText(TagUnit, UnitsToStr(VAbstractValApplication::VApp()->patternUnits())));

    patternElement.appendChild(createElement(TagDescription));
    patternElement.appendChild(createElement(TagNotes));

    patternElement.appendChild(createElement(TagMeasurements));
    patternElement.appendChild(createElement(TagIncrements));
    patternElement.appendChild(createElement(TagPreviewCalculations));

    this->appendChild(patternElement);
    insertBefore(
        createProcessingInstruction(QStringLiteral("xml"), QStringLiteral("version=\"1.0\" encoding=\"UTF-8\"")),
        this->firstChild());

    // Cache values
    m_units = VAbstractValApplication::VApp()->patternUnits();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Parse parse file.
 * @param parse parser file mode.
 */
void VPattern::Parse(const Document &parse)
{
    qCDebug(vXML, "Parsing pattern.");
    switch (parse)
    {
        case Document::FullParse:
            qCDebug(vXML, "Full parse.");
            break;
        case Document::LiteParse:
            qCDebug(vXML, "Lite parse.");
            break;
        case Document::FullLiteParse:
            qCDebug(vXML, "Full lite parse.");
            break;
        case Document::LitePPParse:
            qCDebug(vXML, "Lite pattern piece parse.");
            break;
        default:
            break;
    }

    emit PreParseState();
    SCASSERT(sceneDraw != nullptr)
    SCASSERT(sceneDetail != nullptr)
    PrepareForParse(parse);

    m_fileParsingCompleted = false;
    QDomNode domNode = documentElement().firstChild();
    while (not domNode.isNull())
    {
        ParseRootElement(parse, domNode);
        domNode = domNode.nextSibling();
    }

    m_fileParsingCompleted = true;
    if (IsPatternGraphComplete())
    {
        emit PatternDependencyGraphCompleted();
    }

    if (VApplication::IsGUIMode())
    {
        QTimer::singleShot(1s, Qt::VeryCoarseTimer, this, &VPattern::RefreshPieceGeometry);
    }
    else if (VApplication::CommandLine()->IsTestModeEnabled())
    {
        RefreshPieceGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setCurrentData set current data set.
 *
 * Each time after parsing need set correct data set for current pattern piece. After parsing it is always last.
 * Current data set for pattern pice it is data set for last object in pattern pice (point, arc, spline, spline path so
 * on).
 */
void VPattern::setCurrentData()
{
    const int countPP = CountPatternBlockTags();
    // don't need upadate data if we have only one pattern piece
    if (VAbstractValApplication::VApp()->GetDrawMode() != Draw::Calculation || countPP <= 1)
    {
        return;
    }

    qCDebug(vXML, "Setting current data");
    qCDebug(vXML, "Current PP name %s", qUtf8Printable(PatternBlockMapper()->GetActive()));
    qCDebug(vXML, "PP count %d", countPP);

    const QVector<VToolRecord> localHistory = GetLocalHistory();
    if (localHistory.isEmpty())
    {
        qCDebug(vXML, "History is empty!");
        return;
    }

    const quint32 id = localHistory.constLast().GetId();
    qCDebug(vXML, "Resoring data from tool with id %u", id);

    if (tools.isEmpty())
    {
        qCDebug(vXML, "List of tools is empty!");
        return;
    }

    try
    {
        ToolExists(id);
    }
    catch (VExceptionBadId &e)
    {
        Q_UNUSED(e)
        qCDebug(vXML, "List of tools doesn't containe id= %u", id);
        return;
    }

    const VDataTool *vTool = tools.value(id);
    *data = vTool->getData();
    // Delete special variables if exist
    data->RemoveVariable(currentLength);
    data->RemoveVariable(currentSeamAllowance);
    qCDebug(vXML, "Data successfully updated.");
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief UpdateToolData update tool in list tools.
 * @param id tool id.
 * @param data container with variables.
 */
void VPattern::UpdateToolData(const quint32 &id, VContainer *data)
{
    Q_ASSERT_X(id != 0, Q_FUNC_INFO, "id == 0"); //-V712 //-V654
    SCASSERT(data != nullptr)
    ToolExists(id);
    VDataTool *tool = tools.value(id);
    SCASSERT(tool != nullptr)
    tool->VDataTool::setData(data);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPattern::GetCompleteData() const -> VContainer
{
    const int countPP = CountPatternBlockTags();
    if (countPP <= 0 || history.isEmpty() || tools.isEmpty())
    {
        return (data != nullptr ? *data : VContainer(nullptr, nullptr, VContainer::UniqueNamespace()));
    }

    const quint32 id = (countPP == 1 ? history.constLast().GetId() : LastToolId());

    if (id == NULL_ID)
    {
        return (data != nullptr ? *data : VContainer(nullptr, nullptr, VContainer::UniqueNamespace()));
    }

    try
    {
        ToolExists(id);
    }
    catch (VExceptionBadId &e)
    {
        Q_UNUSED(e)
        return (data != nullptr ? *data : VContainer(nullptr, nullptr, VContainer::UniqueNamespace()));
    }

    const VDataTool *vTool = tools.value(id);
    SCASSERT(vTool != nullptr)
    VContainer lastData = vTool->getData();
    // Delete special variables if exist
    lastData.RemoveVariable(currentLength);
    lastData.RemoveVariable(currentSeamAllowance);
    return lastData;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPattern::GetCompletePPData(const QString &name) const -> VContainer
{
    const int countPP = CountPatternBlockTags();
    if (countPP <= 0 || history.isEmpty() || tools.isEmpty())
    {
        return (data != nullptr ? *data : VContainer(nullptr, nullptr, VContainer::UniqueNamespace()));
    }

    const quint32 id = (countPP == 1 ? history.constLast().GetId() : PPLastToolId(PatternBlockMapper()->FindId(name)));

    if (id == NULL_ID)
    {
        return (data != nullptr ? *data : VContainer(nullptr, nullptr, VContainer::UniqueNamespace()));
    }

    try
    {
        ToolExists(id);
    }
    catch (VExceptionBadId &e)
    {
        Q_UNUSED(e)
        return (data != nullptr ? *data : VContainer(nullptr, nullptr, VContainer::UniqueNamespace()));
    }

    const VDataTool *vTool = tools.value(id);
    SCASSERT(vTool != nullptr)
    VContainer lastData = vTool->getData();
    // Delete special variables if exist
    lastData.RemoveVariable(currentLength);
    lastData.RemoveVariable(currentSeamAllowance);
    return lastData;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SPointActiveDraw return id base point current pattern peace.
 * @return id base point.
 */
// cppcheck-suppress unusedFunction
auto VPattern::SPointActiveDraw() -> quint32
{
    if (QDomElement calcElement; GetActivNodeElement(TagCalculation, calcElement))
    {
        if (const QDomNode domNode = calcElement.firstChild(); not domNode.isNull() && domNode.isElement())
        {
            if (const QDomElement domElement = domNode.toElement();
                not domElement.isNull() && domElement.tagName() == TagPoint &&
                domElement.attribute(AttrType, QString()) == VToolBasePoint::ToolType)
            {
                return GetParametrId(domElement);
            }
        }
    }
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPattern::GetActivePPPieces() const -> QVector<quint32>
{
    QDomElement const drawElement = PatternBlockMapper()->GetActiveElement();
    if (drawElement.isNull())
    {
        return {};
    }

    const QDomElement details = drawElement.firstChildElement(TagDetails);
    if (details.isNull())
    {
        return {};
    }

    QVector<quint32> pieces;
    QDomElement detail = details.firstChildElement(TagDetail);
    while (!detail.isNull())
    {
        if (bool const united = GetParametrBool(detail, VToolSeamAllowance::AttrUnited, falseStr); not united)
        {
            pieces.append(GetParametrId(detail));
        }
        detail = detail.nextSiblingElement(TagDetail);
    }

    return pieces;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPattern::SaveDocument(const QString &fileName, QString &error) -> bool
{
    try
    {
        TestUniqueId();
    }
    catch (const VExceptionWrongId &e)
    {
        qCCritical(vXML, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error not unique id.")), qUtf8Printable(e.ErrorMessage()),
                   qUtf8Printable(e.DetailedInformation()));
        return false;
    }

    // Update comment with Valentina version
    if (QDomNode const commentNode = documentElement().firstChild(); commentNode.isComment())
    {
        QDomComment comment = commentNode.toComment();
        comment.setData(FileComment());
    }

    const bool saved = VAbstractPattern::SaveDocument(fileName, error);
    if (saved && QFileInfo(fileName).suffix() != "autosave"_L1)
    {
        modified = false;
    }

    return saved;
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::LiteParseIncrements()
{
    try
    {
        emit SetEnabledGUI(true);

        data->ClearUniqueIncrementNames();

        Q_STATIC_ASSERT_X(static_cast<int>(VarType::Unknown) == 12, "Check that you used all types");
        data->ClearVariables(VarType::Increment);
        data->ClearVariables(VarType::IncrementSeparator);

        QDomNodeList tags = elementsByTagName(TagIncrements);
        if (not tags.isEmpty())
        {
            if (const QDomNode domElement = tags.at(0); not domElement.isNull())
            {
                ParseIncrementsElement(domElement, Document::LiteParse);
            }
        }

        tags = elementsByTagName(TagPreviewCalculations);
        if (not tags.isEmpty())
        {
            if (const QDomNode domElement = tags.at(0); not domElement.isNull())
            {
                ParseIncrementsElement(domElement, Document::LiteParse);
            }
        }
    }
    catch (const VExceptionUndo &e)
    {
        Q_UNUSED(e)
        /* If user want undo last operation before undo we need finish broken redo operation. For those we post event
         * myself. Later in method customEvent call undo.*/
        QApplication::postEvent(this, new UndoEvent());
        return;
    }
    catch (const VExceptionObjectError &e)
    {
        qCCritical(vXML, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error parsing file.")), //-V807
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        emit SetEnabledGUI(false);
        return;
    }
    catch (const VExceptionConversionError &e)
    {
        qCCritical(vXML, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error can't convert value.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        emit SetEnabledGUI(false);
        return;
    }
    catch (const VExceptionEmptyParameter &e)
    {
        qCCritical(vXML, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error empty parameter.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        emit SetEnabledGUI(false);
        return;
    }
    catch (const VExceptionWrongId &e)
    {
        qCCritical(vXML, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error wrong id.")), qUtf8Printable(e.ErrorMessage()),
                   qUtf8Printable(e.DetailedInformation()));
        emit SetEnabledGUI(false);
        return;
    }
    catch (VException &e)
    {
        qCCritical(vXML, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error parsing file.")), qUtf8Printable(e.ErrorMessage()),
                   qUtf8Printable(e.DetailedInformation()));
        emit SetEnabledGUI(false);
        return;
    }
    catch (const std::bad_alloc &)
    {
        qCCritical(vXML, "%s", qUtf8Printable(tr("Error parsing file (std::bad_alloc).")));
        emit SetEnabledGUI(false);
        return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPattern::ElementsToParse() const -> int
{
    QVector<QString> const tags{TagCalculation, TagDetails, TagModeling, TagIncrements};

    std::function<int(const QString &tagName)> const TagsCount = [this](const QString &tagName)
    { return elementsByTagName(tagName).length(); };

    return QtConcurrent::blockingMappedReduced(tags, TagsCount, GatherCount);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief LiteParseTree lite parse file.
 */
void VPattern::LiteParseTree(const Document &parse)
{
    // Save current pattern block name
    QString const namePP = PatternBlockMapper()->GetActive();

    try
    {
        emit SetEnabledGUI(true);
        switch (parse)
        {
            case Document::LitePPParse:
                ParseCurrentPP();
                break;
            case Document::FullLiteParse:
            case Document::LiteParse:
                Parse(parse);
                break;
            case Document::FullParse:
                qCWarning(vXML, "Lite parsing doesn't support full parsing");
                break;
            default:
                break;
        }
    }
    catch (const VExceptionUndo &e)
    {
        Q_UNUSED(e)
        /* If user want undo last operation before undo we need finish broken redo operation. For those we post event
         * myself. Later in method customEvent call undo.*/
        QApplication::postEvent(this, new UndoEvent());
        return;
    }
    catch (const VExceptionObjectError &e)
    {
        qCCritical(vXML, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error parsing file.")), //-V807
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        emit SetEnabledGUI(false);
        if (not VApplication::IsGUIMode())
        {
            QCoreApplication::exit(V_EX_NOINPUT);
        }
        return;
    }
    catch (const VExceptionConversionError &e)
    {
        qCCritical(vXML, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error can't convert value.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        emit SetEnabledGUI(false);
        if (not VApplication::IsGUIMode())
        {
            QCoreApplication::exit(V_EX_NOINPUT);
        }
        return;
    }
    catch (const VExceptionEmptyParameter &e)
    {
        qCCritical(vXML, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error empty parameter.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        emit SetEnabledGUI(false);
        if (not VApplication::IsGUIMode())
        {
            QCoreApplication::exit(V_EX_NOINPUT);
        }
        return;
    }
    catch (const VExceptionWrongId &e)
    {
        qCCritical(vXML, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error wrong id.")), qUtf8Printable(e.ErrorMessage()),
                   qUtf8Printable(e.DetailedInformation()));
        emit SetEnabledGUI(false);
        if (not VApplication::IsGUIMode())
        {
            QCoreApplication::exit(V_EX_NOINPUT);
        }
        return;
    }
    catch (VException &e)
    {
        qCCritical(vXML, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error parsing file.")), qUtf8Printable(e.ErrorMessage()),
                   qUtf8Printable(e.DetailedInformation()));
        emit SetEnabledGUI(false);
        if (not VApplication::IsGUIMode())
        {
            QCoreApplication::exit(V_EX_NOINPUT);
        }
        return;
    }
    catch (const std::bad_alloc &)
    {
        qCCritical(vXML, "%s", qUtf8Printable(tr("Error parsing file (std::bad_alloc).")));
        emit SetEnabledGUI(false);
        if (not VApplication::IsGUIMode())
        {
            QCoreApplication::exit(V_EX_NOINPUT);
        }
        return;
    }

    // Restore current pattern block name
    PatternBlockMapper()->SetActive(namePP);
    qCDebug(vXML, "Current pattern piece %s", qUtf8Printable(PatternBlockMapper()->GetActive()));
    setCurrentData();
    emit FullUpdateFromFile();
    // Recalculate scene rect
    VMainGraphicsView::NewSceneRect(sceneDraw, VAbstractValApplication::VApp()->getSceneView());
    VMainGraphicsView::NewSceneRect(sceneDetail, VAbstractValApplication::VApp()->getSceneView());
    qCDebug(vXML, "Scene size updated.");
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void VPattern::customEvent(QEvent *event)
{
    if (event->type() == UNDO_EVENT)
    {
        VAbstractApplication::VApp()->getUndoStack()->undo();
    }
    else if (event->type() == LITE_PARSE_EVENT)
    {
        LiteParseTree(Document::LiteParse);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::CollectGarbage()
{
    if (!VAbstractValApplication::VApp()->ValentinaSettings()->IsCollectGarbage())
    {
        m_garbageCollected = true;
        return;
    }

    if (!m_garbageCollected)
    {
        GarbageCollector();
        m_garbageCollected = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPattern::IsPieceGeometryDirty() const -> bool
{
    return m_pieceGeometryDirty;
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::SetPieceGeometryDirty(bool newPieceGeometryDirty)
{
    m_pieceGeometryDirty = newPieceGeometryDirty;
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::RefreshDirtyPieceGeometry(const QList<vidtype> &list)
{
    if (!list.isEmpty())
    {
        if (m_refreshPieceGeometryWatcher->isRunning())
        {
            m_refreshPieceGeometryWatcher->cancel();
            m_refreshPieceGeometryWatcher->waitForFinished();
        }

        auto future = QtConcurrent::run(
            [this, list]()
            {
                RefreshPieceGeometryForList(list);
                PostRefreshActions();
            });

        m_refreshPieceGeometryWatcher->setFuture(future);
    }

    m_pieceGeometryDirty = false;
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::SetGBBackupFilePath(const QString &fileName)
{
    m_garbageCollectBackupFilePath = fileName;
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::RefreshPieceGeometryForList(const QList<vidtype> &list) const
{
    for (auto pieceId : std::as_const(list))
    {
        QMetaObject::invokeMethod(
            QApplication::instance(),
            [pieceId]()
            {
                try
                {
                    if (auto *piece = qobject_cast<VToolSeamAllowance *>(VAbstractPattern::getTool(pieceId)))
                    {
                        piece->RefreshGeometry();
                    }
                }
                catch (const VExceptionBadId &)
                {
                    // do nothing
                }
            },
            Qt::QueuedConnection);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::PostRefreshActions()
{
    QMetaObject::invokeMethod(
        QApplication::instance(),
        [this]()
        {
            emit CheckLayout();
            VMainGraphicsView::NewSceneRect(sceneDetail, VAbstractValApplication::VApp()->getSceneView());
        },
        Qt::QueuedConnection);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPattern::ParseDetailNode(const QDomElement &domElement) -> VNodeDetail
{
    const quint32 id = GetParametrUInt(domElement, AttrIdObject, NULL_ID_STR);
    const qreal mx = GetParametrDouble(domElement, AttrMx, QStringLiteral("0.0"));
    const qreal my = GetParametrDouble(domElement, AttrMy, QStringLiteral("0.0"));
    const bool reverse = GetParametrUInt(domElement, VAbstractPattern::AttrNodeReverse, QChar('0'));
    const NodeDetail nodeType = NodeDetail::Contour;

    const QString t = GetParametrString(domElement, AttrType, QStringLiteral("NodePoint"));
    Tool tool;

    switch (QStringList const types{VAbstractPattern::NodePoint, VAbstractPattern::NodeArc,
                                    VAbstractPattern::NodeSpline, VAbstractPattern::NodeSplinePath,
                                    VAbstractPattern::NodeElArc};
            types.indexOf(t))
    {
        case 0: // NodePoint
            tool = Tool::NodePoint;
            break;
        case 1: // NodeArc
            tool = Tool::NodeArc;
            break;
        case 2: // NodeSpline
            tool = Tool::NodeSpline;
            break;
        case 3: // NodeSplinePath
            tool = Tool::NodeSplinePath;
            break;
        case 4: // NodeElArc
            tool = Tool::NodeElArc;
            break;
        default:
            throw VException(tr("Wrong tag name '%1'.").arg(t));
    }
    return {id, tool, nodeType, mx, my, reverse};
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseRootElement(const Document &parse, const QDomNode &node)
{
    static const QStringList tags({TagDraw, TagIncrements, TagPreviewCalculations});

    if (not node.isElement())
    {
        return;
    }

    const QDomElement domElement = node.toElement();
    if (domElement.isNull())
    {
        return;
    }

    switch (tags.indexOf(domElement.tagName()))
    {
        case 0: // TagDraw
        {
            qCDebug(vXML, "Tag draw.");
            VPatternBlockMapper *blocks = PatternBlockMapper();
            if (parse == Document::FullParse)
            {
                int const id = blocks->AddBlock(GetParametrString(domElement, AttrName), domElement);
                blocks->SetActiveById(id);
            }
            else
            {
                const QSignalBlocker blocker(blocks);
                blocks->SetActive(GetParametrString(domElement, AttrName));
            }
            ParseDrawElement(domElement, parse);
            break;
        }
        case 1: // TagIncrements
            if (parse != Document::LiteParse)
            {
                qCDebug(vXML, "Tag increments.");
                ParseIncrementsElement(domElement, parse);
            }
            break;
        case 2: // TagPreviewCalculations
            if (parse != Document::LiteParse)
            {
                qCDebug(vXML, "Tag preview calculations.");
                ParseIncrementsElement(domElement, parse);
            }
            break;
        default:
            qCDebug(vXML, "Ignoring tag %s", qUtf8Printable(domElement.tagName()));
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ParseDrawElement parse draw tag.
 * @param node node.
 * @param parse parser file mode.
 */
void VPattern::ParseDrawElement(const QDomNode &node, const Document &parse)
{
    QStringList const tags{TagCalculation, TagModeling, TagDetails, TagGroups};
    QDomNode domNode = node.firstChild();
    while (not domNode.isNull())
    {
        if (not domNode.isElement())
        {
            domNode = domNode.nextSibling();
            continue;
        }

        const QDomElement domElement = domNode.toElement();
        if (domElement.isNull())
        {
            domNode = domNode.nextSibling();
            continue;
        }

        switch (tags.indexOf(domElement.tagName()))
        {
            case 0: // TagCalculation
                qCDebug(vXML, "Tag calculation.");
                data->ClearCalculationGObjects();
                ParseDrawMode(domElement, parse, Draw::Calculation);
                break;
            case 1: // TagModeling
                qCDebug(vXML, "Tag modeling.");
                ParseDrawMode(domElement, parse, Draw::Modeling);
                break;
            case 2: // TagDetails
                qCDebug(vXML, "Tag details.");
                ParseDetails(domElement, parse);
                break;
            case 3: // TagGroups
                qCDebug(vXML, "Tag groups.");
                ParseGroups(domElement);
                break;
            default:
                throw VException(tr("Wrong tag name '%1'.").arg(domElement.tagName()));
        }
        domNode = domNode.nextSibling();
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ParseDrawMode parse draw tag with draw mode.
 * @param node node.
 * @param parse parser file mode.
 * @param mode draw mode.
 */
void VPattern::ParseDrawMode(const QDomNode &node, const Document &parse, const Draw &mode)
{
    SCASSERT(sceneDraw != nullptr)
    SCASSERT(sceneDetail != nullptr)

    const QDomNodeList nodeList = node.childNodes();
    QDOM_LOOP(nodeList, i)
    {
        QDomElement domElement = QDOM_ELEMENT(nodeList, i).toElement();
        ParseDrawModeElement(domElement, parse, mode);
    }

    if (parse == Document::FullParse)
    {
        emit MadeProgress();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseDrawModeElement(QDomElement &domElement, const Document &parse, const Draw &mode)
{
    VMainGraphicsScene *scene = mode == Draw::Calculation ? sceneDraw : sceneDetail;
    static const QStringList tags({TagPoint, TagLine, TagSpline, TagArc, TagTools, TagOperation, TagElArc, TagPath});

    if (domElement.isNull())
    {
        return;
    }

    switch (tags.indexOf(domElement.tagName()))
    {
        case 0: // TagPoint
            qCDebug(vXML, "Tag point.");
            ParsePointElement(scene, domElement, parse, domElement.attribute(AttrType, QString()));
            break;
        case 1: // TagLine
            qCDebug(vXML, "Tag line.");
            ParseLineElement(scene, domElement, parse);
            break;
        case 2: // TagSpline
            qCDebug(vXML, "Tag spline.");
            ParseSplineElement(scene, domElement, parse, domElement.attribute(AttrType, QString()));
            break;
        case 3: // TagArc
            qCDebug(vXML, "Tag arc.");
            ParseArcElement(scene, domElement, parse, domElement.attribute(AttrType, QString()));
            break;
        case 4: // TagTools
            qCDebug(vXML, "Tag tools.");
            ParseToolsElement(scene, domElement, parse, domElement.attribute(AttrType, QString()));
            break;
        case 5: // TagOperation
            qCDebug(vXML, "Tag operation.");
            ParseOperationElement(scene, domElement, parse, domElement.attribute(AttrType, QString()));
            break;
        case 6: // TagElArc
            qCDebug(vXML, "Tag elliptical arc.");
            ParseEllipticalArcElement(scene, domElement, parse, domElement.attribute(AttrType, QString()));
            break;
        case 7: // TagPath
            qCDebug(vXML, "Tag path.");
            ParsePathElement(scene, domElement, parse);
            break;
        default:
            throw VException(tr("Wrong tag name '%1'.").arg(domElement.tagName()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ParseDetailElement parse detail tag.
 * @param domElement tag in xml tree.
 * @param parse parser file mode.
 */
void VPattern::ParseDetailElement(QDomElement &domElement, const Document &parse)
{
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");
    try
    {
        VToolSeamAllowanceInitData initData;
        initData.id = GetParametrId(domElement);
        initData.detail.SetName(GetParametrString(domElement, AttrName, tr("Detail")));
        initData.detail.SetUUID(GetParametrEmptyString(domElement, AttrUUID));
        initData.detail.SetShortName(GetParametrEmptyString(domElement, AttrShortName).left(25));
        initData.detail.SetGradationLabel(GetParametrEmptyString(domElement, AttrGradationLabel));
        initData.detail.SetMx(
            VAbstractValApplication::VApp()->toPixel(GetParametrDouble(domElement, AttrMx, QStringLiteral("0.0"))));
        initData.detail.SetMy(
            VAbstractValApplication::VApp()->toPixel(GetParametrDouble(domElement, AttrMy, QStringLiteral("0.0"))));
        initData.detail.SetSeamAllowance(GetParametrBool(domElement, VToolSeamAllowance::AttrSeamAllowance, falseStr));
        initData.detail.SetHideMainPath(
            GetParametrBool(domElement, VToolSeamAllowance::AttrHideMainPath,
                            QString().setNum(VAbstractValApplication::VApp()->ValentinaSettings()->IsHideMainPath())));
        initData.detail.SetShowFullPiece(GetParametrBool(domElement, VToolSeamAllowance::AttrShowFullPiece, trueStr));
        initData.detail.SetSeamAllowanceBuiltIn(
            GetParametrBool(domElement, VToolSeamAllowance::AttrSeamAllowanceBuiltIn, falseStr));
        initData.detail.SetForbidFlipping(GetParametrBool(
            domElement, AttrForbidFlipping,
            QString().setNum(VAbstractValApplication::VApp()->ValentinaSettings()->GetForbidWorkpieceFlipping())));
        initData.detail.SetForceFlipping(GetParametrBool(
            domElement, AttrForceFlipping,
            QString().setNum(VAbstractValApplication::VApp()->ValentinaSettings()->GetForceWorkpieceFlipping())));
        initData.detail.SetSymmetricalCopy(GetParametrBool(domElement, AttrSymmetricalCopy, falseStr));
        initData.detail.SetFollowGrainline(GetParametrBool(domElement, AttrFollowGrainline, falseStr));
        initData.detail.SetSewLineOnDrawing(GetParametrBool(
            domElement, AttrSewLineOnDrawing,
            QString().setNum(VAbstractValApplication::VApp()->ValentinaSettings()->GetSewLineOnDrawing())));
        initData.detail.SetInLayout(GetParametrBool(domElement, AttrInLayout, trueStr));
        initData.detail.SetUnited(GetParametrBool(domElement, VToolSeamAllowance::AttrUnited, falseStr));
        initData.detail.SetPriority(GetParametrUInt(domElement, VToolSeamAllowance::AttrPiecePriority, QChar('0')));
        initData.detail.SetTrueZeroWidth(GetParametrBool(domElement, AttrTrueZeroWidth, falseStr));

        initData.width = GetParametrString(domElement, AttrWidth, QStringLiteral("0.0"));
        const QString w = initData.width; // need for saving fixed formula;

        ParseDetailInternals(domElement, initData.detail);

        initData.scene = sceneDetail;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        if (const VToolSeamAllowance *piece = VToolSeamAllowance::Create(initData); parse == Document::FullParse)
        {
            updatePieces.append(piece->getId());
        }
        // Rewrite attribute formula. Need for situation when we have wrong formula.
        if (w != initData.width)
        {
            SetAttribute(domElement, AttrWidth, initData.width);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating detail"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseDetailInternals(const QDomElement &domElement, VPiece &detail) const
{
    const uint version = GetParametrUInt(domElement, AttrVersion, QChar('1'));

    const QStringList tags{TagNodes,
                           TagData,
                           TagPatternInfo,
                           TagGrainline,
                           VToolSeamAllowance::TagCSA,
                           VToolSeamAllowance::TagIPaths,
                           VToolSeamAllowance::TagPins,
                           VToolSeamAllowance::TagPlaceLabels,
                           VAbstractPattern::TagMirrorLine};

    QFuture<QVector<VPieceNode>> futurePathV1;
    QFuture<VPiecePath> futurePathV2;
    QFuture<VPieceLabelData> futurePPData;
    QFuture<VPatternLabelData> futurePatternInfo;
    QFuture<VGrainlineData> futureGGeometry;
    QFuture<QVector<CustomSARecord>> futureRecords;
    QFuture<QVector<quint32>> futureIPaths;
    QFuture<QVector<quint32>> futurePins;
    QFuture<QVector<quint32>> futurePlaceLabels;
    QFuture<VPieceFoldLineData> futureMirrorLine;

    const QDomNodeList nodeList = domElement.childNodes();
    QDOM_LOOP(nodeList, i)
    {
        if (const QDomElement element = QDOM_ELEMENT(nodeList, i).toElement(); not element.isNull())
        {
            switch (tags.indexOf(element.tagName()))
            {
                case 0: // TagNodes
                    if (version == 1)
                    {
                        // TODO. Delete if minimal supported version is 0.4.0
                        Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 4, 0),
                                          "Time to refactor the code.");
                        futurePathV1 = QtConcurrent::run(
                            [this, domElement, element]() -> QVector<VPieceNode>
                            {
                                const bool closed = GetParametrUInt(domElement, AttrClosed, QChar('1'));
                                const qreal width = GetParametrDouble(domElement, AttrWidth, QStringLiteral("0.0"));
                                return ParseDetailNodes(element, width, closed);
                            });
                    }
                    else
                    {
                        futurePathV2 = QtConcurrent::run(&VPattern::ParsePieceNodes, element);
                    }
                    break;
                case 1: // TagData
                    futurePPData = QtConcurrent::run([this, element, detail]() -> VPieceLabelData
                                                     { return ParsePieceDataTag(element, detail.GetPieceLabelData()); });
                    break;
                case 2: // TagPatternInfo
                    futurePatternInfo = QtConcurrent::run(
                        [this, element, detail]() -> VPatternLabelData
                        { return ParsePiecePatternInfo(element, detail.GetPatternLabelData()); });
                    break;
                case 3: // TagGrainline
                    futureGGeometry = QtConcurrent::run(
                        [this, element, detail]() -> VGrainlineData
                        { return ParsePieceGrainline(element, detail.GetGrainlineGeometry()); });
                    break;
                case 4: // VToolSeamAllowance::TagCSA
                    futureRecords = QtConcurrent::run(&VPattern::ParsePieceCSARecords, element);
                    break;
                case 5: // VToolSeamAllowance::TagIPaths
                    futureIPaths = QtConcurrent::run(&VPattern::ParsePieceInternalPaths, element);
                    break;
                case 6: // VToolSeamAllowance::TagPins
                    futurePins = QtConcurrent::run(&VPattern::ParsePiecePointRecords, element);
                    break;
                case 7: // VToolSeamAllowance::TagPlaceLabels
                    futurePlaceLabels = QtConcurrent::run(&VPattern::ParsePiecePointRecords, element);
                    break;
                case 8: // VToolSeamAllowance::TagMirrorLine
                    futureMirrorLine = QtConcurrent::run(&ParsePieceMirrorLine, element);
                    break;
                default:
                    break;
            }
        }
    }

    if (version == 1 && not futurePathV1.isCanceled())
    {
        detail.GetPath().SetNodes(futurePathV1.result());
    }
    else if (not futurePathV2.isCanceled())
    {
        detail.SetPath(futurePathV2.result());
    }

    if (not futurePPData.isCanceled())
    {
        detail.SetPieceLabelData(futurePPData.result());
    }

    if (not futurePatternInfo.isCanceled())
    {
        detail.SetPatternLabelData(futurePatternInfo.result());
    }

    if (not futureGGeometry.isCanceled())
    {
        detail.SetGrainlineGeometry(futureGGeometry.result());
    }

    if (not futureRecords.isCanceled())
    {
        detail.SetCustomSARecords(futureRecords.result());
    }

    if (not futureIPaths.isCanceled())
    {
        detail.SetInternalPaths(futureIPaths.result());
    }

    if (not futurePins.isCanceled())
    {
        detail.SetPins(futurePins.result());
    }

    if (not futurePlaceLabels.isCanceled())
    {
        detail.SetPlaceLabels(futurePlaceLabels.result());
    }

    if (not futureMirrorLine.isCanceled())
    {
        VPieceFoldLineData const data = futureMirrorLine.result();
        detail.SetMirrorLineStartPoint(data.p1);
        detail.SetMirrorLineEndPoint(data.p2);
        detail.SetShowMirrorLine(data.mirrorLineVisible);
        detail.SetManualFoldHeight(data.manualHeight);
        detail.SetManualFoldWidth(data.manualWidth);
        detail.SetManualFoldCenter(data.manualCenter);
        detail.SetFormulaFoldHeight(data.heightFormula);
        detail.SetFormulaFoldWidth(data.widthFormula);
        detail.SetFormulaFoldCenter(data.centerFormula);
        detail.SetFoldLineType(data.type);
        detail.SetFoldLineSvgFontSize(data.fontSize);
        detail.SetFoldLineLabelFontItalic(data.italic);
        detail.SetFoldLineLabelFontBold(data.bold);
        detail.SetFoldLineLabel(data.label);
        detail.SetFoldLineLabelAlignment(data.alignment);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPattern::ParseDetailNodes(const QDomElement &domElement, qreal width, bool closed) const -> QVector<VPieceNode>
{
    QVector<VNodeDetail> oldNodes;
    const QDomNodeList nodeList = domElement.childNodes();
    QDOM_LOOP(nodeList, i)
    {
        const QDomElement element = QDOM_ELEMENT(nodeList, i).toElement();
        if (not element.isNull() &&
            element.tagName() == VAbstractPattern::TagNode) // Old detail version need this check!
        {
            oldNodes.append(ParseDetailNode(element));
        }
    }

    return VNodeDetail::Convert(data, oldNodes, width, closed);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPattern::ParsePieceDataTag(const QDomElement &domElement, VPieceLabelData ppData) const -> VPieceLabelData
{
    ppData.SetEnabled(GetParametrBool(domElement, AttrVisible, trueStr));
    ppData.SetLetter(GetParametrEmptyString(domElement, AttrLetter));
    ppData.SetAnnotation(GetParametrEmptyString(domElement, AttrAnnotation));
    ppData.SetOrientation(GetParametrEmptyString(domElement, AttrOrientation));
    ppData.SetRotationWay(GetParametrEmptyString(domElement, AttrRotationWay));
    ppData.SetTilt(GetParametrEmptyString(domElement, AttrTilt));
    ppData.SetFoldPosition(GetParametrEmptyString(domElement, AttrFoldPosition));
    ppData.SetQuantity(static_cast<quint16>(GetParametrUInt(domElement, AttrQuantity, QChar('1'))));
    ppData.SetOnFold(GetParametrBool(domElement, AttrOnFold, falseStr));
    ppData.SetPos(
        QPointF(GetParametrDouble(domElement, AttrMx, QChar('0')), GetParametrDouble(domElement, AttrMy, QChar('0'))));
    ppData.SetFontSize(static_cast<int>(GetParametrUInt(domElement, VToolSeamAllowance::AttrFont, QChar('0'))));
    ppData.SetRotation(GetParametrString(domElement, AttrRotation, QChar('0')));

    const quint32 topLeftPin = GetParametrUInt(domElement, VToolSeamAllowance::AttrTopLeftPin, NULL_ID_STR);

    if (const quint32 bottomRightPin = GetParametrUInt(domElement, VToolSeamAllowance::AttrBottomRightPin, NULL_ID_STR);
        topLeftPin != NULL_ID && bottomRightPin != NULL_ID && topLeftPin != bottomRightPin)
    {
        ppData.SetTopLeftPin(topLeftPin);
        ppData.SetBottomRightPin(bottomRightPin);
        ppData.SetCenterPin(NULL_ID);
        ppData.SetLabelWidth(QChar('1'));
        ppData.SetLabelHeight(QChar('1'));
    }
    else
    {
        ppData.SetTopLeftPin(NULL_ID);
        ppData.SetBottomRightPin(NULL_ID);
        ppData.SetCenterPin(GetParametrUInt(domElement, VToolSeamAllowance::AttrCenterPin, NULL_ID_STR));
        ppData.SetLabelWidth(GetParametrString(domElement, AttrWidth, QChar('1')));
        ppData.SetLabelHeight(GetParametrString(domElement, AttrHeight, QChar('1')));
    }

    ppData.SetLabelTemplate(GetLabelTemplate(domElement));
    return ppData;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPattern::ParsePiecePatternInfo(const QDomElement &domElement, VPatternLabelData patternInfo) const
    -> VPatternLabelData
{
    patternInfo.SetEnabled(GetParametrBool(domElement, AttrVisible, trueStr));
    patternInfo.SetPos(
        QPointF(GetParametrDouble(domElement, AttrMx, QChar('0')), GetParametrDouble(domElement, AttrMy, QChar('0'))));
    patternInfo.SetFontSize(static_cast<int>(GetParametrUInt(domElement, VToolSeamAllowance::AttrFont, QChar('0'))));
    patternInfo.SetRotation(GetParametrString(domElement, AttrRotation, QChar('0')));

    const quint32 topLeftPin = GetParametrUInt(domElement, VToolSeamAllowance::AttrTopLeftPin, NULL_ID_STR);

    if (const quint32 bottomRightPin = GetParametrUInt(domElement, VToolSeamAllowance::AttrBottomRightPin, NULL_ID_STR);
        topLeftPin != NULL_ID && bottomRightPin != NULL_ID && topLeftPin != bottomRightPin)
    {
        patternInfo.SetTopLeftPin(topLeftPin);
        patternInfo.SetBottomRightPin(bottomRightPin);
        patternInfo.SetCenterPin(NULL_ID);
        patternInfo.SetLabelWidth(QChar('1'));
        patternInfo.SetLabelHeight(QChar('1'));
    }
    else
    {
        patternInfo.SetTopLeftPin(NULL_ID);
        patternInfo.SetBottomRightPin(NULL_ID);
        patternInfo.SetCenterPin(GetParametrUInt(domElement, VToolSeamAllowance::AttrCenterPin, NULL_ID_STR));
        patternInfo.SetLabelWidth(GetParametrString(domElement, AttrWidth, QChar('1')));
        patternInfo.SetLabelHeight(GetParametrString(domElement, AttrHeight, QChar('1')));
    }

    return patternInfo;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPattern::ParsePieceGrainline(const QDomElement &domElement, VGrainlineData gGeometry) const -> VGrainlineData
{
    gGeometry.SetEnabled(GetParametrBool(domElement, AttrEnabled, falseStr));
    gGeometry.SetVisible(GetParametrBool(domElement, AttrVisible, trueStr));
    gGeometry.SetPos(
        QPointF(GetParametrDouble(domElement, AttrMx, QChar('0')), GetParametrDouble(domElement, AttrMy, QChar('0'))));
    gGeometry.SetArrowType(static_cast<GrainlineArrowDirection>(GetParametrUInt(domElement, AttrArrows, QChar('0'))));

    const quint32 topPin = GetParametrUInt(domElement, VToolSeamAllowance::AttrTopPin, NULL_ID_STR);

    if (const quint32 bottomPin = GetParametrUInt(domElement, VToolSeamAllowance::AttrBottomPin, NULL_ID_STR);
        topPin != NULL_ID && bottomPin != NULL_ID && topPin != bottomPin)
    {
        gGeometry.SetTopPin(topPin);
        gGeometry.SetBottomPin(bottomPin);

        gGeometry.SetLength(QChar('1'));
        gGeometry.SetRotation(QString::number(90));
        gGeometry.SetCenterPin(NULL_ID);
    }
    else
    {
        gGeometry.SetTopPin(NULL_ID);
        gGeometry.SetBottomPin(NULL_ID);

        gGeometry.SetLength(GetParametrString(domElement, AttrLength, QChar('1')));
        gGeometry.SetRotation(GetParametrString(domElement, AttrRotation, QString::number(90)));
        gGeometry.SetCenterPin(GetParametrUInt(domElement, VToolSeamAllowance::AttrCenterPin, NULL_ID_STR));
    }

    return gGeometry;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ParseDetails parse details tag.
 * @param domElement tag in xml tree.
 * @param parse parser file mode.
 */
void VPattern::ParseDetails(const QDomElement &domElement, const Document &parse)
{
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");
    QDomNode domNode = domElement.firstChild();
    while (!domNode.isNull())
    {
        if (domNode.isElement())
        {
            if (QDomElement domElement = domNode.toElement(); !domElement.isNull() && domElement.tagName() == TagDetail)
            {
                ParseDetailElement(domElement, parse);
            }
        }
        domNode = domNode.nextSibling();
    }

    if (parse == Document::FullParse)
    {
        emit MadeProgress();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::PointsWithLineCommonAttributes(const QDomElement &domElement, VToolLinePointInitData &initData)
{
    PointsCommonAttributes(domElement, initData);
    initData.typeLine = GetParametrString(domElement, AttrTypeLine, TypeLineLine);
    initData.lineColor = GetParametrString(domElement, AttrLineColor, ColorBlack);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::PointsCommonAttributes(const QDomElement &domElement, VToolSinglePointInitData &initData)
{
    DrawPointsCommonAttributes(domElement, initData.id, initData.mx, initData.my, initData.notes);
    initData.name = GetParametrString(domElement, AttrName, QChar('A'));
    initData.showLabel = GetParametrBool(domElement, AttrShowLabel, trueStr);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::PointsCommonAttributes(const QDomElement &domElement, quint32 &id, qreal &mx, qreal &my)
{
    ToolsCommonAttributes(domElement, id);
    mx = VAbstractValApplication::VApp()->toPixel(GetParametrDouble(domElement, AttrMx, QString::number(labelMX)));
    my = VAbstractValApplication::VApp()->toPixel(GetParametrDouble(domElement, AttrMy, QString::number(labelMY)));
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::DrawPointsCommonAttributes(const QDomElement &domElement, quint32 &id, qreal &mx, qreal &my,
                                          QString &notes)
{
    DrawToolsCommonAttributes(domElement, id, notes);
    mx = VAbstractValApplication::VApp()->toPixel(GetParametrDouble(domElement, AttrMx, QString::number(labelMX)));
    my = VAbstractValApplication::VApp()->toPixel(GetParametrDouble(domElement, AttrMy, QString::number(labelMY)));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ParsePointElement parse point tag.
 * @param scene scene.
 * @param domElement tag in xml tree.
 * @param parse parser file mode.
 * @param type type of point.
 */
void VPattern::ParsePointElement(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse,
                                 const QString &type)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");
    Q_ASSERT_X(not type.isEmpty(), Q_FUNC_INFO, "type of point is empty");

    static const QStringList points({VToolBasePoint::ToolType,                  /*0*/
                                     VToolEndLine::ToolType,                    /*1*/
                                     VToolAlongLine::ToolType,                  /*2*/
                                     VToolShoulderPoint::ToolType,              /*3*/
                                     VToolNormal::ToolType,                     /*4*/
                                     VToolBisector::ToolType,                   /*5*/
                                     VToolLineIntersect::ToolType,              /*6*/
                                     VToolPointOfContact::ToolType,             /*7*/
                                     VNodePoint::ToolType,                      /*8*/
                                     VToolHeight::ToolType,                     /*9*/
                                     VToolTriangle::ToolType,                   /*10*/
                                     VToolPointOfIntersection::ToolType,        /*11*/
                                     VToolCutSpline::ToolType,                  /*12*/
                                     VToolCutSplinePath::ToolType,              /*13*/
                                     VToolCutArc::ToolType,                     /*14*/
                                     VToolLineIntersectAxis::ToolType,          /*15*/
                                     VToolCurveIntersectAxis::ToolType,         /*16*/
                                     VToolPointOfIntersectionArcs::ToolType,    /*17*/
                                     VToolPointOfIntersectionCircles::ToolType, /*18*/
                                     VToolPointFromCircleAndTangent::ToolType,  /*19*/
                                     VToolPointFromArcAndTangent::ToolType,     /*20*/
                                     VToolTrueDarts::ToolType,                  /*21*/
                                     VToolPointOfIntersectionCurves::ToolType,  /*22*/
                                     VToolPin::ToolType,                        /*23*/
                                     VToolPlaceLabel::ToolType});               /*24*/
    switch (points.indexOf(type))
    {
        case 0: // VToolBasePoint::ToolType
            ParseToolBasePoint(scene, domElement, parse);
            break;
        case 1: // VToolEndLine::ToolType
            ParseToolEndLine(scene, domElement, parse);
            break;
        case 2: // VToolAlongLine::ToolType
            ParseToolAlongLine(scene, domElement, parse);
            break;
        case 3: // VToolShoulderPoint::ToolType
            ParseToolShoulderPoint(scene, domElement, parse);
            break;
        case 4: // VToolNormal::ToolType
            ParseToolNormal(scene, domElement, parse);
            break;
        case 5: // VToolBisector::ToolType
            ParseToolBisector(scene, domElement, parse);
            break;
        case 6: // VToolLineIntersect::ToolType
            ParseToolLineIntersect(scene, domElement, parse);
            break;
        case 7: // VToolPointOfContact::ToolType
            ParseToolPointOfContact(scene, domElement, parse);
            break;
        case 8: // VNodePoint::ToolType
            ParseNodePoint(domElement, parse);
            break;
        case 9: // VToolHeight::ToolType
            ParseToolHeight(scene, domElement, parse);
            break;
        case 10: // VToolTriangle::ToolType
            ParseToolTriangle(scene, domElement, parse);
            break;
        case 11: // VToolPointOfIntersection::ToolType
            ParseToolPointOfIntersection(scene, domElement, parse);
            break;
        case 12: // VToolCutSpline::ToolType
            ParseToolCutSpline(scene, domElement, parse);
            break;
        case 13: // VToolCutSplinePath::ToolType
            ParseToolCutSplinePath(scene, domElement, parse);
            break;
        case 14: // VToolCutArc::ToolType
            ParseToolCutArc(scene, domElement, parse);
            break;
        case 15: // VToolLineIntersectAxis::ToolType
            ParseToolLineIntersectAxis(scene, domElement, parse);
            break;
        case 16: // VToolCurveIntersectAxis::ToolType
            ParseToolCurveIntersectAxis(scene, domElement, parse);
            break;
        case 17: // VToolPointOfIntersectionArcs::ToolType
            ParseToolPointOfIntersectionArcs(scene, domElement, parse);
            break;
        case 18: // VToolPointOfIntersectionCircles::ToolType
            ParseToolPointOfIntersectionCircles(scene, domElement, parse);
            break;
        case 19: // VToolPointFromCircleAndTangent::ToolType
            ParseToolPointFromCircleAndTangent(scene, domElement, parse);
            break;
        case 20: // VToolPointFromArcAndTangent::ToolType
            ParseToolPointFromArcAndTangent(scene, domElement, parse);
            break;
        case 21: // VToolTrueDarts::ToolType
            ParseToolTrueDarts(scene, domElement, parse);
            break;
        case 22: // VToolPointOfIntersectionCurves::ToolType
            ParseToolPointOfIntersectionCurves(scene, domElement, parse);
            break;
        case 23: // VToolPin::ToolType
            ParsePinPoint(domElement, parse);
            break;
        case 24: // VToolPlaceLabel::ToolType
            ParsePlaceLabel(domElement, parse);
            break;
        default:
            throw VException(tr("Unknown point type '%1'.").arg(type));
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ParseLineElement parse line tag.
 * @param scene scene.
 * @param domElement tag in xml tree.
 * @param parse parser file mode.
 */
void VPattern::ParseLineElement(VMainGraphicsScene *scene, const QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");
    try
    {
        VToolLineInitData initData;
        DrawToolsCommonAttributes(domElement, initData.id, initData.notes);
        initData.firstPoint = GetParametrUInt(domElement, AttrFirstPoint, NULL_ID_STR);
        initData.secondPoint = GetParametrUInt(domElement, AttrSecondPoint, NULL_ID_STR);
        initData.typeLine = GetParametrString(domElement, AttrTypeLine, TypeLineLine);
        initData.lineColor = GetParametrString(domElement, AttrLineColor, ColorBlack);
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        VToolLine::Create(initData);
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating line"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::SplinesCommonAttributes(const QDomElement &domElement, quint32 &id, quint32 &idObject, quint32 &idTool)
{
    ToolsCommonAttributes(domElement, id);
    idObject = GetParametrUInt(domElement, AttrIdObject, NULL_ID_STR);
    idTool = GetParametrUInt(domElement, VAbstractNode::AttrIdTool, NULL_ID_STR);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseCurrentPP()
{
    QDomElement domElement = PatternBlockMapper()->GetActiveElement();
    if (domElement.isNull())
    {
        return;
    }

    ParseDrawElement(domElement, Document::LiteParse);
    emit CheckLayout();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPattern::GetLabelBase(quint32 index) const -> QString
{
    QStringList alphabet;
    switch (VApplication::LabelLanguages().indexOf(GetLabelPrefix()))
    {
        case 0: // de
        {
            const auto al = QStringLiteral("A,Ä,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Ö,P,Q,R,S,ß,T,U,Ü,V,W,X,Y,Z");
            alphabet = al.split(','_L1);
            break;
        }
        case 2: // fr
        {
            const auto al = QStringLiteral("A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z");
            alphabet = al.split(','_L1);
            break;
        }
        case 3: // ru
        {
            const auto al = QStringLiteral("А,Б,В,Г,Д,Е,Ж,З,И,К,Л,М,Н,О,П,Р,С,Т,У,Ф,Х,Ц,Ч,Ш,Щ,Э,Ю,Я");
            alphabet = al.split(','_L1);
            break;
        }
        case 4: // uk
        {
            const auto al = QStringLiteral("А,Б,В,Г,Д,Е,Ж,З,І,Ї,Й,К,Л,М,Н,О,П,Р,С,Т,У,Ф,Х,Ц,Ч,Ш,Щ,Є,Ю,Я");
            alphabet = al.split(','_L1);
            break;
        }
        case 5: // hr
        case 7: // bs
        {
            const auto al = QStringLiteral("A,B,C,Č,Ć,D,Dž,Ð,E,F,G,H,I,J,K,L,Lj,M,N,Nj,O,P,R,S,Š,T,U,V,Z,Ž");
            alphabet = al.split(','_L1);
            break;
        }
        case 6: // sr
        {
            const auto al = QStringLiteral("А,Б,В,Г,Д,Ђ,Е,Ж,З,И,Ј,К,Л,Љ,М,Н,Њ,О,П,Р,С,Т,Ћ,У,Ф,Х,Ц,Ч,Џ,Ш");
            alphabet = al.split(','_L1);
            break;
        }
        case 8: // cs
        {
            const auto al = QStringLiteral("a,á,b,c,č,d,ď,e,é,ě,f,g,h,ch,i,í,j,k,l,m,n,ň,o,ó,p,q,r,ř,s,š,t,ť,u,ú,ů,"
                                           "v,w,x,y,ý,z,ž");
            alphabet = al.split(','_L1);
            break;
        }
        case 1:  // en
        default: // en
        {
            const auto al = QStringLiteral("A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z");
            alphabet = al.split(','_L1);
            break;
        }
    }

    QString base;
    const int count = qFloor(index / static_cast<quint32>(alphabet.size()));
    const auto number = static_cast<int>(index) - static_cast<int>(alphabet.size()) * count;
    int i = 0;
    do
    {
        base.append(alphabet.at(number));
        ++i;
    } while (i < count);
    return base;
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolBasePoint(VMainGraphicsScene *scene, const QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    VToolBasePoint *spoint = nullptr;
    try
    {
        VToolBasePointInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        PointsCommonAttributes(domElement, initData);
        initData.x =
            VAbstractValApplication::VApp()->toPixel(GetParametrDouble(domElement, AttrX, QStringLiteral("10.0")));
        initData.y =
            VAbstractValApplication::VApp()->toPixel(GetParametrDouble(domElement, AttrY, QStringLiteral("10.0")));

        spoint = VToolBasePoint::Create(initData);
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating single point"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        scene->removeItem(spoint);
        delete spoint;
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolEndLine(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolEndLineInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        PointsWithLineCommonAttributes(domElement, initData);

        initData.formulaLength = GetParametrString(domElement, AttrLength, QStringLiteral("100.0"));
        const QString f = initData.formulaLength; // need for saving fixed formula;

        initData.basePointId = GetParametrUInt(domElement, AttrBasePoint, NULL_ID_STR);

        initData.formulaAngle = GetParametrString(domElement, AttrAngle, QStringLiteral("0.0"));
        const QString angleFix = initData.formulaAngle;

        VToolEndLine::Create(initData);
        // Rewrite attribute formula. Need for situation when we have wrong formula.
        if (f != initData.formulaLength || angleFix != initData.formulaAngle)
        {
            SetAttribute(domElement, AttrLength, initData.formulaLength);
            SetAttribute(domElement, AttrAngle, initData.formulaAngle);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of end line"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of end line"), domElement);
        excep.AddMoreInformation("Message:     "_L1 + e.GetMsg() + '\n'_L1 + "Expression:  "_L1 + e.GetExpr());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolAlongLine(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        QT_WARNING_PUSH
        QT_WARNING_DISABLE_GCC("-Wnoexcept")

        VToolAlongLineInitData initData;

        QT_WARNING_POP

        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        PointsWithLineCommonAttributes(domElement, initData);
        initData.formula = GetParametrString(domElement, AttrLength, QStringLiteral("100.0"));
        const QString f = initData.formula; // need for saving fixed formula;
        initData.firstPointId = GetParametrUInt(domElement, AttrFirstPoint, NULL_ID_STR);
        initData.secondPointId = GetParametrUInt(domElement, AttrSecondPoint, NULL_ID_STR);

        VToolAlongLine::Create(initData);
        // Rewrite attribute formula. Need for situation when we have wrong formula.
        if (f != initData.formula)
        {
            SetAttribute(domElement, AttrLength, initData.formula);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating point along line"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating point along line"), domElement);
        excep.AddMoreInformation("Message:     "_L1 + e.GetMsg() + '\n'_L1 + "Expression:  "_L1 + e.GetExpr());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolShoulderPoint(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolShoulderPointInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        PointsWithLineCommonAttributes(domElement, initData);
        initData.formula = GetParametrString(domElement, AttrLength, QStringLiteral("100.0"));
        const QString f = initData.formula; // need for saving fixed formula;
        initData.p1Line = GetParametrUInt(domElement, AttrP1Line, NULL_ID_STR);
        initData.p2Line = GetParametrUInt(domElement, AttrP2Line, NULL_ID_STR);
        initData.pShoulder = GetParametrUInt(domElement, AttrPShoulder, NULL_ID_STR);

        VToolShoulderPoint::Create(initData);
        // Rewrite attribute formula. Need for situation when we have wrong formula.
        if (f != initData.formula)
        {
            SetAttribute(domElement, AttrLength, initData.formula);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of shoulder"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of shoulder"), domElement);
        excep.AddMoreInformation("Message:     "_L1 + e.GetMsg() + '\n'_L1 + "Expression:  "_L1 + e.GetExpr());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolNormal(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolNormalInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        PointsWithLineCommonAttributes(domElement, initData);
        initData.formula = GetParametrString(domElement, AttrLength, QStringLiteral("100.0"));
        const QString f = initData.formula; // need for saving fixed formula;
        initData.firstPointId = GetParametrUInt(domElement, AttrFirstPoint, NULL_ID_STR);
        initData.secondPointId = GetParametrUInt(domElement, AttrSecondPoint, NULL_ID_STR);
        initData.angle = GetParametrDouble(domElement, AttrAngle, QStringLiteral("0.0"));

        VToolNormal::Create(initData);
        // Rewrite attribute formula. Need for situation when we have wrong formula.
        if (f != initData.formula)
        {
            SetAttribute(domElement, AttrLength, initData.formula);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of normal"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of normal"), domElement);
        excep.AddMoreInformation("Message:     "_L1 + e.GetMsg() + '\n'_L1 + "Expression:  "_L1 + e.GetExpr());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolBisector(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolBisectorInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        PointsWithLineCommonAttributes(domElement, initData);
        initData.formula = GetParametrString(domElement, AttrLength, QStringLiteral("100.0"));
        const QString f = initData.formula; // need for saving fixed formula;
        initData.firstPointId = GetParametrUInt(domElement, AttrFirstPoint, NULL_ID_STR);
        initData.secondPointId = GetParametrUInt(domElement, AttrSecondPoint, NULL_ID_STR);
        initData.thirdPointId = GetParametrUInt(domElement, AttrThirdPoint, NULL_ID_STR);

        VToolBisector::Create(initData);
        // Rewrite attribute formula. Need for situation when we have wrong formula.
        if (f != initData.formula)
        {
            SetAttribute(domElement, AttrLength, initData.formula);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of bisector"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of bisector"), domElement);
        excep.AddMoreInformation("Message:     "_L1 + e.GetMsg() + '\n'_L1 + "Expression:  "_L1 + e.GetExpr());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolLineIntersect(VMainGraphicsScene *scene, const QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolLineIntersectInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        PointsCommonAttributes(domElement, initData);
        initData.p1Line1Id = GetParametrUInt(domElement, AttrP1Line1, NULL_ID_STR);
        initData.p2Line1Id = GetParametrUInt(domElement, AttrP2Line1, NULL_ID_STR);
        initData.p1Line2Id = GetParametrUInt(domElement, AttrP1Line2, NULL_ID_STR);
        initData.p2Line2Id = GetParametrUInt(domElement, AttrP2Line2, NULL_ID_STR);

        VToolLineIntersect::Create(initData);
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of line intersection"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolPointOfContact(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolPointOfContactInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        PointsCommonAttributes(domElement, initData);
        initData.radius = GetParametrString(domElement, AttrRadius, QChar('0'));
        const QString f = initData.radius; // need for saving fixed formula;
        initData.center = GetParametrUInt(domElement, AttrCenter, NULL_ID_STR);
        initData.firstPointId = GetParametrUInt(domElement, AttrFirstPoint, NULL_ID_STR);
        initData.secondPointId = GetParametrUInt(domElement, AttrSecondPoint, NULL_ID_STR);

        VToolPointOfContact::Create(initData);
        // Rewrite attribute formula. Need for situation when we have wrong formula.
        if (f != initData.radius)
        {
            SetAttribute(domElement, AttrRadius, initData.radius);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of contact"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of contact"), domElement);
        excep.AddMoreInformation("Message:     "_L1 + e.GetMsg() + '\n'_L1 + "Expression:  "_L1 + e.GetExpr());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseNodePoint(const QDomElement &domElement, const Document &parse)
{
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VAbstractNodeInitData initData;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;
        initData.scene = sceneDetail;

        PointsCommonAttributes(domElement, initData.id, initData.mx, initData.my);
        initData.idObject = GetParametrUInt(domElement, AttrIdObject, NULL_ID_STR);
        initData.idTool = GetParametrUInt(domElement, VAbstractNode::AttrIdTool, NULL_ID_STR);
        initData.showLabel = GetParametrBool(domElement, AttrShowLabel, trueStr);

        VNodePoint::Create(initData);
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating modeling point"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParsePinPoint(const QDomElement &domElement, const Document &parse)
{
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolPinInitData initData;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        ToolsCommonAttributes(domElement, initData.id);
        initData.pointId = GetParametrUInt(domElement, AttrIdObject, NULL_ID_STR);
        initData.idTool = GetParametrUInt(domElement, VAbstractNode::AttrIdTool, NULL_ID_STR);

        VToolPin::Create(initData);
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating pin point"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParsePlaceLabel(QDomElement &domElement, const Document &parse)
{
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolPlaceLabelInitData initData;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        ToolsCommonAttributes(domElement, initData.id);
        initData.centerPoint = GetParametrUInt(domElement, AttrIdObject, NULL_ID_STR);

        try
        {
            initData.data->GeometricObject<VPointF>(initData.centerPoint);
        }
        catch (const VExceptionBadId &)
        { // Possible case. Parent was deleted, but the node object is still here.
            qDebug() << "Broken relation. Parent was deleted, but the place label object is still here. Place label "
                        "id ="
                     << initData.id << ".";
            return; // Just ignore
        }

        initData.idTool = GetParametrUInt(domElement, VAbstractNode::AttrIdTool, NULL_ID_STR);

        initData.width = GetParametrString(domElement, AttrWidth, QStringLiteral("1.0"));
        const QString w = initData.width; // need for saving fixed formula;

        initData.height = GetParametrString(domElement, AttrHeight, QStringLiteral("1.0"));
        const QString h = initData.height; // need for saving fixed formula;

        initData.angle = GetParametrString(domElement, AttrAngle, QStringLiteral("0.0"));
        const QString angle = initData.angle; // need for saving fixed formula;

        initData.visibilityTrigger =
            GetParametrString(domElement, VAbstractPattern::AttrVisible, QStringLiteral("1.0"));
        const QString visibility = initData.visibilityTrigger; // need for saving fixed formula;

        initData.type = static_cast<PlaceLabelType>(GetParametrUInt(domElement, AttrPlaceLabelType, QChar('0')));

        initData.notMirrored = GetParametrBool(domElement, AttrNotMirrored, falseStr);

        VToolPlaceLabel::Create(initData);

        // Rewrite attribute formula. Need for situation when we have wrong formula.
        if (w != initData.width || h != initData.height || angle != initData.angle ||
            visibility != initData.visibilityTrigger)
        {
            SetAttribute(domElement, AttrWidth, initData.width);
            SetAttribute(domElement, AttrHeight, initData.height);
            SetAttribute(domElement, AttrAngle, initData.angle);
            SetAttribute(domElement, VAbstractPattern::AttrVisible, initData.visibilityTrigger);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating place label"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating place label"), domElement);
        excep.AddMoreInformation("Message:     "_L1 + e.GetMsg() + '\n'_L1 + "Expression:  "_L1 + e.GetExpr());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolHeight(VMainGraphicsScene *scene, const QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolHeightInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        PointsWithLineCommonAttributes(domElement, initData);
        initData.basePointId = GetParametrUInt(domElement, AttrBasePoint, NULL_ID_STR);
        initData.p1LineId = GetParametrUInt(domElement, AttrP1Line, NULL_ID_STR);
        initData.p2LineId = GetParametrUInt(domElement, AttrP2Line, NULL_ID_STR);

        VToolHeight::Create(initData);
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating height"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolTriangle(VMainGraphicsScene *scene, const QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolTriangleInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        PointsCommonAttributes(domElement, initData);
        initData.axisP1Id = GetParametrUInt(domElement, AttrAxisP1, NULL_ID_STR);
        initData.axisP2Id = GetParametrUInt(domElement, AttrAxisP2, NULL_ID_STR);
        initData.firstPointId = GetParametrUInt(domElement, AttrFirstPoint, NULL_ID_STR);
        initData.secondPointId = GetParametrUInt(domElement, AttrSecondPoint, NULL_ID_STR);

        VToolTriangle::Create(initData);
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating triangle"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolPointOfIntersection(VMainGraphicsScene *scene, const QDomElement &domElement,
                                            const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolPointOfIntersectionInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        PointsCommonAttributes(domElement, initData);
        initData.firstPointId = GetParametrUInt(domElement, AttrFirstPoint, NULL_ID_STR);
        initData.secondPointId = GetParametrUInt(domElement, AttrSecondPoint, NULL_ID_STR);

        VToolPointOfIntersection::Create(initData);
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of intersection"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolCutSpline(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolCutInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        PointsCommonAttributes(domElement, initData);
        initData.formula = GetParametrString(domElement, AttrLength, QChar('0'));
        const QString f = initData.formula; // need for saving fixed formula;
        initData.baseCurveId = GetParametrUInt(domElement, VToolCutSpline::AttrSpline, NULL_ID_STR);
        initData.aliasSuffix1 = GetParametrEmptyString(domElement, AttrAlias1);
        initData.aliasSuffix2 = GetParametrEmptyString(domElement, AttrAlias2);

        // Make name1 and name2 mandatory
        Q_STATIC_ASSERT(VPatternConverter::PatternMinVer < FormatVersion(1, 1, 1));
        initData.name1 = GetParametrEmptyString(domElement, AttrCurveName1); // Optionally empty for compatibility
        initData.name2 = GetParametrEmptyString(domElement, AttrCurveName2); // Optionally empty for compatibility

        VToolCutSpline::Create(initData);
        // Rewrite attribute formula. Need for situation when we have wrong formula.
        if (f != initData.formula)
        {
            SetAttribute(domElement, AttrLength, initData.formula);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating cut spline point"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating cut spline point"), domElement);
        excep.AddMoreInformation("Message:     "_L1 + e.GetMsg() + '\n'_L1 + "Expression:  "_L1 + e.GetExpr());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolCutSplinePath(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolCutInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        PointsCommonAttributes(domElement, initData);
        initData.formula = GetParametrString(domElement, AttrLength, QChar('0'));
        const QString f = initData.formula; // need for saving fixed formula;
        initData.baseCurveId = GetParametrUInt(domElement, VToolCutSplinePath::AttrSplinePath, NULL_ID_STR);
        initData.aliasSuffix1 = GetParametrEmptyString(domElement, AttrAlias1);
        initData.aliasSuffix2 = GetParametrEmptyString(domElement, AttrAlias2);

        // Make name1 and name2 mandatory
        Q_STATIC_ASSERT(VPatternConverter::PatternMinVer < FormatVersion(1, 1, 1));
        initData.name1 = GetParametrEmptyString(domElement, AttrCurveName1); // Optionally empty for compatibility
        initData.name2 = GetParametrEmptyString(domElement, AttrCurveName2); // Optionally empty for compatibility

        VToolCutSplinePath::Create(initData);
        // Rewrite attribute formula. Need for situation when we have wrong formula.
        if (f != initData.formula)
        {
            SetAttribute(domElement, AttrLength, initData.formula);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating cut spline path point"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating cut spline path point"), domElement);
        excep.AddMoreInformation("Message:     "_L1 + e.GetMsg() + '\n'_L1 + "Expression:  "_L1 + e.GetExpr());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolCutArc(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolCutInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        PointsCommonAttributes(domElement, initData);
        initData.formula = GetParametrString(domElement, AttrLength, QChar('0'));
        const QString f = initData.formula; // need for saving fixed formula;
        initData.baseCurveId = GetParametrUInt(domElement, AttrArc, NULL_ID_STR);
        initData.aliasSuffix1 = GetParametrEmptyString(domElement, AttrAlias1);
        initData.aliasSuffix2 = GetParametrEmptyString(domElement, AttrAlias2);

        // Make name1 and name2 mandatory
        Q_STATIC_ASSERT(VPatternConverter::PatternMinVer < FormatVersion(1, 1, 1));
        initData.name1 = GetParametrEmptyString(domElement, AttrCurveName1); // Optionally empty for compatibility
        initData.name2 = GetParametrEmptyString(domElement, AttrCurveName2); // Optionally empty for compatibility

        VToolCutArc::Create(initData);
        // Rewrite attribute formula. Need for situation when we have wrong formula.
        if (f != initData.formula)
        {
            SetAttribute(domElement, AttrLength, initData.formula);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating cut arc point"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating cut arc point"), domElement);
        excep.AddMoreInformation("Message:     "_L1 + e.GetMsg() + '\n'_L1 + "Expression:  "_L1 + e.GetExpr());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolLineIntersectAxis(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolLineIntersectAxisInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        PointsWithLineCommonAttributes(domElement, initData);

        initData.basePointId = GetParametrUInt(domElement, AttrBasePoint, NULL_ID_STR);
        initData.firstPointId = GetParametrUInt(domElement, AttrP1Line, NULL_ID_STR);
        initData.secondPointId = GetParametrUInt(domElement, AttrP2Line, NULL_ID_STR);

        initData.formulaAngle = GetParametrString(domElement, AttrAngle, QStringLiteral("0.0"));
        const QString angleFix = initData.formulaAngle;

        VToolLineIntersectAxis::Create(initData);
        // Rewrite attribute formula. Need for situation when we have wrong formula.
        if (angleFix != initData.formulaAngle)
        {
            SetAttribute(domElement, AttrAngle, initData.formulaAngle);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of intersection line and axis"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of intersection line and axis"), domElement);
        excep.AddMoreInformation("Message:     "_L1 + e.GetMsg() + '\n'_L1 + "Expression:  "_L1 + e.GetExpr());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolCurveIntersectAxis(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        QT_WARNING_PUSH
        QT_WARNING_DISABLE_GCC("-Wnoexcept")

        VToolCurveIntersectAxisInitData initData;

        QT_WARNING_POP

        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        PointsWithLineCommonAttributes(domElement, initData);

        initData.basePointId = GetParametrUInt(domElement, AttrBasePoint, NULL_ID_STR);
        initData.curveId = GetParametrUInt(domElement, AttrCurve, NULL_ID_STR);
        initData.formulaAngle = GetParametrString(domElement, AttrAngle, QStringLiteral("0.0"));
        const QString angleFix = initData.formulaAngle;
        initData.name1 = GetParametrEmptyString(domElement, AttrName1);
        initData.name2 = GetParametrEmptyString(domElement, AttrName2);
        initData.aliasSuffix1 = GetParametrEmptyString(domElement, AttrAlias1);
        initData.aliasSuffix2 = GetParametrEmptyString(domElement, AttrAlias2);

        VToolCurveIntersectAxis::Create(initData);
        // Rewrite attribute formula. Need for situation when we have wrong formula.
        if (angleFix != initData.formulaAngle)
        {
            SetAttribute(domElement, AttrAngle, initData.formulaAngle);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of intersection curve and axis"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of intersection curve and axis"), domElement);
        excep.AddMoreInformation("Message:     "_L1 + e.GetMsg() + '\n'_L1 + "Expression:  "_L1 + e.GetExpr());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolPointOfIntersectionArcs(VMainGraphicsScene *scene, const QDomElement &domElement,
                                                const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolPointOfIntersectionArcsInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        PointsCommonAttributes(domElement, initData);
        initData.firstArcId = GetParametrUInt(domElement, AttrFirstArc, NULL_ID_STR);
        initData.secondArcId = GetParametrUInt(domElement, AttrSecondArc, NULL_ID_STR);
        initData.pType = static_cast<CrossCirclesPoint>(GetParametrUInt(domElement, AttrCrossPoint, QChar('1')));

        VToolPointOfIntersectionArcs::Create(initData);
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of intersection arcs"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolPointOfIntersectionCircles(VMainGraphicsScene *scene, QDomElement &domElement,
                                                   const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolPointOfIntersectionCirclesInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        PointsCommonAttributes(domElement, initData);
        initData.firstCircleCenterId = GetParametrUInt(domElement, AttrC1Center, NULL_ID_STR);
        initData.secondCircleCenterId = GetParametrUInt(domElement, AttrC2Center, NULL_ID_STR);
        initData.firstCircleRadius = GetParametrString(domElement, AttrC1Radius);
        const QString c1R = initData.firstCircleRadius;
        initData.secondCircleRadius = GetParametrString(domElement, AttrC2Radius);
        const QString c2R = initData.secondCircleRadius;
        initData.crossPoint = static_cast<CrossCirclesPoint>(GetParametrUInt(domElement, AttrCrossPoint, QChar('1')));

        VToolPointOfIntersectionCircles::Create(initData);
        // Rewrite attribute formula. Need for situation when we have wrong formula.
        if (c1R != initData.firstCircleRadius || c2R != initData.secondCircleRadius)
        {
            SetAttribute(domElement, AttrC1Radius, initData.firstCircleRadius);
            SetAttribute(domElement, AttrC2Radius, initData.secondCircleRadius);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of intersection circles"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolPointOfIntersectionCurves(VMainGraphicsScene *scene, QDomElement &domElement,
                                                  const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        QT_WARNING_PUSH
        QT_WARNING_DISABLE_GCC("-Wnoexcept")

        VToolPointOfIntersectionCurvesInitData initData;

        QT_WARNING_POP

        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        PointsCommonAttributes(domElement, initData);
        initData.firstCurveId = GetParametrUInt(domElement, AttrCurve1, NULL_ID_STR);
        initData.secondCurveId = GetParametrUInt(domElement, AttrCurve2, NULL_ID_STR);
        initData.vCrossPoint = static_cast<VCrossCurvesPoint>(GetParametrUInt(domElement, AttrVCrossPoint, QChar('1')));
        initData.hCrossPoint = static_cast<HCrossCurvesPoint>(GetParametrUInt(domElement, AttrHCrossPoint, QChar('1')));
        initData.curve1Name1 = GetParametrEmptyString(domElement, AttrCurve1Name1);
        initData.curve1Name2 = GetParametrEmptyString(domElement, AttrCurve1Name2);
        initData.curve2Name1 = GetParametrEmptyString(domElement, AttrCurve2Name1);
        initData.curve2Name2 = GetParametrEmptyString(domElement, AttrCurve2Name2);
        initData.curve1AliasSuffix1 = GetParametrEmptyString(domElement, AttrCurve1Alias1);
        initData.curve1AliasSuffix2 = GetParametrEmptyString(domElement, AttrCurve1Alias2);
        initData.curve2AliasSuffix1 = GetParametrEmptyString(domElement, AttrCurve2Alias1);
        initData.curve2AliasSuffix2 = GetParametrEmptyString(domElement, AttrCurve2Alias2);

        VToolPointOfIntersectionCurves::Create(initData);
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of intersection curves"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolPointFromCircleAndTangent(VMainGraphicsScene *scene, QDomElement &domElement,
                                                  const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolPointFromCircleAndTangentInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        PointsCommonAttributes(domElement, initData);
        initData.circleCenterId = GetParametrUInt(domElement, AttrCCenter, NULL_ID_STR);
        initData.tangentPointId = GetParametrUInt(domElement, AttrTangent, NULL_ID_STR);
        initData.circleRadius = GetParametrString(domElement, AttrCRadius);
        const QString cR = initData.circleRadius;
        initData.crossPoint = static_cast<CrossCirclesPoint>(GetParametrUInt(domElement, AttrCrossPoint, QChar('1')));

        VToolPointFromCircleAndTangent::Create(initData);
        // Rewrite attribute formula. Need for situation when we have wrong formula.
        if (cR != initData.circleRadius)
        {
            SetAttribute(domElement, AttrCRadius, initData.circleRadius);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating point from circle and tangent"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolPointFromArcAndTangent(VMainGraphicsScene *scene, const QDomElement &domElement,
                                               const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolPointFromArcAndTangentInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        PointsCommonAttributes(domElement, initData);
        initData.arcId = GetParametrUInt(domElement, AttrArc, NULL_ID_STR);
        initData.tangentPointId = GetParametrUInt(domElement, AttrTangent, NULL_ID_STR);
        initData.crossPoint = static_cast<CrossCirclesPoint>(GetParametrUInt(domElement, AttrCrossPoint, QChar('1')));

        VToolPointFromArcAndTangent::Create(initData);
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating point from arc and tangent"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolTrueDarts(VMainGraphicsScene *scene, const QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolTrueDartsInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        DrawToolsCommonAttributes(domElement, initData.id, initData.notes);

        initData.p1id = GetParametrUInt(domElement, AttrPoint1, NULL_ID_STR);
        initData.p2id = GetParametrUInt(domElement, AttrPoint2, NULL_ID_STR);

        initData.baseLineP1Id = GetParametrUInt(domElement, AttrBaseLineP1, NULL_ID_STR);
        initData.baseLineP2Id = GetParametrUInt(domElement, AttrBaseLineP2, NULL_ID_STR);
        initData.dartP1Id = GetParametrUInt(domElement, AttrDartP1, NULL_ID_STR);
        initData.dartP2Id = GetParametrUInt(domElement, AttrDartP2, NULL_ID_STR);
        initData.dartP3Id = GetParametrUInt(domElement, AttrDartP3, NULL_ID_STR);

        initData.name1 = GetParametrString(domElement, AttrName1, QChar('A'));
        initData.mx1 =
            VAbstractValApplication::VApp()->toPixel(GetParametrDouble(domElement, AttrMx1, QString::number(labelMX)));
        initData.my1 =
            VAbstractValApplication::VApp()->toPixel(GetParametrDouble(domElement, AttrMy1, QString::number(labelMY)));
        initData.showLabel1 = GetParametrBool(domElement, AttrShowLabel1, trueStr);

        initData.name2 = GetParametrString(domElement, AttrName2, QChar('A'));
        initData.mx2 =
            VAbstractValApplication::VApp()->toPixel(GetParametrDouble(domElement, AttrMx2, QString::number(labelMX)));
        initData.my2 =
            VAbstractValApplication::VApp()->toPixel(GetParametrDouble(domElement, AttrMy2, QString::number(labelMY)));
        initData.showLabel2 = GetParametrBool(domElement, AttrShowLabel2, trueStr);

        VToolTrueDarts::Create(initData);
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating true darts"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
// TODO. Delete if minimal supported version is 0.2.7
void VPattern::ParseOldToolSpline(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolSplineInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        DrawToolsCommonAttributes(domElement, initData.id, initData.notes);
        initData.point1 = GetParametrUInt(domElement, AttrPoint1, NULL_ID_STR);
        initData.point4 = GetParametrUInt(domElement, AttrPoint4, NULL_ID_STR);
        const qreal angle1 = GetParametrDouble(domElement, AttrAngle1, QStringLiteral("270.0"));
        const qreal angle2 = GetParametrDouble(domElement, AttrAngle2, QStringLiteral("90.0"));
        const qreal kAsm1 = GetParametrDouble(domElement, AttrKAsm1, QStringLiteral("1.0"));
        const qreal kAsm2 = GetParametrDouble(domElement, AttrKAsm2, QStringLiteral("1.0"));
        const qreal kCurve = GetParametrDouble(domElement, AttrKCurve, QStringLiteral("1.0"));
        const QString color = GetParametrString(domElement, AttrColor, ColorBlack);
        const quint32 duplicate = GetParametrUInt(domElement, AttrDuplicate, QChar('0'));

        const auto p1 = data->GeometricObject<VPointF>(initData.point1);
        const auto p4 = data->GeometricObject<VPointF>(initData.point4);

        auto *spline = new VSpline(*p1, *p4, angle1, angle2, kAsm1, kAsm2, kCurve);
        if (duplicate > 0)
        {
            spline->SetDuplicate(duplicate);
        }
        spline->SetColor(color);

        VToolSpline::Create(initData, spline);

        // Convert to newer format
        SetAttribute(domElement, AttrType, VToolSpline::ToolType);
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating simple curve"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolSpline(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolSplineInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        DrawToolsCommonAttributes(domElement, initData.id, initData.notes);
        initData.point1 = GetParametrUInt(domElement, AttrPoint1, NULL_ID_STR);
        initData.point4 = GetParametrUInt(domElement, AttrPoint4, NULL_ID_STR);

        initData.a1 = GetParametrString(domElement, AttrAngle1, QChar('0'));
        const QString angle1 = initData.a1; // need for saving fixed formula;

        initData.a2 = GetParametrString(domElement, AttrAngle2, QChar('0'));
        const QString angle2 = initData.a2; // need for saving fixed formula;

        initData.l1 = GetParametrString(domElement, AttrLength1, QChar('0'));
        const QString length1 = initData.l1; // need for saving fixed formula;

        initData.l2 = GetParametrString(domElement, AttrLength2, QChar('0'));
        const QString length2 = initData.l2; // need for saving fixed formula;

        initData.color = GetParametrString(domElement, AttrColor, ColorBlack);
        initData.penStyle = GetParametrString(domElement, AttrPenStyle, TypeLineLine);
        initData.duplicate = GetParametrUInt(domElement, AttrDuplicate, QChar('0'));
        initData.approximationScale = GetParametrDouble(domElement, AttrAScale, QChar('0'));
        initData.aliasSuffix = GetParametrEmptyString(domElement, AttrAlias);

        if (const VToolSpline *spl = VToolSpline::Create(initData); spl != nullptr)
        {
            auto *window = qobject_cast<VAbstractMainWindow *>(VAbstractValApplication::VApp()->getMainWindow());
            SCASSERT(window != nullptr)
            connect(spl, &VToolSpline::ToolTip, window, &VAbstractMainWindow::ShowToolTip);
        }

        // Rewrite attribute formula. Need for situation when we have wrong formula.
        if (angle1 != initData.a1 || angle2 != initData.a2 || length1 != initData.l1 || length2 != initData.l2)
        {
            SetAttribute(domElement, AttrAngle1, initData.a1);
            SetAttribute(domElement, AttrAngle2, initData.a2);
            SetAttribute(domElement, AttrLength1, initData.l1);
            SetAttribute(domElement, AttrLength2, initData.l2);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating simple curve"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating simple interactive spline"), domElement);
        excep.AddMoreInformation("Message:     "_L1 + e.GetMsg() + '\n'_L1 + "Expression:  "_L1 + e.GetExpr());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolCubicBezier(VMainGraphicsScene *scene, const QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolCubicBezierInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        DrawToolsCommonAttributes(domElement, initData.id, initData.notes);

        const quint32 point1 = GetParametrUInt(domElement, AttrPoint1, NULL_ID_STR);
        const quint32 point2 = GetParametrUInt(domElement, AttrPoint2, NULL_ID_STR);
        const quint32 point3 = GetParametrUInt(domElement, AttrPoint3, NULL_ID_STR);
        const quint32 point4 = GetParametrUInt(domElement, AttrPoint4, NULL_ID_STR);

        const QString color = GetParametrString(domElement, AttrColor, ColorBlack);
        const QString penStyle = GetParametrString(domElement, AttrPenStyle, TypeLineLine);
        const quint32 duplicate = GetParametrUInt(domElement, AttrDuplicate, QChar('0'));
        const qreal approximationScale = GetParametrDouble(domElement, AttrAScale, QChar('0'));
        const QString alias = GetParametrEmptyString(domElement, AttrAlias);

        auto p1 = data->GeometricObject<VPointF>(point1);
        auto p2 = data->GeometricObject<VPointF>(point2);
        auto p3 = data->GeometricObject<VPointF>(point3);
        auto p4 = data->GeometricObject<VPointF>(point4);

        initData.spline = new VCubicBezier(*p1, *p2, *p3, *p4);
        if (duplicate > 0)
        {
            initData.spline->SetDuplicate(duplicate);
        }
        initData.spline->SetColor(color);
        initData.spline->SetPenStyle(penStyle);
        initData.spline->SetPenStyle(penStyle);
        initData.spline->SetApproximationScale(approximationScale);
        initData.spline->SetAliasSuffix(alias);

        VToolCubicBezier::Create(initData);
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating cubic bezier curve"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseOldToolSplinePath(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    // TODO. Delete if minimal supported version is 0.2.7
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 7), "Time to refactor the code.");

    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolSplinePathInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        DrawToolsCommonAttributes(domElement, initData.id, initData.notes);
        const qreal kCurve = GetParametrDouble(domElement, AttrKCurve, QStringLiteral("1.0"));
        const QString color = GetParametrString(domElement, AttrColor, ColorBlack);
        const quint32 duplicate = GetParametrUInt(domElement, AttrDuplicate, QChar('0'));
        const qreal approximationScale = GetParametrDouble(domElement, AttrAScale, QChar('0'));

        QVector<VFSplinePoint> points;

        const QDomNodeList nodeList = domElement.childNodes();
        QDOM_LOOP(nodeList, i)
        {
            if (const QDomElement element = QDOM_ELEMENT(nodeList, i).toElement();
                !element.isNull() && element.tagName() == TagPathPoint)
            {
                const qreal kAsm1 = GetParametrDouble(element, AttrKAsm1, QStringLiteral("1.0"));
                const qreal angle = GetParametrDouble(element, AttrAngle, QChar('0'));
                const qreal kAsm2 = GetParametrDouble(element, AttrKAsm2, QStringLiteral("1.0"));
                const quint32 pSpline = GetParametrUInt(element, AttrPSpline, NULL_ID_STR);
                initData.points.append(pSpline);
                const VPointF p = *data->GeometricObject<VPointF>(pSpline);

                QLineF line(0, 0, 100, 0);
                line.setAngle(angle + 180);

                VFSplinePoint const splPoint(p, kAsm1, line.angle(), kAsm2, angle);
                points.append(splPoint);
            }
        }

        auto *path = new VSplinePath(points, kCurve);
        if (duplicate > 0)
        {
            path->SetDuplicate(duplicate);
        }
        path->SetColor(color);
        path->SetApproximationScale(approximationScale);

        VToolSplinePath::Create(initData, path);

        // Convert to newer format
        SetAttribute(domElement, AttrType, VToolSplinePath::ToolType);
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating curve path"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolSplinePath(VMainGraphicsScene *scene, const QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolSplinePathInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        DrawToolsCommonAttributes(domElement, initData.id, initData.notes);
        initData.color = GetParametrString(domElement, AttrColor, ColorBlack);
        initData.penStyle = GetParametrString(domElement, AttrPenStyle, TypeLineLine);
        initData.duplicate = GetParametrUInt(domElement, AttrDuplicate, QChar('0'));
        initData.approximationScale = GetParametrDouble(domElement, AttrAScale, QChar('0'));
        initData.aliasSuffix = GetParametrEmptyString(domElement, AttrAlias);

        const QDomNodeList nodeList = domElement.childNodes();
        QDOM_LOOP(nodeList, i)
        {
            if (const QDomElement element = QDOM_ELEMENT(nodeList, i).toElement();
                not element.isNull() && element.tagName() == TagPathPoint)
            {
                initData.a1.append(GetParametrString(element, AttrAngle1, QChar('0')));
                initData.a2.append(GetParametrString(element, AttrAngle2, QChar('0')));
                initData.l1.append(GetParametrString(element, AttrLength1, QChar('0')));
                initData.l2.append(GetParametrString(element, AttrLength2, QChar('0')));
                const quint32 pSpline = GetParametrUInt(element, AttrPSpline, NULL_ID_STR);
                initData.points.append(pSpline);
            }
        }

        // need for saving fixed formula;
        const QVector<QString> angle1 = initData.a1;
        const QVector<QString> angle2 = initData.a2;
        const QVector<QString> length1 = initData.l1;
        const QVector<QString> length2 = initData.l2;

        if (const VToolSplinePath *spl = VToolSplinePath::Create(initData); spl != nullptr)
        {
            const auto *window = qobject_cast<VAbstractMainWindow *>(VAbstractValApplication::VApp()->getMainWindow());
            SCASSERT(window != nullptr)
            connect(spl, &VToolSplinePath::ToolTip, window, &VAbstractMainWindow::ShowToolTip);
        }

        // Rewrite attribute formula. Need for situation when we have wrong formula.
        int count = 0;
        QDOM_LOOP(nodeList, i)
        {
            if (QDomElement element = QDOM_ELEMENT(nodeList, i).toElement();
                not element.isNull() && element.tagName() == TagPathPoint)
            {
                if (angle1.at(count) != initData.a1.at(count) || angle2.at(count) != initData.a2.at(count) ||
                    length1.at(count) != initData.l1.at(count) || length2.at(count) != initData.l2.at(count))
                {
                    SetAttribute(element, AttrAngle1, initData.a1.at(count));
                    SetAttribute(element, AttrAngle2, initData.a2.at(count));
                    SetAttribute(element, AttrLength1, initData.l1.at(count));
                    SetAttribute(element, AttrLength2, initData.l2.at(count));
                    modified = true;
                    haveLiteChange();
                }
                ++count;
            }
        }
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating curve path"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating interactive spline path"), domElement);
        excep.AddMoreInformation("Message:     "_L1 + e.GetMsg() + '\n'_L1 + "Expression:  "_L1 + e.GetExpr());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolParallelCurve(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolParallelCurveInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        DrawToolsCommonAttributes(domElement, initData.id, initData.notes);
        initData.formulaWidth = GetParametrString(domElement, AttrWidth, QChar('0'));
        const QString fWidth = initData.formulaWidth; // need for saving fixed formula;
        initData.originCurveId = GetParametrUInt(domElement, AttrCurve, NULL_ID_STR);
        initData.color = GetParametrString(domElement, AttrColor, ColorBlack);
        initData.penStyle = GetParametrString(domElement, AttrPenStyle, TypeLineLine);
        initData.approximationScale = GetParametrDouble(domElement, AttrAScale, QChar('0'));
        initData.aliasSuffix = GetParametrEmptyString(domElement, AttrAlias);
        initData.name = GetParametrEmptyString(domElement, AttrName);

        // We no longer need to handle suffix attribute here. The code can be removed.
        Q_STATIC_ASSERT(VPatternConverter::PatternMinVer < FormatVersion(1, 1, 1));
        if (initData.name.isEmpty())
        {
            const QString suffix = GetParametrEmptyString(domElement, AttrSuffix);
            const QSharedPointer<VAbstractCurve> curve = initData.data->GeometricObject<VAbstractCurve>(
                initData.originCurveId);
            initData.name = curve->HeadlessName() + suffix;
        }

        VToolParallelCurve::Create(initData);
        // Rewrite attribute formula. Need for situation when we have wrong formula.
        if (fWidth != initData.formulaWidth)
        {
            SetAttribute(domElement, AttrWidth, initData.formulaWidth);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating a parallel curve"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating a parallel curve"), domElement);
        excep.AddMoreInformation("Message:     "_L1 + e.GetMsg() + '\n'_L1 + "Expression:  "_L1 + e.GetExpr());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolGraduatedCurve(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolGraduatedCurveInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        DrawToolsCommonAttributes(domElement, initData.id, initData.notes);
        initData.originCurveId = GetParametrUInt(domElement, AttrCurve, NULL_ID_STR);
        initData.color = GetParametrString(domElement, AttrColor, ColorBlack);
        initData.penStyle = GetParametrString(domElement, AttrPenStyle, TypeLineLine);
        initData.approximationScale = GetParametrDouble(domElement, AttrAScale, QChar('0'));
        initData.aliasSuffix = GetParametrEmptyString(domElement, AttrAlias);
        initData.name = GetParametrEmptyString(domElement, AttrName);
        initData.offsets = VToolGraduatedCurve::ExtractOffsetData(domElement);

        const QDomNodeList nodeList = domElement.childNodes();
        const qint32 num = nodeList.size();
        initData.offsets.reserve(num);

        QVector<QString> originalFormulas;
        originalFormulas.reserve(initData.offsets.size());
        for (const auto &offsetData : std::as_const(initData.offsets))
        {
            originalFormulas.append(offsetData.formula); // need for saving fixed formula;
        }

        // We no longer need to handle suffix attribute here. The code can be removed.
        Q_STATIC_ASSERT(VPatternConverter::PatternMinVer < FormatVersion(1, 1, 1));
        if (initData.name.isEmpty())
        {
            const QString suffix = GetParametrEmptyString(domElement, AttrSuffix);
            const QSharedPointer<VAbstractCurve> curve = initData.data->GeometricObject<VAbstractCurve>(
                initData.originCurveId);
            initData.name = curve->HeadlessName() + suffix;
        }

        VToolGraduatedCurve::Create(initData);
        // Rewrite attribute formula. Need for situation when we have wrong formula.
        for (qint32 i = 0; i < num; ++i)
        {
            if (QDomElement element = nodeList.at(i).toElement(); not element.isNull() && element.tagName() == TagOffset)
            {
                if (const QString &newFormula = initData.offsets.at(i).formula; originalFormulas.at(i) != newFormula)
                {
                    SetAttribute(element, AttrWidth, newFormula);
                    modified = true;
                    haveLiteChange();
                }
            }
        }
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating a graduated curve"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating a graduated curve"), domElement);
        excep.AddMoreInformation("Message:     "_L1 + e.GetMsg() + '\n'_L1 + "Expression:  "_L1 + e.GetExpr());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolCubicBezierPath(VMainGraphicsScene *scene, const QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolCubicBezierPathInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        DrawToolsCommonAttributes(domElement, initData.id, initData.notes);
        const QString color = GetParametrString(domElement, AttrColor, ColorBlack);
        const QString penStyle = GetParametrString(domElement, AttrPenStyle, TypeLineLine);
        const quint32 duplicate = GetParametrUInt(domElement, AttrDuplicate, QChar('0'));
        const qreal approximationScale = GetParametrDouble(domElement, AttrAScale, QChar('0'));
        const QString alias = GetParametrEmptyString(domElement, AttrAlias);

        QVector<VPointF> points;

        const QDomNodeList nodeList = domElement.childNodes();
        QDOM_LOOP(nodeList, i)
        {
            if (const QDomElement element = QDOM_ELEMENT(nodeList, i).toElement();
                !element.isNull() && element.tagName() == TagPathPoint)
            {
                const quint32 pSpline = GetParametrUInt(element, AttrPSpline, NULL_ID_STR);
                const VPointF p = *data->GeometricObject<VPointF>(pSpline);
                points.append(p);
            }
        }

        initData.path = new VCubicBezierPath(points);
        if (duplicate > 0)
        {
            initData.path->SetDuplicate(duplicate);
        }
        initData.path->SetColor(color);
        initData.path->SetPenStyle(penStyle);
        initData.path->SetApproximationScale(approximationScale);
        initData.path->SetAliasSuffix(alias);

        VToolCubicBezierPath::Create(initData);
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating cubic bezier path curve"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseNodeSpline(const QDomElement &domElement, const Document &parse)
{
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VAbstractNodeInitData initData;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        SplinesCommonAttributes(domElement, initData.id, initData.idObject, initData.idTool);

        VNodeSpline::Create(initData);
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating modeling simple curve"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseNodeSplinePath(const QDomElement &domElement, const Document &parse)
{
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VAbstractNodeInitData initData;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        SplinesCommonAttributes(domElement, initData.id, initData.idObject, initData.idTool);

        VNodeSplinePath::Create(initData);
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating modeling curve path"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolArc(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(domElement.isNull() == false, Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolArcInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        DrawToolsCommonAttributes(domElement, initData.id, initData.notes);
        initData.center = GetParametrUInt(domElement, AttrCenter, NULL_ID_STR);
        initData.radius = GetParametrString(domElement, AttrRadius, QStringLiteral("10"));
        const QString r = initData.radius; // need for saving fixed formula;
        initData.f1 = GetParametrString(domElement, AttrAngle1, QStringLiteral("180"));
        const QString f1Fix = initData.f1; // need for saving fixed formula;
        initData.f2 = GetParametrString(domElement, AttrAngle2, QStringLiteral("270"));
        const QString f2Fix = initData.f2; // need for saving fixed formula;
        initData.color = GetParametrString(domElement, AttrColor, ColorBlack);
        initData.penStyle = GetParametrString(domElement, AttrPenStyle, TypeLineLine);
        initData.approximationScale = GetParametrDouble(domElement, AttrAScale, QChar('0'));
        initData.aliasSuffix = GetParametrEmptyString(domElement, AttrAlias);

        VToolArc::Create(initData);
        // Rewrite attribute formula. Need for situation when we have wrong formula.
        if (r != initData.radius || f1Fix != initData.f1 || f2Fix != initData.f2)
        {
            SetAttribute(domElement, AttrRadius, initData.radius);
            SetAttribute(domElement, AttrAngle1, initData.f1);
            SetAttribute(domElement, AttrAngle2, initData.f2);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating simple arc"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating simple arc"), domElement);
        excep.AddMoreInformation("Message:     "_L1 + e.GetMsg() + '\n'_L1 + "Expression:  "_L1 + e.GetExpr());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolEllipticalArc(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(domElement.isNull() == false, Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolEllipticalArcInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        DrawToolsCommonAttributes(domElement, initData.id, initData.notes);
        initData.center = GetParametrUInt(domElement, AttrCenter, NULL_ID_STR);
        initData.radius1 = GetParametrString(domElement, AttrRadius1, QStringLiteral("10"));
        initData.radius2 = GetParametrString(domElement, AttrRadius2, QStringLiteral("10"));
        const QString r1 = initData.radius1; // need for saving fixed formula;
        const QString r2 = initData.radius2; // need for saving fixed formula;
        initData.f1 = GetParametrString(domElement, AttrAngle1, QStringLiteral("180"));
        const QString f1Fix = initData.f1; // need for saving fixed formula;
        initData.f2 = GetParametrString(domElement, AttrAngle2, QStringLiteral("270"));
        const QString f2Fix = initData.f2; // need for saving fixed formula;
        initData.rotationAngle = GetParametrString(domElement, AttrRotationAngle, QChar('0'));
        const QString frotationFix = initData.rotationAngle; // need for saving fixed formula;
        initData.color = GetParametrString(domElement, AttrColor, ColorBlack);
        initData.penStyle = GetParametrString(domElement, AttrPenStyle, TypeLineLine);
        initData.approximationScale = GetParametrDouble(domElement, AttrAScale, QChar('0'));
        initData.aliasSuffix = GetParametrEmptyString(domElement, AttrAlias);

        VToolEllipticalArc::Create(initData);
        // Rewrite attribute formula. Need for situation when we have wrong formula.
        if (r1 != initData.radius1 || r2 != initData.radius2 || f1Fix != initData.f1 || f2Fix != initData.f2 ||
            frotationFix != initData.rotationAngle)
        {
            SetAttribute(domElement, AttrRadius1, initData.radius1);
            SetAttribute(domElement, AttrRadius2, initData.radius2);
            SetAttribute(domElement, AttrAngle1, initData.f1);
            SetAttribute(domElement, AttrAngle2, initData.f2);
            SetAttribute(domElement, AttrRotationAngle, initData.rotationAngle);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating simple elliptical arc"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating simple elliptical arc"), domElement);
        excep.AddMoreInformation("Message:     "_L1 + e.GetMsg() + '\n'_L1 + "Expression:  "_L1 + e.GetExpr());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseNodeEllipticalArc(const QDomElement &domElement, const Document &parse)
{
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VAbstractNodeInitData initData;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        ToolsCommonAttributes(domElement, initData.id);
        initData.idObject = GetParametrUInt(domElement, AttrIdObject, NULL_ID_STR);
        initData.idTool = GetParametrUInt(domElement, VAbstractNode::AttrIdTool, NULL_ID_STR);

        VNodeEllipticalArc::Create(initData);
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating modeling elliptical arc"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolEllipticalArcWithLength(VMainGraphicsScene *scene,
                                                QDomElement &domElement,
                                                const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(domElement.isNull() == false, Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolEllipticalArcWithLengthInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        DrawToolsCommonAttributes(domElement, initData.id, initData.notes);
        initData.center = GetParametrUInt(domElement, AttrCenter, NULL_ID_STR);
        initData.radius1 = GetParametrString(domElement, AttrRadius1, QStringLiteral("10"));
        initData.radius2 = GetParametrString(domElement, AttrRadius2, QStringLiteral("10"));
        const QString r1 = initData.radius1; // need for saving fixed formula;
        const QString r2 = initData.radius2; // need for saving fixed formula;
        initData.f1 = GetParametrString(domElement, AttrAngle1, QStringLiteral("180"));
        const QString f1Fix = initData.f1; // need for saving fixed formula;
        initData.length = GetParametrString(domElement, AttrLength, QStringLiteral("10"));
        const QString lengthFix = initData.length; // need for saving fixed length;
        initData.rotationAngle = GetParametrString(domElement, AttrRotationAngle, QChar('0'));
        const QString frotationFix = initData.rotationAngle; // need for saving fixed formula;
        initData.color = GetParametrString(domElement, AttrColor, ColorBlack);
        initData.penStyle = GetParametrString(domElement, AttrPenStyle, TypeLineLine);
        initData.approximationScale = GetParametrDouble(domElement, AttrAScale, QChar('0'));
        initData.aliasSuffix = GetParametrEmptyString(domElement, AttrAlias);

        VToolEllipticalArcWithLength::Create(initData);
        // Rewrite attribute formula. Need for situation when we have wrong formula.
        if (r1 != initData.radius1 || r2 != initData.radius2 || f1Fix != initData.f1 || lengthFix != initData.length
            || frotationFix != initData.rotationAngle)
        {
            SetAttribute(domElement, AttrRadius1, initData.radius1);
            SetAttribute(domElement, AttrRadius2, initData.radius2);
            SetAttribute(domElement, AttrAngle1, initData.f1);
            SetAttribute(domElement, AttrLength, initData.length);
            SetAttribute(domElement, AttrRotationAngle, initData.rotationAngle);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating elliptical arc with length"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating elliptical arc with length"), domElement);
        excep.AddMoreInformation("Message:     "_L1 + e.GetMsg() + '\n'_L1 + "Expression:  "_L1 + e.GetExpr());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseNodeArc(const QDomElement &domElement, const Document &parse)
{
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        VAbstractNodeInitData initData;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        ToolsCommonAttributes(domElement, initData.id);
        initData.idObject = GetParametrUInt(domElement, AttrIdObject, NULL_ID_STR);
        initData.idTool = GetParametrUInt(domElement, VAbstractNode::AttrIdTool, NULL_ID_STR);

        VNodeArc::Create(initData);
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating modeling arc"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolArcWithLength(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(domElement.isNull() == false, Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolArcWithLengthInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        DrawToolsCommonAttributes(domElement, initData.id, initData.notes);
        initData.center = GetParametrUInt(domElement, AttrCenter, NULL_ID_STR);
        initData.radius = GetParametrString(domElement, AttrRadius, QStringLiteral("10"));
        const QString r = initData.radius; // need for saving fixed formula;
        initData.f1 = GetParametrString(domElement, AttrAngle1, QStringLiteral("180"));
        const QString f1Fix = initData.f1; // need for saving fixed formula;
        initData.length = GetParametrString(domElement, AttrLength, QStringLiteral("10"));
        const QString lengthFix = initData.length; // need for saving fixed length;
        initData.color = GetParametrString(domElement, AttrColor, ColorBlack);
        initData.penStyle = GetParametrString(domElement, AttrPenStyle, TypeLineLine);
        initData.approximationScale = GetParametrDouble(domElement, AttrAScale, QChar('0'));
        initData.aliasSuffix = GetParametrEmptyString(domElement, AttrAlias);

        VToolArcWithLength::Create(initData);
        // Rewrite attribute formula. Need for situation when we have wrong formula.
        if (r != initData.radius || f1Fix != initData.f1 || lengthFix != initData.length)
        {
            SetAttribute(domElement, AttrRadius, initData.radius);
            SetAttribute(domElement, AttrAngle1, initData.f1);
            SetAttribute(domElement, AttrLength, initData.length);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating simple arc"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating simple arc"), domElement);
        excep.AddMoreInformation("Message:     "_L1 + e.GetMsg() + '\n'_L1 + "Expression:  "_L1 + e.GetExpr());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolRotation(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(domElement.isNull() == false, Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolRotationInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        DrawToolsCommonAttributes(domElement, initData.id, initData.notes);
        initData.origin = GetParametrUInt(domElement, AttrCenter, NULL_ID_STR);
        initData.angle = GetParametrString(domElement, AttrAngle, QStringLiteral("10"));
        const QString a = initData.angle; // need for saving fixed formula;

        // We no longer need to handle suffix attribute here. The code can be removed.
        Q_STATIC_ASSERT(VPatternConverter::PatternMinVer < FormatVersion(1, 1, 1));
        initData.suffix = GetParametrEmptyString(domElement, AttrSuffix);

        VAbstractOperation::ExtractData(domElement, initData);

        VToolRotation::Create(initData);
        // Rewrite attribute formula. Need for situation when we have wrong formula.
        if (a != initData.angle)
        {
            SetAttribute(domElement, AttrAngle, initData.angle);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating operation of rotation"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating operation of rotation"), domElement);
        excep.AddMoreInformation("Message:     "_L1 + e.GetMsg() + '\n'_L1 + "Expression:  "_L1 + e.GetExpr());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolFlippingByLine(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(domElement.isNull() == false, Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolFlippingByLineInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        DrawToolsCommonAttributes(domElement, initData.id, initData.notes);
        initData.firstLinePointId = GetParametrUInt(domElement, AttrP1Line, NULL_ID_STR);
        initData.secondLinePointId = GetParametrUInt(domElement, AttrP2Line, NULL_ID_STR);

        // We no longer need to handle suffix attribute here. The code can be removed.
        Q_STATIC_ASSERT(VPatternConverter::PatternMinVer < FormatVersion(1, 1, 1));
        initData.suffix = GetParametrEmptyString(domElement, AttrSuffix);

        VAbstractOperation::ExtractData(domElement, initData);

        VToolFlippingByLine::Create(initData);
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating operation of flipping by line"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolFlippingByAxis(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(domElement.isNull() == false, Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolFlippingByAxisInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        DrawToolsCommonAttributes(domElement, initData.id, initData.notes);
        initData.originPointId = GetParametrUInt(domElement, AttrCenter, NULL_ID_STR);
        initData.axisType = static_cast<AxisType>(GetParametrUInt(domElement, AttrAxisType, QChar('1')));

        // We no longer need to handle suffix attribute here. The code can be removed.
        Q_STATIC_ASSERT(VPatternConverter::PatternMinVer < FormatVersion(1, 1, 1));
        initData.suffix = GetParametrEmptyString(domElement, AttrSuffix);

        VAbstractOperation::ExtractData(domElement, initData);

        VToolFlippingByAxis::Create(initData);
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating operation of flipping by axis"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolMove(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(domElement.isNull() == false, Q_FUNC_INFO, "domElement is null");

    try
    {
        VToolMoveInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        DrawToolsCommonAttributes(domElement, initData.id, initData.notes);
        initData.formulaAngle = GetParametrString(domElement, AttrAngle, QChar('0'));
        const QString a = initData.formulaAngle; // need for saving fixed formula;
        initData.formulaRotationAngle = GetParametrString(domElement, AttrRotationAngle, QChar('0'));
        const QString r = initData.formulaRotationAngle; // need for saving fixed formula;
        initData.formulaLength = GetParametrString(domElement, AttrLength, QChar('0'));
        const QString len = initData.formulaLength; // need for saving fixed formula;
        initData.rotationOrigin = GetParametrUInt(domElement, AttrCenter, NULL_ID_STR);

        // We no longer need to handle suffix attribute here. The code can be removed.
        Q_STATIC_ASSERT(VPatternConverter::PatternMinVer < FormatVersion(1, 1, 1));
        initData.suffix = GetParametrEmptyString(domElement, AttrSuffix);

        VAbstractOperation::ExtractData(domElement, initData);

        VToolMove::Create(initData);
        // Rewrite attribute formula. Need for situation when we have wrong formula.
        if (a != initData.formulaAngle || r != initData.formulaRotationAngle || len != initData.formulaLength)
        {
            SetAttribute(domElement, AttrAngle, initData.formulaAngle);
            SetAttribute(domElement, AttrRotationAngle, initData.formulaRotationAngle);
            SetAttribute(domElement, AttrLength, initData.formulaLength);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating operation of moving"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating operation of moving"), domElement);
        excep.AddMoreInformation("Message:     "_L1 + e.GetMsg() + '\n'_L1 + "Expression:  "_L1 + e.GetExpr());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPattern::EvalFormula(VContainer *data, const QString &formula, bool *ok) const -> qreal
{
    if (formula.isEmpty())
    {
        *ok = true;
        return 0;
    }

    try
    {
        QScopedPointer<Calculator> cal(new Calculator());
        const qreal result = cal->EvalFormula(data->DataVariables(), formula);

        (qIsInf(result) || qIsNaN(result)) ? *ok = false : *ok = true;
        return result;
    }
    catch (qmu::QmuParserError &e)
    {
        Q_UNUSED(e)
        *ok = false;
        return 0;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPattern::MakeEmptyIncrement(const QString &name, IncrementType type) -> QDomElement
{
    QDomElement element = createElement(TagIncrement);
    SetAttribute(element, AttrName, name);
    if (type != IncrementType::Increment)
    {
        SetAttribute(element, AttrType, IncrementTypeToString(type));
    }
    return element;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPattern::FindIncrement(const QString &name) const -> QDomElement
{
    QDomNodeList const list = elementsByTagName(TagIncrement);

    for (int i = 0; i < list.size(); ++i)
    {
        if (const QDomElement domElement = list.at(i).toElement(); not domElement.isNull())
        {
            if (const QString parameter = domElement.attribute(AttrName); parameter == name)
            {
                return domElement;
            }
        }
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::GarbageCollector()
{
    bool cleared = false;

    VPatternGraph const *graph = PatternGraph();

    QSet<vidtype> candidates = ConvertToSet(graph->GetVerticesByType(VNodeType ::MODELING_OBJECT));
    candidates += ConvertToSet(graph->GetVerticesByType(VNodeType ::MODELING_TOOL));

    EraseIf(candidates,
            [graph, this](auto node) -> bool
            {
                auto Filter = [](const auto &node) -> auto { return node.type == VNodeType::PIECE; };
                const QVector<VNode> nodeDependencies = graph->GetDependentNodes(node, Filter);
                return !nodeDependencies.isEmpty() || FindElementById(node).isNull();
            });

    if (candidates.isEmpty())
    {
        return;
    }

    BackupBeforeGarbageCollector();

    QSet<vidtype> clearedNodes;
    clearedNodes.reserve(candidates.size());

    for (const auto node : std::as_const(candidates))
    {
        QDomElement const domElement = FindElementById(node);
        if (domElement.isNull())
        {
            continue;
        }

        if (QDomElement parent = domElement.parentNode().toElement();
            !parent.isNull() && parent.tagName() == TagModeling)
        {
            parent.removeChild(domElement);
            cleared = true;
            clearedNodes.insert(node);
        }
    }

    if (!cleared)
    {
        return;
    }

    QVector<VToolRecord> newHistory;
    newHistory.reserve(history.size());

    for (const auto &record : std::as_const(history))
    {
        if (!clearedNodes.contains(record.GetId()))
        {
            newHistory.append(record);
        }
    }

    history = newHistory;

    if (cleared)
    {
        RefreshElementIdCache();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::BackupBeforeGarbageCollector() const
{
    if (!m_garbageCollectBackupFilePath.isEmpty())
    {
        QString error;
        QFileInfo const info(m_garbageCollectBackupFilePath);
#if defined(Q_OS_UNIX) || defined(Q_OS_MACOS)
        const QString hidden = QChar('.');
#else
        const QString hidden;
#endif
        const QString backupFileName = u"%1/%2%3.gb.bak"_s.arg(info.absoluteDir().absolutePath(),
                                                               hidden,
                                                               info.fileName());
        if (VDomDocument::SafeCopy(m_garbageCollectBackupFilePath, backupFileName, error))
        {
#if defined(Q_OS_WIN)
            SetFileAttributesW(backupFileName.toStdWString().c_str(), FILE_ATTRIBUTE_HIDDEN);
#endif
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::NewEmptyIncrement(const QString &type, const QString &name, IncrementType varType)
{
    const QDomElement element = MakeEmptyIncrement(name, varType);

    const QDomNodeList list = elementsByTagName(type);
    list.at(0).appendChild(element);
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::NewEmptyIncrementAfter(const QString &type, const QString &after, const QString &name,
                                      IncrementType varType)
{
    const QDomElement element = MakeEmptyIncrement(name, varType);
    const QDomElement sibling = FindIncrement(after);

    const QDomNodeList list = elementsByTagName(type);

    if (sibling.isNull())
    {
        list.at(0).appendChild(element);
    }
    else
    {
        list.at(0).insertAfter(element, sibling);
    }
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::RemoveIncrement(const QString &type, const QString &name)
{
    const QDomNodeList list = elementsByTagName(type);
    list.at(0).removeChild(FindIncrement(name));
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::MoveUpIncrement(const QString &type, const QString &name)
{
    if (const QDomElement node = FindIncrement(name); not node.isNull())
    {
        const QDomElement prSibling = node.previousSiblingElement(TagIncrement);
        if (not prSibling.isNull())
        {
            elementsByTagName(type).at(0).insertBefore(node, prSibling);
        }
    }
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::MoveDownIncrement(const QString &type, const QString &name)
{
    if (const QDomElement node = FindIncrement(name); not node.isNull())
    {
        const QDomElement nextSibling = node.nextSiblingElement(TagIncrement);
        if (not nextSibling.isNull())
        {
            elementsByTagName(type).at(0).insertAfter(node, nextSibling);
        }
    }
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::SetIncrementAttribute(const QString &name, const QString &attr, const QString &text)
{
    if (QDomElement node = FindIncrement(name); not node.isNull())
    {
        SetAttribute(node, attr, text);
        emit patternChanged(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPattern::LastDrawName() const -> QString
{
    const QDomNodeList elements = this->documentElement().elementsByTagName(TagDraw);
    if (elements.isEmpty())
    {
        return {};
    }

    if (const QDomElement &elem = elements.at(elements.size() - 1).toElement(); not elem.isNull())
    {
        return GetParametrString(elem, AttrName);
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPattern::LastToolId() const -> quint32
{
    const QString name = LastDrawName();
    if (name.isEmpty())
    {
        return NULL_ID;
    }

    return PPLastToolId(PatternBlockMapper()->FindId(name));
}

//---------------------------------------------------------------------------------------------------------------------
auto VPattern::PPLastToolId(int blockIndex) const -> quint32
{
    const QVector<VToolRecord> localHistory = GetLocalHistory(blockIndex);

    return (not localHistory.isEmpty() ? localHistory.constLast().GetId() : NULL_ID);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::RefreshPieceGeometry()
{
    if (!VAbstractApplication::VApp()->IsAppInGUIMode()
        || VAbstractValApplication::VApp()->GetDrawMode() == Draw::Modeling)
    {
        if (updatePieces.isEmpty())
        {
            updatePieces = data->DataPieces()->keys();
        }
        RefreshDirtyPieceGeometry(updatePieces);
    }
    else
    {
        m_pieceGeometryDirty = true;
    }

    updatePieces.clear();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ParseSplineElement parse spline tag.
 * @param scene scene.
 * @param domElement tag in xml tree.
 * @param parse parser file mode.
 * @param type type of spline.
 */
void VPattern::ParseSplineElement(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse,
                                  const QString &type)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(domElement.isNull() == false, Q_FUNC_INFO, "domElement is null");
    Q_ASSERT_X(type.isEmpty() == false, Q_FUNC_INFO, "type of spline is empty");

    static const QStringList splines({VToolSpline::OldToolType,        /*0*/
                                      VToolSpline::ToolType,           /*1*/
                                      VToolSplinePath::OldToolType,    /*2*/
                                      VToolSplinePath::ToolType,       /*3*/
                                      VNodeSpline::ToolType,           /*4*/
                                      VNodeSplinePath::ToolType,       /*5*/
                                      VToolCubicBezier::ToolType,      /*6*/
                                      VToolCubicBezierPath::ToolType,  /*7*/
                                      VToolParallelCurve::ToolType,    /*8*/
                                      VToolGraduatedCurve::ToolType}); /*9*/
    switch (splines.indexOf(type))
    {
        case 0: // VToolSpline::OldToolType
            qCDebug(vXML, "VOldToolSpline.");
            // TODO. Delete if minimal supported version is 0.2.7
            Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 7), "Time to refactor the code.");
            ParseOldToolSpline(scene, domElement, parse);
            break;
        case 1: // VToolSpline::ToolType
            qCDebug(vXML, "VToolSpline.");
            ParseToolSpline(scene, domElement, parse);
            break;
        case 2: // VToolSplinePath::OldToolType
            qCDebug(vXML, "VOldToolSplinePath.");
            // TODO. Delete if minimal supported version is 0.2.7
            Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 7), "Time to refactor the code.");
            ParseOldToolSplinePath(scene, domElement, parse);
            break;
        case 3: // VToolSplinePath::ToolType
            qCDebug(vXML, "VToolSplinePath.");
            ParseToolSplinePath(scene, domElement, parse);
            break;
        case 4: // VNodeSpline::ToolType
            qCDebug(vXML, "VNodeSpline.");
            ParseNodeSpline(domElement, parse);
            break;
        case 5: // VNodeSplinePath::ToolType
            qCDebug(vXML, "VNodeSplinePath.");
            ParseNodeSplinePath(domElement, parse);
            break;
        case 6: // VToolCubicBezier::ToolType
            qCDebug(vXML, "VToolCubicBezier.");
            ParseToolCubicBezier(scene, domElement, parse);
            break;
        case 7: // VToolCubicBezierPath::ToolType
            qCDebug(vXML, "VToolCubicBezierPath.");
            ParseToolCubicBezierPath(scene, domElement, parse);
            break;
        case 8: // VToolParallelCurve::ToolType
            qCDebug(vXML, "VToolParallelCurve.");
            ParseToolParallelCurve(scene, domElement, parse);
            break;
        case 9: // VToolGraduatedCurve::ToolType
            qCDebug(vXML, "VToolGraduatedCurve.");
            ParseToolGraduatedCurve(scene, domElement, parse);
            break;
        default:
            throw VException(tr("Unknown spline type '%1'.").arg(type));
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ParseArcElement parse arc tag.
 * @param scene scene.
 * @param domElement tag in xml tree.
 * @param parse parser file mode.
 * @param type type of spline.
 */
void VPattern::ParseArcElement(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse,
                               const QString &type)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");
    Q_ASSERT_X(not type.isEmpty(), Q_FUNC_INFO, "type of arc is empty");

    auto const arcs = QStringList() << VToolArc::ToolType            /*0*/
                                    << VNodeArc::ToolType            /*1*/
                                    << VToolArcWithLength::ToolType; /*2*/

    switch (arcs.indexOf(type))
    {
        case 0: // VToolArc::ToolType
            ParseToolArc(scene, domElement, parse);
            break;
        case 1: // VNodeArc::ToolType
            ParseNodeArc(domElement, parse);
            break;
        case 2: // VToolArcWithLength::ToolType
            ParseToolArcWithLength(scene, domElement, parse);
            break;
        default:
            throw VException(tr("Unknown arc type '%1'.").arg(type));
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ParseEllipticalArcElement parse elliptical arc tag.
 * @param scene scene.
 * @param domElement tag in xml tree.
 * @param parse parser file mode.
 * @param type type of spline.
 */
void VPattern::ParseEllipticalArcElement(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse,
                                         const QString &type)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");
    Q_ASSERT_X(not type.isEmpty(), Q_FUNC_INFO, "type of elliptical arc is empty");

    const auto arcs = QStringList{
        VToolEllipticalArc::ToolType,          /*0*/
        VNodeEllipticalArc::ToolType,          /*1*/
        VToolEllipticalArcWithLength::ToolType /*2*/
    };

    switch (arcs.indexOf(type))
    {
        case 0: // VToolArc::ToolType
            ParseToolEllipticalArc(scene, domElement, parse);
            break;
        case 1: // VNodeEllipticalArc::ToolType
            ParseNodeEllipticalArc(domElement, parse);
            break;
        case 2: // VToolEllipticalArcWithLength::ToolType
            ParseToolEllipticalArcWithLength(scene, domElement, parse);
            break;
        default:
            throw VException(tr("Unknown elliptical arc type '%1'.").arg(type));
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ParseToolsElement parse tools tag.
 * @param scene scene.
 * @param domElement tag in xml tree.
 * @param parse parser file mode.
 * @param type type of spline.
 */
void VPattern::ParseToolsElement(VMainGraphicsScene *scene, const QDomElement &domElement, const Document &parse,
                                 const QString &type)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(domElement.isNull() == false, Q_FUNC_INFO, "domElement is null");
    Q_ASSERT_X(type.isEmpty() == false, Q_FUNC_INFO, "type of spline is empty");

    const auto tools = QStringList() << VToolUnionDetails::ToolType;
    switch (tools.indexOf(type))
    {
        case 0: // VToolUnionDetails::ToolType
            try
            {
                VToolUnionDetailsInitData initData;
                initData.indexD1 = GetParametrUInt(domElement, VToolUnionDetails::AttrIndexD1, NULL_ID_STR);
                initData.indexD2 = GetParametrUInt(domElement, VToolUnionDetails::AttrIndexD2, NULL_ID_STR);
                initData.version = GetParametrUInt(domElement, AttrVersion, QChar('1'));
                initData.scene = scene;
                initData.doc = this;
                initData.data = data;
                initData.parse = parse;
                initData.typeCreation = Source::FromFile;

                ToolsCommonAttributes(domElement, initData.id);

                VToolUnionDetails::Create(initData);
            }
            catch (const VExceptionBadId &e)
            {
                VExceptionObjectError excep(tr("Error creating or updating union details"), domElement);
                excep.AddMoreInformation(e.ErrorMessage());
                throw excep;
            }
            break;
        default:
            throw VException(tr("Unknown tools type '%1'.").arg(type));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseOperationElement(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse,
                                     const QString &type)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");
    Q_ASSERT_X(not type.isEmpty(), Q_FUNC_INFO, "type of operation is empty");

    const auto opers = QStringList() << VToolRotation::ToolType       /*0*/
                                     << VToolFlippingByLine::ToolType /*1*/
                                     << VToolFlippingByAxis::ToolType /*2*/
                                     << VToolMove::ToolType;          /*3*/

    switch (opers.indexOf(type))
    {
        case 0: // VToolRotation::ToolType
            ParseToolRotation(scene, domElement, parse);
            break;
        case 1: // VToolFlippingByLine::ToolType
            ParseToolFlippingByLine(scene, domElement, parse);
            break;
        case 2: // VToolFlippingByAxis::ToolType
            ParseToolFlippingByAxis(scene, domElement, parse);
            break;
        case 3: // VToolMove::ToolType
            ParseToolMove(scene, domElement, parse);
            break;
        default:
            throw VException(tr("Unknown operation type '%1'.").arg(type));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParsePathElement(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr);
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");
    try
    {
        VToolPiecePathInitData initData;
        initData.scene = scene;
        initData.doc = this;
        initData.data = data;
        initData.parse = parse;
        initData.typeCreation = Source::FromFile;

        ToolsCommonAttributes(domElement, initData.id);
        initData.idTool = GetParametrUInt(domElement, VAbstractNode::AttrIdTool, NULL_ID_STR);

        if (const QDomElement element = domElement.firstChildElement(VAbstractPattern::TagNodes); not element.isNull())
        {
            initData.path = ParsePathNodes(element);
        }
        else
        {
            VExceptionObjectError excep(tr("Error creating or updating a piece path"), domElement);
            excep.AddMoreInformation(tr("Piece path doesn't contain nodes"));
            throw excep;
        }

        const QString defType = QString().setNum(static_cast<int>(PiecePathType::CustomSeamAllowance));
        initData.path.SetType(static_cast<PiecePathType>(GetParametrUInt(domElement, AttrType, defType)));
        initData.path.SetName(GetParametrString(domElement, AttrName, tr("Unnamed path")));
        initData.path.SetPenType(LineStyleToPenStyle(GetParametrString(domElement, AttrTypeLine, TypeLineLine)));

        if (initData.path.GetType() == PiecePathType::InternalPath)
        {
            initData.path.SetCutPath(GetParametrBool(domElement, AttrCut, falseStr));
            initData.path.SetFirstToCuttingContour(GetParametrBool(domElement, AttrFirstToContour, falseStr));
            initData.path.SetLastToCuttingContour(GetParametrBool(domElement, AttrLastToContour, falseStr));
            initData.path.SetVisibilityTrigger(GetParametrString(domElement, AttrVisible, QChar('1')));
            initData.path.SetNotMirrored(GetParametrBool(domElement, AttrNotMirrored, falseStr));
        }

        VToolPiecePath::Create(initData);
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating a piece path"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ParseIncrementsElement parse increments tag.
 * @param node tag in xml tree.
 * @param parse parser file mode.
 */
void VPattern::ParseIncrementsElement(const QDomNode &node, const Document &parse)
{
    int index = 0;
    QDomNode domNode = node.firstChild();
    while (not domNode.isNull())
    {
        if (domNode.isElement())
        {
            if (const QDomElement domElement = domNode.toElement();
                not domElement.isNull() && domElement.tagName() == TagIncrement)
            {
                const QString name = GetParametrString(domElement, AttrName, QString()).simplified();
                const QString desc = GetParametrEmptyString(domElement, AttrDescription);
                const IncrementType type =
                    StringToIncrementType(GetParametrString(domElement, AttrType, strTypeIncrement));
                const QString formula = (type == IncrementType::Separator)
                                            ? QChar('0')
                                            : GetParametrString(domElement, AttrFormula, QChar('0'));
                const bool specialUnits = GetParametrBool(domElement, AttrSpecialUnits, falseStr);

                bool ok = false;
                const qreal value = EvalFormula(data, formula, &ok);

                auto *increment = new VIncrement(data, name, type);
                increment->SetIndex(static_cast<quint32>(index++));
                increment->SetFormula(value, formula, ok);
                increment->SetDescription(desc);
                increment->SetSpecialUnits(specialUnits);
                increment->SetPreviewCalculation(node.toElement().tagName() == TagPreviewCalculations);
                data->AddUniqueVariable(increment);
            }
        }
        domNode = domNode.nextSibling();
    }

    if (parse == Document::FullParse)
    {
        emit MadeProgress();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::AddEmptyIncrement(const QString &name, IncrementType type)
{
    NewEmptyIncrement(TagIncrements, name, type);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::AddEmptyPreviewCalculation(const QString &name, IncrementType type)
{
    NewEmptyIncrement(TagPreviewCalculations, name, type);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::AddEmptyIncrementAfter(const QString &after, const QString &name, IncrementType type)
{
    NewEmptyIncrementAfter(TagIncrements, after, name, type);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::AddEmptyPreviewCalculationAfter(const QString &after, const QString &name, IncrementType type)
{
    NewEmptyIncrementAfter(TagPreviewCalculations, after, name, type);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::RemoveIncrement(const QString &name)
{
    RemoveIncrement(TagIncrements, name);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::RemovePreviewCalculation(const QString &name)
{
    RemoveIncrement(TagPreviewCalculations, name);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::MoveUpIncrement(const QString &name)
{
    MoveUpIncrement(TagIncrements, name);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::MoveUpPreviewCalculation(const QString &name)
{
    MoveUpIncrement(TagPreviewCalculations, name);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::MoveDownIncrement(const QString &name)
{
    MoveDownIncrement(TagIncrements, name);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::MoveDownPreviewCalculation(const QString &name)
{
    MoveDownIncrement(TagPreviewCalculations, name);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::SetIncrementName(const QString &name, const QString &text)
{
    SetIncrementAttribute(name, AttrName, text);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::SetIncrementFormula(const QString &name, const QString &text)
{
    SetIncrementAttribute(name, AttrFormula, text);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::SetIncrementDescription(const QString &name, const QString &text)
{
    if (not text.isEmpty())
    {
        SetIncrementAttribute(name, AttrDescription, text);
    }
    else
    {
        if (QDomElement node = FindIncrement(name); not node.isNull())
        {
            node.removeAttribute(AttrDescription);
            emit patternChanged(false);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::SetIncrementSpecialUnits(const QString &name, bool special)
{
    if (QDomElement node = FindIncrement(name); not node.isNull())
    {
        SetAttributeOrRemoveIf<bool>(node, AttrSpecialUnits, special,
                                     [](bool special) noexcept { return not special; });
        emit patternChanged(false);
    }
    else
    {
        qWarning() << tr("Can't find increment '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ReplaceNameInFormula(QVector<VFormulaField> &expressions, const QString &name, const QString &newName)
{
    const auto bias = name.length() - newName.length();

    for (int i = 0; i < expressions.size(); ++i)
    {
        if (expressions.at(i).expression.indexOf(name) != -1)
        {
            QMap<vsizetype, QString> tokens;

            // Eval formula
            try
            {
                QScopedPointer<qmu::QmuTokenParser> const cal(
                    new qmu::QmuTokenParser(expressions.at(i).expression, false, false));
                tokens = cal->GetTokens(); // Tokens (variables, measurements)
            }
            catch (const qmu::QmuParserError &)
            {
                continue; // Because we not sure if used. A formula is broken.
            }

            QList<QString> tValues = tokens.values();
            if (not tValues.contains(name))
            {
                continue;
            }

            QList<vsizetype> tKeys = tokens.keys(); // Take all tokens positions
            QString newFormula = expressions.at(i).expression;

            for (int i = 0; i < tKeys.size(); ++i)
            {
                if (tValues.at(i) != name)
                {
                    continue;
                }

                newFormula.replace(tKeys.at(i), name.length(), newName);

                if (bias != 0)
                { // Translated token has different length than original. Position next tokens need to be corrected.
                    VTranslateVars::BiasTokens(tKeys.at(i), bias, tokens);
                    tKeys = tokens.keys();
                    tValues = tokens.values();
                }
            }

            expressions[i].expression = newFormula;
            expressions[i].element.setAttribute(expressions.at(i).attribute, newFormula);
            emit patternChanged(false);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GenerateLabel create label for pattern piece of point.
 * @param type type of the label.
 * @param reservedName reversed point name. Use when need reserve name, but point is not in data base yet.
 * @return unique name for current pattern piece.
 */
auto VPattern::GenerateLabel(const LabelType &type, const QString &reservedName) const -> QString
{
    if (type == LabelType::NewPatternPiece)
    {
        const QDomNodeList drawList = elementsByTagName(TagDraw);
        QString name;
        int i = 0;
        for (;;)
        {
            name = GetLabelBase(static_cast<quint32>(drawList.size() + i));
            if (data->IsUnique(name))
            {
                return name;
            }
            if (i == INT_MAX)
            {
                break;
            }
            ++i;
        }
        qCDebug(vXML, "Point label: %s", qUtf8Printable(name));
        return name;
    }

    if (type == LabelType::NewLabel)
    {
        const int index = qMax(PatternBlockMapper()->GetActiveId(), 0);
        const QString labelBase = GetLabelBase(static_cast<quint32>(index));

        qint32 num = 1;
        QString name;
        do
        {
            name = QStringLiteral("%1%2").arg(labelBase).arg(num);
            num++;
            if (num == INT_MAX)
            {
                break;
            }
        } while (data->IsUnique(name) == false || name == reservedName);
        qCDebug(vXML, "Point label: %s", qUtf8Printable(name));
        return name;
    }
    qCDebug(vXML, "Got unknow type %d", static_cast<int>(type));
    return QString();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPattern::IsReadOnly() const -> bool
{
    const QDomElement pattern = documentElement();

    if (pattern.isNull())
    {
        return false;
    }

    return GetParametrBool(pattern, AttrReadOnly, falseStr);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::SetReadOnly(bool rOnly)
{
    QDomElement pattern = documentElement();

    if (not pattern.isNull())
    {
        SetAttributeOrRemoveIf<bool>(pattern, AttrReadOnly, rOnly, [](bool rOnly) noexcept { return not rOnly; });
        modified = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPattern::GetLabelPrefix() const -> QString
{
    const QDomElement pattern = documentElement();

    if (pattern.isNull())
    {
        return DefLabelLanguage();
    }

    return GetParametrString(pattern, AttrLabelPrefix, DefLabelLanguage());
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::SetLabelPrefix(const QString &prefix)
{
    QDomElement pattern = documentElement();

    if (not pattern.isNull() && ConvertToSet<QString>(VApplication::LabelLanguages()).contains(prefix))
    {
        SetAttribute(pattern, AttrLabelPrefix, prefix);
        modified = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::PrepareForParse(const Document &parse)
{
    SCASSERT(sceneDraw != nullptr)
    SCASSERT(sceneDetail != nullptr)

    emit CancelLabelRendering();

    // We can run garbage collection only in a narrow window between first successfull parse and user change
    if (!m_garbageCollected && !IsPatternGraphComplete())
    {
        disconnect(this, &VAbstractPattern::PatternDependencyGraphCompleted, this, &VPattern::CollectGarbage);
        m_garbageCollected = true;
    }

    CancelFormulaDependencyChecks();
    PatternGraph()->Clear();

    if (parse == Document::FullParse)
    {
        RefreshElementIdCache();
        sceneDraw->clear();
        sceneDraw->InitOrigins();
        sceneDetail->clear();
        sceneDetail->InitOrigins();
        data->ClearForFullParse();
        PatternBlockMapper()->Clear();

        qDeleteAll(toolsOnRemove); // Remove all invisible on a scene objects.
        toolsOnRemove.clear();

        tools.clear();
        history.clear();
    }
    else if (parse == Document::LiteParse || parse == Document::FullLiteParse)
    {
        Q_STATIC_ASSERT_X(static_cast<int>(VarType::Unknown) == 12, "Check that you used all types");
        QVector<VarType> types{VarType::LineAngle,         VarType::LineLength,       VarType::CurveLength,
                               VarType::CurveCLength,      VarType::ArcRadius,        VarType::CurveAngle,
                               VarType::PieceExternalArea, VarType::PieceSeamLineArea};
        if (parse == Document::FullLiteParse)
        {
            types.append(VarType::Increment);
            types.append(VarType::IncrementSeparator);
        }

        data->ClearVariables(types);
        parse == Document::FullLiteParse ? data->ClearUniqueNames() : data->ClearExceptUniqueIncrementNames();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ToolsCommonAttributes(const QDomElement &domElement, quint32 &id)
{
    id = GetParametrId(domElement);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::DrawToolsCommonAttributes(const QDomElement &domElement, quint32 &id, QString &notes)
{
    ToolsCommonAttributes(domElement, id);
    notes = GetParametrEmptyString(domElement, AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPattern::ActiveDrawBoundingRect() const -> QRectF
{
    // This check helps to find missed tools in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64, "Not all tools were used.");

    QRectF rec;

    const int index = PatternBlockMapper()->GetActiveId();

    for (const auto &tool : history)
    {
        if (tool.GetPatternBlockIndex() != index)
        {
            continue;
        }

        switch (tool.GetToolType())
        {
            case Tool::Arrow:
            case Tool::SinglePoint:
            case Tool::DoublePoint:
            case Tool::LinePoint:
            case Tool::AbstractSpline:
            case Tool::Cut:
            case Tool::Midpoint:                // Same as Tool::AlongLine, but tool will never has such a type
            case Tool::ArcIntersectAxis:        // Same as Tool::CurveIntersectAxis, but tool will never has such a type
            case Tool::BackgroundImage:         // Not part of active draw
            case Tool::BackgroundImageControls: // Not part of active draw
            case Tool::BackgroundPixmapImage:   // Not part of active draw
            case Tool::BackgroundSVGImage:      // Not part of active draw
            case Tool::ArcStart:                // Same as Tool::CutArc, but tool will never has such a type
            case Tool::ArcEnd:                  // Same as Tool::CutArc, but tool will never has such a type
            case Tool::LAST_ONE_DO_NOT_USE:
                Q_UNREACHABLE();
                break;
            case Tool::BasePoint:
            case Tool::LineIntersect:
            case Tool::PointOfContact:
            case Tool::Triangle:
            case Tool::PointOfIntersection:
            case Tool::CutArc:
            case Tool::CutSpline:
            case Tool::CutSplinePath:
            case Tool::PointOfIntersectionArcs:
            case Tool::PointOfIntersectionCircles:
            case Tool::PointOfIntersectionCurves:
            case Tool::PointFromCircleAndTangent:
            case Tool::PointFromArcAndTangent:
                rec = ToolBoundingRect<VToolSinglePoint>(rec, tool.GetId());
                break;
            case Tool::EndLine:
            case Tool::AlongLine:
            case Tool::ShoulderPoint:
            case Tool::Normal:
            case Tool::Bisector:
            case Tool::Height:
            case Tool::LineIntersectAxis:
            case Tool::CurveIntersectAxis:
                rec = ToolBoundingRect<VToolLinePoint>(rec, tool.GetId());
                break;
            case Tool::Line:
                rec = ToolBoundingRect<VToolLine>(rec, tool.GetId());
                break;
            case Tool::Spline:
            case Tool::CubicBezier:
            case Tool::Arc:
            case Tool::SplinePath:
            case Tool::CubicBezierPath:
            case Tool::ArcWithLength:
            case Tool::EllipticalArc:
            case Tool::EllipticalArcWithLength:
            case Tool::ParallelCurve:
            case Tool::GraduatedCurve:
                rec = ToolBoundingRect<VToolAbstractCurve>(rec, tool.GetId());
                break;
            case Tool::TrueDarts:
                rec = ToolBoundingRect<VToolDoublePoint>(rec, tool.GetId());
                break;
            case Tool::Rotation:
            case Tool::FlippingByLine:
            case Tool::FlippingByAxis:
            case Tool::Move:
                rec = ToolBoundingRect<VAbstractOperation>(rec, tool.GetId());
                break;
            // These tools are not accesseble in Draw mode, but still 'history' contains them.
            case Tool::Piece:
            case Tool::UnionDetails:
            case Tool::NodeArc:
            case Tool::NodeElArc:
            case Tool::NodePoint:
            case Tool::NodeSpline:
            case Tool::NodeSplinePath:
            case Tool::Group:
            case Tool::PiecePath:
            case Tool::Pin:
            case Tool::InsertNode:
            case Tool::PlaceLabel:
            case Tool::DuplicateDetail:
            default:
                break;
        }
    }
    return rec;
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> auto VPattern::ToolBoundingRect(const QRectF &rec, quint32 id) const -> QRectF
{
    QRectF recTool = rec;
    if (tools.contains(id))
    {
        if (const T *vTool = qobject_cast<T *>(tools.value(id)); vTool != nullptr)
        {
            QRectF childrenRect = vTool->childrenBoundingRect();
            // map to scene coordinate.
            childrenRect.translate(vTool->scenePos());

            recTool = recTool.united(vTool->sceneBoundingRect());
            recTool = recTool.united(childrenRect);
        }
        else
        {
            qDebug() << "qobject_cast failed for tool with id=" << id;
        }
    }
    else
    {
        qDebug() << "Can't find tool with id=" << id;
    }
    return recTool;
}
