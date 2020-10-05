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

class DialogRestrictDimension : public QDialog
{
    Q_OBJECT

public:
    DialogRestrictDimension(const QList<MeasurementDimension_p> &dimensions,
                            const QMap<QString, QPair<int, int>> &restrictions, bool oneDimesionRestriction,
                            bool fullCircumference, QWidget *parent = nullptr);
    virtual ~DialogRestrictDimension();

    QMap<QString, QPair<int, int> > Restrictions() const;

private:
    Q_DISABLE_COPY(DialogRestrictDimension)
    Ui::DialogRestrictDimension *ui;

    bool m_oneDimesionRestriction;
    bool m_fullCircumference;
    QList<MeasurementDimension_p>  m_dimensions;
    QMap<QString, QPair<int, int>> m_restrictions;
};

//---------------------------------------------------------------------------------------------------------------------
inline QMap<QString, QPair<int, int> > DialogRestrictDimension::Restrictions() const
{
    return m_restrictions;
}

#endif // DIALOGRESTRICTDIMENSION_H
