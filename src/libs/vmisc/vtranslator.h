/************************************************************************
 **
 **  @file   vtranslator.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   25 12, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#ifndef VTRANSLATOR_H
#define VTRANSLATOR_H

#include <QTranslator>

class VTranslator : public QTranslator
{
    Q_OBJECT // NOLINT

public:
    VTranslator(QObject *parent = nullptr);

    auto translate(const char *context, const char *sourceText, const char *disambiguation = nullptr, int n = -1) const
        -> QString override;
};

#endif // VTRANSLATOR_H
