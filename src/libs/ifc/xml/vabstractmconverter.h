/************************************************************************
 **
 **  @file   vabstractmconverter.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   5 9, 2015
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

#ifndef VABSTRACTMCONVERTER_H
#define VABSTRACTMCONVERTER_H

#include <QMap>
#include <QMultiMap>
#include <QString>
#include <QtGlobal>

#include "vabstractconverter.h"

class VAbstractMConverter : public VAbstractConverter
{
    Q_OBJECT // NOLINT

public:
    explicit VAbstractMConverter(const QString &fileName);
    ~VAbstractMConverter() override = default;

    auto Units() const -> Unit;

protected:
    void AddRootComment();
    static auto OldNamesToNewNames_InV0_3_0() -> QMultiMap<QString, QString>;
    static auto OldNamesToNewNames_InV0_3_3() -> QMap<QString, QString>;

private:
    Q_DISABLE_COPY_MOVE(VAbstractMConverter) // NOLINT
};

#endif // VABSTRACTMCONVERTER_H
