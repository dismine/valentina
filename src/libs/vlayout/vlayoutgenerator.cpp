/************************************************************************
 **
 **  @file   vlayoutgenerator.cpp
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

#include "vlayoutgenerator.h"

#include <QElapsedTimer>
#include <QGraphicsRectItem>
#include <QRectF>
#include <QThreadPool>
#include <QtMath>

#include "../ifc/exception/vexceptionterminatedposition.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/def.h"
#include "vlayoutpaper.h"
#include "vlayoutpiece.h"

//---------------------------------------------------------------------------------------------------------------------
VLayoutGenerator::VLayoutGenerator(QObject *parent)
  : QObject(parent),
    papers(),
    bank(new VBank()),
    paperHeight(0),
    paperWidth(0),
    margins(),
    usePrinterFields(true),
#ifdef Q_CC_MSVC
    // See https://stackoverflow.com/questions/15750917/initializing-stdatomic-bool
    stopGeneration(ATOMIC_VAR_INIT(false)),
#else
    stopGeneration(false),
#endif
    state(LayoutErrors::NoError),
    shift(0),
    rotate(true),
    followGrainline(false),
    rotationNumber(2),
    autoCropLength(false),
    autoCropWidth(false),
    saveLength(false),
    unitePages(false),
    stripOptimizationEnabled(false),
    multiplier(1),
    stripOptimization(false),
    textAsPaths(false)
{
}

//---------------------------------------------------------------------------------------------------------------------
VLayoutGenerator::~VLayoutGenerator()
{
    delete bank;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetDetails(const QVector<VLayoutPiece> &details)
{
    bank->SetDetails(details);
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetLayoutWidth(qreal width)
{
    bank->SetLayoutWidth(width);
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetCaseType(Cases caseType)
{
    bank->SetCaseType(caseType);
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
auto VLayoutGenerator::DetailsCount() -> vsizetype
{
    return bank->AllDetailsCount();
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::Generate(const QElapsedTimer &timer, qint64 timeout, LayoutErrors previousState)
{
    auto HasExpired = [this, timer, timeout]()
    {
        if (timer.hasExpired(timeout))
        {
            Timeout();
            return true;
        }
        return false;
    };

    if (HasExpired())
    {
        return;
    }

    if (state != LayoutErrors::Timeout)
    {
        stopGeneration.store(false);
    }

    papers.clear();
    bank->Reset();
    state = previousState;

    int width = PageWidth();
    int height = PageHeight();

    if (VFuzzyComparePossibleNulls(shift, -1))
    {
        if (bank->PrepareDetails())
        {
            SetShift(ToPixel(1, Unit::Cm));
        }
        else
        {
            state = LayoutErrors::PrepareLayoutError;
            return;
        }

        if (stripOptimization)
        {
            const qreal b = bank->GetBiggestDiagonal() * multiplier + bank->GetLayoutWidth();

            auto SetStrip = [this, b](int &side)
            {
                if (side >= b * 2)
                {
                    stripOptimizationEnabled = true;
                    side = qFloor(side / qFloor(side / b));
                }
            };

            IsPortrait() ? SetStrip(height) : SetStrip(width);
        }
    }

    if (HasExpired())
    {
        return;
    }

    if (bank->PrepareUnsorted())
    {
        if (HasExpired())
        {
            return;
        }

        while (bank->AllDetailsCount() > 0)
        {
            if (stopGeneration.load())
            {
                return;
            }

            if (HasExpired())
            {
                return;
            }

            VLayoutPaper paper(height, width, bank->GetLayoutWidth());
            paper.SetShift(shift);
            paper.SetPaperIndex(static_cast<quint32>(papers.count()));
            paper.SetRotate(rotate);
            paper.SetFollowGrainline(followGrainline);
            paper.SetRotationNumber(rotationNumber);
            paper.SetSaveLength(saveLength);
            paper.SetOriginPaperPortrait(IsPortrait());
            do
            {
                const int index = bank->GetNext();
                try
                {
                    if (paper.ArrangeDetail(bank->GetDetail(index), stopGeneration))
                    {
                        bank->Arranged(index);
                    }
                    else
                    {
                        bank->NotArranged(index);
                    }
                }
                catch (const VExceptionTerminatedPosition &e)
                {
                    qCritical() << e.ErrorMessage();
                    state = LayoutErrors::TerminatedByException;
                    return;
                }

                if (stopGeneration.load())
                {
                    break;
                }

                if (HasExpired())
                {
                    return;
                }
            } while (bank->LeftToArrange() > 0);

            if (stopGeneration.load())
            {
                return;
            }

            if (HasExpired())
            {
                return;
            }

            if (paper.Count() > 0)
            {
                papers.append(paper);
            }
            else
            {
                state = LayoutErrors::EmptyPaperError;
                return;
            }
        }
    }
    else
    {
        state = LayoutErrors::PrepareLayoutError;
        return;
    }

    if (HasExpired())
    {
        return;
    }

    if (stripOptimizationEnabled)
    {
        GatherPages();
    }

    if (autoCropWidth)
    {
        OptimizeWidth();
    }

    if (IsUnitePages())
    {
        UnitePages();
    }

    if (bank->FailedToArrange() == 0)
    {
        state = LayoutErrors::NoError;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::LayoutEfficiency() const -> qreal
{
    qreal efficiency = 0;
    if (not papers.isEmpty())
    {
        efficiency = MasterPage().Efficiency();
    }
    return efficiency;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::State() const -> LayoutErrors
{
    return state;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::PapersCount() const -> vsizetype
{
    return papers.size();
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::GetPapersItems() const -> QList<QGraphicsItem *>
{
    QList<QGraphicsItem *> list;
    list.reserve(papers.count());
    for (const auto &paper : papers)
    {
        list.append(paper.GetPaperItem(autoCropLength, autoCropWidth, IsTestAsPaths()));
    }
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::GetGlobalContours() const -> QList<QGraphicsItem *>
{
    QList<QGraphicsItem *> list;
    list.reserve(papers.count());
    for (const auto &paper : papers)
    {
        list.append(paper.GetGlobalContour());
    }
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::GetAllDetailsItems() const -> QList<QList<QGraphicsItem *>>
{
    QList<QList<QGraphicsItem *>> list;
    list.reserve(papers.count());
    for (const auto &paper : papers)
    {
        list.append(paper.GetItemDetails(IsTestAsPaths()));
    }
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::GetAllDetails() const -> QVector<QVector<VLayoutPiece>>
{
    QVector<QVector<VLayoutPiece>> list;
    list.reserve(papers.count());
    for (const auto &paper : papers)
    {
        list.append(paper.GetDetails());
    }
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::Abort()
{
    stopGeneration.store(true);
    state = LayoutErrors::ProcessStoped;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::Timeout()
{
    stopGeneration.store(true);
    if (state == LayoutErrors::NoError)
    {
        state = LayoutErrors::Timeout;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::IsStripOptimization() const -> bool
{
    return stripOptimization;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetStripOptimization(bool value)
{
    stripOptimization = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::IsTestAsPaths() const -> bool
{
    return textAsPaths;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetTextAsPaths(bool value)
{
    textAsPaths = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::IsRotationNeeded() const -> bool
{
    if (followGrainline)
    {
        return bank->IsRotationNeeded();
    }
    else
    {
        return true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::GetMultiplier() const -> quint8
{
    return multiplier;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetMultiplier(quint8 value)
{
    if (value > 10)
    {
        multiplier = 10;
    }
    else if (value == 0)
    {
        multiplier = 1;
    }
    else
    {
        multiplier = value;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::PageHeight() const -> int
{
    return static_cast<int>(paperHeight - (margins.top() + margins.bottom()));
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::PageWidth() const -> int
{
    return static_cast<int>(paperWidth - (margins.left() + margins.right()));
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::IsPortrait() const -> bool
{
    return PageHeight() >= PageWidth();
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::GatherPages()
{
    QList<QList<VLayoutPiece>> nDetails;
    qreal length = 0;
    int j = 0; // papers count

    for (int i = 0; i < papers.size(); ++i)
    {
        const QRectF rec = papers.at(i).DetailsBoundingRect();
        if (IsPortrait())
        {
            int paperHeight = qRound(rec.y() + rec.height());

            if (i != papers.size() - 1)
            {
                paperHeight += qRound(bank->GetLayoutWidth() * 2);
            }

            if (length + paperHeight <= PageHeight())
            {
                UniteDetails(j, nDetails, length, i);
                length += paperHeight;
            }
            else
            {
                length = 0; // Start new paper
                ++j;        // New paper
                UniteDetails(j, nDetails, length, i);
                length += paperHeight;
            }
        }
        else
        {
            int paperWidth = qRound(rec.x() + rec.width());

            if (i != papers.size() - 1)
            {
                paperWidth += qRound(bank->GetLayoutWidth() * 2);
            }

            if (length + paperWidth <= PageWidth())
            {
                UniteDetails(j, nDetails, length, i);
                length += paperWidth;
            }
            else
            {
                length = 0; // Start new paper
                ++j;        // New paper
                UniteDetails(j, nDetails, length, i);
                length += paperWidth;
            }
        }
    }

    QVector<VLayoutPaper> nPapers;
    nPapers.reserve(nDetails.size());
    for (int i = 0; i < nDetails.size(); ++i)
    {
        VLayoutPaper paper(PageHeight(), PageWidth(), bank->GetLayoutWidth());
        paper.SetShift(shift);
        paper.SetPaperIndex(static_cast<quint32>(i));
        paper.SetRotate(rotate);
        paper.SetFollowGrainline(followGrainline);
        paper.SetRotationNumber(rotationNumber);
        paper.SetSaveLength(saveLength);
        paper.SetDetails(nDetails.at(i));

        nPapers.append(paper);
    }

    papers.clear();
    papers = nPapers;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::OptimizeWidth()
{
    QVector<VLayoutPiece> newDetails;
    for (auto &paper : papers)
    {
        const QRectF rec = paper.DetailsBoundingRect();
        QVector<VLayoutPiece> details = paper.GetDetails();

        newDetails.resize(0); // resize(0) preserves capacity, unlike QVector::clear()
        newDetails.reserve(details.size());
        for (auto &d : details)
        {
            IsPortrait() ? d.Translate(-rec.x() + 1, 0) : d.Translate(0, -rec.y() + 1);
            newDetails.append(d);
        }

        paper.SetDetails(newDetails);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::UnitePages()
{
    if (papers.size() < 2)
    {
        return;
    }

    QList<qreal> papersLength;
    QList<QList<VLayoutPiece>> nDetails;
    qreal length = 0;
    int j = 0; // papers count

    for (int i = 0; i < papers.size(); ++i)
    {
        if (IsPortrait())
        {
            int paperHeight = 0;
            if (autoCropLength)
            {
                const QRectF rec = papers.at(i).DetailsBoundingRect();
                paperHeight = qRound(rec.y() + rec.height());
            }
            else
            {
                paperHeight = papers.at(i).GetHeight();
            }

            if (i != papers.size() - 1)
            {
                paperHeight = qRound(paperHeight + bank->GetLayoutWidth() * 2);
            }

            if (length + paperHeight <= QIMAGE_MAX)
            {
                UniteDetails(j, nDetails, length, i);
                length += paperHeight;
                UnitePapers(j, papersLength, length);
            }
            else
            {
                length = 0; // Start new paper
                ++j;        // New paper
                UniteDetails(j, nDetails, length, i);
                length += paperHeight;
                UnitePapers(j, papersLength, length);
            }
        }
        else
        {
            int paperWidth = 0;
            if (autoCropLength)
            {
                const QRectF rec = papers.at(i).DetailsBoundingRect();
                paperWidth = qRound(rec.x() + rec.width());
            }
            else
            {
                paperWidth = papers.at(i).GetWidth();
            }

            if (i != papers.size() - 1)
            {
                paperWidth = qRound(paperWidth + bank->GetLayoutWidth() * 2);
            }

            if (length + paperWidth <= QIMAGE_MAX)
            {
                UniteDetails(j, nDetails, length, i);
                length += paperWidth;
                UnitePapers(j, papersLength, length);
            }
            else
            {
                length = 0; // Start new paper
                ++j;        // New paper
                UniteDetails(j, nDetails, length, i);
                length += paperWidth;
                UnitePapers(j, papersLength, length);
            }
        }
    }

    QVector<VLayoutPaper> nPapers;
    nPapers.reserve(nDetails.size());
    for (int i = 0; i < nDetails.size(); ++i)
    {
        const int height = IsPortrait() ? qRound(papersLength.at(i) + accuracyPointOnLine * 4) : PageHeight();
        const int width = IsPortrait() ? PageWidth() : qRound(papersLength.at(i) + accuracyPointOnLine * 4);

        VLayoutPaper paper(height, width, bank->GetLayoutWidth());
        paper.SetShift(shift);
        paper.SetPaperIndex(static_cast<quint32>(i));
        paper.SetRotate(rotate);
        paper.SetFollowGrainline(followGrainline);
        paper.SetRotationNumber(rotationNumber);
        paper.SetSaveLength(saveLength);
        paper.SetDetails(nDetails.at(i));

        nPapers.append(paper);
    }

    papers.clear();
    papers = nPapers;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::UniteDetails(int j, QList<QList<VLayoutPiece>> &nDetails, qreal length, int i) const
{
    if ((j == 0 && nDetails.isEmpty()) || j >= nDetails.size())
    { // First or new details in paper
        nDetails.insert(j, MoveDetails(length, papers.at(i).GetDetails()));
    }
    else
    {
        nDetails[j].append(MoveDetails(length, papers.at(i).GetDetails()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::UnitePapers(int j, QList<qreal> &papersLength, qreal length)
{
    if ((j == 0 && papersLength.isEmpty()) || j >= papersLength.size())
    {
        papersLength.insert(j, length);
    }
    else
    {
        papersLength[j] = length;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::MoveDetails(qreal length, const QVector<VLayoutPiece> &details) const -> QList<VLayoutPiece>
{
    if (qFuzzyIsNull(length))
    {
        return ConvertToList(details);
    }

    QList<VLayoutPiece> newDetails;
    for (auto d : details)
    {
        IsPortrait() ? d.Translate(0, length) : d.Translate(length, 0);
        newDetails.append(d);
    }

    return newDetails;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief MasterPage return one "master" page combined all pieces on all pages.
 *
 * The main purpose of this method is to return the master page. This way we can efficiently calculate the efficiency
 * of a solution taking into account empty space between bounding rectangles.
 *
 * @return master page
 */
