/************************************************************************
 **
 **  @file   dialogpin.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   31 1, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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

#ifndef DIALOGPIN_H
#define DIALOGPIN_H

#include "../dialogtool.h"

namespace Ui
{
class DialogPin;
}

class DialogPin final : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    explicit DialogPin(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent = nullptr);
    ~DialogPin() override;

    void EnbleShowMode(bool disable);

    auto GetPieceId() const -> quint32;
    void SetPieceId(quint32 id);

    auto GetPointId() const -> quint32;
    void SetPointId(quint32 id);

    void SetPiecesList(const QVector<quint32> &list) override;

public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;

protected:
    void ShowVisualization() override;
    auto IsValid() const -> bool override;

private:
    Q_DISABLE_COPY_MOVE(DialogPin) // NOLINT
    Ui::DialogPin *ui;
    bool m_showMode;
    bool m_flagPoint;
    bool m_flagError;

    void CheckPieces();
    void CheckPoint();
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogPin::IsValid() const -> bool
{
    return m_flagPoint && m_flagError;
}

#endif // DIALOGPIN_H
