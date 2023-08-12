#ifndef VELLIPTICALARC_P
#define VELLIPTICALARC_P

#include <QSharedData>
#include <QTransform>

#include "../vmisc/defglobal.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VEllipticalArcData final : public QSharedData
{
public:
    VEllipticalArcData() = default;
    VEllipticalArcData(qreal radius1, qreal radius2, const QString &formulaRadius1, const QString &formulaRadius2,
                       qreal rotationAngle, const QString &formulaRotationAngle);
    VEllipticalArcData(qreal radius1, qreal radius2, qreal rotationAngle);
    VEllipticalArcData(const VEllipticalArcData &arc) = default;
    ~VEllipticalArcData() = default;

    /** @brief radius1 elliptical arc major radius. */
    qreal radius1{0}; // NOLINT(misc-non-private-member-variables-in-classes)
    /** @brief radius2 elliptical arc minor radius. */
    qreal radius2{0}; // NOLINT(misc-non-private-member-variables-in-classes)
    /** @brief formulaRadius1 formula for elliptical arc major radius. */
    QString formulaRadius1{}; // NOLINT(misc-non-private-member-variables-in-classes)
    /** @brief formulaRadius2 formula for elliptical arc minor radius. */
    QString formulaRadius2{}; // NOLINT(misc-non-private-member-variables-in-classes)
    /** @brief rotationAngle in degree. */
    qreal rotationAngle{0}; // NOLINT(misc-non-private-member-variables-in-classes)
    /** @brief formulaRotationAngle formula for rotationAngle. */
    QString formulaRotationAngle{}; // NOLINT(misc-non-private-member-variables-in-classes)
    QTransform m_transform{};       // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VEllipticalArcData) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
inline VEllipticalArcData::VEllipticalArcData(qreal radius1, qreal radius2, const QString &formulaRadius1,
                                              const QString &formulaRadius2, qreal rotationAngle,
                                              const QString &formulaRotationAngle)
  : radius1(radius1),
    radius2(radius2),
    formulaRadius1(formulaRadius1),
    formulaRadius2(formulaRadius2),
    rotationAngle(rotationAngle),
    formulaRotationAngle(formulaRotationAngle)
{
}

//---------------------------------------------------------------------------------------------------------------------
inline VEllipticalArcData::VEllipticalArcData(qreal radius1, qreal radius2, qreal rotationAngle)
  : radius1(radius1),
    radius2(radius2),
    formulaRadius1(QString::number(radius1)),
    formulaRadius2(QString::number(radius2)),
    rotationAngle(rotationAngle),
    formulaRotationAngle(QString::number(rotationAngle))
{
}

QT_WARNING_POP

#endif // VELLIPTICALARC_P
