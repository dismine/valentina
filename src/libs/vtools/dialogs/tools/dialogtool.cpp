/************************************************************************
 **
 **  @file   dialogtool.cpp
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

#include "dialogtool.h"

#include <QBuffer>
#include <QCloseEvent>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QHash>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMapIterator>
#include <QMessageLogger>
#include <QPalette>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QShowEvent>
#include <QSize>
#include <QTextCursor>
#include <QTimer>
#include <QWidget>
#include <QtDebug>
#include <qiterator.h>
#include <qnumeric.h>

#include "../../tools/vabstracttool.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpiecenode.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

template <class T> class QSharedPointer;

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(vDialog, "v.dialog") // NOLINT

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogTool create dialog
 * @param data container with data
 * @param parent parent widget
 */
DialogTool::DialogTool(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent)
  : QDialog(parent),
    data(data),
    m_doc(doc),
    isInitialized(false),
    bOk(nullptr),
    bApply(nullptr),
    associatedTool(nullptr),
    toolId(toolId),
    prepare(false),
    vis(nullptr)
{
    SCASSERT(data != nullptr);
    SCASSERT(doc != nullptr);
}

//---------------------------------------------------------------------------------------------------------------------
DialogTool::~DialogTool()
{
    delete vis.data();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief closeEvent handle when dialog cloded
 * @param event event
 */
void DialogTool::closeEvent(QCloseEvent *event)
{
    DialogRejected();
    event->accept();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief showEvent handle when window show
 * @param event event
 */
void DialogTool::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    if (event->spontaneous())
    {
        return;
    }
    if (isInitialized)
    {
        return;
    }
    // do your init stuff here

    setMaximumSize(size());
    setMinimumSize(size());

    isInitialized = true; // first show windows are held
    ShowVisualization();

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Escape:
            DialogRejected();
            return; // After reject the dialog will be destroyed, exit imidiately
        default:
            break;
    }
    QDialog::keyPressEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::FillComboBoxPiecesList(QComboBox *box, const QVector<quint32> &list)
{
    SCASSERT(box != nullptr)
    box->blockSignals(true);
    box->clear();
    for (auto id : list)
    {
        box->addItem(data->GetPiece(id).GetName(), id);
    }
    box->blockSignals(false);
    box->setCurrentIndex(-1); // Force a user to choose
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FillComboBoxPoints fill comboBox list of points
 * @param box comboBox
 */
void DialogTool::FillComboBoxPoints(QComboBox *box, FillComboBox rule, quint32 ch1, quint32 ch2) const
{
    FillCombo<VPointF>(box, GOType::Point, rule, ch1, ch2);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::FillComboBoxArcs(QComboBox *box, FillComboBox rule, quint32 ch1, quint32 ch2) const
{
    FillCombo<VAbstractCurve>(box, GOType::Arc, rule, ch1, ch2);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::FillComboBoxSplines(QComboBox *box) const
{
    SCASSERT(box != nullptr)
    box->blockSignals(true);

    const auto *const objs = data->CalculationGObjects();
    QMap<QString, quint32> list;
    for (auto i = objs->constBegin(); i != objs->constEnd(); ++i)
    {
        if (i.key() != toolId && IsSpline(i.value()))
        {
            PrepareList<VAbstractCurve>(list, i.key());
        }
    }
    FillList(box, list);
    box->setCurrentIndex(-1); // force to select
    box->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::FillComboBoxSplinesPath(QComboBox *box) const
{
    SCASSERT(box != nullptr)
    box->blockSignals(true);

    const auto *const objs = data->CalculationGObjects();
    QMap<QString, quint32> list;
    for (auto i = objs->constBegin(); i != objs->constEnd(); ++i)
    {
        if (i.key() != toolId && IsSplinePath(i.value()))
        {
            PrepareList<VAbstractCurve>(list, i.key());
        }
    }
    FillList(box, list);
    box->setCurrentIndex(-1); // force to select
    box->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::FillComboBoxCurves(QComboBox *box) const
{
    SCASSERT(box != nullptr)
    const auto *const objs = data->CalculationGObjects();
    QMap<QString, quint32> list;
    for (auto i = objs->constBegin(); i != objs->constEnd(); ++i)
    {
        if (i.key() != toolId)
        {
            QSharedPointer<VGObject> const obj = i.value();
            if (obj->getType() == GOType::Arc || obj->getType() == GOType::EllipticalArc ||
                obj->getType() == GOType::Spline || obj->getType() == GOType::SplinePath ||
                obj->getType() == GOType::CubicBezier || obj->getType() == GOType::CubicBezierPath)
            {
                PrepareList<VAbstractCurve>(list, i.key());
            }
        }
    }
    FillList(box, list);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FillComboBoxTypeLine fill comboBox list of type lines
 * @param box comboBox
 */
void DialogTool::FillComboBoxTypeLine(QComboBox *box, const QMap<QString, QIcon> &stylesPics, const QString &def) const
{
    SCASSERT(box != nullptr)
    QMap<QString, QIcon>::const_iterator i = stylesPics.constBegin();
    while (i != stylesPics.constEnd())
    {
        box->addItem(i.value(), QString(), QVariant(i.key()));
        ++i;
    }

    const int index = box->findData(QVariant(def));
    if (index != -1)
    {
        box->setCurrentIndex(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::FillComboBoxCrossCirclesPoints(QComboBox *box) const
{
    SCASSERT(box != nullptr)

    box->addItem(tr("First point"), QVariant(static_cast<int>(CrossCirclesPoint::FirstPoint)));
    box->addItem(tr("Second point"), QVariant(static_cast<int>(CrossCirclesPoint::SecondPoint)));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::FillComboBoxVCrossCurvesPoint(QComboBox *box) const
{
    SCASSERT(box != nullptr)

    box->addItem(tr("Highest point"), QVariant(static_cast<int>(VCrossCurvesPoint::HighestPoint)));
    box->addItem(tr("Lowest point"), QVariant(static_cast<int>(VCrossCurvesPoint::LowestPoint)));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::FillComboBoxHCrossCurvesPoint(QComboBox *box) const
{
    SCASSERT(box != nullptr)

    box->addItem(tr("Leftmost point"), QVariant(static_cast<int>(HCrossCurvesPoint::LeftmostPoint)));
    box->addItem(tr("Rightmost point"), QVariant(static_cast<int>(HCrossCurvesPoint::RightmostPoint)));
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogTool::GetComboBoxCurrentData(const QComboBox *box, const QString &def) const -> QString
{
    SCASSERT(box != nullptr)
    QString value;
    value = box->currentData().toString();

    if (value.isEmpty())
    {
        value = def;
    }
    return value;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChangeCurrentData select item in combobox by id
 * @param box combobox
 * @param value id of item
 */
void DialogTool::ChangeCurrentData(QComboBox *box, const QVariant &value) const
{
    SCASSERT(box != nullptr)
    const qint32 index = box->findData(value);
    if (index != -1)
    {
        box->setCurrentIndex(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogTool::eventFilter(QObject *object, QEvent *event) -> bool
{
    const bool fitered = FilterObject(object, event);
    if (fitered)
    {
        return fitered;
    }

    return QDialog::eventFilter(object, event);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogTool::DNumber(const QString &baseName) const -> quint32
{
    quint32 num = 0;
    QString name;
    do
    {
        ++num;
        name = baseName + u"_%1"_s.arg(num);
    } while (not data->IsUnique(name));

    return num;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::NewNodeItem(QListWidget *listWidget, const VPieceNode &node, bool showPassmark, bool showExclusion)
{
    SCASSERT(listWidget != nullptr);
    SCASSERT(node.GetId() > NULL_ID);
    QString name;
    switch (node.GetTypeTool())
    {
        case (Tool::NodePoint):
        case (Tool::NodeArc):
        case (Tool::NodeElArc):
        case (Tool::NodeSpline):
        case (Tool::NodeSplinePath):
            name = GetNodeName(data, node, showPassmark);
            break;
        default:
            qDebug() << "Got wrong tools. Ignore.";
            return;
    }

    bool canAddNewPoint = false;

    if (listWidget->count() == 0)
    {
        canAddNewPoint = true;
    }
    else
    {
        if (RowNode(listWidget, listWidget->count() - 1).GetId() != node.GetId())
        {
            canAddNewPoint = true;
        }
    }

    if (canAddNewPoint)
    {
        auto *item = new QListWidgetItem(name);
        item->setFont(NodeFont(item->font(), showExclusion ? node.IsExcluded() : false));
        item->setData(Qt::UserRole, QVariant::fromValue(node));
        listWidget->addItem(item);
        listWidget->setCurrentRow(listWidget->count() - 1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::InitNodeAngles(QComboBox *box)
{
    SCASSERT(box != nullptr);
    box->clear();

    box->addItem(tr("by length"), static_cast<unsigned char>(PieceNodeAngle::ByLength));
    box->addItem(tr("by points intersetions"), static_cast<unsigned char>(PieceNodeAngle::ByPointsIntersection));
    box->addItem(tr("by first edge symmetry"), static_cast<unsigned char>(PieceNodeAngle::ByFirstEdgeSymmetry));
    box->addItem(tr("by second edge symmetry"), static_cast<unsigned char>(PieceNodeAngle::BySecondEdgeSymmetry));
    box->addItem(tr("by first edge right angle"), static_cast<unsigned char>(PieceNodeAngle::ByFirstEdgeRightAngle));
    box->addItem(tr("by second edge right angle"), static_cast<unsigned char>(PieceNodeAngle::BySecondEdgeRightAngle));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::MoveListRowTop(QListWidget *list)
{
    SCASSERT(list != nullptr)
    const int currentIndex = list->currentRow();
    if (QListWidgetItem *currentItem = list->takeItem(currentIndex))
    {
        list->insertItem(0, currentItem);
        list->setCurrentRow(0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::MoveListRowUp(QListWidget *list)
{
    SCASSERT(list != nullptr)
    int currentIndex = list->currentRow();
    if (QListWidgetItem *currentItem = list->takeItem(currentIndex--))
    {
        if (currentIndex < 0)
        {
            currentIndex = 0;
        }
        list->insertItem(currentIndex, currentItem);
        list->setCurrentRow(currentIndex);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::MoveListRowDown(QListWidget *list)
{
    SCASSERT(list != nullptr)
    int currentIndex = list->currentRow();
    if (QListWidgetItem *currentItem = list->takeItem(currentIndex++))
    {
        if (currentIndex > list->count())
        {
            currentIndex = list->count();
        }
        list->insertItem(currentIndex, currentItem);
        list->setCurrentRow(currentIndex);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::MoveListRowBottom(QListWidget *list)
{
    SCASSERT(list != nullptr)
    const int currentIndex = list->currentRow();
    if (QListWidgetItem *currentItem = list->takeItem(currentIndex))
    {
        list->insertItem(list->count(), currentItem);
        list->setCurrentRow(list->count() - 1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogTool::IsSplinePath(const QSharedPointer<VGObject> &obj) const -> bool
{
    return obj->getType() == GOType::SplinePath || obj->getType() == GOType::CubicBezierPath;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Eval evaluate formula and show result
 * @param formulaData options to control parsing
 */
auto DialogTool::Eval(const FormulaData &formulaData, bool &flag) -> qreal
{
    const qreal result = EvalToolFormula(this, formulaData, flag);
    CheckState(); // Disable Ok and Apply buttons if something wrong.
    return result;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::setCurrentPointId(QComboBox *box, const quint32 &value, FillComboBox rule, const quint32 &ch1,
                                   const quint32 &ch2) const
{
    SCASSERT(box != nullptr)

    box->blockSignals(true);

    FillComboBoxPoints(box, rule, ch1, ch2);
    ChangeCurrentData(box, value);

    box->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setCurrentSplineId set current spline id in combobox
 */
void DialogTool::setCurrentSplineId(QComboBox *box, const quint32 &value) const
{
    SCASSERT(box != nullptr)
    FillComboBoxSplines(box);
    ChangeCurrentData(box, value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setCurrentArcId
 */
void DialogTool::setCurrentArcId(QComboBox *box, const quint32 &value, FillComboBox rule, const quint32 &ch1,
                                 const quint32 &ch2) const
{
    SCASSERT(box != nullptr)
    FillComboBoxArcs(box, rule, ch1, ch2);
    ChangeCurrentData(box, value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setCurrentSplinePathId set current splinePath id in combobox
 * @param box combobox
 * @param value splinePath id
 */
void DialogTool::setCurrentSplinePathId(QComboBox *box, const quint32 &value) const
{
    SCASSERT(box != nullptr)
    FillComboBoxSplinesPath(box);
    ChangeCurrentData(box, value);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::setCurrentCurveId(QComboBox *box, const quint32 &value) const
{
    SCASSERT(box != nullptr)
    FillComboBoxCurves(box);
    ChangeCurrentData(box, value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getCurrentPointId return current point id stored in combobox
 * @param box combobox
 * @return id or 0 if combobox is empty
 */
auto DialogTool::getCurrentObjectId(QComboBox *box) -> quint32
{
    SCASSERT(box != nullptr)
    qint32 const index = box->currentIndex();
    if (index != -1)
    {
        return qvariant_cast<quint32>(box->itemData(index));
    }

    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogTool::SetObject(const quint32 &id, QComboBox *box, const QString &toolTip) -> bool
{
    SCASSERT(box != nullptr)
    const qint32 index = box->findData(id);
    if (index != -1)
    { // -1 for not found
        box->setCurrentIndex(index);
        emit ToolTip(toolTip);
        return true;
    }

    qWarning() << "Can't find object by id" << id;

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FillList fill combobox list
 * @param box combobox
 * @param list list with ids and names
 */
void DialogTool::FillList(QComboBox *box, const QMap<QString, quint32> &list) const
{
    SCASSERT(box != nullptr)
    box->clear();

    QMapIterator iter(list);
    while (iter.hasNext())
    {
        iter.next();
        box->addItem(iter.key(), iter.value());
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> void DialogTool::PrepareList(QMap<QString, quint32> &list, quint32 id) const
{
    const auto obj = data->GeometricObject<T>(id);
    SCASSERT(obj != nullptr)
    list[obj->ObjectName()] = id;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogTool::IsSpline(const QSharedPointer<VGObject> &obj) const -> bool
{
    return obj->getType() == GOType::Spline || obj->getType() == GOType::CubicBezier;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief CheckState enable, when all is correct, or disable, when something wrong, button ok
 */
void DialogTool::CheckState()
{
    SCASSERT(bOk != nullptr)
    bOk->setEnabled(IsValid());
    // In case dialog hasn't apply button
    if (bApply != nullptr)
    {
        bApply->setEnabled(bOk->isEnabled());
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChosenObject gets id and type of selected object. Save right data and ignore wrong.
 * @param id id of point or detail
 * @param type type of object
 */
void DialogTool::ChosenObject(quint32 id, const SceneObject &type)
{
    Q_UNUSED(id)
    Q_UNUSED(type)
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::SelectedObject(bool selected, quint32 object, quint32 tool)
{
    Q_UNUSED(selected)
    Q_UNUSED(object)
    Q_UNUSED(tool)
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogAccepted save data and emit signal about closed dialog.
 */
void DialogTool::DialogAccepted()
{
    SaveData();
    emit DialogClosed(QDialog::Accepted);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::DialogApply()
{
    SaveData();
    emit DialogApplied();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogRejected emit signal dialog rejected
 */
void DialogTool::DialogRejected()
{
    emit DialogClosed(QDialog::Rejected);
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
auto DialogTool::GetToolId() const -> quint32
{
    return toolId;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::SetToolId(const quint32 &value)
{
    toolId = value;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::SetGroupCategories(const QStringList &categories)
{
    Q_UNUSED(categories)
    // do nothing
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::ShowDialog(bool click)
{
    Q_UNUSED(click)
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::Build(const Tool &type)
{
    Q_UNUSED(type)
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::SetPiecesList(const QVector<quint32> &list)
{
    Q_UNUSED(list);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::SetPatternDoc(VAbstractPattern *doc)
{
    Q_UNUSED(doc);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::SetAssociatedTool(VAbstractTool *tool)
{
    if (tool != nullptr)
    {
        associatedTool = tool;
        SetToolId(tool->getId());
        data = tool->getData();
        if (not vis.isNull())
        {
            vis->SetData(data);
        }
    }
    else
    {
        associatedTool = nullptr;
        SetToolId(NULL_ID);
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <typename GObject>
void DialogTool::FillCombo(QComboBox *box, GOType gType, FillComboBox rule, const quint32 &ch1,
                           const quint32 &ch2) const
{
    SCASSERT(box != nullptr)
    box->blockSignals(true);

    const QHash<quint32, QSharedPointer<VGObject>> *objs = data->CalculationGObjects();
    QMap<QString, quint32> list;
    for (auto i = objs->constBegin(); i != objs->constEnd(); ++i)
    {
        if (rule == FillComboBox::NoChildren)
        {
            if (i.key() != toolId && i.value()->getIdTool() != toolId && i.key() != ch1 && i.key() != ch2)
            {
                const QSharedPointer<VGObject> &obj = i.value();
                if (obj->getType() == gType)
                {
                    PrepareList<GObject>(list, i.key());
                }
            }
        }
        else
        {
            if (i.key() != toolId && i.value()->getIdTool() != toolId)
            {
                const QSharedPointer<VGObject> &obj = i.value();
                if (obj->getType() == gType && obj->getMode() == Draw::Calculation)
                {
                    PrepareList<GObject>(list, i.key());
                }
            }
        }
    }
    FillList(box, list);
    box->setCurrentIndex(-1); // force to select
    box->blockSignals(false);
}