auto VLayoutGenerator::MasterPage() const -> VLayoutPaper
{
    if (papers.size() < 2)
    {
        return papers.constFirst();
    }

    QList<QList<VLayoutPiece>> nDetails;
    qreal length = 0;
    const int j = 0; // papers count. Always 1.

    for (int i = 0; i < papers.size(); ++i)
    {
        if (IsPortrait())
        {
            int paperHeight = 0;
            if (autoCropLength)
            {
                const QRectF rec = papers.at(i).DetailsBoundingRect();
                paperHeight = qRound(rec.y() + rec.height());
            }
            else
            {
                paperHeight = papers.at(i).GetHeight();
            }

            if (i != papers.size() - 1)
            {
                paperHeight = qRound(paperHeight + bank->GetLayoutWidth() * 2);
            }

            UniteDetails(j, nDetails, length, i);
            length += paperHeight;
        }
        else
        {
            int paperWidth = 0;
            if (autoCropLength)
            {
                const QRectF rec = papers.at(i).DetailsBoundingRect();
                paperWidth = qRound(rec.x() + rec.width());
            }
            else
            {
                paperWidth = papers.at(i).GetWidth();
            }

            if (i != papers.size() - 1)
            {
                paperWidth = qRound(paperWidth + bank->GetLayoutWidth() * 2);
            }

            UniteDetails(j, nDetails, length, i);
            length += paperWidth;
        }
    }

    const int height = IsPortrait() ? qRound(length + accuracyPointOnLine * 4) : PageHeight();
    const int width = IsPortrait() ? PageWidth() : qRound(length + accuracyPointOnLine * 4);

    VLayoutPaper paper(height, width, bank->GetLayoutWidth());
    paper.SetShift(shift);
    paper.SetPaperIndex(static_cast<quint32>(0));
    paper.SetRotate(rotate);
    paper.SetFollowGrainline(followGrainline);
    paper.SetRotationNumber(rotationNumber);
    paper.SetSaveLength(saveLength);
    paper.SetDetails(nDetails.at(0));

    return paper;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::IsUnitePages() const -> bool
{
    return unitePages;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetUnitePages(bool value)
{
    unitePages = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::IsSaveLength() const -> bool
{
    return saveLength;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetSaveLength(bool value)
{
    saveLength = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::IsPreferOneSheetSolution() const -> bool
{
    return preferOneSheetSolution;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetPreferOneSheetSolution(bool value)
{
    preferOneSheetSolution = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::GetAutoCropLength() const -> bool
{
    return autoCropLength;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetAutoCropLength(bool value)
{
    autoCropLength = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::GetAutoCropWidth() const -> bool
{
    return autoCropWidth;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetAutoCropWidth(bool value)
{
    autoCropWidth = value;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
auto VLayoutGenerator::GetRotationNumber() const -> int
{
    return rotationNumber;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetRotationNumber(int value)
{
    rotationNumber = value;

    if (rotationNumber > 360 || rotationNumber < 1)
    {
        rotationNumber = 2;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::GetRotate() const -> bool
{
    return rotate;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetRotate(bool value)
{
    rotate = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::GetFollowGrainline() const -> bool
{
    return followGrainline;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetFollowGrainline(bool value)
{
    followGrainline = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::GetManualPriority() const -> bool
{
    return bank->GetManualPriority();
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetManualPriority(bool value)
{
    bank->SetManualPriority(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::IsNestQuantity() const -> bool
{
    return bank->IsNestQuantity();
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetNestQuantity(bool value)
{
    bank->SetNestQuantity(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::GetPaperWidth() const -> qreal
{
    return paperWidth;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetPaperWidth(qreal value)
{
    paperWidth = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::GetNestingTime() const -> int
{
    return nestingTime;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::GetNestingTimeMSecs() const -> int
{
    return nestingTime * 60000;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetNestingTime(int value)
{
    nestingTime = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::GetEfficiencyCoefficient() const -> qreal
{
    return efficiencyCoefficient;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetEfficiencyCoefficient(qreal coefficient)
{
    efficiencyCoefficient = coefficient;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::IsUsePrinterFields() const -> bool
{
    return usePrinterFields;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::GetPrinterFields() const -> QMarginsF
{
    return margins;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetPrinterFields(bool usePrinterFields, const QMarginsF &value)
{
    this->usePrinterFields = usePrinterFields;
    margins = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::GetShift() const -> qreal
{
    return shift;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetShift(qreal shift)
{
    this->shift = shift;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutGenerator::GetPaperHeight() const -> qreal
{
    return paperHeight;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetPaperHeight(qreal value)
{
    paperHeight = value;
}
