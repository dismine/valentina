/************************************************************************
 **
 **  @file   vpsheet.cpp
 **  @author Ronan Le Tiec
 **  @date   23 5, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2020 Valentina project
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
#include "vpsheet.h"

#include "vppiecelist.h"
#include "vplayout.h"

//---------------------------------------------------------------------------------------------------------------------
VPSheet::VPSheet(VPLayout* layout) :
    m_layout(layout)
{
    m_pieceList = new VPPieceList(layout, this);
}

//---------------------------------------------------------------------------------------------------------------------
VPSheet::~VPSheet()
{
    delete m_pieceList;
}

//---------------------------------------------------------------------------------------------------------------------
QSizeF VPSheet::GetTemplateSize(PaperSizeTemplate tmpl)
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
QString VPSheet::GetTemplateName(PaperSizeTemplate tmpl)
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
PaperSizeTemplate VPSheet::GetTemplate(QSizeF size)
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
bool VPSheet::isRollTemplate(PaperSizeTemplate tmpl)
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
void VPSheet::PopulateComboBox(QVector<PaperSizeTemplate> *tmpls, QComboBox* comboBox)
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
VPLayout* VPSheet::GetLayout()
{
    return m_layout;
}

//---------------------------------------------------------------------------------------------------------------------
VPPieceList* VPSheet::GetPieceList()
{
    return m_pieceList;
}

//---------------------------------------------------------------------------------------------------------------------
QString VPSheet::GetName() const
{
    return m_name;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetName(const QString &name)
{
    m_name = name;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetSize(qreal width, qreal height)
{
    m_size.setWidth(width);
    m_size.setHeight(height);
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetSizeConverted(qreal width, qreal height)
{
    m_size.setWidth(UnitConvertor(width, m_layout->GetUnit(), Unit::Px));
    m_size.setHeight(UnitConvertor(height, m_layout->GetUnit(), Unit::Px));
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetSize(const QSizeF &size)
{
    m_size = size;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetSizeConverted(const QSizeF &size)
{
    m_size = QSizeF(
                UnitConvertor(size.width(), m_layout->GetUnit(), Unit::Px),
                UnitConvertor(size.height(), m_layout->GetUnit(), Unit::Px)
                );
}

//---------------------------------------------------------------------------------------------------------------------
QSizeF VPSheet::GetSheetSize() const
{
    return m_size;
}

//---------------------------------------------------------------------------------------------------------------------
QSizeF VPSheet::GetSheetSizeConverted() const
{
    QSizeF convertedSize = QSizeF(
                UnitConvertor(m_size.width(), Unit::Px, m_layout->GetUnit()),
                UnitConvertor(m_size.height(), Unit::Px, m_layout->GetUnit())
                );

    return convertedSize;
}

//---------------------------------------------------------------------------------------------------------------------
PageOrientation VPSheet::GetOrientation()
{
    return m_orientation;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetOrientation(PageOrientation orientation)
{
    if(orientation != m_orientation)
    {
        m_orientation = orientation;
    }
}


//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetMargins(qreal left, qreal top, qreal right, qreal bottom)
{
    m_margins.setLeft(left);
    m_margins.setTop(top);
    m_margins.setRight(right);
    m_margins.setBottom(bottom);
}
//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetMarginsConverted(qreal left, qreal top, qreal right, qreal bottom)
{
    m_margins.setLeft(UnitConvertor(left, m_layout->GetUnit(), Unit::Px));
    m_margins.setTop(UnitConvertor(top, m_layout->GetUnit(), Unit::Px));
    m_margins.setRight(UnitConvertor(right, m_layout->GetUnit(), Unit::Px));
    m_margins.setBottom(UnitConvertor(bottom, m_layout->GetUnit(), Unit::Px));
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetMargins(const QMarginsF &margins)
{
    m_margins = margins;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetMarginsConverted(const QMarginsF &margins)
{
    m_margins = UnitConvertor(margins, m_layout->GetUnit(), Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
QMarginsF VPSheet::GetSheetMargins() const
{
    return m_margins;
}

//---------------------------------------------------------------------------------------------------------------------
QMarginsF VPSheet::GetSheetMarginsConverted() const
{
    return UnitConvertor(m_margins, Unit::Px, m_layout->GetUnit());
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetFollowGrainline(FollowGrainline state)
{
    m_followGrainLine = state;
}

//---------------------------------------------------------------------------------------------------------------------
FollowGrainline VPSheet::GetFollowGrainline() const
{
    return m_followGrainLine;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetPiecesGap(qreal value)
{
    m_piecesGap = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetPiecesGapConverted(qreal value)
{
    m_piecesGap = UnitConvertor(value, m_layout->GetUnit(), Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPSheet::GetPiecesGap() const
{
    return m_piecesGap;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPSheet::GetPiecesGapConverted() const
{
    return UnitConvertor(m_piecesGap, Unit::Px, m_layout->GetUnit());
}


//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetStickyEdges(bool state)
{
    m_stickyEdges = state;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPSheet::GetStickyEdges() const
{
    return m_stickyEdges;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::ClearSelection()
{
    m_pieceList->ClearSelection();
}
