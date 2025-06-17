/************************************************************************
 **
 **  @file   dialogrestrictdimension.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   5 10, 2020
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
#ifndef DIALOGRESTRICTDIMENSION_H
#define DIALOGRESTRICTDIMENSION_H

#include <QDialog>
#include <QMap>

#include "../vformat/vdimensions.h"

namespace Ui
{
class DialogRestrictDimension;
}

class QTableWidgetItem;

enum class RestrictDimension : qint8
{
    First = 0,
    Second = 1,
    Third = 2
};

class DialogRestrictDimension : public QDialog
{
    Q_OBJECT // NOLINT

public:
    DialogRestrictDimension(const QList<MeasurementDimension_p> &dimensions,
                            const QMap<QString, VDimensionRestriction> &restrictions, RestrictDimension restrictionType,
                            bool fullCircumference, QWidget *parent = nullptr);
    ~DialogRestrictDimension() override;

    auto Restrictions() const -> QMap<QString, VDimensionRestriction>;

protected:
    void changeEvent(QEvent *event) override;

protected slots:
    void RowSelected();
    void DimensionAChanged();
    void MinRestrictionChanged();
    void MaxRestrictionChanged();

private slots:
    void CellContextMenu(QPoint pos);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(DialogRestrictDimension) // NOLINT
    Ui::DialogRestrictDimension *ui;

    RestrictDimension m_restrictionType;
    bool m_fullCircumference;
    QList<MeasurementDimension_p> m_dimensions;
    QMap<QString, VDimensionRestriction> m_restrictions;

    void InitDimensionsBaseValues();
    void InitDimensionGradation(const MeasurementDimension_p &dimension, QComboBox *control);
    void InitTable();

    void RefreshTable();

    void AddCell(int row, int column, qreal rowValue, qreal columnValue);

    void EnableRestrictionControls(bool enable);

    void FillBases(const QVector<qreal> &bases, const MeasurementDimension_p &dimension, QComboBox *control) const;
    void FillBase(double base, const MeasurementDimension_p &dimension, QComboBox *control) const;
    static auto FillDimensionXBases(const QVector<qreal> &bases, const MeasurementDimension_p &dimension)
        -> QStringList;
    auto FillDimensionYBases(const QVector<qreal> &bases, const MeasurementDimension_p &dimension) const -> QStringList;
    auto FillDimensionWZBases(const QVector<qreal> &bases, const MeasurementDimension_p &dimension) const
        -> QStringList;

    auto DimensionLabels(const QVector<qreal> &bases, const MeasurementDimension_p &dimension) const -> QStringList;
    auto DimensionRestrictedValues(const MeasurementDimension_p &dimension) const -> QVector<qreal>;
    auto StartRow() const -> int;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogRestrictDimension::Restrictions() const -> QMap<QString, VDimensionRestriction>
{
    return m_restrictions;
}

#endif // DIALOGRESTRICTDIMENSION_H
