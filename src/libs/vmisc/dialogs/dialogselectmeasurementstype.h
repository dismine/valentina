/************************************************************************
 **
 **  @file   dialogselectmeasurementstype.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 9, 2025
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2025 Valentina project
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
#ifndef DIALOGSELECTMEASUREMENTSTYPE_H
#define DIALOGSELECTMEASUREMENTSTYPE_H

#include <QDialog>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <qtclasshelpermacros.h>
#endif

#include "../def.h"

namespace Ui
{
class DialogSelectMeasurementsType;
}

class DialogSelectMeasurementsType : public QDialog
{
    Q_OBJECT // NOLINT

public:
    explicit DialogSelectMeasurementsType(QWidget *parent = nullptr);
    ~DialogSelectMeasurementsType() override;

    auto Type() const -> MeasurementsType;

protected:
    void showEvent(QShowEvent *event) override;

private:
    Q_DISABLE_COPY_MOVE(DialogSelectMeasurementsType)
    Ui::DialogSelectMeasurementsType *ui;
    bool m_isInitialized{false};
    MeasurementsType m_type{MeasurementsType::Unknown};
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogSelectMeasurementsType::Type() const -> MeasurementsType
{
    return m_type;
}

#endif // DIALOGSELECTMEASUREMENTSTYPE_H
