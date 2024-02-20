/************************************************************************
 **
 **  @file   dialogdimensionlabels.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   8 10, 2020
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
#ifndef DIALOGDIMENSIONLABELS_H
#define DIALOGDIMENSIONLABELS_H

#include <QDialog>
#include <QMap>
#include <memory>

#include "../vformat/vdimensions.h"

namespace Ui
{
class DialogDimensionLabels;
}

class QTableWidgetItem;

class DialogDimensionLabels : public QDialog
{
    Q_OBJECT // NOLINT

public:
    explicit DialogDimensionLabels(const QMap<MeasurementDimension, MeasurementDimension_p> &dimensions,
                                   bool fullCircumference, QWidget *parent = nullptr);
    ~DialogDimensionLabels() override;

    auto Labels() const -> QMap<MeasurementDimension, DimesionLabels>;

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void DimensionChanged();
    void LabelChanged(QTableWidgetItem *item);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(DialogDimensionLabels) // NOLINT
    std::unique_ptr<Ui::DialogDimensionLabels> ui{};

    QMap<MeasurementDimension, MeasurementDimension_p> m_dimensions;
    QMap<MeasurementDimension, DimesionLabels> m_labels{};
    bool m_fullCircumference;

    void InitLabels();
    void InitDimensions();
    void InitTable();

    auto DimensionValue(const MeasurementDimension_p &dimension, qreal value) const -> QString;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogDimensionLabels::Labels() const -> QMap<MeasurementDimension, DimesionLabels>
{
    return m_labels;
}

#endif // DIALOGDIMENSIONLABELS_H
