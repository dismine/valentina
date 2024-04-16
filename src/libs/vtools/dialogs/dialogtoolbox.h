/************************************************************************
 **
 **  @file   dialogtoolbox.h
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
#ifndef DIALOGTOOLBOX_H
#define DIALOGTOOLBOX_H

#include <QSharedPointer>
#include <QString>
#include <QtGlobal>
#include <chrono>

#include "../vgeometry/vgeometrydef.h"
#include "../vmisc/typedef.h"
#include "../vpatterndb/variables/vinternalvariable.h"

class QPlainTextEdit;
class QPushButton;
class QDialog;
class QObject;
class QEvent;
class QLabel;
class QWidget;
class QColor;
class QLineEdit;
class VContainer;
class QListWidget;
class VPieceNode;
class QDialogButtonBox;

constexpr std::chrono::milliseconds formulaTimerTimeout = std::chrono::milliseconds(300);

extern const QColor errorColor;

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")

struct FormulaData
{
    QString formula;
    const QHash<QString, QSharedPointer<VInternalVariable>> *variables{nullptr};
    QLabel *labelEditFormula{nullptr};
    QLabel *labelResult{nullptr};
    QString postfix{};
    bool checkZero{false};
    bool checkLessThanZero{false};
};

QT_WARNING_POP

auto RowNode(QListWidget *listWidget, int i) -> VPieceNode;
auto NodeRowIndex(QListWidget *listWidget, quint32 id) -> int;
void MoveCursorToEnd(QPlainTextEdit *plainTextEdit);
void DeployFormula(QDialog *dialog, QPlainTextEdit *formula, QPushButton *buttonGrowLength, int formulaBaseHeight);
auto FilterObject(QObject *object, QEvent *event) -> bool;
auto EvalToolFormula(QDialog *dialog, const FormulaData &data, bool &flag) -> qreal;
void ChangeColor(QWidget *widget, const QColor &color);
auto OkColor(const QWidget *widget) -> QColor;
void CheckPointLabel(QDialog *dialog, QLineEdit *edit, QLabel *labelEditNamePoint, const QString &pointName,
                     const VContainer *data, bool &flag);
auto FindNotExcludedNeighborNodeDown(QListWidget *listWidget, int candidate) -> int;
auto FindNotExcludedNeighborNodeUp(QListWidget *listWidget, int candidate) -> int;
auto FindNotExcludedNodeDown(QListWidget *listWidget, int candidate) -> int;
auto FindNotExcludedNodeUp(QListWidget *listWidget, int candidate) -> int;
auto FindNotExcludedPointDown(QListWidget *listWidget, int start) -> int;
auto FindNotExcludedCurveDown(QListWidget *listWidget, int start) -> int;
auto FirstPointEqualLast(QListWidget *listWidget, const VContainer *data, QString &error) -> bool;
auto DoublePoints(QListWidget *listWidget, const VContainer *data, QString &error) -> bool;
auto DoubleCurves(QListWidget *listWidget, const VContainer *data, QString &error) -> bool;
auto EachPointLabelIsUnique(QListWidget *listWidget) -> bool;
auto InvalidSegment(QListWidget *listWidget, const VContainer *data, QString &error) -> bool;
auto DialogWarningIcon() -> QString;
auto NodeFont(QFont font, bool nodeExcluded = false) -> QFont;
void CurrentCurveLength(vidtype curveId, VContainer *data);
void SetTabStopDistance(QPlainTextEdit *edit, int tabWidthChar = 4);
auto LineColor(const QColor &borderColor, int size, const QString &color) -> QIcon;
auto SegmentAliases(GOType curveType, const QString &alias1, const QString &alias2) -> QPair<QString, QString>;
auto GetNodeName(const VContainer *data, const VPieceNode &node, bool showPassmarkDetails = false) -> QString;
void InitDialogButtonBoxIcons(QDialogButtonBox *buttonBox);
auto MirrorLinePointsNeighbors(QListWidget *listWidget, quint32 startPoint, quint32 endPoint) -> bool;

#endif // DIALOGTOOLBOX_H
