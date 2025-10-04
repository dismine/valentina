/************************************************************************
 **
 **  @file   vsponsorcredits.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   2 10, 2025
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
#ifndef VSPONSORCREDITS_H
#define VSPONSORCREDITS_H

#include <QLabel>

#include "../vmisc/defglobal.h"

class QGraphicsOpacityEffect;

class VSponsorCredits : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(qreal LetterSpacing READ LetterSpacing WRITE SetLetterSpacing) // clazy:exclude=qproperty-without-notify

public:
    explicit VSponsorCredits(QWidget *parent = nullptr);
    ~VSponsorCredits() override = default;

    auto LetterSpacing() const -> qreal;
    void SetLetterSpacing(qreal spacing);

private slots:
    void NextSponsor();
    void StartSpreadAndFade();

private:
    Q_DISABLE_COPY_MOVE(VSponsorCredits)
    QStringList sponsorsList;
    vsizetype index{0};
    QGraphicsOpacityEffect *effect;
    int decodeStep{0};
    QString decodeDisplay{};

    void StartDecodingEffect(const QString &name);
};

#endif // VSPONSORCREDITS_H
