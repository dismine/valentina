/************************************************************************
 **
 **  @file   vtheme.h
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
#ifndef VTHEME_H
#define VTHEME_H

#include <QObject>
#include <QString>
#include <QTimer>

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../defglobal.h"
#endif

enum class VColorSheme
{
    Light,
    Dark
};

class QStyle;

class VTheme : public QObject
{
    Q_OBJECT // NOLINT

public:
    ~VTheme() override = default;

    static auto Instance() -> VTheme *;

    void StoreDefaultThemeName(const QString &themeName);

    void SetToAutoTheme() const;
    void ResetThemeSettings() const;

    static auto GetFallbackThemeIcon(const QString &iconName, QSize iconSize) -> QIcon;
    static auto GetIconResource(const QString &root, const QString &iconName) -> QIcon;
    static auto GetPixmapResource(const QString &root, const QString &iconName) -> QPixmap;
    static auto GetResourceName(const QString &root, const QString &iconName) -> QString;

    static auto NativeDarkThemeAvailable() -> bool;
    static auto IsInDarkTheme() -> bool;
    static auto ShouldApplyDarkTheme() -> bool;
    static auto ColorSheme() -> VColorSheme;
    static auto DefaultThemeName() -> QString;
    static void InitApplicationStyle();
    static void SetIconTheme();
    static void InitThemeMode();
    static auto ThemeStylesheet() -> QString;

signals:
    void ThemeSettingsChanged();

private:
    Q_DISABLE_COPY_MOVE(VTheme) // NOLINT

    explicit VTheme(QObject *parent = nullptr);

    QString m_defaultThemeName{};
    QStyle *m_defaultApplicationStyle{nullptr};
#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
    QTimer *m_themeTimer{nullptr};
    bool m_darkTheme{false};
#endif

    auto GetDefaultApplicationStyle() const -> QStyle *;
    void SetDefaultApplicationStyle(QStyle *defaultApplicationStyle);
};

#endif // VTHEME_H
