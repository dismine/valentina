#include "vpuzzlecommandline.h"
#include "puzzlecommands.h"
#include "../vmisc/vsysexits.h"
#include "../vmisc/literals.h"
#include <QDebug>

std::shared_ptr<VPuzzleCommandLine> VPuzzleCommandLine::instance = nullptr;

#define translate(context, source) QCoreApplication::translate((context), source)

//------------------------------------------------------------------------------------------------
bool VPuzzleCommandLine::IsExportEnabled() const
{
    const bool result = IsOptionSet(LONG_OPTION_EXPORT_FILE);
    int argSize = parser.positionalArguments().size();
    if (result && argSize != 1)
    {
        qCritical() << translate("Puzzle", "Export options can be used with single input file only.") << "/n";
        const_cast<VPuzzleCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
    }
    return result;
}

//----------------------------------------------------------------------------------------------
QString VPuzzleCommandLine::OptionExportFile() const
{
    QString path;
    if (IsExportEnabled())
    {
        path = OptionValue(LONG_OPTION_EXPORT_FILE);
    }

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VPuzzleCommandLine::OptionRawLayouts() const
{
    return OptionValues(LONG_OPTION_RAW_LAYOUT);
}

//--------------------------------------------------------------------------------------------
bool VPuzzleCommandLine::IsTestModeEnabled() const
{
    const bool r = IsOptionSet(LONG_OPTION_TEST);
    if (r && parser.positionalArguments().size() != 1)
    {
        qCritical() << translate("VCommandLine", "Test option can be used with single input file only.") << "/n";
        const_cast<VPuzzleCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
    }
    return r;
}

//--------------------------------------------------------------------------------------------
bool VPuzzleCommandLine::IsGuiEnabled() const
{
    return isGuiEnabled;
}

//--------------------------------------------------------------------------------------------
QStringList VPuzzleCommandLine::OptionFileNames() const
{
    return parser.positionalArguments();
}

//-------------------------------------------------------------------------------------------
bool VPuzzleCommandLine::IsNoScalingEnabled() const
{
    return IsOptionSet(LONG_OPTION_NO_HDPI_SCALING);
}

//----------------------------------------------------------------------------------------------
VPuzzleCommandLine::VPuzzleCommandLine():
    parser(),
    isGuiEnabled(false)
{
    parser.setApplicationDescription(translate("Puzzle", "Valentina's manual layout editor."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("filename"), translate("Puzzle", "The manual layout file."));

    InitCommandLineOptions();
}

//-------------------------------------------------------------------------------------------
VPuzzleCommandLinePtr VPuzzleCommandLine::Instance(const QCoreApplication &app)
{
    VPuzzleCommandLine::ProcessInstance(instance, app.arguments());
    return instance;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleCommandLine::ProcessInstance(VPuzzleCommandLinePtr &instance, const QStringList &arguments)
{
    if (instance == nullptr)
    {
        instance.reset(new VPuzzleCommandLine);
    }
    instance->parser.process(arguments);

    instance->isGuiEnabled = not (instance->IsGuiEnabled() || instance->IsExportEnabled());
}

//-------------------------------------------------------------------------------------------
void VPuzzleCommandLine::InitCommandLineOptions()
{
    //keep in mind order here - that is how user will see it, so group-up for usability
    //=================================================================================================================
    parser.addOptions({
        {{SINGLE_OPTION_EXPORT_FILE, LONG_OPTION_EXPORT_FILE},
         translate("VCommandLine", "The filename of exported layout file. Use it to enable console export mode."),
         translate("VCommandLine", "The filename of layout file")},
        {{SINGLE_OPTION_RAW_LAYOUT, LONG_OPTION_RAW_LAYOUT},
         translate("VCommandLine", "Load pattern pieces form the raw layout data file."),
         translate("VCommandLine", "The raw layout data file")},
        {{SINGLE_OPTION_EXP2FORMAT, LONG_OPTION_EXP2FORMAT},
         translate("VCommandLine", "Number corresponding to output format (default = 0, export mode): <not defined>"),
         translate("VCommandLine", "Format number"), QChar('0')},
        {LONG_OPTION_BINARYDXF, translate("VCommandLine", "Export dxf in binary form.")},
        {LONG_OPTION_TEXT2PATHS, translate("VCommandLine", "Export text as paths.")},
    //=================================================================================================================
        {{SINGLE_OPTION_CROP_LENGTH, LONG_OPTION_CROP_LENGTH},
         translate("VCommandLine", "Auto crop unused length (export mode).")},
        {{LONG_OPTION_CROP_WIDTH},
         translate("VCommandLine", "Auto crop unused width (export mode).")},
    //=================================================================================================================
        {LONG_OPTION_TILED_PDF_PAGE_TEMPLATE,
         translate("VCommandLine", "Number corresponding to tiled pdf page template (default = 0, export mode with "
                                   "tiled pdf format): <not defined>"),
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
                                   "portrait.")},
    //=================================================================================================================
        {{SINGLE_OPTION_TEST, LONG_OPTION_TEST},
         translate("VCommandLine", "Run the program in a test mode. The program in this mode loads a single layout "
                                   "file and silently quit without showing the main window. The key have priority "
                                   "before key '%1'.").arg(LONG_OPTION_EXPORT_FILE)},
        {LONG_OPTION_NO_HDPI_SCALING,
         translate("VCommandLine", "Disable high dpi scaling. Call this option if has problem with scaling (by default "
                                   "scaling enabled). Alternatively you can use the %1 environment variable.")
             .arg(QStringLiteral("QT_AUTO_SCREEN_SCALE_FACTOR=0"))},
    });
}

//--------------------------------------------------------------------------------------------
bool VPuzzleCommandLine::IsOptionSet(const QString &option) const
{
    return parser.isSet(option);
}

//-------------------------------------------------------------------------------------------
QString VPuzzleCommandLine::OptionValue(const QString &option) const
{
    return parser.value(option);
}

//--------------------------------------------------------------------------------------------
QStringList VPuzzleCommandLine::OptionValues(const QString &option) const
{
    return parser.values(option);
}


