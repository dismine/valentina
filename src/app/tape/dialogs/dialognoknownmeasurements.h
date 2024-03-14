/************************************************************************
 **
 **  @file   dialognoknownmeasurements.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   14 3, 2024
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2024 Valentina project
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
#ifndef DIALOGNOKNOWNMEASUREMENTS_H
#define DIALOGNOKNOWNMEASUREMENTS_H

#include <QDialog>

namespace Ui
{
class DialogNoKnownMeasurements;
}

class DialogNoKnownMeasurements : public QDialog
{
    Q_OBJECT // NOLINT

public:
    explicit DialogNoKnownMeasurements(QWidget *parent = nullptr);
    ~DialogNoKnownMeasurements() override;

private:
    Ui::DialogNoKnownMeasurements *ui;
    Q_DISABLE_COPY_MOVE(DialogNoKnownMeasurements) // NOLINT
};

#endif // DIALOGNOKNOWNMEASUREMENTS_H
