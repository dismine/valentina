/************************************************************************
 **
 **  @file   vsinglelineoutlinechar.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   19 6, 2023
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
#ifndef VSINGLELINEOUTLINECHAR_H
#define VSINGLELINEOUTLINECHAR_H

#include <QCoreApplication>
#include <QFont>

using VOutlineCorrections = QHash<QChar, QHash<int, bool>>;
using VOutlineCorrectionsCache = QCache<QString, VOutlineCorrections>;

class QPainterPath;

class VSingleLineOutlineChar
{
    Q_DECLARE_TR_FUNCTIONS(VSingleLineOutlineChar) // NOLINT

public:
    explicit VSingleLineOutlineChar(const QFont &font);

    void ExportCorrections(const QString &dirPath) const;
    void LoadCorrections(const QString &dirPath) const;

    void ClearCorrectionsCache();

    auto DrawChar(qreal x, qreal y, QChar c) const -> QPainterPath;

    auto IsPopulated() const -> bool;

private:
    QFont m_font{};
};

#endif // VSINGLELINEOUTLINECHAR_H
