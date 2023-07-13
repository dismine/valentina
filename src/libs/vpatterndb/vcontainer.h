/************************************************************************
 **
 **  @file   vcontainer.h
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

#ifndef VCONTAINER_H
#define VCONTAINER_H

#include <QCoreApplication>
#include <QHash>
#include <QMap>
#include <QMessageLogger>
#include <QSet>
#include <QSharedData>
#include <QSharedDataPointer>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QTypeInfo>
#include <QtGlobal>
#include <new>

#include "../ifc/exception/vexceptionbadid.h"
#include "../vgeometry/vabstractcubicbezierpath.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vgobject.h"
#include "../vmisc/def.h"
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../vmisc/diagnostic.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "variables/vinternalvariable.h"
#include "vpiece.h"
#include "vpiecepath.h"
#include "vtranslatevars.h"

class VEllipticalArc;
class VMeasurement;
class VIncrement;
class VLengthLine;
class VCurveLength;
class VCurveCLength;
class VLineAngle;
class VArcRadius;
class VCurveAngle;
class VPieceArea;

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_INTEL(2021)
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VContainerData final : public QSharedData //-V690
{
public:
    VContainerData(const VTranslateVars *trVars, const Unit *patternUnit, const QString &nspace);
    VContainerData(const VContainerData &data) = default;
    virtual ~VContainerData();

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    QHash<quint32, QSharedPointer<VGObject>> calculationObjects{};
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    QSharedPointer<QHash<quint32, QSharedPointer<VGObject>>> modelingObjects{
        QSharedPointer<QHash<quint32, QSharedPointer<VGObject>>>::create()};

    /**
     * @brief variables container for measurements, increments, lines lengths, lines angles, arcs lengths, curve lengths
     */
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    QHash<QString, QSharedPointer<VInternalVariable>> variables{};

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    QSharedPointer<QHash<quint32, VPiece>> pieces{QSharedPointer<QHash<quint32, VPiece>>::create()};
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    QSharedPointer<QHash<quint32, VPiecePath>> piecePaths{QSharedPointer<QHash<quint32, VPiecePath>>::create()};

    const VTranslateVars *trVars; // NOLINT(misc-non-private-member-variables-in-classes)
    const Unit *patternUnit;      // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief nspace namespace for static variables */
    QString nspace; // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VContainerData) // NOLINT
};

QT_WARNING_POP

/**
 * @brief The VContainer class container of all variables.
 */
class VContainer
{
    Q_DECLARE_TR_FUNCTIONS(VContainer) // NOLINT

public:
    VContainer(const VTranslateVars *trVars, const Unit *patternUnit, const QString &nspace);
    VContainer(const VContainer &data);
    ~VContainer();

    friend class VContainerData;

    auto operator=(const VContainer &data) -> VContainer &;
#ifdef Q_COMPILER_RVALUE_REFS
    VContainer(VContainer &&data) noexcept;
    auto operator=(VContainer &&data) noexcept -> VContainer &;
#endif

    static auto UniqueNamespace() -> QString;

    template <typename T> auto GeometricObject(const quint32 &id) const -> const QSharedPointer<T>;
    auto GetGObject(quint32 id) const -> const QSharedPointer<VGObject>;
    static auto GetFakeGObject(quint32 id) -> const QSharedPointer<VGObject>;
    auto GetPiece(quint32 id) const -> VPiece;
    auto GetPiecePath(quint32 id) const -> VPiecePath;
    auto GetPieceForPiecePath(quint32 id) const -> quint32;
    template <typename T> auto GetVariable(const QString &name) const -> QSharedPointer<T>;
    auto getId() const -> quint32;
    auto getNextId() const -> quint32;
    void UpdateId(quint32 newId) const;
    static void UpdateId(quint32 newId, const QString &nspace);

    void RegisterUniqueName(VGObject *obj) const;
    void RegisterUniqueName(const QSharedPointer<VGObject> &obj) const;

    auto AddGObject(VGObject *obj) -> quint32;
    auto AddGObject(const QSharedPointer<VGObject> &obj) -> quint32;
    auto AddPiece(const VPiece &detail) -> quint32;
    auto AddPiecePath(const VPiecePath &path) -> quint32;
    void AddLine(const quint32 &firstPointId, const quint32 &secondPointId);
    void AddArc(const QSharedPointer<VAbstractCurve> &arc, const quint32 &id, const quint32 &parentId = NULL_ID);
    void AddSpline(const QSharedPointer<VAbstractBezier> &curve, quint32 id, quint32 parentId = NULL_ID);
    void AddCurveWithSegments(const QSharedPointer<VAbstractCubicBezierPath> &curve, const quint32 &id,
                              quint32 parentId = NULL_ID);

