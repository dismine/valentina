/************************************************************************
 **
 **  @file   dialogtoolbox.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   25 1, 2019
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2019 Valentina project
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
#include "dialogtoolbox.h"

#include "../ifc/exception/vexceptionbadid.h"
#include "../qmuparser/qmudef.h"
#include "../tools/drawTools/operation/vabstractoperation.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vcubicbezier.h"
#include "../vgeometry/vcubicbezierpath.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vgeometry/vsplinepath.h"
#include "../vmisc/def.h"
#include "../vmisc/theme/themeDef.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/calculator.h"
#include "../vpatterndb/variables/vcurvelength.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vpropertyexplorer/qtcolorpicker.h"
#include "../vtools/tools/vabstracttool.h"

#include <qnumeric.h>
#include <QBuffer>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QLocale>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QScreen>
#include <QTextCursor>
#include <QTimer>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

const QColor errorColor = Qt::red;

namespace
{
const int dialogMaxFormulaHeight = 80;

//---------------------------------------------------------------------------------------------------------------------
auto DoublePoint(const VPieceNode &firstNode, const VPieceNode &secondNode, const VContainer *data, QString &error)
    -> bool
{
    if (firstNode.GetTypeTool() == Tool::NodePoint && not(firstNode.GetId() == NULL_ID) &&
        secondNode.GetTypeTool() == Tool::NodePoint && not(secondNode.GetId() == NULL_ID))
    {
        QSharedPointer<VPointF> firstPoint;
        QSharedPointer<VPointF> secondPoint;

        try
        {
            firstPoint = data->GeometricObject<VPointF>(firstNode.GetId());
            secondPoint = data->GeometricObject<VPointF>(secondNode.GetId());
        }
        catch (const VExceptionBadId &)
        {
            return false;
        }

        // don't ignore the same point twice
        if (firstNode.GetId() == secondNode.GetId())
        {
            error = QObject::tr("Point '%1' repeats twice").arg(firstPoint->name());
            return true;
        }

        // The same point, but different modeling objects
        if (firstPoint->getIdObject() != NULL_ID && secondPoint->getIdObject() != NULL_ID &&
            firstPoint->getMode() == Draw::Modeling && secondPoint->getMode() == Draw::Modeling &&
            firstPoint->getIdObject() == secondPoint->getIdObject())
        {
            error = QObject::tr("Point '%1' repeats twice").arg(firstPoint->name());
            return true;
        }

        // But ignore the same coordinate if a user wants
        if (not firstNode.IsCheckUniqueness() || not secondNode.IsCheckUniqueness())
        {
            return false;
        }

        bool const sameCoordinates = VFuzzyComparePoints(firstPoint->toQPointF(), secondPoint->toQPointF());
        if (sameCoordinates)
        {
            error = QObject::tr("Points '%1' and '%2' have the same coordinates.")
                        .arg(firstPoint->name(), secondPoint->name());
        }

        return sameCoordinates;
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto DoubleCurve(const VPieceNode &firstNode, const VPieceNode &secondNode, const VContainer *data, QString &error)
    -> bool
{
    if (firstNode.GetTypeTool() != Tool::NodePoint && not(firstNode.GetId() == NULL_ID) &&
        secondNode.GetTypeTool() != Tool::NodePoint && not(secondNode.GetId() == NULL_ID))
    {
        QSharedPointer<VGObject> curve1;
        QSharedPointer<VGObject> curve2;

        try
        {
            curve1 = data->GetGObject(firstNode.GetId());
            curve2 = data->GetGObject(secondNode.GetId());
        }
        catch (const VExceptionBadId &)
        {
            return false;
        }

        // don't ignore the same curve twice
        if (firstNode.GetId() == secondNode.GetId())
        {
            error = QObject::tr("Leave only one copy of curve '%1'").arg(curve1->name());
            return true;
        }

        // The same curve, but different modeling objects
        if (curve1->getMode() == Draw::Modeling && curve2->getMode() == Draw::Modeling &&
            curve1->getIdObject() != NULL_ID && curve2->getIdObject() != NULL_ID &&
            curve1->getIdObject() == curve2->getIdObject())
        {
            error = QObject::tr("Leave only one copy of curve '%1'").arg(curve1->name());
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
template <class T> auto CurveAliases(const QString &alias1, const QString &alias2) -> QPair<QString, QString>
{
    T curve1;
    curve1.SetAliasSuffix(alias1);

    T curve2;
    curve2.SetAliasSuffix(alias2);

    return qMakePair(curve1.GetAlias(), curve2.GetAlias());
}

//---------------------------------------------------------------------------------------------------------------------
template<class T>
auto CurveNames(const QString &name1, const QString &name2) -> QPair<QString, QString>
{
    T curve1;
    curve1.SetNameSuffix(name1);

    T curve2;
    curve2.SetNameSuffix(name2);

    return qMakePair(curve1.name(), curve2.name());
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
auto RowNode(QListWidget *listWidget, int i) -> VPieceNode
{
    SCASSERT(listWidget != nullptr);

    if (i < 0 || i >= listWidget->count())
    {
        return {};
    }

    const QListWidgetItem *rowItem = listWidget->item(i);
    SCASSERT(rowItem != nullptr)
    return qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
}

//---------------------------------------------------------------------------------------------------------------------
void MoveCursorToEnd(QPlainTextEdit *plainTextEdit)
{
    SCASSERT(plainTextEdit != nullptr)
    QTextCursor cursor = plainTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    plainTextEdit->setTextCursor(cursor);
}

//---------------------------------------------------------------------------------------------------------------------
void DeployFormula(QDialog *dialog, QPlainTextEdit *formula, QPushButton *buttonGrowLength, int formulaBaseHeight)
{
    // cppcheck-suppress unknownMacro
    SCASSERT(dialog != nullptr)
    SCASSERT(formula != nullptr)
    SCASSERT(buttonGrowLength != nullptr)

    const QTextCursor cursor = formula->textCursor();

    // Before deploy ned to release dialog size
    // I don't know why, but don't need to fixate again.
    // A dialog will be lefted fixated. That's what we need.
    dialog->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    dialog->setMinimumSize(QSize(0, 0));

    if (formula->height() < dialogMaxFormulaHeight)
    {
        formula->setFixedHeight(dialogMaxFormulaHeight);
        // Set icon from theme (internal for Windows system)
        buttonGrowLength->setIcon(FromTheme(VThemeIcon::GoNext));
    }
    else
    {
        formula->setFixedHeight(formulaBaseHeight);
        // Set icon from theme (internal for Windows system)
        buttonGrowLength->setIcon(FromTheme(VThemeIcon::GoDown));
    }

    // I found that after change size of formula field, it was filed for angle formula, field for formula became black.
    // This code prevent this.
    dialog->setUpdatesEnabled(false);
    dialog->repaint();
    dialog->setUpdatesEnabled(true);

    formula->setFocus();
    formula->setTextCursor(cursor);
}

//---------------------------------------------------------------------------------------------------------------------
auto FilterObject(QObject *object, QEvent *event) -> bool
{
    if (auto *plainTextEdit = qobject_cast<QPlainTextEdit *>(object))
    {
        if (event->type() == QEvent::KeyPress)
        {
            if (const auto *keyEvent = static_cast<QKeyEvent *>(event);
                (keyEvent->key() == Qt::Key_Period) && (keyEvent->modifiers() & Qt::KeypadModifier))
            {
                if (VAbstractApplication::VApp()->Settings()->GetOsSeparator())
                {
                    plainTextEdit->insertPlainText(LocaleDecimalPoint(QLocale()));
                }
                else
                {
                    plainTextEdit->insertPlainText(LocaleDecimalPoint(QLocale::c()));
                }
                return true;
            }
        }
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto EvalToolFormula(QDialog *dialog, const FormulaData &data, bool &flag) -> qreal
{
    SCASSERT(data.labelResult != nullptr)
    SCASSERT(data.labelEditFormula != nullptr)

    qreal result = INT_MIN; // Value can be 0, so use max imposible value

    if (data.formula.isEmpty())
    {
        flag = false;
        ChangeColor(data.labelEditFormula, errorColor);
        data.labelResult->setText(QObject::tr("Error") + " (" + data.postfix + ")");
        data.labelResult->setToolTip(QObject::tr("Empty formula"));
    }
    else
    {
        try
        {
            // Translate to internal look.
            QString const formula = VAbstractApplication::VApp()->TrVars()->FormulaFromUser(
                data.formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
            QScopedPointer<Calculator> const cal(new Calculator());
            result = cal->EvalFormula(data.variables, formula);

            if (qIsInf(result) || qIsNaN(result))
            {
                flag = false;
                ChangeColor(data.labelEditFormula, errorColor);
                data.labelResult->setText(QObject::tr("Error") + " (" + data.postfix + ")");
                data.labelResult->setToolTip(QObject::tr("Invalid result. Value is infinite or NaN. Please, check "
                                                         "your calculations."));
            }
            else
            {
                // if result equal 0
                if (data.checkZero && qFuzzyIsNull(result))
                {
                    flag = false;
                    ChangeColor(data.labelEditFormula, errorColor);
                    data.labelResult->setText(QObject::tr("Error") + " (" + data.postfix + ")");
                    data.labelResult->setToolTip(QObject::tr("Value can't be 0"));
                }
                else if (data.checkLessThanZero && result < 0)
                {
                    flag = false;
                    ChangeColor(data.labelEditFormula, errorColor);
                    data.labelResult->setText(QObject::tr("Error") + " (" + data.postfix + ")");
                    data.labelResult->setToolTip(QObject::tr("Value can't be less than 0"));
                }
                else
                {
                    data.labelResult->setText(VAbstractApplication::VApp()->LocaleToString(result) +
                                              QChar(QChar::Space) + data.postfix);
                    flag = true;
                    ChangeColor(data.labelEditFormula, OkColor(dialog));
                    data.labelResult->setToolTip(QObject::tr("Value"));
                }
            }
        }
        catch (qmu::QmuParserError &e)
        {
            data.labelResult->setText(QObject::tr("Error") + " (" + data.postfix + ")");
            flag = false;
            ChangeColor(data.labelEditFormula, errorColor);
            data.labelResult->setToolTip(QObject::tr("Parser error: %1").arg(e.GetMsg()));
            qDebug() << "\nMath parser error:\n"
                     << "--------------------------------------\n"
                     << "Message:     " << e.GetMsg() << "\n"
                     << "Expression:  " << e.GetExpr() << "\n"
                     << "--------------------------------------";
        }
    }
    return result;
}

//---------------------------------------------------------------------------------------------------------------------
void ChangeColor(QWidget *widget, const QColor &color)
{
    SCASSERT(widget != nullptr)
    QPalette palette = widget->palette();
    palette.setColor(QPalette::Active, widget->foregroundRole(), color);
    palette.setColor(QPalette::Inactive, widget->foregroundRole(), color);
    widget->setPalette(palette);
}

//---------------------------------------------------------------------------------------------------------------------
auto OkColor(const QWidget *widget) -> QColor
{
    SCASSERT(widget != nullptr);
    return widget->palette().color(QPalette::Active, QPalette::WindowText);
}

//---------------------------------------------------------------------------------------------------------------------
void CheckPointLabel(QDialog *dialog, QLineEdit *edit, QLabel *labelEditNamePoint, const QString &pointName,
                     const VContainer *data, bool &flag)
{
    SCASSERT(dialog != nullptr)
    SCASSERT(edit != nullptr)
    SCASSERT(labelEditNamePoint != nullptr)

    const QString name = edit->text();
    if (QRegularExpression const rx(NameRegExp());
        name.isEmpty() || (pointName != name && not data->IsUnique(name)) || not rx.match(name).hasMatch())
    {
        flag = false;
        ChangeColor(labelEditNamePoint, errorColor);
    }
    else
    {
        flag = true;
        ChangeColor(labelEditNamePoint, OkColor(dialog));
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto FindNotExcludedNeighborNodeDown(QListWidget *listWidget, int candidate) -> int
{
    SCASSERT(listWidget != nullptr);

    int index = -1;
    if (candidate < 0 || candidate >= listWidget->count())
    {
        return index;
    }

    int i = candidate;
    VPieceNode rowNode;
    bool foundNeighbor = false;

    do
    {
        const QListWidgetItem *rowItem = listWidget->item(i);
        SCASSERT(rowItem != nullptr)
        rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

        if (not rowNode.IsExcluded())
        {
            index = i;
            foundNeighbor = true;
            break;
        }

        ++i;
        if (i >= listWidget->count())
        {
            // Wrap around to the beginning of the list
            i = 0;
        }

    } while (i != candidate); // Continue until we reach back to the starting point or find a neighbor

    if (!foundNeighbor)
    {
        // No not excluded neighbor found
        index = -1;
    }

    return index;
}

//---------------------------------------------------------------------------------------------------------------------
auto FindNotExcludedNeighborNodeUp(QListWidget *listWidget, int candidate) -> int
{
    SCASSERT(listWidget != nullptr);

    int index = -1;
    if (candidate < 0 || candidate >= listWidget->count())
    {
        return index;
    }

    int i = candidate;
    VPieceNode rowNode;
    bool foundNeighbor = false;

    do
    {
        const QListWidgetItem *rowItem = listWidget->item(i);
        SCASSERT(rowItem != nullptr)
        rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

        if (not rowNode.IsExcluded())
        {
            index = i;
            foundNeighbor = true;
            break;
        }

        --i;
        if (i < 0)
        {
            // Wrap around to the end of the list
            i = listWidget->count() - 1;
        }

    } while (i != candidate);

    if (!foundNeighbor)
    {
        // No not excluded neighbor found
        index = -1;
    }

    return index;
}

//---------------------------------------------------------------------------------------------------------------------
auto FindNotExcludedNodeDown(QListWidget *listWidget, int candidate) -> int
{
    SCASSERT(listWidget != nullptr);

    int index = -1;
    if (candidate < 0 || candidate >= listWidget->count())
    {
        return index;
    }

    int i = candidate;
    VPieceNode rowNode;
    do
    {
        const QListWidgetItem *rowItem = listWidget->item(i);
        SCASSERT(rowItem != nullptr)
        rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

        if (not rowNode.IsExcluded())
        {
            index = i;
        }

        ++i;
    } while (rowNode.IsExcluded() && i < listWidget->count());

    return index;
}

//---------------------------------------------------------------------------------------------------------------------
auto FindNotExcludedNodeUp(QListWidget *listWidget, int candidate) -> int
{
    SCASSERT(listWidget != nullptr);

    int index = -1;
    if (candidate < 0 || candidate >= listWidget->count())
    {
        return index;
    }

    int i = candidate;
    VPieceNode rowNode;
    do
    {
        const QListWidgetItem *rowItem = listWidget->item(i);
        SCASSERT(rowItem != nullptr)
        rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

        if (not rowNode.IsExcluded())
        {
            index = i;
        }

        --i;
    } while (rowNode.IsExcluded() && i > -1);

    return index;
}

//---------------------------------------------------------------------------------------------------------------------
auto FirstPointEqualLast(QListWidget *listWidget, const VContainer *data, QString &error) -> bool
{
    SCASSERT(listWidget != nullptr);
    if (listWidget->count() > 1)
    {
        const VPieceNode topNode = RowNode(listWidget, FindNotExcludedNodeDown(listWidget, 0));
        const VPieceNode bottomNode = RowNode(listWidget, FindNotExcludedNodeUp(listWidget, listWidget->count() - 1));

        return DoublePoint(topNode, bottomNode, data, error);
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto DoublePoints(QListWidget *listWidget, const VContainer *data, QString &error) -> bool
{
    SCASSERT(listWidget != nullptr);
    for (int i = 0, sz = listWidget->count() - 1; i < sz; ++i)
    {
        const int firstIndex = FindNotExcludedNodeDown(listWidget, i);
        const VPieceNode firstNode = RowNode(listWidget, firstIndex);
        const VPieceNode secondNode = RowNode(listWidget, FindNotExcludedNodeDown(listWidget, firstIndex + 1));

        if (DoublePoint(firstNode, secondNode, data, error))
        {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto DoubleCurves(QListWidget *listWidget, const VContainer *data, QString &error) -> bool
{
    SCASSERT(listWidget != nullptr);
    for (int i = 0, sz = listWidget->count() - 1; i < sz; ++i)
    {
        const int firstIndex = FindNotExcludedNodeDown(listWidget, i);
        const VPieceNode firstNode = RowNode(listWidget, firstIndex);
        const VPieceNode secondNode = RowNode(listWidget, FindNotExcludedNodeDown(listWidget, firstIndex + 1));

        if (DoubleCurve(firstNode, secondNode, data, error))
        {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto EachPointLabelIsUnique(QListWidget *listWidget) -> bool
{
    SCASSERT(listWidget != nullptr);
    QSet<quint32> pointLabels;
    int countPoints = 0;
    for (int i = 0; i < listWidget->count(); ++i)
    {
        const QListWidgetItem *rowItem = listWidget->item(i);
        SCASSERT(rowItem != nullptr)
        const auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
        if (rowNode.GetTypeTool() == Tool::NodePoint && not rowNode.IsExcluded())
        {
            ++countPoints;
            pointLabels.insert(rowNode.GetId());
        }
    }

    return countPoints == pointLabels.size();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogWarningIcon(const QWidget *w) -> QString
{
    const QIcon icon = FromTheme(VThemeIcon::DialogWarning);

    qreal scaleFactor = 1;
    if (w != nullptr)
    {
        if (const QScreen *wScreen = w->screen())
        {
            scaleFactor = wScreen->devicePixelRatio();
        }
    }

    const QPixmap pixmap = icon.pixmap(QSize(qRound(16 / scaleFactor), qRound(16 / scaleFactor)));
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    pixmap.save(&buffer, "PNG");
    return QStringLiteral("<img src=\"data:image/png;base64,") + byteArray.toBase64() + "\"/> "_L1;
}

//---------------------------------------------------------------------------------------------------------------------
auto NodeFont(QFont font, bool nodeExcluded) -> QFont
{
    font.setPointSize(12);
    font.setWeight(QFont::Bold);
    font.setStrikeOut(nodeExcluded);
    return font;
}

//---------------------------------------------------------------------------------------------------------------------
void CurrentCurveLength(vidtype curveId, VContainer *data)
{
    SCASSERT(data != nullptr)
    VCurveLength *length = nullptr;
    try
    {
        const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(curveId);
        length = new VCurveLength(curveId, curveId, curve.data(), *data->GetPatternUnit());
    }
    catch (const VExceptionBadId &)
    {
        length = new VCurveLength();
    }

    SCASSERT(length != nullptr)
    length->SetName(currentLength);

    data->AddVariable(length);
}

//---------------------------------------------------------------------------------------------------------------------
void SetTabStopDistance(QPlainTextEdit *edit, int tabWidthChar)
{
    SCASSERT(edit != nullptr)
    const auto fontMetrics = edit->fontMetrics();

    const QString testString(" ");

    // compute the size of a char in double-precision
    static constexpr int bigNumber = 1000; // arbitrary big number.
    const int many_char_width = fontMetrics.horizontalAdvance(testString.repeated(bigNumber));
    const double singleCharWidthDouble = many_char_width / double(bigNumber);
    // set the tab stop with double precision
    edit->setTabStopDistance(tabWidthChar * singleCharWidthDouble);
}

//---------------------------------------------------------------------------------------------------------------------
auto SegmentAliases(GOType curveType, const QString &alias1, const QString &alias2) -> QPair<QString, QString>
{
    // This check helps to find missed objects in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 8, "Not all objects were handled.");

    switch (curveType)
    {
        case GOType::EllipticalArc:
            return CurveAliases<VEllipticalArc>(alias1, alias2);
        case GOType::Arc:
            return CurveAliases<VArc>(alias1, alias2);
        case GOType::CubicBezier:
            return CurveAliases<VCubicBezier>(alias1, alias2);
        case GOType::Spline:
            return CurveAliases<VSpline>(alias1, alias2);
        case GOType::CubicBezierPath:
            return CurveAliases<VCubicBezierPath>(alias1, alias2);
        case GOType::SplinePath:
            return CurveAliases<VSplinePath>(alias1, alias2);
        case GOType::Point:
        case GOType::PlaceLabel:
        case GOType::Unknown:
            Q_UNREACHABLE();
            break;
        default:
            return {};
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto SegmentNames(GOType curveType, const QString &name1, const QString &name2) -> QPair<QString, QString>
{
    // This check helps to find missed objects in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 8, "Not all objects were handled.");

    switch (curveType)
    {
        case GOType::EllipticalArc:
            return CurveNames<VEllipticalArc>(name1, name2);
        case GOType::Arc:
            return CurveNames<VArc>(name1, name2);
        case GOType::CubicBezier:
            return CurveNames<VCubicBezier>(name1, name2);
        case GOType::Spline:
            return CurveNames<VSpline>(name1, name2);
        case GOType::CubicBezierPath:
            return CurveNames<VCubicBezierPath>(name1, name2);
        case GOType::SplinePath:
            return CurveNames<VSplinePath>(name1, name2);
        case GOType::Point:
        case GOType::PlaceLabel:
        case GOType::Unknown:
            Q_UNREACHABLE();
            break;
        default:
            return {};
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto GetNodeName(const VContainer *data, const VPieceNode &node, bool showPassmarkDetails) -> QString
{
    const QSharedPointer<VGObject> obj = data->GetGObject(node.GetId());
    QString name = obj->ObjectName();

    if (node.GetTypeTool() != Tool::NodePoint)
    {
        if (node.GetReverse())
        {
            name = "- "_L1 + name;
        }
    }
    else
    {
        if (showPassmarkDetails && node.IsPassmark())
        {
            switch (node.GetPassmarkLineType())
            {
                case PassmarkLineType::OneLine:
                    name += '|'_L1;
                    break;
                case PassmarkLineType::TwoLines:
                    name += "||"_L1;
                    break;
                case PassmarkLineType::ThreeLines:
                    name += "|||"_L1;
                    break;
                case PassmarkLineType::TMark:
                    name += QStringLiteral("┴");
                    break;
                case PassmarkLineType::ExternalVMark:
                    name += QStringLiteral("⊼");
                    break;
                case PassmarkLineType::InternalVMark:
                    name += QStringLiteral("⊽");
                    break;
                case PassmarkLineType::UMark:
                    name += QStringLiteral("⋃");
                    break;
                case PassmarkLineType::BoxMark:
                    name += QStringLiteral("⎕");
                    break;
                case PassmarkLineType::CheckMark:
                    name += QStringLiteral("✓");
                    break;
                default:
                    break;
            }
        }

        if (not node.IsCheckUniqueness())
        {
            name = '['_L1 + name + ']'_L1;
        }

        if (not node.IsTurnPoint())
        {
            name += QStringLiteral(" ⊙");
        }
    }

    return name;
}

//---------------------------------------------------------------------------------------------------------------------
auto FindNotExcludedPointDown(QListWidget *listWidget, int start) -> int
{
    SCASSERT(listWidget != nullptr);

    int index = -1;
    if (start < 0 || start >= listWidget->count())
    {
        return index;
    }

    int i = start;
    int count = 0;
    do
    {
        const QListWidgetItem *rowItem = listWidget->item(i);
        SCASSERT(rowItem != nullptr)
        auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

        if (not rowNode.IsExcluded() && rowNode.GetTypeTool() == Tool::NodePoint && rowNode.GetId() != NULL_ID)
        {
            index = i;
            break;
        }

        ++i;
        if (i >= listWidget->count())
        {
            i = 0;
        }

        ++count;
    } while (count < listWidget->count());

    return index;
}

//---------------------------------------------------------------------------------------------------------------------
auto FindNotExcludedCurveDown(QListWidget *listWidget, int start) -> int
{
    SCASSERT(listWidget != nullptr);

    int index = -1;
    if (start < 0 || start >= listWidget->count())
    {
        return index;
    }

    int i = start;
    int count = 0;
    do
    {
        const QListWidgetItem *rowItem = listWidget->item(i);
        SCASSERT(rowItem != nullptr)
        auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

        if (not rowNode.IsExcluded() && rowNode.GetTypeTool() != Tool::NodePoint && rowNode.GetId() != NULL_ID)
        {
            index = i;
            break;
        }

        ++i;
        if (i >= listWidget->count())
        {
            i = 0;
        }

        ++count;
    } while (count < listWidget->count());

    return index;
}

//---------------------------------------------------------------------------------------------------------------------
auto InvalidSegment(QListWidget *listWidget, const VContainer *data, QString &error) -> bool
{
    SCASSERT(listWidget != nullptr);

    for (int index = 0; index < listWidget->count(); ++index)
    {
        int firstCurveIndex = -1;
        int pointIndex = -1;
        int secondCurveIndex = -1;

        auto FindPair = [listWidget, &firstCurveIndex, &pointIndex, &secondCurveIndex](int start)
        {
            for (int i = start; i < listWidget->count(); ++i)
            {
                firstCurveIndex = FindNotExcludedCurveDown(listWidget, i);
                if (firstCurveIndex == -1)
                {
                    continue;
                }

                pointIndex = FindNotExcludedPointDown(listWidget, firstCurveIndex + 1);
                if (pointIndex == -1)
                {
                    continue;
                }

                secondCurveIndex = FindNotExcludedCurveDown(listWidget, pointIndex + 1);
                if (secondCurveIndex == -1 || firstCurveIndex == secondCurveIndex)
                {
                    continue;
                }

                return true;
            }

            return false;
        };

        if (not FindPair(index))
        {
            continue;
        }

        const VPieceNode firstCurveNode = RowNode(listWidget, firstCurveIndex);
        const VPieceNode secondCurveNode = RowNode(listWidget, secondCurveIndex);

        QString errorDoubleCurve;
        if (not DoubleCurve(firstCurveNode, secondCurveNode, data, errorDoubleCurve))
        {
            continue;
        }

        const VPieceNode pointNode = RowNode(listWidget, pointIndex);
        if (pointNode.GetId() == NULL_ID)
        {
            continue;
        }

        try
        {
            const QSharedPointer<VAbstractCurve> curve1 = data->GeometricObject<VAbstractCurve>(firstCurveNode.GetId());
            const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(pointNode.GetId());

            error = QObject::tr("Point '%1' does not lie on a curve '%2'").arg(point->name(), curve1->name());

            bool const validSegment = curve1->IsPointOnCurve(point->toQPointF());
            if (not validSegment)
            {
                return true;
            }
        }
        catch (const VExceptionBadId &)
        {
            continue;
        }
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void InitDialogButtonBoxIcons(QDialogButtonBox *buttonBox)
{
    SCASSERT(buttonBox != nullptr)

    QStyle *style = QApplication::style();

    if (QPushButton *bOk = buttonBox->button(QDialogButtonBox::Ok))
    {
        bOk->setIcon(style->standardIcon(QStyle::SP_DialogOkButton));
    }

    if (QPushButton *bApply = buttonBox->button(QDialogButtonBox::Apply))
    {
        bApply->setIcon(style->standardIcon(QStyle::SP_DialogApplyButton));
    }

    if (QPushButton *bCancel = buttonBox->button(QDialogButtonBox::Cancel))
    {
        bCancel->setIcon(style->standardIcon(QStyle::SP_DialogCancelButton));
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto MirrorLinePointsNeighbors(QListWidget *listWidget, quint32 startPoint, quint32 endPoint) -> bool
{
    SCASSERT(listWidget != nullptr)

    if (startPoint == endPoint || startPoint == NULL_ID || endPoint == NULL_ID)
    {
        return false;
    }

    const int startIndex = NodeRowIndex(listWidget, startPoint);
    if (startIndex == -1)
    {
        return false;
    }

    int nextIndex = startIndex + 1;
    if (nextIndex >= listWidget->count())
    {
        nextIndex = 0;
    }

    int prevIndex = startIndex - 1;
    if (prevIndex < 0)
    {
        prevIndex = listWidget->count() - 1;
    }

    const int next = FindNotExcludedNeighborNodeDown(listWidget, nextIndex);
    const int prev = FindNotExcludedNeighborNodeUp(listWidget, prevIndex);

    return (next >= 0 && endPoint == RowNode(listWidget, next).GetId()) ||
           (prev >= 0 && endPoint == RowNode(listWidget, prev).GetId());
}

//---------------------------------------------------------------------------------------------------------------------
auto NodeRowIndex(QListWidget *listWidget, quint32 id) -> int
{
    SCASSERT(listWidget != nullptr)

    for (int i = 0, sz = listWidget->count(); i < sz; ++i)
    {
        const QListWidgetItem *rowItem = listWidget->item(i);
        SCASSERT(rowItem != nullptr)
        auto rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

        if (id == rowNode.GetId())
        {
            return i;
        }
    }

    return -1;
}

//---------------------------------------------------------------------------------------------------------------------
void InitColorPicker(VPE::QtColorPicker *button, const QVector<QColor> &customColor)
{
    SCASSERT(button != nullptr)

    const QMap<QString, QString> defaultColors = VAbstractTool::ColorsList();

    auto i = defaultColors.constBegin();
    while (i != defaultColors.constEnd())
    {
        button->insertColor(QColor(i.key()), i.value());
        ++i;
    }

    for (auto color : customColor)
    {
        button->insertCustomColor(color);
    }

    button->setCurrentColor(QColor(Qt::black));
}

//---------------------------------------------------------------------------------------------------------------------
void InitOperationColorPicker(VPE::QtColorPicker *button, const QVector<QColor> &customColor)
{
    SCASSERT(button != nullptr)

    QMap<QString, QString> defaultColors = VAbstractOperation::OperationColorsList();

    if (defaultColors.contains(ColorDefault))
    {
        button->insertColor(QColor(), defaultColors.value(ColorDefault));
        defaultColors.remove(ColorDefault);
    }

    auto i = defaultColors.constBegin();
    while (i != defaultColors.constEnd())
    {
        button->insertColor(QColor(i.key()), i.value());
        ++i;
    }

    for (auto color : customColor)
    {
        button->insertCustomColor(color);
    }

    button->setCurrentColor(QColor());
}

//---------------------------------------------------------------------------------------------------------------------
void FillComboBoxTypeLine(QComboBox *box, const QMap<QString, QIcon> &stylesPics, const QString &def)
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
/**
 * @brief ChangeCurrentData select item in combobox by id
 * @param box combobox
 * @param value id of item
 */
