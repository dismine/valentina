/************************************************************************
 **
 **  @file   dialogsavelayout.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   9 10, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2017 Valentina project
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

#include "vabstractlayoutdialog.h"
#include "../vmisc/def.h"
#include "../vmisc/vmath.h"

#include <QApplication>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
VAbstractLayoutDialog::VAbstractLayoutDialog(QWidget *parent)
  : QDialog(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractLayoutDialog::InitTemplates(QComboBox *comboBoxTemplates)
{
    SCASSERT(comboBoxTemplates != nullptr)
    const QIcon icoPaper("://icon/16x16/template.png"_L1);
    const QIcon icoRoll("://icon/16x16/roll.png"_L1);
    const auto pdi = QStringLiteral("(%1ppi)").arg(PrintDPI);

    auto cntr = static_cast<VIndexType>(PaperSizeTemplate::A0);
    for (const auto &v : PageFormatNames())
    {
        if (cntr <= static_cast<int>(PaperSizeTemplate::Tabloid))
        {
            comboBoxTemplates->addItem(icoPaper, v + QChar(QChar::Space) + pdi, QVariant(cntr++));
        }
        else if (cntr <= static_cast<int>(PaperSizeTemplate::Roll72in))
        {
            comboBoxTemplates->addItem(icoRoll, v + QChar(QChar::Space) + pdi, QVariant(cntr++));
        }
        else
        {
            comboBoxTemplates->addItem(v + QChar(QChar::Space) + pdi, QVariant(cntr++));
        }
    }
    comboBoxTemplates->setCurrentIndex(-1);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractLayoutDialog::InitTileTemplates(QComboBox *comboBoxTemplates, bool keepCustom)
{
    // cppcheck-suppress unknownMacro
    SCASSERT(comboBoxTemplates != nullptr)
    InitTemplates(comboBoxTemplates);

    // remove unused formats
    for (auto i = static_cast<int>(PaperSizeTemplate::Roll24in); i <= static_cast<int>(PaperSizeTemplate::Custom); ++i)
    {
        if (keepCustom && i == static_cast<int>(PaperSizeTemplate::Custom))
        {
            continue;
        }
        comboBoxTemplates->removeItem(comboBoxTemplates->findData(i));
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractLayoutDialog::PageFormatNames() -> const VAbstractLayoutDialog::FormatsVector &
{
    // must be the same order as PaperSizeTemplate constants
    static const FormatsVector pageFormatNames{
        QStringLiteral("A0"),
        QStringLiteral("A1"),
        QStringLiteral("A2"),
        QStringLiteral("A3"),
        QStringLiteral("A4"),
        QApplication::translate("VAbstractLayoutDialog", "Letter", "Paper format"),
        QApplication::translate("VAbstractLayoutDialog", "Legal", "Paper format"),
        QApplication::translate("VAbstractLayoutDialog", "Tabloid", "Paper format"),
        QApplication::translate("VAbstractLayoutDialog", "Roll 24in", "Paper format"),
        QApplication::translate("VAbstractLayoutDialog", "Roll 30in", "Paper format"),
        QApplication::translate("VAbstractLayoutDialog", "Roll 36in", "Paper format"),
        QApplication::translate("VAbstractLayoutDialog", "Roll 42in", "Paper format"),
        QApplication::translate("VAbstractLayoutDialog", "Roll 44in", "Paper format"),
        QApplication::translate("VAbstractLayoutDialog", "Roll 48in", "Paper format"),
        QApplication::translate("VAbstractLayoutDialog", "Roll 62in", "Paper format"),
        QApplication::translate("VAbstractLayoutDialog", "Roll 72in", "Paper format"),
        QApplication::translate("VAbstractLayoutDialog", "Custom", "Paper format")};
    return pageFormatNames;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractLayoutDialog::GetTemplateSize(PaperSizeTemplate tmpl, Unit unit) -> QSizeF
{
    qreal width = 0;
    qreal height = 0;

    switch (tmpl)
    {
        case PaperSizeTemplate::A0:
            width = UnitConvertor(841, Unit::Mm, unit);
            height = UnitConvertor(1189, Unit::Mm, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeTemplate::A1:
            width = UnitConvertor(594, Unit::Mm, unit);
            height = UnitConvertor(841, Unit::Mm, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeTemplate::A2:
            width = UnitConvertor(420, Unit::Mm, unit);
            height = UnitConvertor(594, Unit::Mm, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeTemplate::A3:
            width = UnitConvertor(297, Unit::Mm, unit);
            height = UnitConvertor(420, Unit::Mm, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeTemplate::A4:
            width = UnitConvertor(210, Unit::Mm, unit);
            height = UnitConvertor(297, Unit::Mm, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeTemplate::Letter:
            width = UnitConvertor(8.5, Unit::Inch, unit);
            height = UnitConvertor(11, Unit::Inch, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeTemplate::Legal:
            width = UnitConvertor(8.5, Unit::Inch, unit);
            height = UnitConvertor(14, Unit::Inch, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeTemplate::Tabloid:
            width = UnitConvertor(11, Unit::Inch, unit);
            height = UnitConvertor(17, Unit::Inch, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeTemplate::Roll24in:
            width = UnitConvertor(24, Unit::Inch, unit);
            height = UnitConvertor(QIMAGE_MAX, Unit::Px, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeTemplate::Roll30in:
            width = UnitConvertor(30, Unit::Inch, unit);
            height = UnitConvertor(QIMAGE_MAX, Unit::Px, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeTemplate::Roll36in:
            width = UnitConvertor(36, Unit::Inch, unit);
            height = UnitConvertor(QIMAGE_MAX, Unit::Px, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeTemplate::Roll42in:
            width = UnitConvertor(42, Unit::Inch, unit);
            height = UnitConvertor(QIMAGE_MAX, Unit::Px, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeTemplate::Roll44in:
            width = UnitConvertor(44, Unit::Inch, unit);
            height = UnitConvertor(QIMAGE_MAX, Unit::Px, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeTemplate::Roll48in:
            width = UnitConvertor(48, Unit::Inch, unit);
            height = UnitConvertor(QIMAGE_MAX, Unit::Px, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeTemplate::Roll62in:
            width = UnitConvertor(62, Unit::Inch, unit);
            height = UnitConvertor(QIMAGE_MAX, Unit::Px, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeTemplate::Roll72in:
            width = UnitConvertor(72, Unit::Inch, unit);
            height = UnitConvertor(QIMAGE_MAX, Unit::Px, unit);
            return RoundTemplateSize(width, height, unit);
        default:
            break;
    }
    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractLayoutDialog::RoundTemplateSize(qreal width, qreal height, Unit unit) -> QSizeF
{
    qreal w = 0;
    qreal h = 0;

    switch (unit)
    {
        case Unit::Cm:
        case Unit::Mm:
        case Unit::Px:
            w = vRound(width, 2);
            h = vRound(height, 2);
            return {w, h};
        case Unit::Inch:
            w = vRound(width, 5);
            h = vRound(height, 5);
            return {w, h};
        default:
            break;
    }

    return {width, height};
}