    template <typename T> void AddUniqueVariable(T *var);
    template <typename T> void AddUniqueVariable(const QSharedPointer<T> &var);
    template <typename T> void AddVariable(T *var);
    template <typename T> void AddVariable(const QSharedPointer<T> &var);
    void RemoveVariable(const QString &name);
    void RemovePiece(quint32 id);

    template <class T> void UpdateGObject(quint32 id, T *obj);
    template <class T> void UpdateGObject(quint32 id, const QSharedPointer<T> &obj);
    void UpdatePiece(quint32 id, const VPiece &detail);
    void UpdatePiecePath(quint32 id, const VPiecePath &path);

    void Clear();
    void ClearForFullParse();
    void ClearGObjects();
    void ClearCalculationGObjects();
    void ClearVariables(const VarType &type = VarType::Unknown);
    void ClearVariables(const QVector<VarType> &types);
    void ClearUniqueNames() const;
    void ClearUniqueIncrementNames() const;
    void ClearExceptUniqueIncrementNames() const;

    void RemoveIncrement(const QString &name);

    void FillPiecesAreas(Unit unit);

    auto CalculationGObjects() const -> const QHash<quint32, QSharedPointer<VGObject>> *;
    auto DataPieces() const -> const QHash<quint32, VPiece> *;
    auto DataVariables() const -> const QHash<QString, QSharedPointer<VInternalVariable>> *;

    auto DataMeasurements() const -> const QMap<QString, QSharedPointer<VMeasurement>>;
    auto DataMeasurementsWithSeparators() const -> const QMap<QString, QSharedPointer<VMeasurement>>;
    auto DataIncrements() const -> const QMap<QString, QSharedPointer<VIncrement>>;
    auto DataIncrementsWithSeparators() const -> const QMap<QString, QSharedPointer<VIncrement>>;
    auto DataLengthLines() const -> const QMap<QString, QSharedPointer<VLengthLine>>;
    auto DataLengthCurves() const -> const QMap<QString, QSharedPointer<VCurveLength>>;
    auto DataCurvesCLength() const -> const QMap<QString, QSharedPointer<VCurveCLength>>;
    auto DataAngleLines() const -> const QMap<QString, QSharedPointer<VLineAngle>>;
    auto DataRadiusesArcs() const -> const QMap<QString, QSharedPointer<VArcRadius>>;
    auto DataAnglesCurves() const -> const QMap<QString, QSharedPointer<VCurveAngle>>;
    auto DataPieceArea() const -> const QMap<QString, QSharedPointer<VPieceArea>>;

    auto IsUnique(const QString &name) const -> bool;
    static auto IsUnique(const QString &name, const QString &nspace) -> bool;

    auto AllUniqueNames() const -> QStringList;
    static auto AllUniqueNames(const QString &nspace) -> QStringList;

    auto GetPatternUnit() const -> const Unit *;
    auto GetTrVars() const -> const VTranslateVars *;

private:
    /**
     * @brief _id current id. New object will have value +1. For empty class equal 0.
     */
    static QMap<QString, quint32> _id;
    static QMap<QString, QSet<QString>> uniqueNames;
    static QMap<QString, quint32> copyCounter;

    QSharedDataPointer<VContainerData> d;

    void AddCurve(const QSharedPointer<VAbstractCurve> &curve, const quint32 &id, quint32 parentId = NULL_ID);

    template <typename T> void AddVariable(const QSharedPointer<T> &var, const QString &name);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    template <class T> auto qHash(const QSharedPointer<T> &p, uint seed = 0) -> uint;
#else
    template <class T> auto qHash(const QSharedPointer<T> &p, size_t seed = 0) -> size_t;
#endif

    template <typename T> void UpdateObject(const quint32 &id, const QSharedPointer<T> &point);

    template <typename T> auto DataVar(const VarType &type) const -> const QMap<QString, QSharedPointer<T>>;

    static void ClearNamespace(const QString &nspace);
};

Q_DECLARE_TYPEINFO(VContainer, Q_MOVABLE_TYPE); // NOLINT

/*
 *  Defintion of templated member functions of VContainer
 */

