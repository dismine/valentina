/************************************************************************
 **
 **  @file   vpatternlabeldata.h
 **  @author Bojan Kverh
 **  @date   June 16, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#ifndef VPATTERNINFOGEOMETRY_H
#define VPATTERNINFOGEOMETRY_H

#include <QPointF>

#include "vabstractfloatitemdata.h"

class VPatternLabelDataPrivate;

/**
 * @brief The VPatternLabelData class holds the information about pattern info label geometry
 */
class VPatternLabelData : public VAbstractFloatItemData
{
public:
    VPatternLabelData();
    VPatternLabelData(const VPatternLabelData &data);

    virtual ~VPatternLabelData();

    auto operator=(const VPatternLabelData &data) -> VPatternLabelData &;
#ifdef Q_COMPILER_RVALUE_REFS
    VPatternLabelData(VPatternLabelData &&data) noexcept;
    auto operator=(VPatternLabelData &&data) noexcept->VPatternLabelData &;
#endif

    // methods, which set up label parameters
    auto GetLabelWidth() const -> QString;
    void    SetLabelWidth(const QString &dLabelW);

    auto GetLabelHeight() const -> QString;
    void    SetLabelHeight(const QString &dLabelH);

    auto GetFontSize() const -> int;
    void  SetFontSize(int iSize);

    auto GetRotation() const -> QString;
    void    SetRotation(const QString &dRot);

    auto CenterPin() const -> quint32;
    void    SetCenterPin(const quint32 &centerPin);

    auto TopLeftPin() const -> quint32;
    void    SetTopLeftPin(const quint32 &topLeftPin);

    auto BottomRightPin() const -> quint32;
    void    SetBottomRightPin(const quint32 &bottomRightPin);

private:
    QSharedDataPointer<VPatternLabelDataPrivate> d;
};

#endif // VPATTERNINFOGEOMETRY_H
