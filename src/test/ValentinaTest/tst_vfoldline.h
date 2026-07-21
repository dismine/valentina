/************************************************************************
 **
 **  @file   tst_vfoldline.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 7, 2026
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

#ifndef TST_VFOLDLINE_H
#define TST_VFOLDLINE_H

#include "../vtest/abstracttest.h"

class TST_VFoldLine : public AbstractTest
{
    Q_OBJECT // NOLINT
public:
    explicit TST_VFoldLine(QObject *parent = nullptr);

private slots:
    void LabelStaysAnchoredAcrossAngles_data() const;
    void LabelStaysAnchoredAcrossAngles() const;

    void LabelOrientationMatchesNonFlipped_data() const;
    void LabelOrientationMatchesNonFlipped() const;

    void LabelStaysCloseToFoldLine_data() const;
    void LabelStaysCloseToFoldLine() const;

    void LabelPathStaysCloseToFoldLine_data() const;
    void LabelPathStaysCloseToFoldLine() const;

    void SVGLabelPathStaysCloseToFoldLine_data() const;
    void SVGLabelPathStaysCloseToFoldLine() const;

    void LabelPathOrientationMatchesNonFlipped_data() const;
    void LabelPathOrientationMatchesNonFlipped() const;
};

#endif // TST_VFOLDLINE_H
