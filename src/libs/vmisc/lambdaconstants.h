/************************************************************************
 **
 **  @file   lambdaconstants.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   29 1, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2022 Valentina project
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
#ifndef LAMBDACONSTANTS_H
#define LAMBDACONSTANTS_H

/*
    given a lambda that wants to capture two constants

    const auto k1 = 1000;
    const auto k2 = 2000;
    int v = 0;

    auto lambda = [&v, &k1, &k2]() {
        v = k1 * k2;
    }

    Then unfortunately clang will correctly warn about unnecessary captures. And MSVC will fail to compile if you don't
    capture.

    https://stackoverflow.com/questions/52416362/unused-lambda-capture-warning-when-capture-is-actually-used

    An imperfect solution is to declare the lambda using the V_LAMBDA_CONSTANTS macro.

    auto lambda = [&v
    V_LAMBDA_CONSTANTS(&k1, &k2)
    ](){
        v = k1 * k2;
    }

    This should work correctly. Most of the time.

NOTE: There is no comma after the final capture variable before the V_LAMBDA_CONSTANTS macro. The macro is variadic and
will work with 1 or more captures.
*/
#ifndef V_LAMBDA_CONSTANTS
#if defined(Q_CC_MSVC) && _MSC_VER < 1932 // MSVC versions before VS 2022 17.2
#define V_LAMBDA_CONSTANTS(...) ,__VA_ARGS__
#else
#define V_LAMBDA_CONSTANTS(...)
#endif
#endif

#endif // LAMBDACONSTANTS_H
