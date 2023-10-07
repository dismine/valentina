/************************************************************************
 **
 **  @file   dialogduplicatedetail.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   25 10, 2017
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
#ifndef DIALOGDUPLICATEDETAIL_H
#define DIALOGDUPLICATEDETAIL_H

#include "../dialogtool.h"

namespace Ui
{
    class DialogDuplicateDetail;
}

class DialogDuplicateDetail : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    explicit DialogDuplicateDetail(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent = nullptr);
    virtual ~DialogDuplicateDetail();

    auto Duplicate() const -> quint32;
    auto MoveDuplicateX() const -> qreal;
    auto MoveDuplicateY() const -> qreal;

    virtual void ShowDialog(bool click) override;
public slots:
    virtual void ChosenObject(quint32 id, const SceneObject &type) override;
protected:
    virtual auto IsValid() const -> bool final { return true; }

private:
    Q_DISABLE_COPY_MOVE(DialogDuplicateDetail) // NOLINT
    Ui::DialogDuplicateDetail *ui;
    quint32 m_idDetail;
    qreal m_mx;
    qreal m_my;
    bool m_firstRelease;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogDuplicateDetail::Duplicate() const -> quint32
{
    return m_idDetail;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogDuplicateDetail::MoveDuplicateX() const -> qreal
{
    return m_mx;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogDuplicateDetail::MoveDuplicateY() const -> qreal
{
    return m_my;
}

#endif // DIALOGDUPLICATEDETAIL_H
