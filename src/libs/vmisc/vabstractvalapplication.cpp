/************************************************************************
 **
 **  @file   vabstractvalapplication.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 10, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2020 Valentina project
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
#include "vabstractvalapplication.h"
#include "customevents.h"
#include "vvalentinasettings.h"

#include <QTranslator>
#include <QWidget>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
VAbstractValApplication::VAbstractValApplication(int &argc, char **argv)
  : VAbstractApplication(argc, argv)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractValApplication::toPixel(double val) const -> double
{
    return ToPixel(val, m_patternUnits);
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractValApplication::fromPixel(double pix) const -> double
{
    return FromPixel(pix, m_patternUnits);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractValApplication::PostWarningMessage(const QString &message, QtMsgType severity) const
{
    QApplication::postEvent(mainWindow,
                            new WarningMessageEvent(VAbstractValApplication::ClearMessage(message), severity));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief OpenSettings get acsses to application settings.
 *
 * Because we can create object in constructor we open file separately.
 */
void VAbstractValApplication::OpenSettings()
{
    settings = new VValentinaSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(),
                                      QCoreApplication::applicationName(), this);
    connect(settings, &VValentinaSettings::SVGFontsPathChanged, this, &VAbstractValApplication::SVGFontsPathChanged);
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractValApplication::ValentinaSettings() -> VValentinaSettings *
{
    SCASSERT(settings != nullptr)

    return qobject_cast<VValentinaSettings *>(settings);
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractValApplication::VApp() -> VAbstractValApplication *
{
    return qobject_cast<VAbstractValApplication *>(QCoreApplication::instance());
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractValApplication::GetPlaceholderTranslator() -> QSharedPointer<QTranslator>
{
    VValentinaSettings *settings = ValentinaSettings();

    QString pieceLabelLocale = settings->GetPieceLabelLocale();
    if (pieceLabelLocale == VCommonSettings::defaultPieceLabelLocale)
    {
        pieceLabelLocale = settings->GetLocale();
    }

    if (pieceLabelLocale.startsWith("ru"_L1))
    {
        return QSharedPointer<QTranslator>(new QTranslator);
    }

    QSharedPointer<QTranslator> translator = QSharedPointer<QTranslator>(new QTranslator);
    const QString appQmDir = VAbstractApplication::translationsPath(settings->GetLocale());
    if (translator->load(QStringLiteral("valentina_") + pieceLabelLocale, appQmDir))
    {
        return translator;
    }

    return QSharedPointer<QTranslator>(new QTranslator);
}
