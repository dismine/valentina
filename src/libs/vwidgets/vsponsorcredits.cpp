/************************************************************************
 **
 **  @file   vsponsorcredits.cpp
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
#include "vsponsorcredits.h"

#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QParallelAnimationGroup>
#include <QPauseAnimation>
#include <QPropertyAnimation>
#include <QRandomGenerator>
#include <QSequentialAnimationGroup>
#include <QTimer>

#include "../vmisc/dialogs/dialogcredits.h"

using namespace std::chrono_literals;

//---------------------------------------------------------------------------------------------------------------------
VSponsorCredits::VSponsorCredits(QWidget *parent)
  : QLabel{parent},
    sponsorsList(DialogCredits::sponsors.split('\n')),
    effect(new QGraphicsOpacityEffect(this))
{
    // opacity for appear
    setGraphicsEffect(effect);
    effect->setOpacity(1.0);

    // timer starts first sponsor
    QTimer::singleShot(1500ms, this, &VSponsorCredits::NextSponsor);
}

//---------------------------------------------------------------------------------------------------------------------
auto VSponsorCredits::LetterSpacing() const -> qreal
{
    return font().letterSpacing();
}

//---------------------------------------------------------------------------------------------------------------------
void VSponsorCredits::SetLetterSpacing(qreal spacing)
{
    QFont f = font();
    f.setLetterSpacing(QFont::AbsoluteSpacing, spacing);
    setFont(f);
}

//---------------------------------------------------------------------------------------------------------------------
void VSponsorCredits::NextSponsor()
{
    QString const name = sponsorsList.at(index);
    index = (index + 1) % sponsorsList.size();

    // reset font spacing
    SetLetterSpacing(0);

    // --- Generate encoded name BEFORE fade-in ---
    QString encoded = name;
    std::generate(encoded.begin(), encoded.end(), []() { return QChar('!' + QRandomGenerator::global()->bounded(93)); });

    // Set encoded text first
    setText(encoded);

    // fade in
    auto *fadeIn = new QPropertyAnimation(effect, "opacity");
    fadeIn->setDuration(400);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);

    connect(fadeIn, &QPropertyAnimation::finished, this, [this, name]() { StartDecodingEffect(name); });

    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
}

//---------------------------------------------------------------------------------------------------------------------
void VSponsorCredits::StartSpreadAndFade()
{
    // pause
    auto *stay = new QPauseAnimation(1200);

    // letter spacing grow
    auto *spacingAnim = new QPropertyAnimation(this, "LetterSpacing");
    spacingAnim->setDuration(1000);
    spacingAnim->setStartValue(0.0);
    spacingAnim->setEndValue(20.0); // adjust how far characters spread
    spacingAnim->setEasingCurve(QEasingCurve::OutQuad);

    // fade out at same time as spacing
    auto *fadeOut = new QPropertyAnimation(effect, "opacity");
    fadeOut->setDuration(1000);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);

    // run spacing + fade together
    auto *spreadAndFade = new QParallelAnimationGroup(this);
    spreadAndFade->addAnimation(spacingAnim);
    spreadAndFade->addAnimation(fadeOut);

    // sequence all parts
    auto *seq = new QSequentialAnimationGroup(this);
    seq->addAnimation(stay);
    seq->addAnimation(spreadAndFade);

    connect(seq, &QSequentialAnimationGroup::finished, this, &VSponsorCredits::NextSponsor);
    seq->start(QAbstractAnimation::DeleteWhenStopped);
}

//---------------------------------------------------------------------------------------------------------------------
void VSponsorCredits::StartDecodingEffect(const QString &name)
{
    decodeDisplay = text(); // already contains the encoded letters
    decodeStep = 0;

    auto *decodeTimer = new QTimer(this);

    connect(decodeTimer,
            &QTimer::timeout,
            this,
            [this, name, decodeTimer]()
            {
                vsizetype const length = name.length();
                if (decodeStep < length)
                {
                    // Lock in correct characters up to step
                    for (int i = 0; i <= decodeStep; ++i)
                    {
                        decodeDisplay[i] = name[i];
                    }

                    // Randomize remaining characters
                    for (int i = decodeStep + 1; i < length; ++i)
                    {
                        auto c = QChar('!' + QRandomGenerator::global()->bounded(93));
                        decodeDisplay[i] = c;
                    }

                    setText(decodeDisplay);
                    ++decodeStep;
                }
                else
                {
                    // Fully decoded
                    setText(name);
                    decodeTimer->stop();
                    decodeTimer->deleteLater();

                    // Start letter spacing + fade out
                    StartSpreadAndFade();
                }
            });

    decodeTimer->start(50ms); // milliseconds per character
}
