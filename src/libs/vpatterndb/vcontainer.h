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
#include <QThread>
#include <QTypeInfo>
#include <QtGlobal>
#include <atomic>
#include <new>

#include <immer/map.hpp>

#include "../ifc/exception/vexceptionbadid.h"
#include "../vgeometry/vabstractcubicbezierpath.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vgobject.h"
#include "../vmisc/def.h"
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
    VContainerData(const VTranslateVars *trVars, const Unit *patternUnit, QString nspace);
    VContainerData(const VContainerData &data) = default;
    ~VContainerData();

    // Persistent (structurally shared) storage: a VContainerData copy is O(1) and later structural
    // mutations only rebuild the HAMT nodes on the path to the changed key, so an old tool snapshot
    // no longer forces a full deep copy of the whole map. See docs/superpowers/plans/
    // 2026-08-28-persistent-container.md.
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    immer::map<quint32, QSharedPointer<VGObject>> calculationObjects{};
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    QSharedPointer<QHash<quint32, QSharedPointer<VGObject>>> modelingObjects{
        QSharedPointer<QHash<quint32, QSharedPointer<VGObject>>>::create()};

    /**
     * @brief variables container for measurements, increments, lines lengths, lines angles, arcs lengths, curve lengths
     */
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    immer::map<QString, QSharedPointer<VInternalVariable>> variables{};

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

    VContainer(VContainer &&data) noexcept;
    auto operator=(VContainer &&data) noexcept -> VContainer &;

    static auto UniqueNamespace() -> QString;

    // ponytail: a tripwire, NOT a lock. Layout piece preparation reads this container from many
    // worker threads at once (MainWindowsNoGUI::PrepareDetailsForLayout -> VLayoutPiece::Create),
    // while the main thread sits in a nested event loop and could still dispatch a mutation.
    // Nothing enforces "no writes during that window", so assert it instead of assuming it.
    // Upgrade to real synchronisation only if this actually fires.
    //
    // The hazard is specifically a main-thread write racing worker-thread reads, so
    // CheckNotFrozen() below only trips for mutations happening on the main/GUI thread. A worker
    // thread mutating its own private, thread-local VContainer copy is safe by construction --
    // nothing else can see that instance yet -- and must never trip this, regardless of `frozen`.
    // (Discovered false positive: VPattern::GetCompleteData() mutates a worker-local `lastData`
    // copy via RemoveVariable() during this window and used to abort before this check existed.)
    //
    // The flag itself is private: FrozenScope is a nested class and therefore already has access
    // to VContainer's private members, which makes it the only way to raise or clear the tripwire.
    class FrozenScope
    {
    public:
        FrozenScope() { VContainer::frozen.store(true, std::memory_order_relaxed); }
        ~FrozenScope() { VContainer::frozen.store(false, std::memory_order_relaxed); }
        Q_DISABLE_COPY_MOVE(FrozenScope) // NOLINT
    };

    template <typename T> auto GeometricObject(const quint32 &id) const -> QSharedPointer<T>;
    auto GetGObject(quint32 id) const -> QSharedPointer<VGObject>;
    static auto GetFakeGObject(quint32 id) -> QSharedPointer<VGObject>;
    auto GetPiece(quint32 id) const -> VPiece;
    auto GetPiecePath(quint32 id) const -> VPiecePath;
    auto ContainsPiecePath(quint32 id) const -> bool;
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
    void AddLine(quint32 firstPointId, quint32 secondPointId, quint32 mainReference = NULL_ID);
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

    void RemoveUniqueName(const QString &name) const;

    void RemoveIncrement(const QString &name);

    void FillPiecesAreas(Unit unit);

    auto CalculationGObjects() const -> const immer::map<quint32, QSharedPointer<VGObject>> *;
    auto DataPieces() const -> QHash<quint32, VPiece> *;
    auto DataVariables() const -> const immer::map<QString, QSharedPointer<VInternalVariable>> *;

    auto DataMeasurements() const -> QMap<QString, QSharedPointer<VMeasurement>>;
    auto DataMeasurementsWithSeparators() const -> QMap<QString, QSharedPointer<VMeasurement>>;
    auto DataIncrements() const -> QMap<QString, QSharedPointer<VIncrement>>;
    auto DataIncrementsWithSeparators() const -> QMap<QString, QSharedPointer<VIncrement>>;
    auto DataLengthLines() const -> QMap<QString, QSharedPointer<VLengthLine>>;
    auto DataLengthCurves() const -> QMap<QString, QSharedPointer<VCurveLength>>;
    auto DataCurvesCLength() const -> QMap<QString, QSharedPointer<VCurveCLength>>;
    auto DataAngleLines() const -> QMap<QString, QSharedPointer<VLineAngle>>;
    auto DataRadiusesArcs() const -> QMap<QString, QSharedPointer<VArcRadius>>;
    auto DataAnglesCurves() const -> QMap<QString, QSharedPointer<VCurveAngle>>;
    auto DataPieceArea() const -> QMap<QString, QSharedPointer<VPieceArea>>;

    auto DataDependencyVariables() const -> QHash<QString, QList<quint32>>;

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

    // Tripwire state raised by FrozenScope -- see the comment on FrozenScope above for what it
    // guards and why it is an assert rather than a lock.
    static std::atomic_bool frozen;

    // Tripwire check for `frozen`, called from every mutator. A static member function rather than
    // a free function in an anonymous namespace: it is called from the templated mutators below,
    // which have external linkage and are instantiated in arbitrary translation units, so an
    // internal-linkage helper would make each instantiation call a different entity (an ODR
    // violation). Keep this compiled in for release builds too -- the interesting evidence comes
    // from real users on real patterns, not from debug runs.
    static void CheckNotFrozen(const char *where);

    QSharedDataPointer<VContainerData> d;

    void AddCurve(const QSharedPointer<VAbstractCurve> &curve, const quint32 &id, quint32 parentId = NULL_ID);

    template <typename T> void AddVariable(const QSharedPointer<T> &var, const QString &name);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    template <class T> auto qHash(const QSharedPointer<T> &p, uint seed = 0) -> uint;
