/************************************************************************
 **
 **  @file   vabstractvalapplication.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 10, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2020 Valentina project
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
#ifndef VABSTRACTVALAPPLICATION_H
#define VABSTRACTVALAPPLICATION_H

#include "vabstractapplication.h"

#include <QDate>

class VAbstractValApplication; // use in define
class VAbstractPattern;
class VMainGraphicsView;
class QGraphicsScene;
class VValentinaSettings;
class QTranslator;

class VAbstractValApplication : public VAbstractApplication
{
    Q_OBJECT // NOLINT

        public : VAbstractValApplication(int &argc, char **argv);
    virtual ~VAbstractValApplication() = default;

    auto GetCustomerName() const -> QString;
    void SetCustomerName(const QString &name);

    auto GetCustomerBirthDate() const -> QDate;
    void SetCustomerBirthDate(const QDate &date);

    auto CustomerEmail() const -> QString;
    void SetCustomerEmail(const QString &email);

    auto patternUnits() const -> Unit;
    auto patternUnitsP() const -> const Unit *;
    void SetPatternUnits(const Unit &patternUnit);

    auto GetMeasurementsType() const -> MeasurementsType;
    void SetMeasurementsType(const MeasurementsType &patternType);

    auto MeasurementsUnits() const -> Unit;
    void SetMeasurementsUnits(const Unit &measurementsUnits);

    auto DimensionSizeUnits() const -> Unit;
    void SetDimensionSizeUnits(const Unit &sizeUnits);

    auto toPixel(double val) const -> double;
    auto fromPixel(double pix) const -> double;

    void setCurrentDocument(VAbstractPattern *doc);
    auto getCurrentDocument() const -> VAbstractPattern *;

    auto getSceneView() const -> VMainGraphicsView *;
    void setSceneView(VMainGraphicsView *value);

    auto getCurrentScene() const -> QGraphicsScene *;
    void setCurrentScene(QGraphicsScene **value);

    auto getMainWindow() const -> QWidget *;
    void setMainWindow(QWidget *value);

    auto GetPatternPath() const -> QString;
    void SetPatternPath(const QString &value);

    auto GetUserMaterials() const -> QMap<int, QString>;
    void SetUserMaterials(const QMap<int, QString> &userMaterials);

    auto GetDrawMode() const -> const Draw &;
    void SetDrawMode(const Draw &value);

    auto getOpeningPattern() const -> bool;
    void setOpeningPattern();

    void PostWarningMessage(const QString &message, QtMsgType severity) const;

    auto GetDimensionHeight() const -> qreal;
    void SetDimensionHeight(qreal dimensionHeight);

    auto GetDimensionSize() const -> qreal;
    void SetDimensionSize(qreal dimensionSize);

    auto GetDimensionHip() const -> qreal;
    void SetDimensionHip(qreal dimensionHip);

    auto GetDimensionWaist() const -> qreal;
    void SetDimensionWaist(qreal dimensionWaist);

    auto GetDimensionHeightLabel() const -> QString;
    void SetDimensionHeightLabel(const QString &label);

    auto GetDimensionSizeLabel() const -> QString;
    void SetDimensionSizeLabel(const QString &label);

    auto GetDimensionHipLabel() const -> QString;
    void SetDimensionHipLabel(const QString &label);

    auto GetDimensionWaistLabel() const -> QString;
    void SetDimensionWaistLabel(const QString &label);

    virtual void OpenSettings() override;
    auto ValentinaSettings() -> VValentinaSettings *;

    static auto VApp() -> VAbstractValApplication *;

    auto GetPlaceholderTranslator() -> QSharedPointer<QTranslator>;

protected:
    QString m_customerName{};
    QDate m_customerBirthDate{};
    QString m_customerEmail{};

    Unit m_patternUnits{Unit::Cm};
    MeasurementsType m_measurementsType{MeasurementsType::Unknown};
    Unit m_measurementsUnits{Unit::Cm};
    VAbstractPattern *m_doc{nullptr};
    VMainGraphicsView *m_sceneView{nullptr};
    QGraphicsScene **m_currentScene{nullptr};
    QString m_patternFilePath{};
    QMap<int, QString> m_userMaterials{};

    qreal m_dimensionHeight{0};
    qreal m_dimensionSize{0};
    qreal m_dimensionHip{0};
    qreal m_dimensionWaist{0};

    QString m_dimensionHeightLabel{};
    QString m_dimensionSizeLabel{};
    QString m_dimensionHipLabel{};
    QString m_dimensionWaistLabel{};

    Unit m_dimensionSizeUnits{Unit::Cm};

    /**
     * @brief mainWindow pointer to main window. Usefull if need create modal dialog. Without pointer to main window
     * modality doesn't work.
     */
    QWidget *mainWindow{nullptr};

    /** @brief mode keep current draw mode. */
    Draw m_mode{Draw::Calculation};

    /**
     * @brief openingPattern true when we opening pattern. If something will be wrong in formula this help understand if
     * we can allow user use Undo option.
     */
    bool openingPattern{false};

