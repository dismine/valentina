/************************************************************************
 **
 **  @file   vstandardtablecell.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
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

#ifndef VMULTISIZETABLEROW_H
#define VMULTISIZETABLEROW_H

#include <qcompilerdetection.h>
#include <QMap>
#include <QSharedDataPointer>
#include <QString>
#include <QStringList>
#include <QTypeInfo>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "../ifc/ifcdef.h"
#include "vvariable.h"

class VContainer;
class VMeasurementData;

/**
 * @brief The VMeasurement class keep data row of multisize table
 */
class VMeasurement :public VVariable
{
public:
    VMeasurement(quint32 index, const QString &name, qreal baseA, qreal baseB, qreal baseC, qreal base);
    VMeasurement(VContainer *data, quint32 index, const QString &name, const qreal &base, const QString &formula,
                 bool ok);
    VMeasurement(const VMeasurement &m);

    virtual ~VMeasurement() override;

    VMeasurement &operator=(const VMeasurement &m);
#ifdef Q_COMPILER_RVALUE_REFS
    VMeasurement(const VMeasurement &&m) Q_DECL_NOTHROW;
    VMeasurement &operator=(VMeasurement &&m) Q_DECL_NOTHROW;
#endif

    QString GetGuiText() const;
    void    SetGuiText(const QString &guiText);

    QString GetFormula() const;

    bool    IsCustom() const;

    int     Index() const;
    bool    IsFormulaOk() const;

    virtual bool IsNotUsed() const override;

    virtual qreal  GetValue() const override;
    virtual qreal* GetValue() override;

    VContainer *GetData();

    void SetBaseA(qreal base);
    void SetBaseB(qreal base);
    void SetBaseC(qreal base);

    qreal GetBase() const;
    void  SetBase(qreal value);

    qreal GetShiftA() const;
    void  SetShiftA(qreal value);

    qreal GetShiftB() const;
    void  SetShiftB(qreal value);

    qreal GetShiftC() const;
    void  SetShiftC(qreal value);

    qreal GetStepA() const;
    void  SetStepA(qreal value);

    qreal GetStepB() const;
    void  SetStepB(qreal value);

    qreal GetStepC() const;
    void  SetStepC(qreal value);

    bool IsSpecialUnits() const;
    void SetSpecialUnits(bool special);

    IMD  GetDimension() const;
    void SetDimension(IMD type);

    qreal GetCorrection(int baseA, int baseB, int baseC) const;

    QMap<QString, qreal> GetCorrections() const;
    void  SetCorrections(const QMap<QString, qreal> &corrections);

    static QStringList ListHeights(const QMap<GHeights, bool> &heights, Unit patternUnit);
    static QStringList ListSizes(const QMap<GSizes, bool> &sizes, Unit patternUnit);
    static QStringList WholeListHeights(Unit patternUnit);
    static QStringList WholeListSizes(Unit patternUnit);
    static bool IsGradationSizeValid(const QString &size);
    static bool IsGradationHeightValid(const QString &height);
    static QString CorrectionHash(qreal baseA, qreal baseB=0, qreal baseC=0);
private:
    QSharedDataPointer<VMeasurementData> d;

    qreal CalcValue() const;
    qreal Correction() const;
};

Q_DECLARE_TYPEINFO(VMeasurement, Q_MOVABLE_TYPE);

#endif // VMULTISIZETABLEROW_H
