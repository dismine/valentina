/************************************************************************
 **
 **  @file   tst_vtextcodec.h
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

#ifndef TST_VTEXTCODEC_H
#define TST_VTEXTCODEC_H

#include <QObject>

class TST_VTextCodec : public QObject
{
    Q_OBJECT // NOLINT

public:
    explicit TST_VTextCodec(QObject *parent = nullptr);
    ~TST_VTextCodec() override = default;

private slots:
    void UnknownEncodingName();

private:
    Q_DISABLE_COPY_MOVE(TST_VTextCodec) // NOLINT
};

#endif // TST_VTEXTCODEC_H
