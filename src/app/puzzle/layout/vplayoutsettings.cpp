/************************************************************************
 **
 **  @file   vplayoutsettings.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   29 7, 2021
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2021 Valentina project
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
#include "vplayoutsettings.h"

#include <QComboBox>
#include <QIcon>

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetUnit(Unit unit)
{
    m_unit = unit;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetUnit() const -> Unit
{
    return m_unit;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetWarningSuperpositionOfPieces(bool state)
{
    m_warningSuperpositionOfPieces = state;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetWarningSuperpositionOfPieces() const -> bool
{
    return m_warningSuperpositionOfPieces;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetWarningPiecesOutOfBound(bool state)
{
    m_warningPiecesOutOfBound = state;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetWarningPiecesOutOfBound() const -> bool
{
    return m_warningPiecesOutOfBound;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetTitle(const QString &title)
{
    m_title = title;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetTitle() const -> QString
{
    return m_title;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetDescription(const QString &description)
{
    m_description = description;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetDescription() const -> QString
{
    return m_description;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetTilesSize(qreal width, qreal height)
{
    m_tilesSize.setWidth(width);
    m_tilesSize.setHeight(height);
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetTilesSizeConverted(qreal width, qreal height)
{
    m_tilesSize.setWidth(UnitConvertor(width, GetUnit(), Unit::Px));
    m_tilesSize.setHeight(UnitConvertor(height, GetUnit(), Unit::Px));
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetTilesSize(const QSizeF &size)
{
    m_tilesSize = size;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetTilesSizeConverted(const QSizeF &size)
{
    m_tilesSize = QSizeF(
                UnitConvertor(size.width(), GetUnit(), Unit::Px),
                UnitConvertor(size.height(), GetUnit(), Unit::Px)
                );
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetTilesSize() const -> QSizeF
{
    return m_tilesSize;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetTilesSize(Unit unit) const -> QSizeF
{
    QSizeF convertedSize = QSizeF(
                UnitConvertor(m_tilesSize.width(), Unit::Px, unit),
                UnitConvertor(m_tilesSize.height(), Unit::Px, unit)
                );

    return convertedSize;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetTilesSizeConverted() const -> QSizeF
{
    return GetTilesSize(GetUnit());
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetTilesOrientation() -> PageOrientation
{
    return m_tilesSize.height() >= m_tilesSize.width() ? PageOrientation::Portrait : PageOrientation::Landscape;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetTilesMargins(qreal left, qreal top, qreal right, qreal bottom)
{
    m_tilesMargins.setLeft(left);
    m_tilesMargins.setTop(top);
    m_tilesMargins.setRight(right);
    m_tilesMargins.setBottom(bottom);
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetTilesMarginsConverted(qreal left, qreal top, qreal right, qreal bottom)
{
    m_tilesMargins.setLeft(UnitConvertor(left, GetUnit(), Unit::Px));
    m_tilesMargins.setTop(UnitConvertor(top, GetUnit(), Unit::Px));
    m_tilesMargins.setRight(UnitConvertor(right, GetUnit(), Unit::Px));
    m_tilesMargins.setBottom(UnitConvertor(bottom, GetUnit(), Unit::Px));
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetTilesMargins(const QMarginsF &margins)
{
    m_tilesMargins = margins;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetTilesMarginsConverted(const QMarginsF &margins)
{
    m_tilesMargins = UnitConvertor(margins, GetUnit(), Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetTilesMargins() const -> QMarginsF
{
    return m_tilesMargins;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetTilesMargins(Unit unit) const -> QMarginsF
{
    return UnitConvertor(m_tilesMargins, Unit::Px, unit);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetTilesMarginsConverted() const -> QMarginsF
{
    return UnitConvertor(m_tilesMargins, Unit::Px, GetUnit());
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetShowTiles() const -> bool
{
    return m_showTiles;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetShowTiles(bool value)
{
    m_showTiles = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetTemplateSize(PaperSizeTemplate tmpl) -> QSizeF
{
    qreal height = 0;
    qreal width = 0;

    switch (tmpl)
    {
        case PaperSizeTemplate::A0:
            width = UnitConvertor(841, Unit::Mm, Unit::Px);
            height = UnitConvertor(1189, Unit::Mm, Unit::Px);
        break;

        case PaperSizeTemplate::A1:
            width = UnitConvertor(594, Unit::Mm, Unit::Px);
            height = UnitConvertor(841, Unit::Mm, Unit::Px);
        break;

        case PaperSizeTemplate::A2:
            width = UnitConvertor(420, Unit::Mm, Unit::Px);
            height = UnitConvertor(594, Unit::Mm, Unit::Px);
        break;

        case PaperSizeTemplate::A3:
            width = UnitConvertor(297, Unit::Mm, Unit::Px);
            height = UnitConvertor(420, Unit::Mm, Unit::Px);
        break;

        case PaperSizeTemplate::A4:
            width = UnitConvertor(210, Unit::Mm, Unit::Px);
            height = UnitConvertor(297, Unit::Mm, Unit::Px);
        break;

        case PaperSizeTemplate::Letter:
            width = UnitConvertor(8.5, Unit::Inch, Unit::Px);
            height = UnitConvertor(11, Unit::Inch, Unit::Px);
        break;

        case PaperSizeTemplate::Legal:
            width = UnitConvertor(8.5, Unit::Inch, Unit::Px);
            height = UnitConvertor(14, Unit::Inch, Unit::Px);
        break;

        case PaperSizeTemplate::Tabloid:
            width = UnitConvertor(11, Unit::Inch, Unit::Px);
            height = UnitConvertor(17, Unit::Inch, Unit::Px);
        break;

        case PaperSizeTemplate::Roll24in:
            width = UnitConvertor(24, Unit::Inch, Unit::Px);
            height = UnitConvertor(48, Unit::Inch, Unit::Px);
        break;

        case PaperSizeTemplate::Roll30in:
            width = UnitConvertor(30, Unit::Inch, Unit::Px);
            height = UnitConvertor(60, Unit::Inch, Unit::Px);
        break;

        case PaperSizeTemplate::Roll36in:
            width = UnitConvertor(36, Unit::Inch, Unit::Px);
            height = UnitConvertor(72, Unit::Inch, Unit::Px);
        break;

        case PaperSizeTemplate::Roll42in:
            width = UnitConvertor(42, Unit::Inch, Unit::Px);
            height = UnitConvertor(84, Unit::Inch, Unit::Px);
        break;

        case PaperSizeTemplate::Roll44in:
            width = UnitConvertor(44, Unit::Inch, Unit::Px);
            height = UnitConvertor(88, Unit::Inch, Unit::Px);
        break;

        case PaperSizeTemplate::Roll48in:
            width = UnitConvertor(48, Unit::Inch, Unit::Px);
            height = UnitConvertor(96, Unit::Inch, Unit::Px);
        break;

        case PaperSizeTemplate::Roll62in:
            width = UnitConvertor(62, Unit::Inch, Unit::Px);
            height = UnitConvertor(124, Unit::Inch, Unit::Px);
        break;

        case PaperSizeTemplate::Roll72in:
            width = UnitConvertor(72, Unit::Inch, Unit::Px);
            height = UnitConvertor(144, Unit::Inch, Unit::Px);
        break;

        default:
            break;
    }

    return QSizeF(width, height);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetTemplateName(PaperSizeTemplate tmpl) -> QString
{
    QString tmplName;
    switch (tmpl)
    {
        case PaperSizeTemplate::A0:
            tmplName = QString("A0");
        break;

        case PaperSizeTemplate::A1:
            tmplName = QString("A1");
        break;

        case PaperSizeTemplate::A2:
            tmplName = QString("A2");
        break;

        case PaperSizeTemplate::A3:
            tmplName = QString("A3");
        break;

        case PaperSizeTemplate::A4:
            tmplName = QString("A4");
        break;

        case PaperSizeTemplate::Letter:
            tmplName = tr("Letter");
        break;

        case PaperSizeTemplate::Legal:
            tmplName = tr("Legal");
        break;

        case PaperSizeTemplate::Tabloid:
            tmplName = tr("Tabloid");
        break;

        case PaperSizeTemplate::Roll24in:
            tmplName = tr("Roll 24in");
        break;

        case PaperSizeTemplate::Roll30in:
            tmplName = tr("Roll 30in");
        break;

        case PaperSizeTemplate::Roll36in:
            tmplName = tr("Roll 36in");
        break;

        case PaperSizeTemplate::Roll42in:
            tmplName = tr("Roll 42in");
        break;

        case PaperSizeTemplate::Roll44in:
            tmplName = tr("Roll 44in");
        break;

        case PaperSizeTemplate::Roll48in:
            tmplName = tr("Roll 48in");
        break;

        case PaperSizeTemplate::Roll62in:
            tmplName = tr("Roll 62in");
        break;

        case PaperSizeTemplate::Roll72in:
            tmplName = tr("Roll 72in");
        break;

        case PaperSizeTemplate::Custom:
            tmplName = tr("Custom");
        break;

        default:
            break;
    }

    if(not tmplName.isEmpty())
    {
        tmplName += " " + QStringLiteral("(%1ppi)").arg(PrintDPI);
    }

    return tmplName;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetTemplate(QSizeF size) -> PaperSizeTemplate
{
    Q_UNUSED(size);
    // TODO, float comparision not safe and problems with
    // inch / cm

//    const int max = static_cast<int>(PaperSizeTemplate::Custom);

//    for (int i=0; i < max; i++)
//    {
//        PaperSizeTemplate tmpl = static_cast<PaperSizeTemplate>(i);
//        const QSizeF tmplSize = GetTemplateSize(tmpl);

//        if(size.width() == tmplSize.width())
//        {
//            if(isRollTemplate(tmpl))
//            {
//                return tmpl;
//            }
//            else if(size.height() == tmplSize.height())
//            {
//                return tmpl;
//            }
//        }
//    }

    return PaperSizeTemplate::Custom;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::isRollTemplate(PaperSizeTemplate tmpl) -> bool
{
    switch (tmpl) {
        case PaperSizeTemplate::Roll24in:
        case PaperSizeTemplate::Roll30in:
        case PaperSizeTemplate::Roll36in:
        case PaperSizeTemplate::Roll42in:
        case PaperSizeTemplate::Roll44in:
        case PaperSizeTemplate::Roll48in:
        case PaperSizeTemplate::Roll62in:
        case PaperSizeTemplate::Roll72in:
            return true;
        default:
            return false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::PopulateComboBox(QVector<PaperSizeTemplate> *tmpls, QComboBox* comboBox)
{
    const QIcon icoPaper("://puzzleicon/16x16/template.png");
    const QIcon icoRoll("://puzzleicon/16x16/roll.png");

    QIcon icon;
    for (auto tmpl : *tmpls)
    {
        icon = (isRollTemplate(tmpl))? icoRoll : icoPaper;
        comboBox->addItem(icon, GetTemplateName(tmpl), QVariant(static_cast<int>(tmpl)));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetSheetSize(qreal width, qreal height)
{
    m_size.setWidth(width);
    m_size.setHeight(height);
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetSheetSizeConverted(qreal width, qreal height)
{
    m_size.setWidth(UnitConvertor(width, m_unit, Unit::Px));
    m_size.setHeight(UnitConvertor(height, m_unit, Unit::Px));
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetSheetSize(const QSizeF &size)
{
    m_size = size;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetSheetSizeConverted(const QSizeF &size)
{
    m_size = QSizeF(
                UnitConvertor(size.width(), m_unit, Unit::Px),
                UnitConvertor(size.height(), m_unit, Unit::Px)
                );
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetSheetSize() const -> QSizeF
{
    return m_size;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetSheetSizeConverted() const -> QSizeF
{
    QSizeF convertedSize = QSizeF(
                UnitConvertor(m_size.width(), Unit::Px, m_unit),
                UnitConvertor(m_size.height(), Unit::Px, m_unit)
                );

    return convertedSize;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetOrientation() -> PageOrientation
{
    return m_orientation;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetOrientation(PageOrientation orientation)
{
    if(orientation != m_orientation)
    {
        m_orientation = orientation;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetSheetMargins(qreal left, qreal top, qreal right, qreal bottom)
{
    m_margins.setLeft(left);
    m_margins.setTop(top);
    m_margins.setRight(right);
    m_margins.setBottom(bottom);
}
//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetSheetMarginsConverted(qreal left, qreal top, qreal right, qreal bottom)
{
    m_margins.setLeft(UnitConvertor(left, m_unit, Unit::Px));
    m_margins.setTop(UnitConvertor(top, m_unit, Unit::Px));
    m_margins.setRight(UnitConvertor(right, m_unit, Unit::Px));
    m_margins.setBottom(UnitConvertor(bottom, m_unit, Unit::Px));
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetSheetMargins(const QMarginsF &margins)
{
    m_margins = margins;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetSheetMarginsConverted(const QMarginsF &margins)
{
    m_margins = UnitConvertor(margins, m_unit, Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetSheetMargins() const -> QMarginsF
{
    return m_margins;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetSheetMarginsConverted() const -> QMarginsF
{
    return UnitConvertor(m_margins, Unit::Px, m_unit);
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetFollowGrainline(bool state)
{
    m_followGrainLine = state;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetFollowGrainline() const -> bool
{
    return m_followGrainLine;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetPiecesGap(qreal value)
{
    m_piecesGap = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetPiecesGapConverted(qreal value)
{
    m_piecesGap = UnitConvertor(value, m_unit, Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetPiecesGap() const -> qreal
{
    return m_piecesGap;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetPiecesGapConverted() const -> qreal
{
    return UnitConvertor(m_piecesGap, Unit::Px, m_unit);
}


//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetStickyEdges(bool state)
{
    m_stickyEdges = state;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetStickyEdges() const -> bool
{
    return m_stickyEdges;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetShowGrid() const -> bool
{
    return m_showGrid;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetShowGrid(bool value)
{
    m_showGrid = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetGridColWidth() const -> qreal
{
    return m_gridColWidth;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetGridColWidthConverted() const -> qreal
{
    return UnitConvertor(m_gridColWidth, Unit::Px, m_unit);
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetGridColWidth(qreal value)
{
    m_gridColWidth = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetGridColWidthConverted(qreal value)
{
    m_gridColWidth = UnitConvertor(value, m_unit, Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetGridRowHeight() const -> qreal
{
    return m_gridRowHeight;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetGridRowHeightConverted() const -> qreal
{
    return UnitConvertor(m_gridRowHeight, Unit::Px, m_unit);
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetGridRowHeight(qreal value)
{
    m_gridRowHeight = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetGridRowHeightConverted(qreal value)
{
    m_gridRowHeight = UnitConvertor(value, m_unit, Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::IgnoreTilesMargins() const -> bool
{
    return m_ignoreTilesMargins;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetIgnoreTilesMargins(bool newIgnoreTilesMargins)
{
    m_ignoreTilesMargins = newIgnoreTilesMargins;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::IgnoreMargins() const -> bool
{
    return m_ignoreMargins;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetIgnoreMargins(bool newIgnoreMargins)
{
    m_ignoreMargins = newIgnoreMargins;
}
