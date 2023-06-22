/************************************************************************
 **
 **  @file   vsvgpathtokenizer.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   8 6, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#ifndef VSVGPATHTOKENIZER_H
#define VSVGPATHTOKENIZER_H

#include <QChar>
#include <QList>
#include <QMetaType>
#include <QPointF>
#include <QString>

class QPainterPath;

struct VSVGPathCommand
{
    QChar m_command{};
    std::vector<qreal> m_arguments{};
};

Q_DECLARE_METATYPE(VSVGPathCommand) // NOLINT

class VSVGPathTokenizer
{
public:
    explicit VSVGPathTokenizer(const QString &path);
    ~VSVGPathTokenizer() = default;

    auto ToPainterPath(QPainterPath &path) const -> bool;

    auto GetCommands() const -> QList<VSVGPathCommand>;

    auto GetSinglePath() const -> bool;
    void SetSinglePath(bool newSinglePath);

private:
    Q_DISABLE_COPY_MOVE(VSVGPathTokenizer)

    QList<VSVGPathCommand> m_commands{};

    bool m_singlePath{true};

    mutable const qreal *m_num{nullptr};
    mutable int m_count{0};
    mutable qreal m_offsetX{0};
    mutable qreal m_offsetY{0};
    mutable qreal m_x0{0}; // starting point X
    mutable qreal m_y0{0}; // starting point Y
    mutable qreal m_x{0};  // current point X
    mutable qreal m_y{0};  // current point Y
    mutable char m_lastMode{0};
    mutable QPointF m_ctrlPt{};
    mutable QChar m_pathElem{};

    void TokenizePathString(const QString &path);

    auto AddCommandIfNotNull(VSVGPathCommand command) -> bool;

    void Command_m(QPainterPath &path) const;
    void Command_M(QPainterPath &path) const;
    void Command_z(QPainterPath &path) const;
    void Command_l(QPainterPath &path) const;
    void Command_L(QPainterPath &path) const;
    void Command_h(QPainterPath &path) const;
    void Command_H(QPainterPath &path) const;
    void Command_v(QPainterPath &path) const;
    void Command_V(QPainterPath &path) const;
    void Command_c(QPainterPath &path) const;
    void Command_C(QPainterPath &path) const;
    void Command_s(QPainterPath &path) const;
    void Command_S(QPainterPath &path) const;
    void Command_q(QPainterPath &path) const;
    void Command_Q(QPainterPath &path) const;
    void Command_t(QPainterPath &path) const;
    void Command_T(QPainterPath &path) const;
    void Command_a(QPainterPath &path) const;
    void Command_A(QPainterPath &path) const;
};

#endif // VSVGPATHTOKENIZER_H
