/************************************************************************
 **
 **  @file   vmeasurements.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   14 7, 2015
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

#ifndef VMEASUREMENTS_H
#define VMEASUREMENTS_H

#include <QCoreApplication>
#include <QDomElement>
#include <QString>
#include <QStringList>
#include <QtGlobal>

#include "../ifc/xml/vdomdocument.h"
#include "../vmisc/def.h"
#include "vdimensions.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../vmisc/defglobal.h"
#endif

class VContainer;
class VPatternImage;
class VMeasurement;

enum class GenderType : qint8
{
    Male,
    Female,
    Unknown
};

using VDimensions = QMap<MeasurementDimension, MeasurementDimension_p>;

class VMeasurements : public VDomDocument
{
    Q_OBJECT // NOLINT

public:
    explicit VMeasurements(VContainer *data, QObject *parent = nullptr);
    VMeasurements(Unit unit, VContainer *data, QObject *parent = nullptr);
    VMeasurements(Unit unit, const QVector<MeasurementDimension_p> &dimensions, VContainer *data,
                  QObject *parent = nullptr);
    ~VMeasurements() override = default;

    void setXMLContent(const QString &fileName) override;
    auto SaveDocument(const QString &fileName, QString &error) -> bool override;

    void AddEmpty(const QString &name, const QString &formula = QString(), bool specialUnits = false);
    void AddEmptyAfter(const QString &after, const QString &name, const QString &formula = QString(),
                       bool specialUnits = false);
    void AddSeparator(const QString &name);
    void AddSeparatorAfter(const QString &after, const QString &name);
    void Remove(const QString &name);
    void MoveTop(const QString &name);
    void MoveUp(const QString &name);
    void MoveDown(const QString &name);
    void MoveBottom(const QString &name);

    auto Units() const -> Unit;

    void StoreNames(bool store);

    void ReadMeasurements(qreal baseA, qreal baseB = 0, qreal baseC = 0) const;
    void ClearForExport();

    auto Type() const -> MeasurementsType;
    auto DimensionABase() const -> qreal;
    auto DimensionBBase() const -> qreal;
    auto DimensionCBase() const -> qreal;

    auto DimensionAStep() const -> qreal;
    auto DimensionBStep() const -> qreal;
    auto DimensionCStep() const -> qreal;

    auto Notes() const -> QString;
    void SetNotes(const QString &text);

    auto Customer() const -> QString;
    void SetCustomer(const QString &text);

    auto BirthDate() const -> QDate;
    void SetBirthDate(const QDate &date);

    auto Gender() const -> GenderType;
    void SetGender(const GenderType &gender);

    auto KnownMeasurements() const -> QUuid;
    void SetKnownMeasurements(const QUuid &system);

    auto Email() const -> QString;
    void SetEmail(const QString &text);

    auto IsReadOnly() const -> bool;
    void SetReadOnly(bool ro);

    auto IsFullCircumference() const -> bool;
    void SetFullCircumference(bool fc);

    void SetMName(const QString &name, const QString &text);
    void SetMValue(const QString &name, const QString &text);
    void SetMBaseValue(const QString &name, double value);
    void SetMShiftA(const QString &name, double value);
    void SetMShiftB(const QString &name, double value);
    void SetMShiftC(const QString &name, double value);
    void SetMSpecialUnits(const QString &name, bool special);
    void SetMCorrectionValue(const QString &name, qreal baseA, qreal baseB, qreal baseC, double value);
    void SetMDescription(const QString &name, const QString &text);
    void SetMFullName(const QString &name, const QString &text);
    void SetMDimension(const QString &name, IMD type);
    void SetMImage(const QString &name, const VPatternImage &image);

    auto MeasurementForDimension(IMD type) const -> QString;

    auto Dimensions() const -> VDimensions;

    auto GetRestrictions() const -> QMap<QString, VDimensionRestriction>;
    void SetRestrictions(const QMap<QString, VDimensionRestriction> &restrictions);
    auto Restriction(qreal base, qreal base2 = 0) const -> VDimensionRestriction;

    void SetDimensionLabels(const QMap<MeasurementDimension, DimesionLabels> &labels);
    void SetDimensionCustomNames(const QMap<MeasurementDimension, QString> &names);

    static const QString TagVST;
    static const QString TagVIT;
    static const QString TagBodyMeasurements;
    static const QString TagNotes;
    static const QString TagPersonal;
    static const QString TagCustomer;
    static const QString TagBirthDate;
    static const QString TagGender;
    static const QString TagPMSystem;
    static const QString TagEmail;
    static const QString TagReadOnly;
    static const QString TagMeasurement;
    static const QString TagDimensions;
    static const QString TagDimension;
    static const QString TagRestrictions;
    static const QString TagRestriction;
    static const QString TagCorrections;
    static const QString TagCorrection;
    static const QString TagLabels;
    static const QString TagLabel;
    static const QString TagImage;

    static const QString AttrBase;
    static const QString AttrValue;
    static const QString AttrShiftA;
    static const QString AttrShiftB;
    static const QString AttrShiftC;
    static const QString AttrCorrection;
    static const QString AttrCoordinates;
    static const QString AttrExclude;
    static const QString AttrSpecialUnits;
    static const QString AttrDescription;
    static const QString AttrName;
    static const QString AttrFullName;
    static const QString AttrMin;
    static const QString AttrMax;
    static const QString AttrStep;
    static const QString AttrMeasurement;
    static const QString AttrFullCircumference;
    static const QString AttrLabel;
    static const QString AttrDimension;
    static const QString AttrCustomName;
    static const QString AttrContentType;

    static const QString GenderMale;
    static const QString GenderFemale;
    static const QString GenderUnknown;

    static const QString DimensionN;
    static const QString DimensionX;
    static const QString DimensionY;
    static const QString DimensionW;
    static const QString DimensionZ;

    static auto GenderToStr(const GenderType &sex) -> QString;
    static auto StrToGender(const QString &sex) -> GenderType;

    static auto DimensionTypeToStr(const MeasurementDimension &type) -> QString;
    static auto StrToDimensionType(const QString &type) -> MeasurementDimension;

    static auto IMDToStr(const IMD &type) -> QString;
    static auto StrToIMD(const QString &type) -> IMD;

    static auto IMDName(IMD type) -> QString;

    auto ListAll() const -> QStringList;
    auto ListKnown() const -> QStringList;

    auto GetData() const -> VContainer *;

private:
    Q_DISABLE_COPY_MOVE(VMeasurements) // NOLINT

    /** @brief data container with data. */
    VContainer *data;
    MeasurementsType type;

    // Cache data to quick access
    Unit m_units{Unit::LAST_UNIT_DO_NOT_USE};
    VDimensions m_dimensions{};

    /** @brief m_keepNames store names in container to check uniqueness. */
    bool m_keepNames{true};

    void CreateEmptyMultisizeFile(Unit unit, const QVector<MeasurementDimension_p> &dimensions);
    void CreateEmptyIndividualFile(Unit unit);

    auto CreateDimensions(const QVector<MeasurementDimension_p> &dimensions) -> QDomElement;

    auto UniqueTagAttr(const QString &tag, const QString &attr, qreal defValue) const -> qreal;

    auto MakeEmpty(const QString &name, const QString &formula, MeasurementType varType) -> QDomElement;
    auto FindM(const QString &name) const -> QDomElement;
    auto ReadType() const -> MeasurementsType;
    auto ReadUnits() const -> Unit;
    auto ReadDimensions() const -> VDimensions;

    auto EvalFormula(VContainer *data, const QString &formula, bool *ok) const -> qreal;

    auto ReadCorrections(const QDomElement &mElement) const -> QMap<QString, qreal>;
    void WriteCorrections(QDomElement &mElement, const QMap<QString, qreal> &corrections);

    static auto ReadImage(const QDomElement &mElement) -> VPatternImage;
    void WriteImage(QDomElement &mElement, const VPatternImage &image);

    void SaveDimesionLabels(QDomElement &dElement, const DimesionLabels &labels);
    auto ReadDimensionLabels(const QDomElement &dElement) const -> DimesionLabels;

    void ClearDimension(IMD type);

    void ReadMeasurement(const QDomElement &dom, QSharedPointer<VContainer> &tempData,
                         QSharedPointer<VMeasurement> &meash, QSharedPointer<VMeasurement> &tempMeash, int i,
                         qreal baseA, qreal baseB, qreal baseC) const;
};

#endif // VMEASUREMENTS_H
