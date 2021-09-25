/************************************************************************
 **
 **  @file   vabstractapplication.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   18 6, 2015
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

#ifndef VABSTRACTAPPLICATION_H
#define VABSTRACTAPPLICATION_H

#include <qcompilerdetection.h>
#include <QApplication>
#include <QCoreApplication>
#include <QLocale>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QtGlobal>
#include <QTranslator>
#include <QFileDialog>

#include "../vmisc/def.h"
#include "../vpatterndb/vtranslatevars.h"
#include "vcommonsettings.h"

class QUndoStack;
class VAbstractApplication;// use in define
class VCommonSettings;

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")

class VAbstractApplication : public QApplication
{
    Q_OBJECT
public:
    VAbstractApplication(int &argc, char ** argv);
    virtual ~VAbstractApplication() =default;

    virtual const VTranslateVars *TrVars()=0;

    QString translationsPath(const QString &locale = QString()) const;

    void LoadTranslation(const QString &locale);

    virtual void     OpenSettings()=0;
    VCommonSettings *Settings();

    template <typename T>
    QString LocaleToString(const T &value);

    QUndoStack *getUndoStack() const;

    virtual bool IsAppInGUIMode()const =0;
    virtual bool IsPedantic() const;

    static QString ClearMessage(QString msg);

    static const QString warningMessageSignature;
    bool IsWarningMessage(const QString &message) const;

    QFileDialog::Options NativeFileDialog(QFileDialog::Options options = QFileDialog::Options()) const;

#if defined(Q_OS_WIN)
    static void WinAttachConsole();
#endif

    static VAbstractApplication *VApp();

protected:
    QUndoStack *undoStack;

    /**
     * @brief settings pointer to settings. Help hide constructor creation settings. Make make code more readable.
     */
    VCommonSettings *settings{nullptr};

    QPointer<QTranslator> qtTranslator{nullptr};
    QPointer<QTranslator> qtxmlTranslator{nullptr};
    QPointer<QTranslator> qtBaseTranslator{nullptr};
    QPointer<QTranslator> appTranslator{nullptr};
    QPointer<QTranslator> pmsTranslator{nullptr};

    virtual void InitTrVars()=0;

protected slots:
    virtual void AboutToQuit()=0;

private:
    Q_DISABLE_COPY(VAbstractApplication)

    void ClearTranslation();
};

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline QString VAbstractApplication::LocaleToString(const T &value)
{
    QLocale loc;
    VAbstractApplication::VApp()->Settings()->GetOsSeparator() ? loc = QLocale() : loc = QLocale::c();
    return loc.toString(value);
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractApplication::VApp() -> VAbstractApplication *
{
    return qobject_cast<VAbstractApplication*>(QCoreApplication::instance());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getSettings hide settings constructor.
 * @return pointer to class for acssesing to settings in ini file.
 */
inline VCommonSettings *VAbstractApplication::Settings()
{
    SCASSERT(settings != nullptr)
    return settings;
}

#endif // VABSTRACTAPPLICATION_H
