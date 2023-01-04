/************************************************************************
 **
 **  @file   vcmdexport.cpp
 **  @author Alex Zaharov <alexzkhr@gmail.com>
 **  @date   25 8, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#include "vcmdexport.h"
#include "../dialogs/dialoglayoutsettings.h"
#include "../dialogs/dialogsavelayout.h"
#include "../ifc/xml/vdomdocument.h"
#include "../vmisc/commandoptions.h"
#include "../vmisc/vsysexits.h"
#include "../vmisc/vvalentinasettings.h"
#include "../vmisc/dialogs/dialogexporttocsv.h"
#include "../vlayout/vlayoutgenerator.h"
#include <QDebug>
#include <QTextCodec>

VCommandLinePtr VCommandLine::instance = nullptr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

#define translate(context, source) QCoreApplication::translate((context), (source)) // NOLINT(cppcoreguidelines-macro-usage)

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto Lo2Px(const QString &src, const DialogLayoutSettings &converter, bool *ok) -> qreal
{
    return converter.LayoutToPixels(src.toDouble(ok));
}

//---------------------------------------------------------------------------------------------------------------------
auto Pg2Px(const QString& src, const DialogLayoutSettings& converter, bool *ok) -> qreal
{
    return converter.PageToPixels(src.toDouble(ok));
}
} // anonymous namespace

//---------------------------------------------------------------------------------------------------------------------
VCommandLine::VCommandLine()
{
    parser.setApplicationDescription(translate("VCommandLine", "Pattern making program."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("filename"), translate("VCommandLine", "Pattern file."));

    InitCommandLineOptions();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::FormatSize(const QString &key) const -> VAbstractLayoutDialog::PaperSizeTemplate
{
    int ppsize = 0;
    if (IsOptionSet(key))
    {
        ppsize = OptionValue(key).toInt();
    }
    return static_cast<VAbstractLayoutDialog::PaperSizeTemplate>(ppsize);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::DefaultGenerator() const -> VLayoutGeneratorPtr
{
    //this functions covers all options found into layout setup dialog, nothing to add here, unless dialog extended

    VLayoutGeneratorPtr res(new VLayoutGenerator());
    DialogLayoutSettings diag(res.get(), nullptr, true);

    TestPageformat();
    TestGapWidth();
    TestMargins();

    // if present units MUST be set before any other to keep conversions correct
    if (!diag.SelectTemplate(OptPaperSize()))
    {
        qCritical() << translate("VCommandLine", "Unknown page templated selected.") << "\n";
        const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
    }

    if (IsOptionSet(LONG_OPTION_PAGEH))
    { //at this point we already sure 3 are set or none

        if (!diag.SelectPaperUnit(OptionValue(LONG_OPTION_PAGEUNITS)))
        {
            qCritical() << translate("VCommandLine", "Unsupported paper units.") << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }

        bool ok = false;
        qreal height = Pg2Px(OptionValue(LONG_OPTION_PAGEH), diag, &ok);
        if (not ok)
        {
            qCritical() << translate("VCommandLine", "Invalid page height value.") << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }
        diag.SetPaperHeight(height);

        ok = false;
        qreal width = Pg2Px(OptionValue(LONG_OPTION_PAGEW), diag, &ok);
        if (not ok)
        {
            qCritical() << translate("VCommandLine", "Invalid page width value.") << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }
        diag.SetPaperWidth(width);
    }
    else
    { // Not explicit page size
        if (IsOptionSet(LONG_OPTION_LANDSCAPE_ORIENTATION))
        {
            diag.EnableLandscapeOrientation();
        }
    }

    if (IsOptionSet(LONG_OPTION_SHIFTUNITS))
    {
        if (!diag.SelectLayoutUnit(OptionValue(LONG_OPTION_SHIFTUNITS)))
        {
            qCritical() << translate("VCommandLine", "Unsupported layout units.") << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }
    }

    if (IsOptionSet(LONG_OPTION_GAPWIDTH))
    {
        bool ok = false;
        qreal width = Lo2Px(OptionValue(LONG_OPTION_GAPWIDTH), diag, &ok);
        if (not ok)
        {
            qCritical() << translate("VCommandLine", "Invalid gap width.") << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }
        diag.SetLayoutWidth(width);
    }

    diag.SetAutoCropLength(IsOptionSet(LONG_OPTION_CROP_LENGTH));
    diag.SetAutoCropWidth(IsOptionSet(LONG_OPTION_CROP_WIDTH));
    diag.SetUnitePages(IsOptionSet(LONG_OPTION_UNITE));
    diag.SetSaveLength(IsOptionSet(LONG_OPTION_SAVELENGTH));
    diag.SetPreferOneSheetSolution(IsOptionSet(LONG_OPTION_PREFER_ONE_SHEET_SOLUTION));
    diag.SetGroup(OptGroup());

    if (IsOptionSet(LONG_OPTION_IGNORE_MARGINS))
    {
        diag.SetIgnoreAllFields(true);
    }

    diag.SetFields(ParseMargins(diag));
    diag.SetFollowGrainline(IsOptionSet(LONG_OPTION_FOLLOW_GRAINLINE));
    diag.SetManualPriority(IsOptionSet(LONG_OPTION_MANUAL_PRIORITY));
    diag.SetNestQuantity(IsOptionSet(LONG_OPTION_NEST_QUANTITY));
    diag.SetNestingTime(OptNestingTime());
    diag.SetEfficiencyCoefficient(OptEfficiencyCoefficient());

    diag.DialogAccepted(); // filling VLayoutGenerator

    return res;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::Get(const QCoreApplication& app) -> VCommandLinePtr
{
    if (instance == nullptr)
    {
        instance.reset(new VCommandLine());
    }
    instance->parser.process(app);

    //fixme: in case of additional options/modes which will need to disable GUI - add it here too
    instance->isGuiEnabled = not (instance->IsExportEnabled()
                                  || instance->IsTestModeEnabled()
                                  || instance->IsExportFMEnabled());

    return instance;
}

//---------------------------------------------------------------------------------------------------------------------
void VCommandLine::Reset()
{
    instance.reset();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::IsTestModeEnabled() const -> bool
{
    const bool r = IsOptionSet(LONG_OPTION_TEST);
    if (r && parser.positionalArguments().size() != 1)
    {
        qCritical() << translate("VCommandLine", "Test option can be used with single input file only.") << "/n";
        const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
    }
    return r;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::IsPedantic() const -> bool
{
    // Pedantic doesn't work in GUI mode
    return IsGuiEnabled() ? false : IsOptionSet(LONG_OPTION_PENDANTIC);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::IsNoScalingEnabled() const -> bool
{
    return IsOptionSet(LONG_OPTION_NO_HDPI_SCALING);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::IsExportEnabled() const -> bool
{
    const bool r = IsOptionSet(LONG_OPTION_BASENAME);
    if (r && parser.positionalArguments().size() != 1)
    {
        qCritical() << translate("VCommandLine", "Export options can be used with single input file only.") << "/n";
        const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
    }
    return r;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::IsExportFMEnabled() const -> bool
{
    const bool r = IsOptionSet(LONG_OPTION_CSVEXPORTFM);
    if (r && parser.positionalArguments().size() != 1)
    {
        qCritical() << translate("VCommandLine", "Export options can be used with single input file only.") << "/n";
        const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
    }
    return r;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::OptPaperSize() const -> VAbstractLayoutDialog::PaperSizeTemplate
{
    return FormatSize(LONG_OPTION_PAGETEMPLATE);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::OptGroup() const -> Cases
{
    int r = OptionValue(LONG_OPTION_GROUPPING).toInt();
    if ( r < 0 || r >= static_cast<int>(Cases::UnknownCase))
    {
        r = 0;
    }
    return static_cast<Cases>(r);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::OptMeasurePath() const -> QString
{
    QString measure;
    if (IsOptionSet(LONG_OPTION_MEASUREFILE)
            && (IsExportEnabled() || IsTestModeEnabled()))
            //todo: don't want yet to allow user set measure file for general loading,
            //because need to fix multiply opened windows as well
    {
        measure = OptionValue(LONG_OPTION_MEASUREFILE);
    }

    return measure;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::OptBaseName() const -> QString
{
    QString path;
    if (IsExportEnabled())
    {
        path = OptionValue(LONG_OPTION_BASENAME);
    }

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::OptDestinationPath() const -> QString
{
    QString path;
    if (IsExportEnabled())
    {
        path = OptionValue(LONG_OPTION_DESTINATION);
    }

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::OptExportType() const -> int
{
    int r = 0;
    if (IsOptionSet(LONG_OPTION_EXP2FORMAT))
    {
        r = OptionValue(LONG_OPTION_EXP2FORMAT).toInt();
    }
    return r;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::IsBinaryDXF() const -> bool
{
    return IsOptionSet(LONG_OPTION_BINARYDXF);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::IsTextAsPaths() const -> bool
{
    return IsOptionSet(LONG_OPTION_TEXT2PATHS);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::IsExportOnlyDetails() const -> bool
{
    return IsOptionSet(LONG_OPTION_EXPORTONLYDETAILS);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::IsCSVWithHeader() const -> bool
{
    return IsOptionSet(LONG_OPTION_CSVWITHHEADER);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::ExportXScale() const -> qreal
{
    qreal xs = 1;
    if (IsOptionSet(LONG_OPTION_EXPXSCALE))
    {
        xs = qBound(0.01, OptionValue(LONG_OPTION_EXPXSCALE).toDouble(), 3.0);
    }
    return xs;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::ExportYScale() const -> qreal
{
    qreal ys = 1;
    if (IsOptionSet(LONG_OPTION_EXPYSCALE))
    {
        ys = qBound(0.01, OptionValue(LONG_OPTION_EXPYSCALE).toDouble(), 3.0);
    }
    return ys;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::OptExportSuchDetails() const -> QString
{
    QString path;
    if (IsExportEnabled())
    {
        path = OptionValue(LONG_OPTION_EXPORTSUCHDETAILS);
    }

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::OptCSVCodecName() const -> QString
{
    return OptionValue(LONG_OPTION_CSVCODEC);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::OptCSVSeparator() const -> QChar
{
    const QString value = OptionValue(LONG_OPTION_CSVSEPARATOR);
    return not value.isEmpty() ? value.at(0) : QChar();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::OptExportFMTo() const -> QString
{
    return OptionValue(LONG_OPTION_CSVEXPORTFM);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::OptUserMaterials() const -> QMap<int, QString>
{
    QMap<int, QString> userMaterials;
    const QStringList values = OptionValues(LONG_OPTION_USER_MATERIAL);
    for(const auto &value : values)
    {
        const QStringList parts = value.split('@');
        if (parts.size() != 2)
        {
            qCritical() << translate("VCommandLine", "Invalid user material '%1'. Separator is missing.").arg(value)
                        << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }

        bool ok = false;
        const int number = ConstFirst<QString>(parts).toInt(&ok);

        if (not ok or number < 1 or number > userMaterialPlaceholdersQuantity)
        {
            qCritical() << translate("VCommandLine", "Invalid user material '%1'. Wrong material number.").arg(value)
                        << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }

        userMaterials.insert(number, ConstLast<QString>(parts));
    }

    return userMaterials;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::OptInputFileNames() const -> QStringList
{
    return parser.positionalArguments();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::IsGuiEnabled() const -> bool
{
    return isGuiEnabled;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::IsSetDimensionA() const -> bool
{
    return IsOptionSet(LONG_OPTION_DIMENSION_A);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::IsSetDimensionB() const -> bool
{
    return IsOptionSet(LONG_OPTION_DIMENSION_B);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::IsSetDimensionC() const -> bool
{
    return IsOptionSet(LONG_OPTION_DIMENSION_C);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::OptDimensionA() const -> int
{
    const QString value = OptionValue(LONG_OPTION_DIMENSION_A);

    bool ok = false;
    int dimensionAValue = value.toInt(&ok);
    if(ok && dimensionAValue > 0)
    {
        return dimensionAValue;
    }

    qCritical() << translate("VCommandLine", "Invalid dimension A value.") << "\n";
    const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::OptDimensionB() const -> int
{
    const QString value = OptionValue(LONG_OPTION_DIMENSION_B);

    bool ok = false;
    int dimensionBValue = value.toInt(&ok);
    if(ok && dimensionBValue > 0)
    {
        return dimensionBValue;
    }

    qCritical() << translate("VCommandLine", "Invalid dimension B value.") << "\n";
    const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::OptDimensionC() const -> int
{
    const QString value = OptionValue(LONG_OPTION_DIMENSION_C);

    bool ok = false;
    int dimensionCValue = value.toInt(&ok);
    if(ok && dimensionCValue > 0)
    {
        return dimensionCValue;
    }

    qCritical() << translate("VCommandLine", "Invalid dimension C value.") << "\n";
    const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::TiledPageMargins() const -> QMarginsF
{
    QMarginsF margins(10, 10, 10, 10); // mm

    Unit unit = Unit::Cm;

    if (IsOptionSet(LONG_OPTION_SHIFTUNITS))
    {
        const QString value = OptionValue(LONG_OPTION_SHIFTUNITS);
        const QStringList supportedUnits = QStringList() << unitMM << unitCM << unitINCH;
        if (not supportedUnits.contains(value))
        {
            qCritical() << translate("VCommandLine", "Unsupported paper units.") << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }
        unit = StrToUnits(value);
    }

    if (IsOptionSet(LONG_OPTION_LEFT_MARGIN))
    {
        bool ok = false;
        qreal margin = UnitConvertor(OptionValue(LONG_OPTION_LEFT_MARGIN).toDouble(&ok), unit, Unit::Mm);
        if (not ok)
        {
            qCritical() << translate("VCommandLine", "Invalid tiled page left margin.") << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }
        margins.setLeft(margin);
    }

    if (IsOptionSet(LONG_OPTION_RIGHT_MARGIN))
    {
        bool ok = false;
        qreal margin = UnitConvertor(OptionValue(LONG_OPTION_RIGHT_MARGIN).toDouble(&ok), unit, Unit::Mm);
        if (not ok)
        {
            qCritical() << translate("VCommandLine", "Invalid tiled page right margin.") << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }
        margins.setLeft(margin);
    }

    if (IsOptionSet(LONG_OPTION_TOP_MARGIN))
    {
        bool ok = false;
        qreal margin = UnitConvertor(OptionValue(LONG_OPTION_TOP_MARGIN).toDouble(&ok), unit, Unit::Mm);
        if (not ok)
        {
            qCritical() << translate("VCommandLine", "Invalid tiled page top margin.") << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }
        margins.setLeft(margin);
    }

    if (IsOptionSet(LONG_OPTION_BOTTOM_MARGIN))
    {
        bool ok = false;
        qreal margin = UnitConvertor(OptionValue(LONG_OPTION_BOTTOM_MARGIN).toDouble(&ok), unit, Unit::Mm);
        if (not ok)
        {
            qCritical() << translate("VCommandLine", "Invalid tiled page bottom margin.") << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }
        margins.setLeft(margin);
    }

    return margins;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::OptTiledPaperSize() const -> VAbstractLayoutDialog::PaperSizeTemplate
{
    return FormatSize(LONG_OPTION_TILED_PDF_PAGE_TEMPLATE);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::OptTiledPageOrientation() const -> PageOrientation
{
    return static_cast<PageOrientation>(not IsOptionSet(LONG_OPTION_TILED_PDF_LANDSCAPE));
}

//---------------------------------------------------------------------------------------------------------------------
void VCommandLine::InitCommandLineOptions()
{
    //keep in mind order here - that is how user will see it, so group-up for usability
    //=================================================================================================================
    parser.addOptions({
        {{SINGLE_OPTION_BASENAME, LONG_OPTION_BASENAME},
         translate("VCommandLine", "The base filename of exported layout files. Use it to enable console export mode."),
         translate("VCommandLine", "The base filename of layout files")},
        {{SINGLE_OPTION_DESTINATION, LONG_OPTION_DESTINATION},
         translate("VCommandLine", "The path to output destination folder. By default the directory at which the "
         "application was started."),
         translate("VCommandLine", "The destination folder")},
        {{SINGLE_OPTION_MEASUREFILE, LONG_OPTION_MEASUREFILE},
         translate("VCommandLine", "Path to custom measure file (export mode)."),
         translate("VCommandLine", "The measure file")},
        {{SINGLE_OPTION_NESTING_TIME, LONG_OPTION_NESTING_TIME},
         translate("VCommandLine", "<Time> in minutes given for the algorithm to find best layout. Time must be in "
         "range from 1 minute to 60 minutes. Default value 1 minute."),
         translate("VCommandLine", "Time")},
        {LONG_OPTION_EFFICIENCY_COEFFICIENT,
         translate("VCommandLine", "Set layout efficiency <coefficient>. Layout efficiency coefficient is the ratio of "
         "the area occupied by the pieces to the bounding rect of all pieces. If nesting reaches required level the "
         "process stops. If value is 0 no check will be made. Coefficient must be in range from 0 to 100. Default "
         "value 0."),
         translate("VCommandLine", "Coefficient")},
        {{SINGLE_OPTION_EXP2FORMAT, LONG_OPTION_EXP2FORMAT},
         translate("VCommandLine", "Number corresponding to output format (default = 0, export mode):") +
         DialogSaveLayout::MakeHelpFormatList(),
         translate("VCommandLine", "Format number"), QChar('0')},
        {LONG_OPTION_BINARYDXF, translate("VCommandLine", "Export dxf in binary form.")},
        {LONG_OPTION_TEXT2PATHS, translate("VCommandLine", "Export text as paths.")},
        {LONG_OPTION_EXPORTONLYDETAILS,
         translate("VCommandLine", "Export only details. Export details as they positioned in the details mode. Any "
         "layout related options will be ignored.")},
        {LONG_OPTION_EXPORTSUCHDETAILS,
         translate("VCommandLine", "Export only details that match a piece name regex."),
         translate("VCommandLine", "The name regex")},

        {LONG_OPTION_DIMENSION_A,
         translate("VCommandLine", "Set base for dimension A in the multisize measurements units (export mode)."),
         translate("VCommandLine", "The dimension A base")},

        {LONG_OPTION_DIMENSION_B,
         translate("VCommandLine", "Set base for dimension B in the multisize measurements units (export mode)."),
         translate("VCommandLine", "The dimension B base")},

        {LONG_OPTION_DIMENSION_C,
         translate("VCommandLine", "Set base for dimension C in the multisize measurements units (export mode)."),
         translate("VCommandLine", "The dimension C base")},

        {LONG_OPTION_USER_MATERIAL,
         translate("VCommandLine", "Use this option to override user material defined in pattern. The value must be in "
         "form <number>@<user matrial name>. The number should be in range from 1 to %1. For example, 1@Fabric2. The "
         "key can be used multiple times. Has no effect in GUI mode.").arg(userMaterialPlaceholdersQuantity),
         translate("VCommandLine", "User material")},
    //=================================================================================================================
        {{SINGLE_OPTION_PAGETEMPLATE, LONG_OPTION_PAGETEMPLATE},
         translate("VCommandLine", "Number corresponding to layout page template (default = 0, export mode):") +
         DialogLayoutSettings::MakeHelpTemplateList(),
         translate("VCommandLine", "Template number"), QChar('0')},
        {LONG_OPTION_LANDSCAPE_ORIENTATION,
         translate("VCommandLine", "Switch page template orientation to landscape (export mode). This option has "
         "effect only for one of predefined page templates.")},
        {{SINGLE_OPTION_PAGEW, LONG_OPTION_PAGEW},
         translate("VCommandLine", "Page width in current units like 12.0 (cannot be used with \"%1\", export mode).")
         .arg(LONG_OPTION_PAGETEMPLATE),
         translate("VCommandLine", "The page width")},
        {{SINGLE_OPTION_PAGEH, LONG_OPTION_PAGEH},
         translate("VCommandLine", "Page height in current units like 12.0 (cannot be used with \"%1\", export mode).")
         .arg(LONG_OPTION_PAGETEMPLATE),
         translate("VCommandLine", "The page height")},
        {{SINGLE_OPTION_PAGEUNITS, LONG_OPTION_PAGEUNITS},
         translate("VCommandLine", "Page measure units (export mode). Valid values: %1.")
         .arg(VDomDocument::UnitsHelpString()),
         translate("VCommandLine", "The measure unit")},
        {{SINGLE_OPTION_IGNORE_MARGINS, LONG_OPTION_IGNORE_MARGINS},
         translate("VCommandLine", "Ignore printer margins (export mode). Use if need full paper space. In case of "
         "later printing you must account for the margins themselves.")},
        {{SINGLE_OPTION_LEFT_MARGIN, LONG_OPTION_LEFT_MARGIN},
         translate("VCommandLine", "Page left margin in current units like 3.0 (export mode). If not set will be used "
         "value from default printer. Or 0 if none printers was found."),
         translate("VCommandLine", "The left margin")},
        {{SINGLE_OPTION_RIGHT_MARGIN, LONG_OPTION_RIGHT_MARGIN},
         translate("VCommandLine", "Page right margin in current units like 3.0 (export mode). If not set will be used "
         "value from default printer. Or 0 if none printers was found."),
         translate("VCommandLine", "The right margin")},
        {{SINGLE_OPTION_TOP_MARGIN, LONG_OPTION_TOP_MARGIN},
         translate("VCommandLine", "Page top margin in current units like 3.0 (export mode). If not set will be used "
         "value from default printer. Or 0 if none printers was found."),
         translate("VCommandLine", "The top margin")},
        {{SINGLE_OPTION_BOTTOM_MARGIN, LONG_OPTION_BOTTOM_MARGIN},
         translate("VCommandLine", "Page bottom margin in current units like 3.0 (export mode). If not set will be "
         "used value from default printer. Or 0 if none printers was found."),
         translate("VCommandLine", "The bottom margin")},
        {LONG_OPTION_EXPXSCALE,
         translate("VCommandLine", "Set horizontal scale factor from 0.01 to 3.0 (default = 1.0, export mode)."),
         translate("VCommandLine", "Horizontal scale")},
        {LONG_OPTION_EXPYSCALE,
         translate("VCommandLine", "Set vertical scale factor from 0.01 to 3.0 (default = 1.0, export mode)."),
         translate("VCommandLine", "Vertical scale")},
    //=================================================================================================================
        {LONG_OPTION_FOLLOW_GRAINLINE,
         translate("VCommandLine", "Order detail to follow grainline direction (export mode).")},
        {LONG_OPTION_MANUAL_PRIORITY,
         translate("VCommandLine", "Follow manual priority over priority by square (export mode).")},
        {LONG_OPTION_NEST_QUANTITY,
         translate("VCommandLine", "Nest quantity copies of each piece (export mode).")},
        {{SINGLE_OPTION_CROP_LENGTH, LONG_OPTION_CROP_LENGTH},
         translate("VCommandLine", "Auto crop unused length (export mode).")},
        {LONG_OPTION_CROP_WIDTH,
         translate("VCommandLine", "Auto crop unused width (export mode).")},
        {{SINGLE_OPTION_UNITE, LONG_OPTION_UNITE},
         translate("VCommandLine", "Unite pages if possible (export mode). Maximum value limited by QImage that "
         "supports only a maximum of 32768x32768 px images.")},
        {LONG_OPTION_PREFER_ONE_SHEET_SOLUTION,
         translate("VCommandLine", "Prefer one sheet layout solution (export mode).")},
    //=================================================================================================================
        {{SINGLE_OPTION_SAVELENGTH, LONG_OPTION_SAVELENGTH},
         translate("VCommandLine", "Save length of the sheet if set (export mode). The option tells the program to use "
         "as much as possible width of sheet. Quality of a layout can be worse when this option was used.")},
        {{SINGLE_OPTION_SHIFTUNITS, LONG_OPTION_SHIFTUNITS},
         translate("VCommandLine", "Layout units (as paper's one except px, export mode). Default units cm."),
         translate("VCommandLine", "The unit")},
        {{SINGLE_OPTION_GAPWIDTH, LONG_OPTION_GAPWIDTH},
         translate("VCommandLine", "The layout gap width x2, measured in layout units (export mode). Set distance "
         "between details and a detail and a sheet."),
         translate("VCommandLine", "The gap width")},
        {{SINGLE_OPTION_GROUPPING, LONG_OPTION_GROUPPING},
         translate("VCommandLine", "Sets layout groupping cases (export mode): %1.")
         .arg(DialogLayoutSettings::MakeGroupsHelp()),
         translate("VCommandLine", "Grouping type"), QChar('2')},
        {{SINGLE_OPTION_TEST, LONG_OPTION_TEST},
         translate("VCommandLine", "Run the program in a test mode. The program in this mode loads a single pattern "
         "file and silently quit without showing the main window. The key have priority before key '%1'.")
         .arg(LONG_OPTION_BASENAME)},
        {LONG_OPTION_PENDANTIC,
         translate("VCommandLine", "Make all parsing warnings into errors. Have effect only in console mode. Use to "
         "force Valentina to immediately terminate if a pattern contains a parsing warning.")},
        {LONG_OPTION_NO_HDPI_SCALING,
         translate("VCommandLine", "Disable high dpi scaling. Call this option if has problem with scaling (by default "
         "scaling enabled). Alternatively you can use the %1 environment variable.")
         .arg(QStringLiteral("QT_AUTO_SCREEN_SCALE_FACTOR=0"))},
    //=================================================================================================================
        {LONG_OPTION_CSVWITHHEADER,
         translate("VCommandLine", "Export to csv with header. By default disabled.")},
        {LONG_OPTION_CSVCODEC,
         translate("VCommandLine", "Specify codec that will be used to save data. List of supported codecs provided by "
         "Qt. Default value depend from system. On Windows, the codec will be based on a system locale. On Unix "
         "systems, the codec will might fall back to using the iconv library if no builtin codec for the locale can be "
         "found. Valid values for this installation:") + DialogExportToCSV::MakeHelpCodecsList(),
         translate("VCommandLine", "Codec name"), QString(QTextCodec::codecForLocale()->name())},
        {LONG_OPTION_CSVSEPARATOR,
         translate("VCommandLine", "Specify csv separator character. Default value is '%1'. Valid characters:")
         .arg(VCommonSettings::GetDefCSVSeparator()) + DialogExportToCSV::MakeHelpSeparatorList(),
         translate("VCommandLine", "Separator character"), QString(VCommonSettings::GetDefCSVSeparator())},
        {LONG_OPTION_CSVEXPORTFM,
         translate("VCommandLine", "Calling this command enable exporting final measurements. Specify path to csv file "
         "with final measurements. The path must contain path to directory and name of file. It can be absolute or "
         "relatetive. In case of relative path will be used current working directory to calc a destination path."),
         translate("VCommandLine", "Path to csv file")},
    //=================================================================================================================
        {LONG_OPTION_TILED_PDF_PAGE_TEMPLATE,
         translate("VCommandLine", "Number corresponding to tiled pdf page template (default = 0, export mode with "
         "tiled pdf format):") + DialogLayoutSettings::MakeHelpTiledPdfTemplateList(),
         translate("VCommandLine", "Template number"), QChar('0')},
        {LONG_OPTION_TILED_PDF_LEFT_MARGIN,
         translate("VCommandLine","Tiled page left margin in current units like 3.0 (export mode). If not set will be "
         "used default value 1 cm."),
         translate("VCommandLine", "The left margin")},
        {LONG_OPTION_TILED_PDF_RIGHT_MARGIN,
         translate("VCommandLine", "Tiled page right margin in current units like 3.0 (export mode). If not set will "
         "be used default value 1 cm."),
         translate("VCommandLine", "The right margin")},
        {LONG_OPTION_TILED_PDF_TOP_MARGIN,
         translate("VCommandLine", "Tiled page top margin in current units like 3.0 (export mode). If not set will be "
         "used value default value 1 cm."),
         translate("VCommandLine", "The top margin")},
        {LONG_OPTION_TILED_PDF_BOTTOM_MARGIN,
         translate("VCommandLine", "Tiled page bottom margin in current units like 3.0 (export mode). If not set will "
         "be used value default value 1 cm."),
         translate("VCommandLine", "The bottom margin")},
        {LONG_OPTION_TILED_PDF_LANDSCAPE,
         translate("VCommandLine", "Set tiled page orienatation to landscape (export mode). Default value if not set "
         "portrait.")}
    });
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::IsOptionSet(const QString &option) const -> bool
{
    return parser.isSet(option);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::OptionValue(const QString &option) const -> QString
{
    return parser.value(option);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::OptionValues(const QString &option) const -> QStringList
{
    return parser.values(option);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::OptNestingTime() const -> int
{
    int time = VValentinaSettings::GetDefNestingTime();
    if (IsOptionSet(LONG_OPTION_NESTING_TIME))
    {
        bool ok = false;
        time = OptionValue(LONG_OPTION_NESTING_TIME).toInt(&ok);

        if (not ok || time < 1 || time > 60)
        {
            qCritical() << translate("VCommandLine", "Time must be in range from 1 minute to 60 minutes.")
                        << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }
    }

    return time;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::OptEfficiencyCoefficient() const -> qreal
{
    qreal coefficient = VValentinaSettings::GetDefEfficiencyCoefficient();
    if (IsOptionSet(LONG_OPTION_EFFICIENCY_COEFFICIENT))
    {
        bool ok = false;
        coefficient = OptionValue(LONG_OPTION_EFFICIENCY_COEFFICIENT).toDouble(&ok);

        if (not ok || coefficient < 0 || coefficient > 100)
        {
            qCritical() << translate("VCommandLine", "Coefficient must be in range from 0 to 100.")
                        << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }
    }

    return coefficient;
}

//---------------------------------------------------------------------------------------------------------------------
void VCommandLine::TestPageformat() const
{
    bool x = IsOptionSet(LONG_OPTION_PAGETEMPLATE);

    bool a = IsOptionSet(LONG_OPTION_PAGEH);
    bool b = IsOptionSet(LONG_OPTION_PAGEW);
    bool c = IsOptionSet(LONG_OPTION_PAGEUNITS);

    if ((a || b) && x)
    {
        qCritical() << translate("VCommandLine", "Cannot use pageformat and page explicit size together.")
                    << "\n";
        const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
    }

    if ((a || b || c) && !(a && b && c))
    {
        qCritical() << translate("VCommandLine", "Page height, width, units must be used all 3 at once.") << "\n";
        const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VCommandLine::TestGapWidth() const
{
    bool a = IsOptionSet(LONG_OPTION_GAPWIDTH);
    bool b = IsOptionSet(LONG_OPTION_SHIFTUNITS);

    if ((a || b) && !(a && b))
    {
        qCritical() << translate("VCommandLine", "Gap width must be used together with shift units.") << "\n";
        const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VCommandLine::TestMargins() const
{
    auto CheckKey = [this](const QString &key, const QString &message)
    {
        bool a = IsOptionSet(key);
        bool b = IsOptionSet(LONG_OPTION_PAGEUNITS);

        if (a && !(a && b))
        {
            qCritical() << message << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }
    };

    if (not IsOptionSet(LONG_OPTION_IGNORE_MARGINS))
    {
        CheckKey(LONG_OPTION_LEFT_MARGIN,
                 translate("VCommandLine", "Left margin must be used together with page units."));
        CheckKey(LONG_OPTION_RIGHT_MARGIN,
                 translate("VCommandLine", "Right margin must be used together with page units."));
        CheckKey(LONG_OPTION_TOP_MARGIN,
                 translate("VCommandLine", "Top margin must be used together with page units."));
        CheckKey(LONG_OPTION_BOTTOM_MARGIN,
                 translate("VCommandLine", "Bottom margin must be used together with page units."));
    }

    if (static_cast<LayoutExportFormats>(OptExportType()) == LayoutExportFormats::PDFTiled)
    {
        CheckKey(LONG_OPTION_TILED_PDF_LEFT_MARGIN,
                 translate("VCommandLine", "Tiled left margin must be used together with page units."));
        CheckKey(LONG_OPTION_TILED_PDF_RIGHT_MARGIN,
                 translate("VCommandLine", "Tiled right margin must be used together with page units."));
        CheckKey(LONG_OPTION_TILED_PDF_TOP_MARGIN,
                 translate("VCommandLine", "Tiled top margin must be used together with page units."));
        CheckKey(LONG_OPTION_TILED_PDF_BOTTOM_MARGIN,
                 translate("VCommandLine", "Tiled bottom margin must be used together with page units."));
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommandLine::ParseMargins(const DialogLayoutSettings &diag) const -> QMarginsF
{
    QMarginsF margins = diag.GetFields();

    if (IsOptionSet(LONG_OPTION_LEFT_MARGIN))
    {
        bool ok = false;
        qreal margin = Pg2Px(OptionValue(LONG_OPTION_LEFT_MARGIN), diag, &ok);
        if (not ok)
        {
            qCritical() << translate("VCommandLine", "Invalid layout page left margin.") << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }
        margins.setLeft(margin);
    }

    if (IsOptionSet(LONG_OPTION_RIGHT_MARGIN))
    {
        bool ok = false;
        qreal margin = Pg2Px(OptionValue(LONG_OPTION_RIGHT_MARGIN), diag, &ok);
        if (not ok)
        {
            qCritical() << translate("VCommandLine", "Invalid layout page right margin.") << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }
        margins.setRight(margin);
    }

    if (IsOptionSet(LONG_OPTION_TOP_MARGIN))
    {
        bool ok = false;
        qreal margin = Pg2Px(OptionValue(LONG_OPTION_TOP_MARGIN), diag, &ok);
        if (not ok)
        {
            qCritical() << translate("VCommandLine", "Invalid layout page top margin.") << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }
        margins.setTop(margin);
    }

    if (IsOptionSet(LONG_OPTION_BOTTOM_MARGIN))
    {
        bool ok = false;
        qreal margin = Pg2Px(OptionValue(LONG_OPTION_BOTTOM_MARGIN), diag, &ok);
        if (not ok)
        {
            qCritical() << translate("VCommandLine", "Invalid layout page bottom margin.") << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }
        margins.setBottom(margin);
    }

    return margins;
}

#undef translate
