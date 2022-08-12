/************************************************************************
 **
 **  @file   vpcommands.h
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
#ifndef VPCOMMANDS_H
#define VPCOMMANDS_H


#include <QString>

extern const QString LONG_OPTION_EXPORT_FILE;
extern const QString SINGLE_OPTION_EXPORT_FILE;

extern const QString LONG_OPTION_TEST;
extern const QString SINGLE_OPTION_TEST;

extern const QString LONG_OPTION_RAW_LAYOUT;
extern const QString SINGLE_OPTION_RAW_LAYOUT;

extern const QString LONG_OPTION_EXP2FORMAT;
extern const QString SINGLE_OPTION_EXP2FORMAT;

extern const QString LONG_OPTION_BINARYDXF;
extern const QString LONG_OPTION_TEXT2PATHS;

extern const QString LONG_OPTION_CROP_LENGTH;
extern const QString SINGLE_OPTION_CROP_LENGTH;

extern const QString LONG_OPTION_CROP_WIDTH;

extern const QString LONG_OPTION_TILED_PDF_PAGE_TEMPLATE;
extern const QString LONG_OPTION_TILED_PDF_LEFT_MARGIN;
extern const QString LONG_OPTION_TILED_PDF_RIGHT_MARGIN;
extern const QString LONG_OPTION_TILED_PDF_TOP_MARGIN;
extern const QString LONG_OPTION_TILED_PDF_BOTTOM_MARGIN;
extern const QString LONG_OPTION_TILED_PDF_LANDSCAPE;

auto AllKeys() -> QStringList;

#endif // VPCOMMANDS_H
