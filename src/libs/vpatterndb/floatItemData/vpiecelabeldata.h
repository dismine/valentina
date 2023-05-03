/************************************************************************
 **
 **  @file   vpiecelabeldata.h
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

#ifndef VPATTERNPIECEDATA_H
#define VPATTERNPIECEDATA_H

#include <QString>

#include "vpatternlabeldata.h"
#include "floatitemdef.h"

class VPieceLabelDataPrivate;
struct VLabelTemplateLine;

/**
 * @brief The VPieceLabelData class holds some information about a single
 * piece like letter, name, material type, cut number and placement.
 */
class VPieceLabelData : public VPatternLabelData
{
public:
    VPieceLabelData();
    VPieceLabelData(const VPieceLabelData &data);

    virtual ~VPieceLabelData();

    auto operator=(const VPieceLabelData &data) -> VPieceLabelData &;
#ifdef Q_COMPILER_RVALUE_REFS
    VPieceLabelData(VPieceLabelData &&data) Q_DECL_NOTHROW;
    auto operator=(VPieceLabelData &&data) Q_DECL_NOTHROW->VPieceLabelData &;
#endif

    void Clear();

    // methods, which operate on other members
    auto GetLetter() const -> QString;
    void    SetLetter(const QString &qsLetter);

    auto GetAnnotation() const -> QString;
    void    SetAnnotation(const QString &val);

    auto GetOrientation() const -> QString;
    void    SetOrientation(const QString &val);

    auto GetRotationWay() const -> QString;
    void    SetRotationWay(const QString &val);

    auto GetTilt() const -> QString;
    void    SetTilt(const QString &val);

    auto GetFoldPosition() const -> QString;
    void    SetFoldPosition(const QString &val);

    auto GetQuantity() const -> quint16;
    void    SetQuantity(quint16 val);

    auto IsOnFold() const -> bool;
    void SetOnFold(bool onFold);

    auto GetAreaShartName() const -> QString;
    void    SetAreaShartName(const QString &val);

    auto GetLabelTemplate() const -> QVector<VLabelTemplateLine>;
    void                        SetLabelTemplate(const QVector<VLabelTemplateLine> &lines);

private:
    QSharedDataPointer<VPieceLabelDataPrivate> d;
};

#endif // VPATTERNPIECEDATA_H
