#include "vpuzzlecommandline.h"
#include "../vmisc/commandoptions.h"
#include "../vmisc/vsysexits.h"
#include "../vmisc/literals.h"
#include <QDebug>

std::shared_ptr<VPuzzleCommandLine> VPuzzleCommandLine::instance = nullptr;

#define translate(context, source) QCoreApplication::translate((context), source)

//------------------------------------------------------------------------------------------------
bool VPuzzleCommandLine::IsExportEnabled() const
{
    const bool result = IsOptionSet(QStringLiteral("destination"));
    int argSize = parser.positionalArguments().size();
    if (result && argSize != 1)
    {
        qCritical() << translate("Puzzle", "Export options can be used with single input file only.") << "/n";
        const_cast<VPuzzleCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
    }
    return result;
}

//----------------------------------------------------------------------------------------------
QString VPuzzleCommandLine::OptionBaseName() const
{
    QString path;
    if (IsExportEnabled())
    {
        path = OptionValue(QStringLiteral("destination"));
    }

    return path;
}

//--------------------------------------------------------------------------------------------
bool VPuzzleCommandLine::IsTestModeEnabled() const
{
    const bool r = IsOptionSet(QStringLiteral("test"));
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

    InitCommandLineOptions();
}

//-------------------------------------------------------------------------------------------
VPuzzleCommandLinePtr VPuzzleCommandLine::Instance(const QCoreApplication &app)
{
    if (instance == nullptr)
    {
        instance.reset(new VPuzzleCommandLine);
    }
    instance->parser.process(app);

    instance->isGuiEnabled = not (instance->IsGuiEnabled() || instance->IsExportEnabled());
    return instance;
}

//-------------------------------------------------------------------------------------------
void VPuzzleCommandLine::InitCommandLineOptions()
{
    QStringList args = parser.positionalArguments();
    parser.setSingleDashWordOptionMode(
                QCommandLineParser::SingleDashWordOptionMode(args.takeFirst().toInt()));
    QString source = args.isEmpty() ? QString() : args.at(0);
    QString destination = args.isEmpty() ? QString() : args.at(1);
    parser.clearPositionalArguments();
    parser.addPositionalArgument(source,
                                 translate("Puzzle", "The raw layout input file."));
    parser.addPositionalArgument(destination,
                                 translate("Puzzle", "The destination folder"));

    QCommandLineOption forceOption(QStringList() << "f" << "force",
                translate("Puzzle", "Overwrite existing files."));
    parser.addOption(forceOption);

    QCommandLineOption testOption(QStringList() << "test",
            tr("Use for unit testing. Run the program and open a file without showing the main window."));
    parser.addOption(testOption);

    QCommandLineOption scalingOption(QStringList() << LONG_OPTION_NO_HDPI_SCALING,
            tr("Disable high dpi scaling. Call this option if has problem with scaling (by default scaling enabled). "
               "Alternatively you can use the %1 environment variable.").arg("QT_AUTO_SCREEN_SCALE_FACTOR=0"));
    parser.addOption(scalingOption);

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


