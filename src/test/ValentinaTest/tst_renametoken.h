/************************************************************************
 **
 **  @file   tst_renametoken.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   13 5, 2026
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
#ifndef TST_RENAMETOKEN_H
#define TST_RENAMETOKEN_H

#include <QObject>

class TST_RenameToken : public QObject
{
    Q_OBJECT // NOLINT

public:
    explicit TST_RenameToken(QObject *parent = nullptr);

private slots:
    void ReplaceTokenLabel_data();
    void ReplaceTokenLabel();

    void ReplaceTokenPair_data();
    void ReplaceTokenPair();

    void ReplaceTokenAlias_data();
    void ReplaceTokenAlias();

    void ReplaceTokenSegmentCurve_data();
    void ReplaceTokenSegmentCurve();

    void ReplaceTokenArc_data();
    void ReplaceTokenArc();

private:
    Q_DISABLE_COPY_MOVE(TST_RenameToken) // NOLINT
};

#endif // TST_RENAMETOKEN_H
