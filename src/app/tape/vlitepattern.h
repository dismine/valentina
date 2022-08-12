/************************************************************************
 **
 **  @file   vlitepattern.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   9 8, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#ifndef VLITEPATTERN_H
#define VLITEPATTERN_H

#include "../ifc/xml/vabstractpattern.h"

class VLitePattern : public VAbstractPattern
{
    Q_OBJECT // NOLINT
public:
    explicit VLitePattern(QObject *parent = nullptr);
    ~VLitePattern() override = default;

    void CreateEmptyFile() override;

    void IncrementReferens(quint32 id) const override;
    void DecrementReferens(quint32 id) const override;

    auto GenerateLabel(const LabelType &type, const QString &reservedName = QString())const -> QString override;
    auto GenerateSuffix() const -> QString override;

    void UpdateToolData(const quint32 &id, VContainer *data) override;

public slots:
    void LiteParseTree(const Document &parse) override;

private:
    Q_DISABLE_COPY_MOVE(VLitePattern) // NOLINT
};

#endif // VLITEPATTERN_H
