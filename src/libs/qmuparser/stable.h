/***************************************************************************************************
 **
 **  @file   stable.h
 **  @author Copyright (C) 2014 Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   April 30, 2014
 **
 **  @brief
 **  @copyright
 **  Permission is hereby granted, free of charge, to any person obtaining a copy of this
 **  software and associated documentation files (the "Software"), to deal in the Software
 **  without restriction, including without limitation the rights to use, copy, modify,
 **  merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 **  permit persons to whom the Software is furnished to do so, subject to the following conditions:
 **
 **  The above copyright notice and this permission notice shall be included in all copies or
 **  substantial portions of the Software.
 **
 **  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 **  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 **  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 **  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 **  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 **
 ******************************************************************************************************/

#ifndef STABLE_H
#define STABLE_H

/* I like to include this pragma too, so the build log indicates if pre-compiled headers were in use. */
#pragma message("Compiling precompiled headers for QmuParser library.\n")

/* Add C includes here */

#if defined __cplusplus
/* Add C++ includes here */

#ifdef QT_CORE_LIB
#   include <QtCore>
#endif

#ifdef QT_GUI_LIB
#   include <QtGui>
#endif

#ifdef QT_XML_LIB
#   include <QtXml>
#endif

//In Windows you can't use same header in all modes.
#if !defined(Q_OS_WIN)
#   ifdef QT_WIDGETS_LIB
#       include <QtWidgets>
#   endif

#   ifdef QT_SVG_LIB
#       include <QtSvg/QtSvg>
#   endif

#   ifdef QT_PRINTSUPPORT_LIB
#       include <QtPrintSupport>
#   endif

//Build doesn't work, if include this headers on Windows.
#   ifdef QT_XMLPATTERNS_LIB
#       include <QtXmlPatterns>
#   endif

#   ifdef QT_NETWORK_LIB
#       include <QtNetwork>
#   endif
#endif/*Q_OS_WIN*/

#   ifdef QT_CONCURRENT_LIB
#       include <QtConcurrent>
#   endif
#endif

#endif // STABLE_H
