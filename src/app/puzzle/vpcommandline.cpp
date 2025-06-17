/************************************************************************
 **
 **  @file   vpcommandline.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 4, 2020
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
#include "vpcommandline.h"
#include "../vmisc/literals.h"
#include "../vmisc/vsysexits.h"
#include "vpcommands.h"
#include <QDebug>

std::shared_ptr<VPCommandLine> VPCommandLine::instance =
    nullptr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

#define translate(context, source)                                                                                     \
    QCoreApplication::translate((context), source) // NOLINT(cppcoreguidelines-macro-usage)

//---------------------------------------------------------------------------------------------------------------------
auto VPCommandLine::OptionRawLayouts() const -> QStringList
{
    return OptionValues(LONG_OPTION_RAW_LAYOUT);
}

//--------------------------------------------------------------------------------------------
auto VPCommandLine::IsGuiEnabled() const -> bool
{
    return isGuiEnabled;
}

//--------------------------------------------------------------------------------------------
auto VPCommandLine::OptionFileNames() const -> QStringList
{
    return parser.positionalArguments();
}

//-------------------------------------------------------------------------------------------
auto VPCommandLine::IsNoScalingEnabled() const -> bool
{
    return IsOptionSet(LONG_OPTION_NO_HDPI_SCALING);
}

//----------------------------------------------------------------------------------------------------------------------
void VPCommandLine::ShowHelp(int exitCode)
{
    parser.showHelp(exitCode);
}

//----------------------------------------------------------------------------------------------------------------------
VPCommandLine::VPCommandLine()
{
    parser.setApplicationDescription(translate("Puzzle", "Valentina's manual layout editor."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("filename"), translate("Puzzle", "The manual layout file."));

    InitCommandLineOptions();
}

//-------------------------------------------------------------------------------------------
auto VPCommandLine::Instance() -> VPCommandLinePtr
{
    VPCommandLine::ProcessInstance(instance, QCoreApplication::arguments());
    return instance;
}

//---------------------------------------------------------------------------------------------------------------------
void VPCommandLine::ProcessInstance(VPCommandLinePtr &instance, const QStringList &arguments)
{
    if (instance == nullptr)
    {
        instance.reset(new VPCommandLine);
    }
    instance->parser.process(arguments);
}

//-------------------------------------------------------------------------------------------
void VPCommandLine::InitCommandLineOptions()
{
    // keep in mind order here - that is how user will see it, so group-up for usability
    //=================================================================================================================
    parser.addOptions({
        {{SINGLE_OPTION_RAW_LAYOUT, LONG_OPTION_RAW_LAYOUT},
         translate("VCommandLine", "Load pattern pieces from the raw layout data file."),
         translate("VCommandLine", "The raw layout data file")},
        {LONG_OPTION_NO_HDPI_SCALING,
         translate("VCommandLine", "Disable high dpi scaling. Call this option if has problem with scaling (by default "
                                   "scaling enabled). Alternatively you can use the %1 environment variable.")
             .arg(QStringLiteral("QT_AUTO_SCREEN_SCALE_FACTOR=0"))},
    });
}

//--------------------------------------------------------------------------------------------
auto VPCommandLine::IsOptionSet(const QString &option) const -> bool
{
    return parser.isSet(option);
}

//-------------------------------------------------------------------------------------------
auto VPCommandLine::OptionValue(const QString &option) const -> QString
{
    return parser.value(option);
}

//--------------------------------------------------------------------------------------------
auto VPCommandLine::OptionValues(const QString &option) const -> QStringList
{
    return parser.values(option);
}
