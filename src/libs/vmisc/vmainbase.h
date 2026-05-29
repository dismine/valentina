/************************************************************************
 **
 **  @file   vmainbase.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   29 5, 2026
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project
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

#ifndef VMAINBASE_H
#define VMAINBASE_H

#include <QtGlobal>

// Fix bug in Qt. Deprecation warning in QMessageBox::critical.
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0) && QT_VERSION < QT_VERSION_CHECK(6, 6, 0)
#include <QApplication>
#include <QMessageBox>
#undef QT_REQUIRE_VERSION
#define QT_REQUIRE_VERSION(argc, argv, str) \
    { \
        QString s = QString::fromLatin1(str); \
        QString sq = QString::fromLatin1(qVersion()); \
        if ((sq.section(QChar::fromLatin1('.'), 0, 0).toInt() << 16) \
                + (sq.section(QChar::fromLatin1('.'), 1, 1).toInt() << 8) \
                + sq.section(QChar::fromLatin1('.'), 2, 2).toInt() \
            < (s.section(QChar::fromLatin1('.'), 0, 0).toInt() << 16) \
                  + (s.section(QChar::fromLatin1('.'), 1, 1).toInt() << 8) \
                  + s.section(QChar::fromLatin1('.'), 2, 2).toInt()) \
        { \
            if (!qApp) \
            { \
                new QApplication(argc, argv); \
            } \
            QString s = QApplication::tr("Executable '%1' requires Qt %2, found Qt %3.") \
                            .arg(qAppName()) \
                            .arg(QString::fromLatin1(str)) \
                            .arg(QString::fromLatin1(qVersion())); \
            QMessageBox::critical(nullptr, \
                                  QApplication::tr("Incompatible Qt Library Error"), \
                                  s, \
                                  QMessageBox::Abort, \
                                  static_cast<QMessageBox::StandardButton>(0)); \
            qFatal("%s", s.toLatin1().data()); \
        } \
    }
#endif // QT_VERSION >= QT_VERSION_CHECK(6, 2, 0) && QT_VERSION < QT_VERSION_CHECK(6, 6, 0)

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <xercesc/util/PlatformUtils.hpp>

struct VXercesGuard
{
    VXercesGuard() { XERCES_CPP_NAMESPACE::XMLPlatformUtils::Initialize(); }
    ~VXercesGuard() { XERCES_CPP_NAMESPACE::XMLPlatformUtils::Terminate(); }
    Q_DISABLE_COPY_MOVE(VXercesGuard)
};
#endif // QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

#endif // VMAINBASE_H
