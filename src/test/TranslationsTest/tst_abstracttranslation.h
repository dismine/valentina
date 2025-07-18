/************************************************************************
 **
 **  @file   tst_abstracttranslation.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   17 2, 2018
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2018 Valentina project
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
#ifndef TST_ABSTRACTTRANSLATION_H
#define TST_ABSTRACTTRANSLATION_H

#include <QDomDocument>
#include <QFile>
#include <QObject>
#include <QSharedPointer>

// Header <ciso646> is removed in C++20.
#if defined(Q_CC_MSVC) && __cplusplus <= 201703L
#include <ciso646> // and, not, or
#endif

class TST_AbstractTranslation : public QObject
{
    Q_OBJECT // NOLINT

public:
    explicit TST_AbstractTranslation(QObject *parent = nullptr);

protected:
    auto LoadTSFile(const QString &filename) -> QDomNodeList;

    static const QString TagName;
    static const QString TagMessage;
    static const QString TagSource;
    static const QString TagTranslation;

    static const QString AttrType;
    static const QString AttrValVanished;
    static const QString AttrValUnfinished;
    static const QString AttrValObsolete;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(TST_AbstractTranslation) // NOLINT

    QSharedPointer<QFile> tsFile;
    QSharedPointer<QDomDocument> tsXML;
};

#endif // TST_ABSTRACTTRANSLATION_H