private:
    Q_DISABLE_COPY_MOVE(VAbstractValApplication) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractValApplication::GetCustomerName() const -> QString
{
    return m_customerName;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetCustomerName(const QString &name)
{
    m_customerName = name;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractValApplication::patternUnits() const -> Unit
{
    return m_patternUnits;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractValApplication::patternUnitsP() const -> const Unit *
{
    return &m_patternUnits;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetPatternUnits(const Unit &patternUnit)
{
    m_patternUnits = patternUnit;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractValApplication::GetMeasurementsType() const -> MeasurementsType
{
    return m_measurementsType;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetMeasurementsType(const MeasurementsType &patternType)
{
    m_measurementsType = patternType;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractValApplication::MeasurementsUnits() const -> Unit
{
    return m_measurementsUnits;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetMeasurementsUnits(const Unit &measurementsUnits)
{
    m_measurementsUnits = measurementsUnits;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractValApplication::DimensionSizeUnits() const -> Unit
{
    return m_dimensionSizeUnits;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetDimensionSizeUnits(const Unit &sizeUnits)
{
    m_dimensionSizeUnits = sizeUnits;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::setCurrentDocument(VAbstractPattern *doc)
{
    m_doc = doc;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractValApplication::getCurrentDocument() const -> VAbstractPattern *
{
    return m_doc;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractValApplication::getSceneView() const -> VMainGraphicsView *
{
    return m_sceneView;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::setSceneView(VMainGraphicsView *value)
{
    m_sceneView = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractValApplication::getCurrentScene() const -> QGraphicsScene *
{
    return *m_currentScene;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::setCurrentScene(QGraphicsScene **value)
{
    m_currentScene = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractValApplication::getMainWindow() const -> QWidget *
{
    return mainWindow;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::setMainWindow(QWidget *value)
{
    mainWindow = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractValApplication::GetPatternPath() const -> QString
{
    return m_patternFilePath;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetPatternPath(const QString &value)
{
    m_patternFilePath = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractValApplication::GetUserMaterials() const -> QMap<int, QString>
{
    return m_userMaterials;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetUserMaterials(const QMap<int, QString> &userMaterials)
{
    m_userMaterials = userMaterials;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractValApplication::GetDrawMode() const -> const Draw &
{
    return m_mode;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetDrawMode(const Draw &value)
{
    m_mode = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractValApplication::getOpeningPattern() const -> bool
{
    return openingPattern;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::setOpeningPattern()
{
    openingPattern = !openingPattern;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractValApplication::GetDimensionHeight() const -> qreal
{
    return m_dimensionHeight;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetDimensionHeight(qreal dimensionHeight)
{
    m_dimensionHeight = dimensionHeight;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractValApplication::GetDimensionSize() const -> qreal
{
    return m_dimensionSize;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetDimensionSize(qreal dimensionSize)
{
    m_dimensionSize = dimensionSize;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractValApplication::GetDimensionHip() const -> qreal
{
    return m_dimensionHip;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetDimensionHip(qreal dimensionHip)
{
    m_dimensionHip = dimensionHip;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractValApplication::GetDimensionWaist() const -> qreal
{
    return m_dimensionWaist;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetDimensionWaist(qreal dimensionWaist)
{
    m_dimensionWaist = dimensionWaist;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractValApplication::GetDimensionHeightLabel() const -> QString
{
    return m_dimensionHeightLabel;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetDimensionHeightLabel(const QString &label)
{
    m_dimensionHeightLabel = label;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractValApplication::GetDimensionSizeLabel() const -> QString
{
    return m_dimensionSizeLabel;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetDimensionSizeLabel(const QString &label)
{
    m_dimensionSizeLabel = label;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractValApplication::GetDimensionHipLabel() const -> QString
{
    return m_dimensionHipLabel;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetDimensionHipLabel(const QString &label)
{
    m_dimensionHipLabel = label;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractValApplication::GetDimensionWaistLabel() const -> QString
{
    return m_dimensionWaistLabel;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetDimensionWaistLabel(const QString &label)
{
    m_dimensionWaistLabel = label;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractValApplication::GetCustomerBirthDate() const -> QDate
{
    return m_customerBirthDate;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetCustomerBirthDate(const QDate &date)
{
    m_customerBirthDate = date;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractValApplication::CustomerEmail() const -> QString
{
    return m_customerEmail;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetCustomerEmail(const QString &email)
{
    m_customerEmail = email;
}

#endif // VABSTRACTVALAPPLICATION_H
