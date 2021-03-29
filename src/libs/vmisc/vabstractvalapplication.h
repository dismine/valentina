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

class VAbstractValApplication;// use in define
class VAbstractPattern;
class VMainGraphicsView;
class QGraphicsScene;
class VSettings;

class VAbstractValApplication : public VAbstractApplication
{
    Q_OBJECT
public:
    VAbstractValApplication(int &argc, char **argv);
    virtual ~VAbstractValApplication() =default;

    QString GetCustomerName() const;
    void    SetCustomerName(const QString &name);

    QDate   GetCustomerBirthDate() const;
    void    SetCustomerBirthDate(const QDate &date);

    QString CustomerEmail() const;
    void    SetCustomerEmail(const QString &email);

    Unit        patternUnits() const;
    const Unit *patternUnitsP() const;
    void        SetPatternUnits(const Unit &patternUnit);

    MeasurementsType GetMeasurementsType() const;
    void             SetMeasurementsType(const MeasurementsType &patternType);

    Unit MeasurementsUnits() const;
    void SetMeasurementsUnits(const Unit &measurementsUnits);

    Unit DimensionSizeUnits() const;
    void SetDimensionSizeUnits(const Unit &sizeUnits);

    double toPixel(double val) const;
    double fromPixel(double pix) const;

    void              setCurrentDocument(VAbstractPattern *doc);
    VAbstractPattern *getCurrentDocument() const;

    VMainGraphicsView *getSceneView() const;
    void               setSceneView(VMainGraphicsView *value);

    QGraphicsScene *getCurrentScene() const;
    void            setCurrentScene(QGraphicsScene **value);

    QWidget *getMainWindow() const;
    void     setMainWindow(QWidget *value);

    QString GetPatternPath() const;
    void    SetPatternPath(const QString &value);

    QMap<int, QString> GetUserMaterials() const;
    void               SetUserMaterials(const QMap<int, QString> &userMaterials);

    const Draw &GetDrawMode() const;
    void        SetDrawMode(const Draw &value);

    bool getOpeningPattern() const;
    void setOpeningPattern();

    void PostWarningMessage(const QString &message, QtMsgType severity) const;

    qreal GetDimensionHeight() const;
    void  SetDimensionHeight(qreal dimensionHeight);

    qreal GetDimensionSize() const;
    void  SetDimensionSize(qreal dimensionSize);

    qreal GetDimensionHip() const;
    void  SetDimensionHip(qreal dimensionHip);

    qreal GetDimensionWaist() const;
    void  SetDimensionWaist(qreal dimensionWaist);

    QString GetDimensionHeightLabel() const;
    void    SetDimensionHeightLabel(const QString &label);

    QString GetDimensionSizeLabel() const;
    void    SetDimensionSizeLabel(const QString &label);

    QString GetDimensionHipLabel() const;
    void    SetDimensionHipLabel(const QString &label);

    QString GetDimensionWaistLabel() const;
    void    SetDimensionWaistLabel(const QString &label);

    virtual void OpenSettings() override;
    VSettings *ValentinaSettings();

    static VAbstractValApplication *VApp();

protected:
    QString m_customerName{};
    QDate   m_customerBirthDate{};
    QString m_customerEmail{};

    Unit                m_patternUnits{Unit::Cm};
    MeasurementsType    m_measurementsType{MeasurementsType::Unknown};
    Unit                m_measurementsUnits{Unit::Cm};
    VAbstractPattern   *m_doc{nullptr};
    VMainGraphicsView  *m_sceneView{nullptr};
    QGraphicsScene    **m_currentScene{nullptr};
    QString             m_patternFilePath{};
    QMap<int, QString>  m_userMaterials{};

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
    Q_DISABLE_COPY(VAbstractValApplication)
};

