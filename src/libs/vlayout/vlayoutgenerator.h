/************************************************************************
 **
 **  @file   vlayoutgenerator.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   2 1, 2015
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

#ifndef VLAYOUTGENERATOR_H
#define VLAYOUTGENERATOR_H

#include <QList>
#include <QMargins>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QVector>
#include <QtGlobal>
#include <atomic>
#include <memory>

#include "vbank.h"
#include "vlayoutdef.h"
#include "vlayoutpaper.h"

class QGraphicsItem;
class QElapsedTimer;

class VLayoutGenerator : public QObject
{
    Q_OBJECT // NOLINT

public:
    explicit VLayoutGenerator(QObject *parent = nullptr);
    ~VLayoutGenerator() override;

    void SetDetails(const QVector<VLayoutPiece> &details);
    void SetLayoutWidth(qreal width);
    void SetCaseType(Cases caseType);
    auto DetailsCount() -> vsizetype;

    auto GetPaperHeight() const -> qreal;
    void SetPaperHeight(qreal value);

    auto GetPaperWidth() const -> qreal;
    void SetPaperWidth(qreal value);

    auto GetNestingTime() const -> int;
    auto GetNestingTimeMSecs() const -> int;
    void SetNestingTime(int value);

    auto GetEfficiencyCoefficient() const -> qreal;
    void SetEfficiencyCoefficient(qreal coefficient);

    auto IsUsePrinterFields() const -> bool;
    auto GetPrinterFields() const -> QMarginsF;
    void SetPrinterFields(bool usePrinterFields, const QMarginsF &value);

    auto GetShift() const -> qreal;
    void SetShift(qreal shift);

    void Generate(const QElapsedTimer &timer, qint64 timeout, LayoutErrors previousState = LayoutErrors::NoError);

    auto LayoutEfficiency() const -> qreal;

    auto State() const -> LayoutErrors;

    auto PapersCount() const -> vsizetype;

    Q_REQUIRED_RESULT auto GetPapersItems() const -> QList<QGraphicsItem *>;
    Q_REQUIRED_RESULT auto GetGlobalContours() const -> QList<QGraphicsItem *>;
    Q_REQUIRED_RESULT auto GetAllDetailsItems() const -> QList<QList<QGraphicsItem *>>;

    auto GetAllDetails() const -> QVector<QVector<VLayoutPiece>>;

    auto GetRotate() const -> bool;
    void SetRotate(bool value);

    auto GetFollowGrainline() const -> bool;
    void SetFollowGrainline(bool value);

    auto GetManualPriority() const -> bool;
    void SetManualPriority(bool value);

    auto IsNestQuantity() const -> bool;
    void SetNestQuantity(bool value);

    auto GetRotationNumber() const -> int;
    void SetRotationNumber(int value);

    auto GetAutoCropLength() const -> bool;
    void SetAutoCropLength(bool value);

    auto GetAutoCropWidth() const -> bool;
    void SetAutoCropWidth(bool value);

    auto IsSaveLength() const -> bool;
    void SetSaveLength(bool value);

    auto IsPreferOneSheetSolution() const -> bool;
    void SetPreferOneSheetSolution(bool value);

    auto IsUnitePages() const -> bool;
    void SetUnitePages(bool value);

    auto GetMultiplier() const -> quint8;
    void SetMultiplier(quint8 value);

    auto IsStripOptimization() const -> bool;
    void SetStripOptimization(bool value);

    auto IsTestAsPaths() const -> bool;
    void SetTextAsPaths(bool value);

    auto IsBoundaryTogetherWithNotches() const -> bool;
    void SetBoundaryTogetherWithNotches(bool value);

    auto IsShowLayoutAllowance() const -> bool;
    void SetShowLayoutAllowance(bool value);

    auto IsRotationNeeded() const -> bool;

    auto IsPortrait() const -> bool;

public slots:
    void Abort();
    void Timeout();

private:
    Q_DISABLE_COPY_MOVE(VLayoutGenerator) // NOLINT
    QVector<VLayoutPaper> papers{};
    VBank *bank{new VBank()};
    qreal paperHeight{0};
    qreal paperWidth{0};
    QMarginsF margins{};
    bool usePrinterFields{true};
    std::atomic_bool stopGeneration{
#ifdef Q_CC_MSVC
        ATOMIC_VAR_INIT(false)
#else
        false
#endif
    };
    LayoutErrors state{LayoutErrors::NoError};
    qreal shift{0};
    bool rotate{true};
    bool followGrainline{false};
    int rotationNumber{2};
    bool autoCropLength{false};
    bool autoCropWidth{false};
    bool saveLength{false};
    bool preferOneSheetSolution{false};
    bool unitePages{false};
    bool stripOptimizationEnabled{false};
    quint8 multiplier{1};
    bool stripOptimization{false};
    bool textAsPaths{false};
    bool togetherWithNotches{false};
    int nestingTime{1};
    qreal efficiencyCoefficient{0.0};
    bool showLayoutAllowance{false};

    auto PageHeight() const -> int;
    auto PageWidth() const -> int;

    void OptimizeWidth();
    void GatherPages();
    void UnitePages();
    void UniteDetails(int j, QList<QList<VLayoutPiece>> &nDetails, qreal length, int i) const;
    void UnitePapers(int j, QList<qreal> &papersLength, qreal length);
    auto MoveDetails(qreal length, const QVector<VLayoutPiece> &details) const -> QList<VLayoutPiece>;
    auto MasterPage() const -> VLayoutPaper;
};

#endif // VLAYOUTGENERATOR_H
