/************************************************************************
 **
 **  @file   vtheme.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   17 7, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#include "vtheme.h"
#include "../vcommonsettings.h"

#include <QIcon>
#include <QOperatingSystemVersion>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QStyle>
#include <QStyleFactory>
#include <QTextStream>
#include <QtDebug>
#include <QtGlobal>
#include <QtSvg/QSvgRenderer>

#if defined(Q_OS_MACX)
#include "macutils.h"
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QStyleHints>
#endif

#include "../vabstractapplication.h"
#include "vapplicationstyle.h"
#include "vscenestylesheet.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../compatibility.h"
#endif

using namespace std::chrono_literals;
using namespace Qt::Literals::StringLiterals;

namespace
{
//---------------------------------------------------------------------------------------------------------------------
#ifdef Q_OS_WIN
auto NativeWindowsDarkThemeAvailable() -> bool
{
    // dark mode supported Windows 10 1809 10.0.17763 onward
    // https://stackoverflow.com/questions/53501268/win10-dark-theme-how-to-use-in-winapi
    if (QOperatingSystemVersion::current().majorVersion() > 10)
    {
        return true;
    }

    if (QOperatingSystemVersion::current().majorVersion() == 10)
    {
        return QOperatingSystemVersion::current().microVersion() >= 17763;
    }

    return false;
}
#endif // Q_OS_WIN

//---------------------------------------------------------------------------------------------------------------------
#if defined(Q_OS_MACX)
inline auto NativeMacDarkThemeAvailable() -> bool
{
    return NSNativeMacDarkThemeAvailable();
}
#endif

//---------------------------------------------------------------------------------------------------------------------
#if defined(Q_OS_LINUX)
inline auto NativeLinuxDarkThemeAvailable() -> bool
{
    // There is no way to check native support. Assume always available.
    return true;
}
#endif

//---------------------------------------------------------------------------------------------------------------------
void ActivateCustomLightTheme()
{
    QFile f(QStringLiteral(":/light/stylesheet.qss"));
    if (!f.exists())
    {
        qDebug() << "Unable to set stylesheet, file not found\n";
    }
    else
    {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ActivateCustomDarkTheme()
{
    QFile f(QStringLiteral(":/dark/stylesheet.qss"));
    if (!f.exists())
    {
        qDebug() << "Unable to set stylesheet, file not found\n";
    }
    else
    {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
}

//---------------------------------------------------------------------------------------------------------------------
#if defined(Q_OS_WIN)
void ActivateDefaultThemeWin()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
// Fix for issue QTBUG-123722
#if QT_VERSION == QT_VERSION_CHECK(6, 7, 0)
    if (!VTheme::IsInDarkTheme())
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
        qApp->setStyleSheet(QStringLiteral("QComboBox QAbstractItemView {background-color: white}"));
    }
#else
    qApp->setStyleSheet(QString()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
#endif
#else
    if (VTheme::IsInDarkTheme())
    {
        ActivateCustomDarkTheme();
    }
    else
    {
        qApp->setStyleSheet(QString()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
#endif // QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
}
#endif // defined(Q_OS_WIN)

//---------------------------------------------------------------------------------------------------------------------
#if defined(Q_OS_MACX)
void ActivateDefaultThemeMac()
{
    qApp->setStyleSheet(QString()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
}
#endif // defined(Q_OS_MACX)

//---------------------------------------------------------------------------------------------------------------------
void ActivateDefaultTheme()
{
#if defined(Q_OS_WIN)
    ActivateDefaultThemeWin();
#elif defined(Q_OS_MACX)
    ActivateDefaultThemeMac();
#else
    if (VTheme::IsInDarkTheme())
    {
        ActivateCustomDarkTheme();
    }
    else
    {
        qApp->setStyleSheet(QString()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
#endif
}

//---------------------------------------------------------------------------------------------------------------------
auto GetResourceName(const QString &root, const QString &iconName, bool dark) -> QString
{
    return QStringLiteral(":/%1/%2/%3").arg(root, dark ? "dark"_L1 : "light"_L1, iconName);
}

//---------------------------------------------------------------------------------------------------------------------
auto GetPixmapResource(const QString &root, const QString &iconName, bool dark) -> QPixmap
{
    QString const resourceName = GetResourceName(root, iconName, dark);
    QPixmap pixmap = QPixmap(resourceName);
    Q_ASSERT(!pixmap.isNull());
    return pixmap;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
auto VTheme::Instance() -> VTheme *
{
    static VTheme *instance = nullptr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
    if (instance == nullptr)
    {
        instance = new VTheme();
    }

    return instance;
}

//---------------------------------------------------------------------------------------------------------------------
void VTheme::StoreDefaultThemeName(const QString &themeName)
{
    m_defaultThemeName = themeName;
}

//---------------------------------------------------------------------------------------------------------------------
auto VTheme::NativeDarkThemeAvailable() -> bool
{
#if defined(Q_OS_MACX)
    return NativeMacDarkThemeAvailable();
#elif defined(Q_OS_WIN)
    return NativeWindowsDarkThemeAvailable();
#elif defined(Q_OS_LINUX)
    return NativeLinuxDarkThemeAvailable();
#else
    return false;
#endif
}

//---------------------------------------------------------------------------------------------------------------------
auto VTheme::IsInDarkTheme() -> bool
{
    if (NativeDarkThemeAvailable())
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        QStyleHints *hints = QGuiApplication::styleHints();
        return hints->colorScheme() == Qt::ColorScheme::Dark;
#else
#if defined(Q_OS_MACX)
        return NSMacIsInDarkTheme();
#elif defined(Q_OS_WIN)
        QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                           QSettings::NativeFormat);
        return settings.value("AppsUseLightTheme", 1).toInt() == 0;
#elif defined(Q_OS_LINUX)
        return ShouldApplyDarkTheme();
#endif
#endif // QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto VTheme::ShouldApplyDarkTheme() -> bool
{
    QPalette const palette = qApp->palette(); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    return palette.color(QPalette::WindowText).lightness() > palette.color(QPalette::Window).lightness();
}

//---------------------------------------------------------------------------------------------------------------------
auto VTheme::ColorSheme() -> VColorSheme
{
    VThemeMode const themeMode = VAbstractApplication::VApp()->Settings()->GetThemeMode();

    if (themeMode == VThemeMode::Light)
    {
        return VColorSheme::Light;
    }

    if (themeMode == VThemeMode::Dark)
    {
        return VColorSheme::Dark;
    }

    if (NativeDarkThemeAvailable())
    {
        if (IsInDarkTheme())
        {
            return VColorSheme::Dark;
        }

        return VColorSheme::Light;
    }

    if (ShouldApplyDarkTheme())
    {
        return VColorSheme::Dark;
    }

    return VColorSheme::Light;
}

//---------------------------------------------------------------------------------------------------------------------
auto VTheme::DefaultThemeName() -> QString
{
    VColorSheme const colorScheme = ColorSheme();
    QString const themePrefix = (colorScheme == VColorSheme::Light ? QStringLiteral("Light") : QStringLiteral("Dark"));

#if defined(Q_OS_MACX)
    return QStringLiteral("La-Sierra-%1").arg(themePrefix);
#else
    return QStringLiteral("Eleven-%1").arg(themePrefix);
#endif
}

//---------------------------------------------------------------------------------------------------------------------
void VTheme::InitApplicationStyle()
{
    VThemeMode const themeMode = VAbstractApplication::VApp()->Settings()->GetThemeMode();

    if (themeMode == VThemeMode::Light || themeMode == VThemeMode::Dark)
    {
        QStyle *style = QStyleFactory::create(QStringLiteral("fusion"));
        if (style != nullptr)
        {
            style = new VApplicationStyle(style);
            QApplication::setStyle(style);
        }

        return;
    }

#if defined(Q_OS_WIN)
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if (NativeDarkThemeAvailable())
    {
        if (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark)
        {
            QApplication::setStyle(QStyleFactory::create(QStringLiteral("fusion")));
        }
    }
#endif
#endif
}

//---------------------------------------------------------------------------------------------------------------------
void VTheme::SetIconTheme()
{
    if (not HasThemeIcon(VThemeIcon::DocumentOpen))
    {
        // If there is no default working icon theme then we should
        // use an icon theme that we provide via a .qrc file
        // This case happens under Windows and Mac OS X
        // This does not happen under GNOME or KDE
        QIcon::setThemeName(DefaultThemeName());
    }
    else
    {
        VThemeMode const themeMode = VAbstractApplication::VApp()->Settings()->GetThemeMode();

        if ((themeMode == VThemeMode::Dark && !ShouldApplyDarkTheme()) ||
            (themeMode == VThemeMode::Light && ShouldApplyDarkTheme()))
        {
            QIcon::setThemeName(DefaultThemeName());
        }
        else
        {
            QIcon::setFallbackThemeName(VTheme::DefaultThemeName());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VTheme::SetToAutoTheme() const
{
// Fix for issue QTBUG-123722
#if defined(Q_OS_WIN) && QT_VERSION == QT_VERSION_CHECK(6, 7, 0)
    if (!IsInDarkTheme())
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
        qApp->setStyleSheet(QStringLiteral("QComboBox QAbstractItemView {background-color: white}"));
    }
#else
    qApp->setStyleSheet(QString()); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
#endif
    QIcon::setThemeName(m_defaultThemeName);
}

//---------------------------------------------------------------------------------------------------------------------
void VTheme::InitThemeMode()
{
    VThemeMode const themeMode = VAbstractApplication::VApp()->Settings()->GetThemeMode();

    if (themeMode == VThemeMode::Light)
    {
        if (NativeDarkThemeAvailable())
        {
            if (IsInDarkTheme())
            {
#if defined(Q_OS_MACX)
                NSMacSetToLightTheme();
#else
                ActivateCustomLightTheme();
#endif
            }
        }
        else if (ShouldApplyDarkTheme())
        {
            ActivateCustomLightTheme();
        }
    }
    else if (themeMode == VThemeMode::Dark)
    {
        if (NativeDarkThemeAvailable())
        {
            if (!IsInDarkTheme())
            {
#if defined(Q_OS_MACX)
                NSMacSetToDarkTheme();
#else
                ActivateCustomDarkTheme();
#endif
            }
        }
        else
        {
            ActivateCustomDarkTheme();
        }
    }
    else
    {
        if (NativeDarkThemeAvailable())
        {
            ActivateDefaultTheme();
        }
        else
        {
            if (ShouldApplyDarkTheme())
            {
                ActivateCustomDarkTheme();
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VTheme::ThemeStylesheet() -> QString
{
    VThemeMode const themeMode = VAbstractApplication::VApp()->Settings()->GetThemeMode();

    if (themeMode == VThemeMode::Light)
    {
        if (NativeDarkThemeAvailable())
        {
            if (IsInDarkTheme())
            {
#if defined(Q_OS_MACX)
                return QStringLiteral("native");
#else
                return QStringLiteral("light");
#endif
            }
        }
        else if (ShouldApplyDarkTheme())
        {
            return QStringLiteral("light");
        }

        return QStringLiteral("native");
    }

    if (themeMode == VThemeMode::Dark)
    {
        if (NativeDarkThemeAvailable())
        {
            if (!IsInDarkTheme())
            {
#if defined(Q_OS_MACX)
                return QStringLiteral("native");
#else
                return QStringLiteral("dark");
#endif
            }
        }
        else
        {
            return QStringLiteral("dark");
        }

        return QStringLiteral("native");
    }

    if (!NativeDarkThemeAvailable())
    {
        if (ShouldApplyDarkTheme())
        {
            return QStringLiteral("dark");
        }

        return QStringLiteral("light");
    }

    return QStringLiteral("native");
}

//---------------------------------------------------------------------------------------------------------------------
void VTheme::ResetThemeSettings() const
{
    InitApplicationStyle();
    SetToAutoTheme();
    SetIconTheme();
    InitThemeMode();
    VSceneStylesheet::ResetStyles();

    emit Instance()->ThemeSettingsChanged();
}

//---------------------------------------------------------------------------------------------------------------------
auto VTheme::GetFallbackThemeIcon(const QString &iconName, QSize iconSize) -> QIcon
{
    const QString themePrefix = (ColorSheme() == VColorSheme::Light ? QStringLiteral("Light") : QStringLiteral("Dark"));
    const auto themeName = QStringLiteral("Eleven-%1").arg(themePrefix);
    const auto filePath = QStringLiteral(":icons/%1/%2.svg").arg(themeName, iconName);

    QIcon icon;
    icon.addFile(filePath, iconSize, QIcon::Normal, QIcon::On);
    iconSize *= 2;
    icon.addFile(filePath, iconSize, QIcon::Normal, QIcon::On);
    return icon;
}

//---------------------------------------------------------------------------------------------------------------------
auto VTheme::GetIconResource(const QString &root, const QString &iconName) -> QIcon
{
    QIcon icon;
    bool const dark = (ColorSheme() == VColorSheme::Dark);
    QPixmap pixmap = ::GetPixmapResource(root, iconName, dark);
    icon.addPixmap(pixmap);
    if (dark)
    {
        // automatic disabled icon is no good for dark
        // paint transparent black to get disabled look
        QPainter p(&pixmap);
        p.fillRect(pixmap.rect(), QColor(48, 47, 47, 128));
        icon.addPixmap(pixmap, QIcon::Disabled);
    }
    return icon;
}

//---------------------------------------------------------------------------------------------------------------------
auto VTheme::GetPixmapResource(const QString &root, const QString &iconName) -> QPixmap
{
    bool const dark = (ColorSheme() == VColorSheme::Dark);
    return ::GetPixmapResource(root, iconName, dark);
}

//---------------------------------------------------------------------------------------------------------------------
auto VTheme::GetResourceName(const QString &root, const QString &iconName) -> QString
{
    return ::GetResourceName(root, iconName, ColorSheme() == VColorSheme::Dark);
}

//---------------------------------------------------------------------------------------------------------------------
VTheme::VTheme(QObject *parent)
  : QObject(parent)
{
    bool isProcessingColorSchemeChange = false;

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    auto colorSchemeChangedSlot = [this, &isProcessingColorSchemeChange]()
    {
        if (isProcessingColorSchemeChange)
        {
            return; // Already processing, avoid recursion
        }

        isProcessingColorSchemeChange = true;
        QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
        VThemeMode const themeMode = settings->GetThemeMode();
        if (themeMode == VThemeMode::System && VTheme::NativeDarkThemeAvailable())
        {
            if (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark)
            {
                settings->SetThemeMode(VThemeMode::Light);
            }
            else
            {
                settings->SetThemeMode(VThemeMode::Dark);
            }

            ResetThemeSettings();
            QCoreApplication::processEvents();
            settings->SetThemeMode(themeMode);
        }

        ResetThemeSettings();
        QGuiApplication::restoreOverrideCursor();

        isProcessingColorSchemeChange = false;
    };

    QStyleHints *hints = QGuiApplication::styleHints();
    connect(hints, &QStyleHints::colorSchemeChanged, this, colorSchemeChangedSlot);
#else
    if (VTheme::NativeDarkThemeAvailable())
    {
        m_darkTheme = IsInDarkTheme();
        m_themeTimer = new QTimer(this);
        m_themeTimer->setTimerType(Qt::VeryCoarseTimer);

        auto colorSchemeTimeoutCheck = [this, &isProcessingColorSchemeChange]()
        {
            if (isProcessingColorSchemeChange)
            {
                return; // Already processing, avoid recursion
            }

            isProcessingColorSchemeChange = true;
            QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

            if (bool darkTheme = IsInDarkTheme(); m_darkTheme != darkTheme)
            {
                m_darkTheme = darkTheme;
                ResetThemeSettings();
            }

            QGuiApplication::restoreOverrideCursor();
            isProcessingColorSchemeChange = false;
        };

        connect(m_themeTimer, &QTimer::timeout, this, colorSchemeTimeoutCheck);
        m_themeTimer->start(5s);
    }
#endif // QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
}
