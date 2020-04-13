/************************************************************************
 **
 **  @file   commands.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   13 4, 2020
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
#include "puzzlecommands.h"

#include <QStringList>

const QString LONG_OPTION_EXPORT_FILE   = QStringLiteral("exportFile");
const QString SINGLE_OPTION_EXPORT_FILE = QStringLiteral("e");

const QString LONG_OPTION_TEST          = QStringLiteral("test");
const QString SINGLE_OPTION_TEST        = QStringLiteral("t");

const QString LONG_OPTION_RAW_LAYOUT    = QStringLiteral("rawLayout");
const QString SINGLE_OPTION_RAW_LAYOUT  = QStringLiteral("r");

const QString LONG_OPTION_EXP2FORMAT    = QStringLiteral("format");
const QString SINGLE_OPTION_EXP2FORMAT  = QStringLiteral("f");

const QString LONG_OPTION_BINARYDXF     = QStringLiteral("bdxf");
const QString LONG_OPTION_TEXT2PATHS    = QStringLiteral("text2paths");

const QString LONG_OPTION_CROP_LENGTH   = QStringLiteral("crop");
const QString SINGLE_OPTION_CROP_LENGTH = QStringLiteral("c");

const QString LONG_OPTION_CROP_WIDTH    = QStringLiteral("cropWidth");

const QString LONG_OPTION_TILED_PDF_PAGE_TEMPLATE = QStringLiteral("tiledPageformat");
const QString LONG_OPTION_TILED_PDF_LEFT_MARGIN   = QStringLiteral("tiledlmargin");
const QString LONG_OPTION_TILED_PDF_RIGHT_MARGIN  = QStringLiteral("tiledrmargin");
const QString LONG_OPTION_TILED_PDF_TOP_MARGIN    = QStringLiteral("tiledtmargin");
const QString LONG_OPTION_TILED_PDF_BOTTOM_MARGIN = QStringLiteral("tiledbmargin");
const QString LONG_OPTION_TILED_PDF_LANDSCAPE     = QStringLiteral("tiledLandscape");

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AllKeys return list with all command line keys (short and long forms). Used for testing on conflicts.
 * @return list with all command line keys
 */
QStringList AllKeys()
{
    return QStringList
    {
        LONG_OPTION_EXPORT_FILE,
        SINGLE_OPTION_EXPORT_FILE,
        LONG_OPTION_TEST,
        SINGLE_OPTION_TEST,
        LONG_OPTION_RAW_LAYOUT,
        SINGLE_OPTION_RAW_LAYOUT,
        LONG_OPTION_EXP2FORMAT,
        SINGLE_OPTION_EXP2FORMAT,
        LONG_OPTION_BINARYDXF,
        LONG_OPTION_TEXT2PATHS,
        LONG_OPTION_CROP_LENGTH,
        SINGLE_OPTION_CROP_LENGTH,
        LONG_OPTION_CROP_WIDTH,
        LONG_OPTION_TILED_PDF_PAGE_TEMPLATE,
        LONG_OPTION_TILED_PDF_LEFT_MARGIN,
        LONG_OPTION_TILED_PDF_RIGHT_MARGIN,
        LONG_OPTION_TILED_PDF_TOP_MARGIN,
        LONG_OPTION_TILED_PDF_BOTTOM_MARGIN,
        LONG_OPTION_TILED_PDF_LANDSCAPE
    };
}
