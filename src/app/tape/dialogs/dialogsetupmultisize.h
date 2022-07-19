/************************************************************************
 **
 **  @file   dialogsetupmultisize.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 9, 2020
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
#ifndef DIALOGSETUPMULTISIZE_H
#define DIALOGSETUPMULTISIZE_H

#include <QDialog>

#include "../vformat/vdimensions.h"
#include "../vmisc/def.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../vmisc/defglobal.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 13, 0)

class QDoubleSpinBox;

namespace Ui
{
class DialogSetupMultisize;
}

class DialogSetupMultisize : public QDialog
{
    Q_OBJECT // NOLINT

public:
    explicit DialogSetupMultisize(Unit unit, QWidget *parent = nullptr);
    ~DialogSetupMultisize() override;

    auto Dimensions() const -> QVector<MeasurementDimension_p>;

    auto FullCircumference() const -> bool;

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void ShowFullCircumference();

    void XDimensionBodyMeasurementChanged();
    void YDimensionBodyMeasurementChanged();
    void WDimensionBodyMeasurementChanged();
    void ZDimensionBodyMeasurementChanged();

private:
    Q_DISABLE_COPY_MOVE(DialogSetupMultisize) // NOLINT
    Ui::DialogSetupMultisize *ui;
    bool m_isInitialized{false};
    QSharedPointer<VXMeasurementDimension> m_xDimension;
    QSharedPointer<VYMeasurementDimension> m_yDimension;
    QSharedPointer<VWMeasurementDimension> m_wDimension;
    QSharedPointer<VZMeasurementDimension> m_zDimension;

    void CheckState();

    void InitDimensionMinMax(QDoubleSpinBox *doubleSpinBoxMinValue, QDoubleSpinBox *doubleSpinBoxMaxValue,
                             const MeasurementDimension_p &dimension);
    void InitDimensionStep(QComboBox *comboBoxStep, const MeasurementDimension_p &dimension);

    void InitDimension(QDoubleSpinBox *doubleSpinBoxMinValue, QDoubleSpinBox *doubleSpinBoxMaxValue,
                       QComboBox *comboBoxStep, const MeasurementDimension_p &dimension);
    void InitXDimension();
    void InitYDimension();
    void InitWDimension();
    void InitZDimension();

    void DimensionMinValueChanged(qreal value, QDoubleSpinBox *doubleSpinBoxMaxValue, QComboBox *comboBoxStep,
                                  QComboBox *comboBoxBase, const MeasurementDimension_p &dimension);
    void DimensionMaxValueChanged(qreal value, QDoubleSpinBox *doubleSpinBoxMinValue, QComboBox *comboBoxStep,
                                  QComboBox *comboBoxBase, const MeasurementDimension_p &dimension);

    void DimensionStepChanged(int index, QComboBox *comboBoxStep, QComboBox *comboBoxBase,
                              const MeasurementDimension_p &dimension);
    void DimensionBaseChanged(int index, QComboBox *comboBoxBase,
                              const MeasurementDimension_p &dimension);

    void UpdateSteps(QComboBox *comboBoxStep, const MeasurementDimension_p &dimension);
    void UpdateBase(QComboBox *comboBoxBase, const MeasurementDimension_p &dimension);
};

#endif // DIALOGSETUPMULTISIZE_H
