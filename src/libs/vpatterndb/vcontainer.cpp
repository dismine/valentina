/************************************************************************
 **
 **  @file   vcontainer.cpp
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

#include "vcontainer.h"

#include <QLoggingCategory>
#include <QUuid>
#include <QVector>
#include <QtDebug>
#include <climits>

#include "../ifc/exception/vexception.h"
#include "../vgeometry/vabstractcubicbezierpath.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vgeometrydef.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vmisc/literals.h"
#include "variables/varcradius.h"
#include "variables/vcurveangle.h"
#include "variables/vcurveclength.h"
#include "variables/vcurvelength.h"
#include "variables/vincrement.h"
#include "variables/vlineangle.h"
#include "variables/vlinelength.h"
#include "variables/vmeasurement.h"
#include "variables/vpiecearea.h"
#include "vtranslatevars.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(vCon, "v.container") // NOLINT

QT_WARNING_POP

QMap<QString, quint32> VContainer::_id = QMap<QString, quint32>();
QMap<QString, QSet<QString>> VContainer::uniqueNames = QMap<QString, QSet<QString>>();
QMap<QString, quint32> VContainer::copyCounter = QMap<QString, quint32>();

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VContainer create empty container
 */
VContainer::VContainer(const VTranslateVars *trVars, const Unit *patternUnit, const QString &nspace)
  : d(new VContainerData(trVars, patternUnit, nspace))
{
    if (nspace.isEmpty())
    {
        qFatal("Namesapce is empty.");
    }

    if (VContainer::_id.contains(nspace))
    {
        qFatal("Namespace is not unique.");
    }

    if (not _id.contains(d->nspace))
    {
        _id[d->nspace] = NULL_ID;
    }

    if (not uniqueNames.contains(d->nspace))
    {
        uniqueNames[d->nspace] = QSet<QString>();
    }

    if (not copyCounter.contains(d->nspace))
    {
        copyCounter[d->nspace] = 1;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief operator = copy constructor
 * @param data container
 * @return copy container
 */
auto VContainer::operator=(const VContainer &data) -> VContainer &
{
    if (&data == this)
    {
        return *this;
    }
    d = data.d;
    ++copyCounter[d->nspace];
    return *this;
}

#ifdef Q_COMPILER_RVALUE_REFS
//---------------------------------------------------------------------------------------------------------------------
VContainer::VContainer(VContainer &&data) noexcept
  : d(std::move(data.d))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::operator=(VContainer &&data) noexcept -> VContainer &
{
    std::swap(d, data.d);
    return *this;
}
#endif

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VContainer create container from another container
 * @param data container
 */
VContainer::VContainer(const VContainer &data)
  : d(data.d)
{
    ++copyCounter[d->nspace];
}

//---------------------------------------------------------------------------------------------------------------------
VContainer::~VContainer() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::UniqueNamespace() -> QString
{
    QString candidate;
    do
    {
        candidate = QUuid::createUuid().toString();
    } while (_id.contains(candidate));

    return candidate;
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::ClearNamespace(const QString &nspace)
{
    _id.remove(nspace);
    uniqueNames.remove(nspace);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetGObject returns a point by id
 * @param id id of point
 * @return point
 */
// cppcheck-suppress unusedFunction
auto VContainer::GetGObject(quint32 id) const -> const QSharedPointer<VGObject>
{
    if (d->calculationObjects.contains(id))
    {
        return d->calculationObjects.value(id);
    }

    if (d->modelingObjects->contains(id))
    {
        return d->modelingObjects->value(id);
    }

    throw VExceptionBadId(QCoreApplication::translate("VContainer", "Can't find object"), id);
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::GetFakeGObject(quint32 id) -> const QSharedPointer<VGObject>
{
    auto *obj = new VGObject();
    obj->setId(id);
    QSharedPointer<VGObject> pointer(obj);
    return pointer;
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::GetPiece(quint32 id) const -> VPiece
{
    if (d->pieces->contains(id))
    {
        return d->pieces->value(id);
    }

    throw VExceptionBadId(tr("Can't find object"), id);
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::GetPiecePath(quint32 id) const -> VPiecePath
{
    if (d->piecePaths->contains(id))
    {
        return d->piecePaths->value(id);
    }
    else
    {
        throw VExceptionBadId(tr("Can't find object"), id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::GetPieceForPiecePath(quint32 id) const -> quint32
{
    auto i = d->pieces->constBegin();
    while (i != d->pieces->constEnd())
    {
        if (i.value().GetInternalPaths().contains(id))
        {
            return i.key();
        }
        ++i;
    }

    return NULL_ID;
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::RegisterUniqueName(VGObject *obj) const
{
    SCASSERT(obj != nullptr)
    QSharedPointer<VGObject> pointer(obj);
    RegisterUniqueName(pointer);
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::RegisterUniqueName(const QSharedPointer<VGObject> &obj) const
{
    SCASSERT(not obj.isNull())

    uniqueNames[d->nspace].insert(obj->name());

    if (not obj->GetAlias().isEmpty())
    {
        uniqueNames[d->nspace].insert(obj->GetAlias());
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddGObject add new GObject to container
 * @param obj new object
 * @return return id of new object in container
 */
auto VContainer::AddGObject(VGObject *obj) -> quint32
{
    SCASSERT(obj != nullptr)
    QSharedPointer<VGObject> pointer(obj);
    return AddGObject(pointer);
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::AddGObject(const QSharedPointer<VGObject> &obj) -> quint32
{
    SCASSERT(not obj.isNull())

    if (obj->getMode() == Draw::Layout)
    {
        qWarning("Can't add an object with mode 'Layout'");
        return NULL_ID;
    }

    RegisterUniqueName(obj);

    const quint32 id = getNextId();
    obj->setId(id);

    if (obj->getMode() == Draw::Calculation)
    {
        d->calculationObjects.insert(id, obj);
    }
    else if (obj->getMode() == Draw::Modeling)
    {
        d->modelingObjects->insert(id, obj);
    }

    return id;
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::AddPiece(const VPiece &detail) -> quint32
{
    const quint32 id = getNextId();
    d->pieces->insert(id, detail);
    return id;
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::AddPiecePath(const VPiecePath &path) -> quint32
{
    const quint32 id = getNextId();
    d->piecePaths->insert(id, path);
    return id;
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::getId() const -> quint32
{
    return _id.value(d->nspace);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getNextId generate next unique id
 * @return next unique id
 */
auto VContainer::getNextId() const -> quint32
{
    // TODO. Current count of ids are very big and allow us save time before someone will reach its max value.
    // Better way, of cource, is to seek free ids inside the set of values and reuse them.
    // But for now better to keep it as it is now.
    if (_id.value(d->nspace) == UINT_MAX)
    {
        qCritical() << (tr("Number of free id exhausted."));
    }
    _id[d->nspace]++;
    return _id.value(d->nspace);
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::UpdateId(quint32 newId, const QString &nspace)
{
    if (_id.contains(nspace))
    {
        if (newId > _id.value(nspace))
        {
            _id[nspace] = newId;
        }
    }
    else
    {
        throw VException(QStringLiteral("Unknown namespace"));
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief UpdateId update id. If new id bigger when current save new like current.
 * @param newId id
 */
void VContainer::UpdateId(quint32 newId) const
{
    VContainer::UpdateId(newId, d->nspace);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Clear clear data in container. Id will be 0.
 */
void VContainer::Clear()
{
    qCDebug(vCon, "Clearing container data.");
    _id[d->nspace] = NULL_ID;

    d->pieces->clear();
    d->piecePaths->clear();
    ClearVariables();
    ClearGObjects();
    ClearUniqueNames();
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::ClearForFullParse()
{
    qCDebug(vCon, "Clearing container data for full parse.");
    _id[d->nspace] = NULL_ID;

    d->pieces->clear();
    d->piecePaths->clear();
    Q_STATIC_ASSERT_X(static_cast<int>(VarType::Unknown) == 12, "Check that you used all types");
    ClearVariables(QVector<VarType>{VarType::Increment, VarType::IncrementSeparator, VarType::LineAngle,
                                    VarType::LineLength, VarType::CurveLength, VarType::CurveCLength,
                                    VarType::ArcRadius, VarType::CurveAngle, VarType::PieceExternalArea,
                                    VarType::PieceSeamLineArea});
    ClearGObjects();
    ClearUniqueNames();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ClearObject points, splines, arcs, spline paths will be cleared.
 */
void VContainer::ClearGObjects()
{
    d->calculationObjects.clear();
    d->modelingObjects->clear();
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::ClearCalculationGObjects()
{
    d->calculationObjects.clear();
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::ClearVariables(const VarType &type)
{
    ClearVariables(QVector<VarType>({type}));
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::ClearVariables(const QVector<VarType> &types)
{
    if (not d->variables.isEmpty()) //-V807
    {
        if (types.isEmpty() || types.contains(VarType::Unknown))
        {
            d->variables.clear();
        }
        else
        {
            QHash<QString, QSharedPointer<VInternalVariable>>::iterator i;
            for (i = d->variables.begin(); i != d->variables.end();)
            {
                if (types.contains(i.value()->GetType()))
                {
                    i = d->variables.erase(i);
                }
                else
                {
                    ++i;
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddLine add line to container
 * @param firstPointId id of first point of line
 * @param secondPointId id of second point of line
 */
void VContainer::AddLine(const quint32 &firstPointId, const quint32 &secondPointId)
{
    const QSharedPointer<VPointF> first = GeometricObject<VPointF>(firstPointId);
    const QSharedPointer<VPointF> second = GeometricObject<VPointF>(secondPointId);

    AddVariable(new VLengthLine(first.data(), firstPointId, second.data(), secondPointId, *GetPatternUnit()));
    AddVariable(new VLineAngle(first.data(), firstPointId, second.data(), secondPointId));
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::AddArc(const QSharedPointer<VAbstractCurve> &arc, const quint32 &id, const quint32 &parentId)
{
    AddCurve(arc, id, parentId);

    if (arc->getType() == GOType::Arc)
    {
        const QSharedPointer<VArc> casted = arc.staticCast<VArc>();

        AddVariable(new VArcRadius(id, parentId, casted.data(), *GetPatternUnit()));
    }
    else if (arc->getType() == GOType::EllipticalArc)
    {
        const QSharedPointer<VEllipticalArc> casted = arc.staticCast<VEllipticalArc>();

        AddVariable(new VArcRadius(id, parentId, casted.data(), 1, *GetPatternUnit()));
        AddVariable(new VArcRadius(id, parentId, casted.data(), 2, *GetPatternUnit()));
        AddVariable(new VEllipticalArcRotation(id, parentId, casted.data()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::AddCurve(const QSharedPointer<VAbstractCurve> &curve, const quint32 &id, quint32 parentId)
{
    const GOType curveType = curve->getType();
    if (curveType != GOType::Spline && curveType != GOType::SplinePath && curveType != GOType::CubicBezier &&
        curveType != GOType::CubicBezierPath && curveType != GOType::Arc && curveType != GOType::EllipticalArc)
    {
        throw VException(tr("Can't create a curve with type '%1'").arg(static_cast<int>(curveType)));
    }

    AddVariable(new VCurveLength(id, parentId, curve.data(), *GetPatternUnit()));
    AddVariable(new VCurveAngle(id, parentId, curve.data(), CurveAngle::StartAngle));
    AddVariable(new VCurveAngle(id, parentId, curve.data(), CurveAngle::EndAngle));
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::AddSpline(const QSharedPointer<VAbstractBezier> &curve, quint32 id, quint32 parentId)
{
    AddCurve(curve, id, parentId);
    AddVariable(new VCurveCLength(id, parentId, curve.data(), CurveCLength::C1, *GetPatternUnit()));
    AddVariable(new VCurveCLength(id, parentId, curve.data(), CurveCLength::C2, *GetPatternUnit()));
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::AddCurveWithSegments(const QSharedPointer<VAbstractCubicBezierPath> &curve, const quint32 &id,
                                      quint32 parentId)
{
    AddSpline(curve, id, parentId);

    for (qint32 i = 1; i <= curve->CountSubSpl(); ++i)
    {
        const VSpline spl = curve->GetSpline(i);

        AddVariable(new VCurveLength(id, parentId, curve.data(), spl, *GetPatternUnit(), i));
        AddVariable(new VCurveAngle(id, parentId, curve.data(), spl, CurveAngle::StartAngle, i));
        AddVariable(new VCurveAngle(id, parentId, curve.data(), spl, CurveAngle::EndAngle, i));
        AddVariable(new VCurveCLength(id, parentId, curve.data(), spl, CurveCLength::C1, *GetPatternUnit(), i));
        AddVariable(new VCurveCLength(id, parentId, curve.data(), spl, CurveCLength::C2, *GetPatternUnit(), i));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::RemoveVariable(const QString &name)
{
    d->variables.remove(name);
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::RemovePiece(quint32 id)
{
    d->pieces->remove(id);
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::UpdatePiece(quint32 id, const VPiece &detail)
{
    Q_ASSERT_X(id != NULL_ID, Q_FUNC_INFO, "id == 0"); //-V654 //-V712
    d->pieces->insert(id, detail);
    UpdateId(id);
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::UpdatePiecePath(quint32 id, const VPiecePath &path)
{
    Q_ASSERT_X(id != NULL_ID, Q_FUNC_INFO, "id == 0"); //-V654 //-V712
    d->piecePaths->insert(id, path);
    UpdateId(id);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RemoveIncrement remove increment by name from increment table
 * @param name name of existing increment
 */
void VContainer::RemoveIncrement(const QString &name)
{
    d->variables[name].clear();
    d->variables.remove(name);
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::FillPiecesAreas(Unit unit)
{
    QHash<quint32, VPiece> *pieces = d->pieces.data();

    auto i = pieces->constBegin();
    while (i != pieces->constEnd())
    {
        AddVariable(QSharedPointer<VPieceArea>::create(PieceAreaType::External, i.key(), i.value(), this, unit));
        AddVariable(QSharedPointer<VPieceArea>::create(PieceAreaType::SeamLine, i.key(), i.value(), this, unit));
        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::DataMeasurements() const -> const QMap<QString, QSharedPointer<VMeasurement>>
{
    return DataVar<VMeasurement>(VarType::Measurement);
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::DataMeasurementsWithSeparators() const -> const QMap<QString, QSharedPointer<VMeasurement>>
{
    QMap<QString, QSharedPointer<VMeasurement>> measurements = DataVar<VMeasurement>(VarType::Measurement);
    QMap<QString, QSharedPointer<VMeasurement>> separators = DataVar<VMeasurement>(VarType::MeasurementSeparator);

    Insert(measurements, separators);

    return measurements;
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::DataIncrements() const -> const QMap<QString, QSharedPointer<VIncrement>>
{
    return DataVar<VIncrement>(VarType::Increment);
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::DataIncrementsWithSeparators() const -> const QMap<QString, QSharedPointer<VIncrement>>
{
    QMap<QString, QSharedPointer<VIncrement>> increments = DataVar<VIncrement>(VarType::Increment);
    QMap<QString, QSharedPointer<VIncrement>> separators = DataVar<VIncrement>(VarType::IncrementSeparator);

    Insert(increments, separators);

    return increments;
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::DataLengthLines() const -> const QMap<QString, QSharedPointer<VLengthLine>>
{
    return DataVar<VLengthLine>(VarType::LineLength);
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::DataLengthCurves() const -> const QMap<QString, QSharedPointer<VCurveLength>>
{
    return DataVar<VCurveLength>(VarType::CurveLength);
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::DataCurvesCLength() const -> const QMap<QString, QSharedPointer<VCurveCLength>>
{
    return DataVar<VCurveCLength>(VarType::CurveCLength);
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::DataAngleLines() const -> const QMap<QString, QSharedPointer<VLineAngle>>
{
    return DataVar<VLineAngle>(VarType::LineAngle);
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::DataRadiusesArcs() const -> const QMap<QString, QSharedPointer<VArcRadius>>
{
    return DataVar<VArcRadius>(VarType::ArcRadius);
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::DataAnglesCurves() const -> const QMap<QString, QSharedPointer<VCurveAngle>>
{
    return DataVar<VCurveAngle>(VarType::CurveAngle);
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::DataPieceArea() const -> const QMap<QString, QSharedPointer<VPieceArea>>
{
    QMap<QString, QSharedPointer<VPieceArea>> externalAreas = DataVar<VPieceArea>(VarType::PieceExternalArea);
    QMap<QString, QSharedPointer<VPieceArea>> seamLineAreas = DataVar<VPieceArea>(VarType::PieceSeamLineArea);

    Insert(externalAreas, seamLineAreas);

    return externalAreas;
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::IsUnique(const QString &name) const -> bool
{
    return VContainer::IsUnique(name, d->nspace);
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::IsUnique(const QString &name, const QString &nspace) -> bool
{
    if (uniqueNames.contains(nspace))
    {
        return (!uniqueNames.value(nspace).contains(name) && !builInFunctions.contains(name));
    }
    else
    {
        throw VException(QStringLiteral("Unknown namespace"));
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::AllUniqueNames() const -> QStringList
{
    return AllUniqueNames(d->nspace);
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::AllUniqueNames(const QString &nspace) -> QStringList
{
    if (uniqueNames.contains(nspace))
    {
        QStringList names = builInFunctions;
        names.append(uniqueNames.value(nspace).values());
        return names;
    }
    else
    {
        throw VException(QStringLiteral("Unknown namespace"));
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::GetPatternUnit() const -> const Unit *
{
    return d->patternUnit;
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::GetTrVars() const -> const VTranslateVars *
{
    return d->trVars;
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> auto VContainer::DataVar(const VarType &type) const -> const QMap<QString, QSharedPointer<T>>
{
    QMap<QString, QSharedPointer<T>> map;
    // Sorting QHash by id
    QHash<QString, QSharedPointer<VInternalVariable>>::const_iterator i;
    for (i = d->variables.constBegin(); i != d->variables.constEnd(); ++i)
    {
        if (i.value()->GetType() == type)
        {
            QSharedPointer<T> var = GetVariable<T>(i.key());
            map.insert(d->trVars->VarToUser(i.key()), var);
        }
    }
    return map;
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::ClearUniqueNames() const
{
    uniqueNames[d->nspace].clear();
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::ClearUniqueIncrementNames() const
{
    const QList<QString> list = uniqueNames.value(d->nspace).values();
    ClearUniqueNames();

    for (const auto &name : list)
    {
        if (not name.startsWith('#'))
        {
            uniqueNames[d->nspace].insert(name);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::ClearExceptUniqueIncrementNames() const
{
    const QList<QString> list = uniqueNames.value(d->nspace).values();
    ClearUniqueNames();

    for (const auto &name : list)
    {
        if (name.startsWith('#'))
        {
            uniqueNames[d->nspace].insert(name);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief data container with datagObjects return container of gObjects
 * @return pointer on container of gObjects
 */
auto VContainer::CalculationGObjects() const -> const QHash<quint32, QSharedPointer<VGObject>> *
{
    return &d->calculationObjects;
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::DataPieces() const -> const QHash<quint32, VPiece> *
{
    return d->pieces.data();
}

//---------------------------------------------------------------------------------------------------------------------
auto VContainer::DataVariables() const -> const QHash<QString, QSharedPointer<VInternalVariable>> *
{
    return &d->variables;
}

//---------------------------------------------------------------------------------------------------------------------
VContainerData::VContainerData(const VTranslateVars *trVars, const Unit *patternUnit, const QString &nspace)
  : trVars(trVars),
    patternUnit(patternUnit),
    nspace(nspace)
{
}

//---------------------------------------------------------------------------------------------------------------------
VContainerData::~VContainerData()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    if (ref.loadRelaxed() == 0)
#else
    if (ref.load() == 0)
#endif
    {
        --VContainer::copyCounter[nspace];
    }

    if (VContainer::copyCounter.value(nspace) == 0)
    {
        VContainer::ClearNamespace(nspace);
    }
}
