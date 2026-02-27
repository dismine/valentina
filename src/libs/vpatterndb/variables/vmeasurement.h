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

#include <QMap>
#include <QSharedDataPointer>
#include <QString>
#include <QStringList>
#include <QTypeInfo>
#include <QUuid>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "vvariable.h"

class VContainer;
class VMeasurementData;
class VPatternImage;

/**
 * @brief The VMeasurement class keep data row of multisize table
 */
class VMeasurement final : public VVariable
{
public:
    VMeasurement(quint32 index, const QString &name);
    VMeasurement(quint32 index, const QString &name, qreal baseA, qreal baseB, qreal baseC, qreal base);
    VMeasurement(VContainer *data, quint32 index, const QString &name, const qreal &base, const QString &formula,
                 bool ok);
    VMeasurement(const VMeasurement &m);

    ~VMeasurement() override;

    auto operator=(const VMeasurement &m) -> VMeasurement &;

    VMeasurement(VMeasurement &&m) noexcept;
    auto operator=(VMeasurement &&m) noexcept -> VMeasurement &;

    auto GetKnownMeasurementsId() const -> QUuid;
    void SetKnownMeasurementsId(const QUuid &id);

    auto GetGuiText() const -> QString;
    void SetGuiText(const QString &guiText);

    auto GetFormula() const -> QString;

    auto IsCustom() const -> bool;

    auto Index() const -> int;
    auto IsFormulaOk() const -> bool;

    auto GetMeasurementType() const -> MeasurementType;

    auto IsNotUsed() const -> bool override;

    auto GetValue() const -> qreal override;
    auto GetValue() -> qreal * override;

    auto GetData() -> VContainer *;

    void SetBaseA(qreal base);
    void SetBaseB(qreal base);
    void SetBaseC(qreal base);

    auto GetBase() const -> qreal;
    void SetBase(qreal value);

    auto GetShiftA() const -> qreal;
    void SetShiftA(qreal value);

    auto GetShiftB() const -> qreal;
    void SetShiftB(qreal value);

    auto GetShiftC() const -> qreal;
    void SetShiftC(qreal value);

    auto GetStepA() const -> qreal;
    void SetStepA(qreal value);

    auto GetStepB() const -> qreal;
    void SetStepB(qreal value);

    auto GetStepC() const -> qreal;
    void SetStepC(qreal value);

    auto IsSpecialUnits() const -> bool;
    void SetSpecialUnits(bool special);

    auto GetDimension() const -> IMD;
    void SetDimension(IMD type);

    auto GetCorrection(qreal baseA, qreal baseB, qreal baseC) const -> qreal;

    auto GetCorrections() const -> QMap<QString, VMeasurementCorrection>;
    void SetCorrections(const QMap<QString, VMeasurementCorrection> &corrections);

    auto GetCustomImage() const -> VPatternImage;
    void SetCustomImage(const VPatternImage &image);

    auto GetImage() const -> VPatternImage;

    auto GetValueAlias(qreal baseA, qreal baseB, qreal baseC) const -> QString;

    auto GetValueAlias() const -> QString;
    void SetValueAlias(const QString &alias);

    static auto CorrectionHash(qreal baseA, qreal baseB = 0, qreal baseC = 0) -> QString;

private:
    QSharedDataPointer<VMeasurementData> d;

    auto CalcValue() const -> qreal;
    auto Correction() const -> qreal;
};

Q_DECLARE_TYPEINFO(VMeasurement, Q_MOVABLE_TYPE); // NOLINT

#endif // VMULTISIZETABLEROW_H
