/************************************************************************
 **
 **  @file   dialoghistory.cpp
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

#include "dialoghistory.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vcubicbezier.h"
#include "../vgeometry/vcubicbezierpath.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vsplinepath.h"
#include "../vmisc/vtablesearch.h"
#include "../vmisc/vvalentinasettings.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcutarc.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcutspline.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcutsplinepath.h"
#include "../xml/vpattern.h"
#include "ui_dialoghistory.h"

#include <QDebug>
#include <QtConcurrent>
#include <functional>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto AttrUInt(const QDomElement &domElement, const QString &name) -> quint32
{
    return VDomDocument::GetParametrUInt(domElement, name, QChar('0'));
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogHistory create dialog
 * @param data container with data
 * @param doc dom document container
 * @param parent parent widget
 */
DialogHistory::DialogHistory(VContainer *data, VPattern *doc, QWidget *parent)
  : DialogTool(data, doc, NULL_ID, parent),
    ui(new Ui::DialogHistory),
    m_searchHistory(new QMenu(this))
{
    ui->setupUi(this);

    VAbstractApplication::VApp()->Settings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    bOk = ui->buttonBox->button(QDialogButtonBox::Ok);
    connect(bOk, &QPushButton::clicked, this, &DialogHistory::DialogAccepted);
    FillTable();
    InitialTable();
    connect(ui->tableWidget, &QTableWidget::cellClicked, this, &DialogHistory::cellClicked);
    connect(this, &DialogHistory::ShowHistoryTool, doc,
            [doc](quint32 id, bool enable) { emit doc->ShowTool(id, enable); });
    connect(doc, &VPattern::ChangedCursor, this, &DialogHistory::ChangedCursor);
    connect(doc, &VPattern::patternChanged, this, &DialogHistory::UpdateHistory);
    ShowPoint();

    InitSearch();
}

