/************************************************************************
 **
 **  @file   velidedlabel.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   8 5, 2020
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
#ifndef VELIDEDLABEL_H
#define VELIDEDLABEL_H

#include <QFrame>

class VElidedLabel : public QFrame
{
    Q_OBJECT // NOLINT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(bool IsElided READ IsElided)
    Q_PROPERTY(bool showElided READ ShowElided WRITE SetShowElided)

public:
    explicit VElidedLabel(QWidget *parent = nullptr);
    explicit VElidedLabel(const QString &text, QWidget *parent = nullptr);

    void setText(const QString &text);
    auto text() const -> const QString &;

    auto IsElided() const -> bool;

    auto ShowElided() const -> bool;
    void SetShowElided(bool showElided);

protected:
    virtual void paintEvent(QPaintEvent *event) override;

signals:
    void ElisionChanged(bool m_elided);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VElidedLabel) // NOLINT
    bool m_elided{false};
    bool m_showElided{true};
    QString m_content{};

    auto DrawParagraph(QPainter &painter, const QString &paragraph, int &y, bool &didElide) -> bool;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VElidedLabel::text() const -> const QString &
{
    return m_content;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VElidedLabel::IsElided() const -> bool
{
    return m_elided;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VElidedLabel::ShowElided() const -> bool
{
    return m_showElided;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VElidedLabel::SetShowElided(bool showElided)
{
    m_showElided = showElided;
}

#endif // VELIDEDLABEL_H