//---------------------------------------------------------------------------------------------------------------------
template <typename T> auto VContainer::GeometricObject(const quint32 &id) const -> const QSharedPointer<T>
{
    if (id == NULL_ID)
    {
        throw VExceptionBadId(tr("Can't find object"), id);
    }

    QSharedPointer<VGObject> gObj;
    if (d->calculationObjects.contains(id))
    {
        gObj = d->calculationObjects.value(id);
    }
    else if (d->modelingObjects->contains(id))
    {
        gObj = d->modelingObjects->value(id);
    }
    else
    {
        throw VExceptionBadId(tr("Can't find object"), id);
    }

    QSharedPointer<T> obj = qSharedPointerDynamicCast<T>(gObj);
    SCASSERT(obj.isNull() == false)
    return obj;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetVariable return varible by name
 * @param name variable's name
 * @return variable
 */
template <typename T> auto VContainer::GetVariable(const QString &name) const -> QSharedPointer<T>
{
    SCASSERT(name.isEmpty() == false)
    if (d->variables.contains(name))
    {
        try
        {
            QSharedPointer<T> value = qSharedPointerDynamicCast<T>(d->variables.value(name));
            SCASSERT(value.isNull() == false)
            return value;
        }
        catch (const std::bad_alloc &)
        {
            throw VExceptionBadId(QCoreApplication::translate("VContainer", "Can't cast object"), name);
        }
    }
    else
    {
        throw VExceptionBadId(QCoreApplication::translate("VContainer", "Can't find object"), name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> void VContainer::AddUniqueVariable(T *var)
{
    AddUniqueVariable(QSharedPointer<T>(var));
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> void VContainer::AddUniqueVariable(const QSharedPointer<T> &var)
{
    AddVariable(var);

    if (d->variables.contains(var->GetName()))
    {
        uniqueNames[d->nspace].insert(var->GetName());
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> void VContainer::AddVariable(T *var)
{
    AddVariable(QSharedPointer<T>(var));
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> void VContainer::AddVariable(const QSharedPointer<T> &var)
{
    SCASSERT(not var->GetName().isEmpty())
    AddVariable(var, var->GetName());

    if (not var->GetAlias().isEmpty())
    {
        AddVariable(var, var->GetAlias());
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> void VContainer::AddVariable(const QSharedPointer<T> &var, const QString &name)
{
    if (name.isEmpty())
    {
        return;
    }

    if (d->variables.contains(name))
    {
        if (d->variables.value(name)->GetType() == var->GetType())
        {
            QSharedPointer<T> v = qSharedPointerDynamicCast<T>(d->variables.value(name));
            if (v.isNull())
            {
                throw VExceptionBadId(tr("Can't cast object."), name);
            }
            *v = *var;
        }
        else
        {
            throw VExceptionBadId(tr("Can't find object. Type mismatch."), name);
        }
    }
    else
    {
        d->variables.insert(name, var);
    }
}

//---------------------------------------------------------------------------------------------------------------------
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
template <class T> auto VContainer::qHash(const QSharedPointer<T> &p, uint seed) -> uint
#else
template <class T> auto VContainer::qHash(const QSharedPointer<T> &p, size_t seed) -> size_t
#endif
{
    return qHash(p.data(), seed);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief UpdateGObject update GObject by id
 * @param id id of existing GObject
 * @param obj object
 */
template <class T> void VContainer::UpdateGObject(quint32 id, T *obj)
{
    SCASSERT(obj != nullptr)
    UpdateGObject(id, QSharedPointer<T>(obj));
}

//---------------------------------------------------------------------------------------------------------------------
template <class T> void VContainer::UpdateGObject(quint32 id, const QSharedPointer<T> &obj)
{
    SCASSERT(not obj.isNull())
    UpdateObject(id, obj);
    RegisterUniqueName(obj);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief UpdateObject update object in container
 * @param id id of existing object
 * @param point object
 */
template <typename T> void VContainer::UpdateObject(const quint32 &id, const QSharedPointer<T> &point)
{
    Q_ASSERT_X(id != NULL_ID, Q_FUNC_INFO, "id == 0"); //-V654 //-V712
    SCASSERT(point.isNull() == false)
    point->setId(id);

    if (d->calculationObjects.contains(id) && point->getMode() == Draw::Calculation)
    {
        QSharedPointer<T> obj = qSharedPointerDynamicCast<T>(d->calculationObjects.value(id));
        if (obj.isNull())
        {
            throw VExceptionBadId(tr("Can't cast object"), id);
        }
        *obj = *point;
    }
    else if (d->modelingObjects->contains(id) && point->getMode() == Draw::Modeling)
    {
        QSharedPointer<T> obj = qSharedPointerDynamicCast<T>(d->modelingObjects->value(id));
        if (obj.isNull())
        {
            throw VExceptionBadId(tr("Can't cast object"), id);
        }
        *obj = *point;
    }
    else if (point->getMode() == Draw::Calculation)
    {
        d->calculationObjects.insert(id, point);
    }
    else if (point->getMode() == Draw::Modeling)
    {
        d->modelingObjects->insert(id, point);
    }
    else
    {
        qWarning("Can't update an object with mode 'Layout'");
        return;
    }

    UpdateId(id);
}
#endif // VCONTAINER_H
