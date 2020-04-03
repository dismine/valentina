#include "vpuzzlecommandline.h"

VPuzzleCommandLine::VPuzzleCommandLine():
    parser(),
    isGuiEnabled(false)
{
    parser.setApplicationDescription(tr("Valentina's manual layout editor."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("filename", tr("The raw layout file."));
}