#else
    template <class T> auto qHash(const QSharedPointer<T> &p, size_t seed = 0) -> size_t;
#endif

    template <typename T> void UpdateObject(const quint32 &id, const QSharedPointer<T> &point);

    template <typename T> auto DataVar(const VarType &type) const -> QMap<QString, QSharedPointer<T>>;

    static void ClearNamespace(const QString &nspace);
};

Q_DECLARE_TYPEINFO(VContainer, Q_MOVABLE_TYPE); // NOLINT

//---------------------------------------------------------------------------------------------------------------------
// Defined out of line so that it is visible both to the templated mutators below (instantiated in
// arbitrary translation units) and to the plain mutators in vcontainer.cpp, which includes this
// header. See the declaration for why this is a member function and not a free one.
inline void VContainer::CheckNotFrozen(const char *where)
{
    QCoreApplication *app = QCoreApplication::instance();
    if (VContainer::frozen.load(std::memory_order_relaxed) && app != nullptr &&
        QThread::currentThread() == app->thread())
    {
        qCritical() << "VContainer mutated while layout workers were reading it:" << where;
        Q_ASSERT(false);
    }
}

/*
 *  Defintion of templated member functions of VContainer
 */

//---------------------------------------------------------------------------------------------------------------------
template <typename T> auto VContainer::GeometricObject(const quint32 &id) const -> QSharedPointer<T>
{
    if (id == NULL_ID)
    {
        throw VExceptionBadId(tr("Can't find object"), id);
    }

    QSharedPointer<VGObject> gObj;
    if (const auto *found = d->calculationObjects.find(id))
    {
        gObj = *found;
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
    if (obj.isNull())
    {
        const QString msg = tr("Can't cast object to the requested type. Object id = %1, actual type = %2.")
                                .arg(id)
                                .arg(static_cast<int>(gObj->getType()));
        throw VExceptionBadId(msg, id);
    }
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
    if (const auto *found = d->variables.find(name))
    {
        try
        {
            QSharedPointer<VInternalVariable> const gVar = *found;
            QSharedPointer<T> value = qSharedPointerDynamicCast<T>(gVar);
            if (value.isNull())
            {
                // SCASSERT is compiled away in release builds (V_NO_ASSERT), so a failed cast would
                // silently return a null pointer that callers dereference and crash on. Throw instead
                // so the existing VExceptionBadId handling deals with the wrong-type case.
                const QString msg = QCoreApplication::translate(
                                        "VContainer", "Can't cast object to the requested type. Name = '%1', type = %2.")
                                        .arg(name)
                                        .arg(static_cast<int>(gVar->GetType()));
                throw VExceptionBadId(msg, name);
            }
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

    if (d->variables.count(var->GetName()) != 0)
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
template<typename T>
void VContainer::AddVariable(const QSharedPointer<T> &var, const QString &name)
{
    CheckNotFrozen("AddVariable");

    if (name.isEmpty())
    {
        return;
    }

    // Existing-name branch below deliberately mutates the already-stored variable *in place*
    // (`*v = *var;`) instead of replacing the map entry. Old tool snapshots share that pointee and
    // must keep observing new values -- Document::FullLiteParse depends on it. Only the structural
    // (first-time insert) branch touches the persistent map. Do not "fix" this into immutability.
    if (const auto *found = d->variables.find(name))
    {
        if ((*found)->GetType() == var->GetType())
        {
            QSharedPointer<T> v = qSharedPointerDynamicCast<T>(*found);
            if (v.isNull())
            {
                throw VExceptionBadId(tr("Can't cast object. Name = '%1', type = %2.")
                                          .arg(name)
                                          .arg(static_cast<int>((*found)->GetType())),
                                      name);
            }

            *v = *var;
        }
        else
        {
            throw VExceptionBadId(tr("Can't find object. Type mismatch. Name = '%1', existing type = %2, "
                                     "incoming type = %3.")
                                      .arg(name)
                                      .arg(static_cast<int>((*found)->GetType()))
                                      .arg(static_cast<int>(var->GetType())),
                                  name);
        }
    }
    else
    {
        d->variables = d->variables.set(name, var);
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
    CheckNotFrozen("UpdateObject");

    Q_ASSERT_X(id != NULL_ID, Q_FUNC_INFO, "id == 0"); //-V654 //-V712
    SCASSERT(point.isNull() == false)
    point->setId(id);

    // As in AddVariable(), the existing-id branches mutate the already-stored object in place
    // (`*obj = *point;`) so old tool snapshots keep seeing value updates. Only the two "not there
    // yet" branches below change the map structure.
    const auto *found = d->calculationObjects.find(id);
    if (found != nullptr && point->getMode() == Draw::Calculation)
    {
        QSharedPointer<T> obj = qSharedPointerDynamicCast<T>(*found);
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
        d->calculationObjects = d->calculationObjects.set(id, point);
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
