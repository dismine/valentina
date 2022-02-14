/************************************************************************
 **
 **  @file   dialogdimensioncustomnames.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   14 2, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2022 Valentina project
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
#ifndef DIALOGDIMENSIONCUSTOMNAMES_H
#define DIALOGDIMENSIONCUSTOMNAMES_H

#include <QDialog>

#include "../vformat/vdimensions.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../vmisc/defglobal.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 13, 0)

namespace Ui
{
class DialogDimensionCustomNames;
}

class DialogDimensionCustomNames : public QDialog
{
    Q_OBJECT // NOLINT

public:
    explicit DialogDimensionCustomNames(const QMap<MeasurementDimension, MeasurementDimension_p > &dimensions,
                                        QWidget *parent = nullptr);
    ~DialogDimensionCustomNames() override;

    auto CustomNames() const -> QMap<MeasurementDimension, QString>;

private:
    Q_DISABLE_COPY_MOVE(DialogDimensionCustomNames) // NOLINT
    Ui::DialogDimensionCustomNames *ui;

    void InitTable(const QMap<MeasurementDimension, MeasurementDimension_p> &dimensions);
};

#endif // DIALOGDIMENSIONCUSTOMNAMES_H