//---------------------------------------------------------------------------------------------------------------------
DialogHistory::~DialogHistory()
{
    ui->lineEditFind->blockSignals(true); // prevents crash
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogAccepted save data and emit signal about closed dialog.
 */
void DialogHistory::DialogAccepted()
{
    QTableWidgetItem *item = ui->tableWidget->item(m_cursorToolRecordRow, 0);
    if (item != nullptr)
    {
        auto id = qvariant_cast<quint32>(item->data(Qt::UserRole));
        emit ShowHistoryTool(id, false);
    }
    emit DialogClosed(QDialog::Accepted);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief cellClicked changed history record
 * @param row number row in table
 * @param column number column in table
 */
void DialogHistory::cellClicked(int row, int column)
{
    if (column == 0)
    {
        QTableWidgetItem *item = ui->tableWidget->item(m_cursorRow, 0);
        item->setIcon(QIcon());

        item = ui->tableWidget->item(row, 0);
        m_cursorRow = row;
        item->setIcon(QIcon("://icon/32x32/put_after.png"));
        const auto id = qvariant_cast<quint32>(item->data(Qt::UserRole));
        m_doc->blockSignals(true);
        row == ui->tableWidget->rowCount() - 1 ? m_doc->setCursor(0) : m_doc->setCursor(id);
        m_doc->blockSignals(false);
    }
    else
    {
        QTableWidgetItem *item = ui->tableWidget->item(m_cursorToolRecordRow, 0);
        auto id = qvariant_cast<quint32>(item->data(Qt::UserRole));
        emit ShowHistoryTool(id, false);

        m_cursorToolRecordRow = row;
        item = ui->tableWidget->item(m_cursorToolRecordRow, 0);
        id = qvariant_cast<quint32>(item->data(Qt::UserRole));
        emit ShowHistoryTool(id, true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChangedCursor changed cursor of input. Cursor show after which record we will insert new object
 * @param id id of object
 */
void DialogHistory::ChangedCursor(quint32 id)
{
    for (qint32 i = 0; i < ui->tableWidget->rowCount(); ++i)
    {
        QTableWidgetItem *item = ui->tableWidget->item(i, 0);
        auto rId = qvariant_cast<quint32>(item->data(Qt::UserRole));
        if (rId == id)
        {
            QTableWidgetItem *oldCursorItem = ui->tableWidget->item(m_cursorRow, 0);
            oldCursorItem->setIcon(QIcon());
            m_cursorRow = i;
            item->setIcon(QIcon("://icon/32x32/put_after.png"));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief UpdateHistory update history table
 */
void DialogHistory::UpdateHistory()
{
    FillTable();
    InitialTable();

    m_search->RefreshList(ui->lineEditFind->text());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FillTable fill table
 */
void DialogHistory::FillTable()
{
    ui->tableWidget->clear();
    QVector<VToolRecord> history = m_doc->getLocalHistory();
    qint32 currentRow = -1;
    qint32 count = 0;
    ui->tableWidget->setRowCount(static_cast<int>(history.size())); // Make row count max possible number

    std::function<HistoryRecord(const VToolRecord &tool)> CreateRecord = [this](const VToolRecord &tool)
    { return Record(tool); };

    QVector<HistoryRecord> historyRecords = QtConcurrent::blockingMapped(history, CreateRecord);

    for (auto &record : historyRecords)
    {
        if (not record.description.isEmpty())
        {
            currentRow++;

            {
                auto *item = new QTableWidgetItem(QString());
                item->setTextAlignment(Qt::AlignHCenter);
                item->setData(Qt::UserRole, record.id);
                item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                ui->tableWidget->setItem(currentRow, 0, item);
            }

            auto *item = new QTableWidgetItem(record.description);
            item->setFont(QFont(QStringLiteral("Times"), 12, QFont::Bold));
            item->setFlags(item->flags() ^ Qt::ItemIsEditable);
            ui->tableWidget->setItem(currentRow, 1, item);
            ++count;
        }
    }
    ui->tableWidget->setRowCount(count); // Real row count
    if (count > 0)
    {
        m_cursorRow = CursorRow();
        QTableWidgetItem *item = ui->tableWidget->item(m_cursorRow, 0);
        // cppcheck-suppress unknownMacro
        SCASSERT(item != nullptr)
        item->setIcon(QIcon("://icon/32x32/put_after.png"));
    }
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(20);
}

//---------------------------------------------------------------------------------------------------------------------
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")
/**
 * @brief Record return description for record
 * @param tool record data
 * @return description
 */
auto DialogHistory::Record(const VToolRecord &tool) const -> HistoryRecord
{
    HistoryRecord record;
    record.id = tool.getId();

    bool updateCache = false;
    const QDomElement domElem = m_doc->elementById(tool.getId(), QString(), updateCache);
    if (not domElem.isElement())
    {
        qDebug() << "Can't find element by id" << record.id << Q_FUNC_INFO;
        return record;
    }

    try
    {
        record = RecordDescription(tool, record, domElem);
    }
    catch (const VExceptionBadId &e)
    {
        qDebug() << e.ErrorMessage() << Q_FUNC_INFO;
        return record;
    }
    qDebug() << "Can't create history record for the tool" << record.id;
    return record;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogHistory::RecordDescription(const VToolRecord &tool, HistoryRecord record, const QDomElement &domElem) const
    -> HistoryRecord
{
    // This check helps to find missed tools in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 59, "Not all tools were used in history.");

    switch (tool.getTypeTool())
    {
        case Tool::Arrow:
        case Tool::SinglePoint:
        case Tool::DoublePoint:
        case Tool::LinePoint:
        case Tool::AbstractSpline:
        case Tool::Cut:
        case Tool::Midpoint:         // Same as Tool::AlongLine, but tool will never has such type
        case Tool::ArcIntersectAxis: // Same as Tool::CurveIntersectAxis, but tool will never has such type
        case Tool::BackgroundImage:
        case Tool::BackgroundImageControls:
        case Tool::BackgroundPixmapImage:
        case Tool::BackgroundSVGImage:
        case Tool::LAST_ONE_DO_NOT_USE:
            Q_UNREACHABLE(); //-V501
            break;
        case Tool::BasePoint:
            record.description = tr("%1 - Base point").arg(PointName(tool.getId()));
            return record;
        case Tool::EndLine:
            record.description = tr("%1_%2 - Line from point %1 to point %2")
                                     .arg(PointName(AttrUInt(domElem, AttrBasePoint)), PointName(tool.getId()));
            return record;
        case Tool::Line:
            record.description =
                tr("%1_%2 - Line from point %1 to point %2")
                    .arg(PointName(AttrUInt(domElem, AttrFirstPoint)), PointName(AttrUInt(domElem, AttrSecondPoint)));
            return record;
        case Tool::AlongLine:
            record.description = tr("%3 - Point along line %1_%2")
                                     .arg(PointName(AttrUInt(domElem, AttrFirstPoint)),
                                          PointName(AttrUInt(domElem, AttrSecondPoint)), PointName(tool.getId()));
            return record;
        case Tool::ShoulderPoint:
            record.description = tr("%1 - Point of shoulder").arg(PointName(tool.getId()));
            return record;
        case Tool::Normal:
            record.description = tr("%3 - normal to line %1_%2")
                                     .arg(PointName(AttrUInt(domElem, AttrFirstPoint)),
                                          PointName(AttrUInt(domElem, AttrSecondPoint)), PointName(tool.getId()));
            return record;
        case Tool::Bisector:
            record.description =
                tr("%4 - bisector of angle %1_%2_%3")
                    .arg(PointName(AttrUInt(domElem, AttrFirstPoint)), PointName(AttrUInt(domElem, AttrSecondPoint)),
                         PointName(AttrUInt(domElem, AttrThirdPoint)), PointName(tool.getId()));
            return record;
        case Tool::LineIntersect:
            record.description =
                tr("%5 - intersection of lines %1_%2 and %3_%4")
                    .arg(PointName(AttrUInt(domElem, AttrP1Line1)), PointName(AttrUInt(domElem, AttrP2Line1)),
                         PointName(AttrUInt(domElem, AttrP1Line2)), PointName(AttrUInt(domElem, AttrP2Line2)),
                         PointName(tool.getId()));
            return record;
        case Tool::Spline:
        {
            const QSharedPointer<VSpline> spl = data->GeometricObject<VSpline>(tool.getId());
            record.description = spl->NameForHistory(tr("Curve"));
            return record;
        }
        case Tool::CubicBezier:
        {
            const QSharedPointer<VCubicBezier> spl = data->GeometricObject<VCubicBezier>(tool.getId());
            record.description = spl->NameForHistory(tr("Cubic bezier curve"));
            return record;
        }
        case Tool::Arc:
        {
            const QSharedPointer<VArc> arc = data->GeometricObject<VArc>(tool.getId());
            record.description = arc->NameForHistory(tr("Arc"));
            return record;
        }
        case Tool::ArcWithLength:
        {
            const QSharedPointer<VArc> arc = data->GeometricObject<VArc>(tool.getId());
            record.description = tr("%1 with length %2").arg(arc->NameForHistory(tr("Arc"))).arg(arc->GetLength());
            return record;
        }
        case Tool::SplinePath:
        {
            const QSharedPointer<VSplinePath> splPath = data->GeometricObject<VSplinePath>(tool.getId());
            record.description = splPath->NameForHistory(tr("Spline path"));
            return record;
        }
        case Tool::CubicBezierPath:
        {
            const QSharedPointer<VCubicBezierPath> splPath = data->GeometricObject<VCubicBezierPath>(tool.getId());
            record.description = splPath->NameForHistory(tr("Cubic bezier curve path"));
            return record;
        }
        case Tool::PointOfContact:
            record.description =
                tr("%4 - point of contact of arc with the center in point %1 and line %2_%3")
                    .arg(PointName(AttrUInt(domElem, AttrCenter)), PointName(AttrUInt(domElem, AttrFirstPoint)),
                         PointName(AttrUInt(domElem, AttrSecondPoint)), PointName(tool.getId()));
            return record;
        case Tool::Height:
            record.description =
                tr("Point of perpendicular from point %1 to line %2_%3")
                    .arg(PointName(AttrUInt(domElem, AttrBasePoint)), PointName(AttrUInt(domElem, AttrP1Line)),
                         PointName(AttrUInt(domElem, AttrP2Line)));
            return record;
        case Tool::Triangle:
            record.description =
                tr("Triangle: axis %1_%2, points %3 and %4")
                    .arg(PointName(AttrUInt(domElem, AttrAxisP1)), PointName(AttrUInt(domElem, AttrAxisP2)),
                         PointName(AttrUInt(domElem, AttrFirstPoint)), PointName(AttrUInt(domElem, AttrSecondPoint)));
            return record;
        case Tool::PointOfIntersection:
            record.description = tr("%1 - point of intersection %2 and %3")
                                     .arg(PointName(tool.getId()), PointName(AttrUInt(domElem, AttrFirstPoint)),
                                          PointName(AttrUInt(domElem, AttrSecondPoint)));
            return record;
        case Tool::CutArc:
        {
            const QSharedPointer<VArc> arc = data->GeometricObject<VArc>(AttrUInt(domElem, AttrArc));
            record.description = tr("%1 - cut %2").arg(PointName(tool.getId()), arc->NameForHistory(tr("arc")));
            return record;
        }
        case Tool::CutSpline:
        {
            const quint32 splineId = AttrUInt(domElem, VToolCutSpline::AttrSpline);
            const QSharedPointer<VAbstractCubicBezier> spl = data->GeometricObject<VAbstractCubicBezier>(splineId);
            record.description = tr("%1 - cut %2").arg(PointName(tool.getId()), spl->NameForHistory(tr("curve")));
            return record;
        }
        case Tool::CutSplinePath:
        {
            const quint32 splinePathId = AttrUInt(domElem, VToolCutSplinePath::AttrSplinePath);
            const QSharedPointer<VAbstractCubicBezierPath> splPath =
                data->GeometricObject<VAbstractCubicBezierPath>(splinePathId);
            record.description =
                tr("%1 - cut %2").arg(PointName(tool.getId()), splPath->NameForHistory(tr("curve path")));
            return record;
        }
        case Tool::LineIntersectAxis:
            record.description =
                tr("%1 - point of intersection line %2_%3 and axis through point %4")
                    .arg(PointName(tool.getId()), PointName(AttrUInt(domElem, AttrP1Line)),
                         PointName(AttrUInt(domElem, AttrP2Line)), PointName(AttrUInt(domElem, AttrBasePoint)));
            return record;
        case Tool::CurveIntersectAxis:
            record.description = tr("%1 - point of intersection curve and axis through point %2")
                                     .arg(PointName(tool.getId()), PointName(AttrUInt(domElem, AttrBasePoint)));
            return record;
        case Tool::PointOfIntersectionArcs:
            record.description = tr("%1 - point of arcs intersection").arg(PointName(tool.getId()));
            return record;
        case Tool::PointOfIntersectionCircles:
            record.description = tr("%1 - point of circles intersection").arg(PointName(tool.getId()));
            return record;
        case Tool::PointOfIntersectionCurves:
            record.description = tr("%1 - point of curves intersection").arg(PointName(tool.getId()));
            return record;
        case Tool::PointFromCircleAndTangent:
            record.description = tr("%1 - point from circle and tangent").arg(PointName(tool.getId()));
            return record;
        case Tool::PointFromArcAndTangent:
            record.description = tr("%1 - point from arc and tangent").arg(PointName(tool.getId()));
            return record;
        case Tool::TrueDarts:
            record.description =
                tr("Correction the dart %1_%2_%3")
                    .arg(PointName(AttrUInt(domElem, AttrDartP1)), PointName(AttrUInt(domElem, AttrDartP2)),
                         PointName(AttrUInt(domElem, AttrDartP2)));
            return record;
        case Tool::EllipticalArc:
        {
            const QSharedPointer<VEllipticalArc> elArc = data->GeometricObject<VEllipticalArc>(tool.getId());
            record.description =
                tr("%1 with length %2").arg(elArc->NameForHistory(tr("Elliptical arc"))).arg(elArc->GetLength());
            return record;
        }
        case Tool::Rotation:
            record.description = tr("Rotate objects around point %1. Suffix '%2'")
                                     .arg(PointName(AttrUInt(domElem, AttrCenter)),
                                          VDomDocument::GetParametrString(domElem, AttrSuffix, QString()));
            return record;
        case Tool::FlippingByLine:
            record.description =
                tr("Flipping by line %1_%2. Suffix '%3'")
                    .arg(PointName(AttrUInt(domElem, AttrP1Line)), PointName(AttrUInt(domElem, AttrP2Line)),
                         VDomDocument::GetParametrString(domElem, AttrSuffix, QString()));
            return record;
        case Tool::FlippingByAxis:
            record.description = tr("Flipping by axis through %1 point. Suffix '%2'")
                                     .arg(PointName(AttrUInt(domElem, AttrCenter)),
                                          VDomDocument::GetParametrString(domElem, AttrSuffix, QString()));
            return record;
        case Tool::Move:
            record.description =
                tr("Move objects. Suffix '%1'").arg(VDomDocument::GetParametrString(domElem, AttrSuffix, QString()));
            return record;
        // Because "history" not only show history of pattern, but help restore current data for each pattern's
        // piece, we need add record about details and nodes, but don't show them.
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
        case Tool::PlaceLabel:
        case Tool::InsertNode:
        case Tool::DuplicateDetail:
            return record;
    }

    return record;
}

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief InitialTable set initial option of table
 */
void DialogHistory::InitialTable()
{
    ui->tableWidget->setSortingEnabled(false);
    ui->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(QChar(QChar::Space)));
    ui->tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Tool")));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ShowPoint show selected point
 */
void DialogHistory::ShowPoint()
{
    const QVector<VToolRecord> *history = m_doc->getHistory();
    if (not history->empty())
    {
        QTableWidgetItem *item = ui->tableWidget->item(0, 1);
        item->setSelected(true);
        m_cursorToolRecordRow = 0;
        item = ui->tableWidget->item(0, 0);
        auto id = qvariant_cast<quint32>(item->data(Qt::UserRole));
        emit ShowHistoryTool(id, true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogHistory::PointName return point name by id.
 *
 * Refacoring what hide ugly string getting point name by id.
 * @param pointId point if in data.
 * @return point name.
 */
auto DialogHistory::PointName(quint32 pointId) const -> QString
{
    return data->GeometricObject<VPointF>(pointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief closeEvent handle when windows is closing
 * @param event event
 */
void DialogHistory::closeEvent(QCloseEvent *event)
{
    QTableWidgetItem *item = ui->tableWidget->item(m_cursorToolRecordRow, 0);
    auto id = qvariant_cast<quint32>(item->data(Qt::UserRole));
    emit ShowHistoryTool(id, false);

    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
    settings->SetHistorySearchOptionMatchCase(m_search->IsMatchCase());
    settings->SetHistorySearchOptionWholeWord(m_search->IsMatchWord());
    settings->SetHistorySearchOptionRegexp(m_search->IsMatchRegexp());
    settings->SetHistorySearchOptionUseUnicodeProperties(m_search->IsUseUnicodePreperties());

    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogHistory::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
        RetranslateUi();
    }

    // remember to call base class implementation
    QDialog::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogHistory::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event); // return default behavior NOLINT(bugprone-parent-virtual-call)
}

//---------------------------------------------------------------------------------------------------------------------
void DialogHistory::RetranslateUi()
{
    qint32 currentRow = m_cursorRow;
    UpdateHistory();

    QTableWidgetItem *item = ui->tableWidget->item(m_cursorRow, 0);
    SCASSERT(item != nullptr)
    item->setIcon(QIcon(QString()));

    m_cursorRow = currentRow;
    cellClicked(m_cursorRow, 0);
    ui->lineEditFind->setPlaceholderText(m_search->SearchPlaceholder());
    UpdateSearchControlsTooltips();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogHistory::CursorRow() const -> int
{
    const quint32 cursor = m_doc->getCursor();
    if (cursor == 0)
    {
        return ui->tableWidget->rowCount() - 1;
    }

    for (int i = 0; i < ui->tableWidget->rowCount(); ++i)
    {
        QTableWidgetItem *item = ui->tableWidget->item(i, 0);
        const auto id = qvariant_cast<quint32>(item->data(Qt::UserRole));
        if (cursor == id)
        {
            return i;
        }
    }
    return ui->tableWidget->rowCount() - 1;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogHistory::InitSearch()
{
    m_search = QSharedPointer<VTableSearch>(new VTableSearch(ui->tableWidget));

    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
    m_search->SetUseUnicodePreperties(settings->GetHistorySearchOptionUseUnicodeProperties());
    m_search->SetMatchWord(settings->GetHistorySearchOptionWholeWord());
    m_search->SetMatchRegexp(settings->GetHistorySearchOptionRegexp());
    m_search->SetMatchCase(settings->GetHistorySearchOptionMatchCase());

    ui->lineEditFind->setPlaceholderText(m_search->SearchPlaceholder());

    UpdateSearchControlsTooltips();

    connect(ui->lineEditFind, &QLineEdit::textEdited, this, [this](const QString &term) { m_search->Find(term); });
    connect(ui->lineEditFind, &QLineEdit::editingFinished, this,
            [this]()
            {
                SaveSearchRequest();
                InitSearchHistory();
                m_search->Find(ui->lineEditFind->text());
            });
    connect(ui->toolButtonFindPrevious, &QToolButton::clicked, this,
            [this]()
            {
                SaveSearchRequest();
                InitSearchHistory();
                m_search->FindPrevious();
                ui->labelResults->setText(
                    QStringLiteral("%1/%2").arg(m_search->MatchIndex() + 1).arg(m_search->MatchCount()));
            });
    connect(ui->toolButtonFindNext, &QToolButton::clicked, this,
            [this]()
            {
                SaveSearchRequest();
                InitSearchHistory();
                m_search->FindNext();
                ui->labelResults->setText(
                    QStringLiteral("%1/%2").arg(m_search->MatchIndex() + 1).arg(m_search->MatchCount()));
            });

    connect(m_search.data(), &VTableSearch::HasResult, this,
            [this](bool state)
            {
                ui->toolButtonFindPrevious->setEnabled(state);
                ui->toolButtonFindNext->setEnabled(state);

                if (state)
                {
                    ui->labelResults->setText(
                        QStringLiteral("%1/%2").arg(m_search->MatchIndex() + 1).arg(m_search->MatchCount()));
                }
                else
                {
                    ui->labelResults->setText(tr("0 results"));
                }

                QPalette palette;

                if (not state && not ui->lineEditFind->text().isEmpty())
                {
                    palette.setColor(QPalette::Text, Qt::red);
                    ui->lineEditFind->setPalette(palette);

                    palette.setColor(QPalette::Active, ui->labelResults->foregroundRole(), Qt::red);
                    palette.setColor(QPalette::Inactive, ui->labelResults->foregroundRole(), Qt::red);
                    ui->labelResults->setPalette(palette);
                }
                else
                {
                    ui->lineEditFind->setPalette(palette);
                    ui->labelResults->setPalette(palette);
                }
            });

    connect(ui->toolButtonCaseSensitive, &QToolButton::toggled, this,
            [this](bool checked)
            {
                m_search->SetMatchCase(checked);
                m_search->Find(ui->lineEditFind->text());
                ui->lineEditFind->setPlaceholderText(m_search->SearchPlaceholder());
            });

    connect(ui->toolButtonWholeWord, &QToolButton::toggled, this,
            [this](bool checked)
            {
                m_search->SetMatchWord(checked);
                m_search->Find(ui->lineEditFind->text());
                ui->lineEditFind->setPlaceholderText(m_search->SearchPlaceholder());
            });

    connect(ui->toolButtonRegexp, &QToolButton::toggled, this,
            [this](bool checked)
            {
                m_search->SetMatchRegexp(checked);

                if (checked)
                {
                    ui->toolButtonWholeWord->blockSignals(true);
                    ui->toolButtonWholeWord->setChecked(false);
                    ui->toolButtonWholeWord->blockSignals(false);
                    ui->toolButtonWholeWord->setEnabled(false);

                    ui->toolButtonUseUnicodeProperties->setEnabled(true);
                }
                else
                {
                    ui->toolButtonWholeWord->setEnabled(true);
                    ui->toolButtonUseUnicodeProperties->blockSignals(true);
                    ui->toolButtonUseUnicodeProperties->setChecked(false);
                    ui->toolButtonUseUnicodeProperties->blockSignals(false);
                    ui->toolButtonUseUnicodeProperties->setEnabled(false);
                }
                m_search->Find(ui->lineEditFind->text());
                ui->lineEditFind->setPlaceholderText(m_search->SearchPlaceholder());
            });

    connect(ui->toolButtonUseUnicodeProperties, &QToolButton::toggled, this,
            [this](bool checked)
            {
                m_search->SetUseUnicodePreperties(checked);
                m_search->Find(ui->lineEditFind->text());
            });

    m_searchHistory->setStyleSheet(QStringLiteral("QMenu { menu-scrollable: 1; }"));
    InitSearchHistory();
    ui->pushButtonSearch->setMenu(m_searchHistory);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogHistory::InitSearchHistory()
{
    QStringList searchHistory = VAbstractValApplication::VApp()->ValentinaSettings()->GetHistorySearchHistory();
    m_searchHistory->clear();
    for (const auto &term : searchHistory)
    {
        QAction *action = m_searchHistory->addAction(term);
        action->setData(term);
        connect(action, &QAction::triggered, this,
                [this]()
                {
                    auto *action = qobject_cast<QAction *>(sender());
                    if (action != nullptr)
                    {
                        QString term = action->data().toString();
                        ui->lineEditFind->setText(term);
                        m_search->Find(term);
                        ui->lineEditFind->setFocus();
                    }
                });
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogHistory::SaveSearchRequest()
{
    QStringList searchHistory = VAbstractValApplication::VApp()->ValentinaSettings()->GetHistorySearchHistory();
    QString term = ui->lineEditFind->text();
    if (term.isEmpty())
    {
        return;
    }

    searchHistory.removeAll(term);
    searchHistory.prepend(term);
    while (searchHistory.size() > VTableSearch::MaxHistoryRecords)
    {
        searchHistory.removeLast();
    }
    VAbstractValApplication::VApp()->ValentinaSettings()->SetHistorySearchHistory(searchHistory);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogHistory::UpdateSearchControlsTooltips()
{
    auto UpdateToolTip = [](QAbstractButton *button)
    {
        if (button->toolTip().contains("%1"_L1))
        {
            button->setToolTip(button->toolTip().arg(button->shortcut().toString(QKeySequence::NativeText)));
        }
    };

    UpdateToolTip(ui->toolButtonCaseSensitive);
    UpdateToolTip(ui->toolButtonWholeWord);
    UpdateToolTip(ui->toolButtonRegexp);
    UpdateToolTip(ui->toolButtonUseUnicodeProperties);
    UpdateToolTip(ui->pushButtonSearch);
    UpdateToolTip(ui->toolButtonFindPrevious);
    UpdateToolTip(ui->toolButtonFindNext);
}