void ChangeCurrentData(QComboBox *box, const QVariant &value)
{
    SCASSERT(box != nullptr)
    const qint32 index = box->findData(value);
    if (index != -1)
    {
        box->setCurrentIndex(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto GetComboBoxCurrentData(const QComboBox *box, const QString &def) -> QString
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
auto GenerateDefSubCurveName(const VContainer *data, quint32 curveId, const QString &derBase, const QString &base)
    -> QString
{
    QSharedPointer<VAbstractCurve> const curve = data->GeometricObject<VAbstractCurve>(curveId);

    auto GenerateName = [data, curve](const QString &base) -> QString
    {
        qint32 num = 1;
        QString name;
        do
        {
            name = base + QString::number(num++);
        } while (!data->IsUnique(curve->GetTypeHead() + name));

        return name;
    };

    if (!curve->IsDerivative())
    {
        return GenerateName(curve->HeadlessName() + derBase);
    }

    return GenerateName(base);
}

//---------------------------------------------------------------------------------------------------------------------
auto GenerateDefOffsetCurveName(const VContainer *data, quint32 curveId, const QString &derBase, const QString &base)
    -> QString
{
    auto GenerateName = [data](const QString &base) -> QString
    {
        VSplinePath path;
        qint32 num = 1;
        QString name;
        do
        {
            name = base + QString::number(num++);
            path.SetNameSuffix(name);

        } while (!data->IsUnique(path.name()));

        return name;
    };
    QSharedPointer<VAbstractCurve> const curve = data->GeometricObject<VAbstractCurve>(curveId);
    if (!curve->IsDerivative())
    {
        return GenerateName(curve->HeadlessName() + derBase);
    }

    return GenerateName(base);
}
