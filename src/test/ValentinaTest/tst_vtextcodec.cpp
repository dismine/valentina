/************************************************************************
 **
 **  @file   tst_vtextcodec.cpp
 **  @author Paco Arjonilla <pacoarjonilla(at)yahoo.es>
 **  @date   15 7, 2026
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project
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

#include "tst_vtextcodec.h"
#include "../vmisc/vtextcodec.h"

#include <QtTest>

namespace
{
// VTextCodec's constructor is protected. Expose it to emulate an encoding QStringConverter may add in the future.
class TestCodec : public VTextCodec
{
public:
    explicit TestCodec(QStringConverter::Encoding encoding)
      : VTextCodec(encoding)
    {
    }
};
} // namespace

//---------------------------------------------------------------------------------------------------------------------
TST_VTextCodec::TST_VTextCodec(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
// name() must tolerate an encoding that has no entry in the alias table instead of crashing.
void TST_VTextCodec::UnknownEncodingName()
{
    TestCodec const codec(static_cast<QStringConverter::Encoding>(QStringConverter::LastEncoding + 1));
    QCOMPARE(codec.name(), QString());
}
