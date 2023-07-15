/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   25 10, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#ifndef TST_ABSTRACTREGEXP_H
#define TST_ABSTRACTREGEXP_H

#include "../vtest/abstracttest.h"
#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../vmisc/defglobal.h"
#endif

#include <QPointer>

class QTranslator;
class VTranslateVars;

class TST_AbstractRegExp : public AbstractTest
{
    Q_OBJECT // NOLINT

public:
    TST_AbstractRegExp(const QString &locale, QObject *parent = nullptr);
    virtual ~TST_AbstractRegExp();

protected:
    QString m_locale;
    QPointer<QTranslator> m_vTranslator;
    VTranslateVars *m_trMs;

    virtual void PrepareData() = 0;
    virtual auto AllNames() -> QStringList = 0;

    auto LoadVariables(const QString &checkedLocale) -> int;
    void RemoveTrVariables(const QString &checkedLocale);
    void InitTrMs();

    void CallTestCheckNoEndLine();
    void CallTestCheckRegExpNames();
    void CallTestCheckNoOriginalNamesInTranslation();
    void CallTestForValidCharacters();

private:
    Q_DISABLE_COPY_MOVE(TST_AbstractRegExp) // NOLINT
};

#endif // TST_ABSTRACTREGEXP_H