//---------------------------------------------------------------------------------------------------------------------
inline QString VAbstractValApplication::GetCustomerName() const
{
    return m_customerName;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetCustomerName(const QString &name)
{
    m_customerName = name;
}

//---------------------------------------------------------------------------------------------------------------------
inline Unit VAbstractValApplication::patternUnits() const
{
    return m_patternUnits;
}

//---------------------------------------------------------------------------------------------------------------------
inline const Unit *VAbstractValApplication::patternUnitsP() const
{
    return &m_patternUnits;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetPatternUnits(const Unit &patternUnit)
{
    m_patternUnits = patternUnit;
}

//---------------------------------------------------------------------------------------------------------------------
inline MeasurementsType VAbstractValApplication::GetMeasurementsType() const
{
    return m_measurementsType;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetMeasurementsType(const MeasurementsType &patternType)
{
    m_measurementsType = patternType;
}

//---------------------------------------------------------------------------------------------------------------------
inline Unit VAbstractValApplication::MeasurementsUnits() const
{
    return m_measurementsUnits;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetMeasurementsUnits(const Unit &measurementsUnits)
{
    m_measurementsUnits = measurementsUnits;
}

//---------------------------------------------------------------------------------------------------------------------
inline Unit VAbstractValApplication::DimensionSizeUnits() const
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
inline VAbstractPattern *VAbstractValApplication::getCurrentDocument() const
{
    return m_doc;
}

//---------------------------------------------------------------------------------------------------------------------
inline VMainGraphicsView *VAbstractValApplication::getSceneView() const
{
    return m_sceneView;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::setSceneView(VMainGraphicsView *value)
{
    m_sceneView = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline QGraphicsScene *VAbstractValApplication::getCurrentScene() const
{
    return *m_currentScene;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::setCurrentScene(QGraphicsScene **value)
{
    m_currentScene = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline QWidget *VAbstractValApplication::getMainWindow() const
{
    return mainWindow;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::setMainWindow(QWidget *value)
{
    mainWindow = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline QString VAbstractValApplication::GetPatternPath() const
{
    return m_patternFilePath;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetPatternPath(const QString &value)
{
    m_patternFilePath = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline QMap<int, QString> VAbstractValApplication::GetUserMaterials() const
{
    return m_userMaterials;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetUserMaterials(const QMap<int, QString> &userMaterials)
{
    m_userMaterials = userMaterials;
}

//---------------------------------------------------------------------------------------------------------------------
inline const Draw &VAbstractValApplication::GetDrawMode() const
{
    return m_mode;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetDrawMode(const Draw &value)
{
    m_mode = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline bool VAbstractValApplication::getOpeningPattern() const
{
    return openingPattern;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::setOpeningPattern()
{
    openingPattern = !openingPattern;
}

//---------------------------------------------------------------------------------------------------------------------
inline qreal VAbstractValApplication::GetDimensionHeight() const
{
    return m_dimensionHeight;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetDimensionHeight(qreal dimensionHeight)
{
    m_dimensionHeight = dimensionHeight;
}

//---------------------------------------------------------------------------------------------------------------------
inline qreal VAbstractValApplication::GetDimensionSize() const
{
    return m_dimensionSize;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetDimensionSize(qreal dimensionSize)
{
    m_dimensionSize = dimensionSize;
}

//---------------------------------------------------------------------------------------------------------------------
inline qreal VAbstractValApplication::GetDimensionHip() const
{
    return m_dimensionHip;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetDimensionHip(qreal dimensionHip)
{
    m_dimensionHip = dimensionHip;
}

//---------------------------------------------------------------------------------------------------------------------
inline qreal VAbstractValApplication::GetDimensionWaist() const
{
    return m_dimensionWaist;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetDimensionWaist(qreal dimensionWaist)
{
    m_dimensionWaist = dimensionWaist;
}

//---------------------------------------------------------------------------------------------------------------------
inline QString VAbstractValApplication::GetDimensionHeightLabel() const
{
    return m_dimensionHeightLabel;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetDimensionHeightLabel(const QString &label)
{
    m_dimensionHeightLabel = label;
}

//---------------------------------------------------------------------------------------------------------------------
inline QString VAbstractValApplication::GetDimensionSizeLabel() const
{
    return m_dimensionSizeLabel;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetDimensionSizeLabel(const QString &label)
{
    m_dimensionSizeLabel = label;
}

//---------------------------------------------------------------------------------------------------------------------
inline QString VAbstractValApplication::GetDimensionHipLabel() const
{
    return m_dimensionHipLabel;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetDimensionHipLabel(const QString &label)
{
    m_dimensionHipLabel = label;
}

//---------------------------------------------------------------------------------------------------------------------
inline QString VAbstractValApplication::GetDimensionWaistLabel() const
{
    return m_dimensionWaistLabel;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetDimensionWaistLabel(const QString &label)
{
    m_dimensionWaistLabel = label;
}

//---------------------------------------------------------------------------------------------------------------------
inline QDate VAbstractValApplication::GetCustomerBirthDate() const
{
    return m_customerBirthDate;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetCustomerBirthDate(const QDate &date)
{
    m_customerBirthDate = date;
}

//---------------------------------------------------------------------------------------------------------------------
inline QString VAbstractValApplication::CustomerEmail() const
{
    return m_customerEmail;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractValApplication::SetCustomerEmail(const QString &email)
{
    m_customerEmail = email;
}

#endif // VABSTRACTVALAPPLICATION_H
