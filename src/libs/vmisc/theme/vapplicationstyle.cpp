/************************************************************************
 **
 **  @file   vapplicationstyle.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   18 7, 2023
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

#include "vapplicationstyle.h"

#include <QFileInfo>
#include <QHash>
#include <QString>

#include "vtheme.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
auto StandardIconPaths() -> QHash<QStyle::StandardPixmap, QString>
{
    QHash<QStyle::StandardPixmap, QString> icons{
        {QStyle::SP_TitleBarMinButton, "minimize.svg"},
        {QStyle::SP_TitleBarMenuButton, "menu.svg"},
        {QStyle::SP_TitleBarMaxButton, "maximize.svg"},
        {QStyle::SP_TitleBarCloseButton, "window_close.svg"},
        {QStyle::SP_TitleBarNormalButton, "restore.svg"},
        {QStyle::SP_TitleBarShadeButton, "shade.svg"},
        {QStyle::SP_TitleBarUnshadeButton, "unshade.svg"},
        {QStyle::SP_TitleBarContextHelpButton, "help.svg"},
        {QStyle::SP_MessageBoxInformation, "message_information.svg"},
        {QStyle::SP_MessageBoxWarning, "message_warning.svg"},
        {QStyle::SP_MessageBoxCritical, "message_critical.svg"},
        {QStyle::SP_MessageBoxQuestion, "message_question.svg"},
        {QStyle::SP_DesktopIcon, "desktop.svg"},
        {QStyle::SP_TrashIcon, "trash.svg"},
        {QStyle::SP_ComputerIcon, "computer.svg"},
        {QStyle::SP_DriveFDIcon, "floppy_drive.svg"},
        {QStyle::SP_DriveHDIcon, "hard_drive.svg"},
        {QStyle::SP_DriveCDIcon, "disc_drive.svg"},
        {QStyle::SP_DriveDVDIcon, "disc_drive.svg"},
        {QStyle::SP_DriveNetIcon, "network_drive.svg"},
        {QStyle::SP_DirHomeIcon, "home_directory.svg"},
        {QStyle::SP_DirOpenIcon, "folder_open.svg"},
        {QStyle::SP_DirClosedIcon, "folder.svg"},
        {QStyle::SP_DirIcon, "folder.svg"},
        {QStyle::SP_DirLinkIcon, "folder_link.svg"},
        {QStyle::SP_DirLinkOpenIcon, "folder_open_link.svg"},
        {QStyle::SP_FileIcon, "file.svg"},
        {QStyle::SP_FileLinkIcon, "file_link.svg"},
        {QStyle::SP_FileDialogStart, "file_dialog_start.svg"},
        {QStyle::SP_FileDialogEnd, "file_dialog_end.svg"},
        {QStyle::SP_FileDialogToParent, "up_arrow.svg"},
        {QStyle::SP_FileDialogNewFolder, "folder.svg"},
        {QStyle::SP_FileDialogDetailedView, "file_dialog_detailed.svg"},
        {QStyle::SP_FileDialogInfoView, "file_dialog_info.svg"},
        {QStyle::SP_FileDialogContentsView, "file_dialog_contents.svg"},
        {QStyle::SP_FileDialogListView, "file_dialog_list.svg"},
        {QStyle::SP_FileDialogBack, "left_arrow.svg"},
        {QStyle::SP_DockWidgetCloseButton, "close.svg"},
        {QStyle::SP_ToolBarHorizontalExtensionButton, "horizontal_extension.svg"},
        {QStyle::SP_ToolBarVerticalExtensionButton, "vertical_extension.svg"},
        {QStyle::SP_DialogOkButton, "dialog_ok.svg"},
        {QStyle::SP_DialogCancelButton, "dialog_cancel.svg"},
        {QStyle::SP_DialogHelpButton, "dialog_help.svg"},
        {QStyle::SP_DialogOpenButton, "dialog_open.svg"},
        {QStyle::SP_DialogSaveButton, "dialog_save.svg"},
        {QStyle::SP_DialogCloseButton, "dialog_close.svg"},
        {QStyle::SP_DialogApplyButton, "dialog_apply.svg"},
        {QStyle::SP_DialogResetButton, "dialog_reset.svg"},
        {QStyle::SP_DialogDiscardButton, "dialog_discard.svg"},
        {QStyle::SP_DialogYesButton, "dialog_apply.svg"},
        {QStyle::SP_DialogNoButton, "dialog_no.svg"},
        {QStyle::SP_ArrowUp, "up_arrow.svg"},
        {QStyle::SP_ArrowDown, "down_arrow.svg"},
        {QStyle::SP_ArrowLeft, "left_arrow.svg"},
        {QStyle::SP_ArrowRight, "right_arrow.svg"},
        {QStyle::SP_ArrowBack, "left_arrow.svg"},
        {QStyle::SP_ArrowForward, "right_arrow.svg"},
        {QStyle::SP_CommandLink, "right_arrow.svg"},
        {QStyle::SP_VistaShield, "vista_shield.svg"},
        {QStyle::SP_BrowserReload, "browser_refresh.svg"},
        {QStyle::SP_BrowserStop, "browser_refresh_stop.svg"},
        {QStyle::SP_MediaPlay, "play.svg"},
        {QStyle::SP_MediaStop, "stop.svg"},
        {QStyle::SP_MediaPause, "pause.svg"},
        {QStyle::SP_MediaSkipForward, "skip_backward.svg"},
        {QStyle::SP_MediaSkipBackward, "skip_forward.svg"},
        {QStyle::SP_MediaSeekForward, "seek_forward.svg"},
        {QStyle::SP_MediaSeekBackward, "seek_backward.svg"},
        {QStyle::SP_MediaVolume, "volume.svg"},
        {QStyle::SP_MediaVolumeMuted, "volume_muted.svg"},
        {QStyle::SP_LineEditClearButton, "clear_text.svg"},
    };

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    icons.insert(QStyle::SP_DialogYesToAllButton, QStringLiteral("dialog_yes_to_all.svg"));
    icons.insert(QStyle::SP_DialogNoToAllButton, QStringLiteral("dialog_no.svg"));
    icons.insert(QStyle::SP_DialogSaveAllButton, QStringLiteral("dialog_save_all.svg"));
    icons.insert(QStyle::SP_DialogAbortButton, QStringLiteral("dialog_cancel.svg"));
    icons.insert(QStyle::SP_DialogRetryButton, QStringLiteral("dialog_retry.svg"));
    icons.insert(QStyle::SP_DialogIgnoreButton, QStringLiteral("dialog_ignore.svg"));
    icons.insert(QStyle::SP_RestoreDefaultsButton, QStringLiteral("restore_defaults.svg"));
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
    icons.insert(QStyle::SP_TabCloseButton, QStringLiteral("tab_close.svg"));
#endif

    return icons;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VApplicationStyle::VApplicationStyle(QStyle *style)
  : m_style(style)
{
    m_style->setParent(this);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 1, 0)
//---------------------------------------------------------------------------------------------------------------------
auto VApplicationStyle::name() const -> QString
{
    return m_style->name();
}
#endif

//---------------------------------------------------------------------------------------------------------------------
void VApplicationStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w) const
{
    m_style->drawPrimitive(pe, opt, p, w);
}

//---------------------------------------------------------------------------------------------------------------------
void VApplicationStyle::drawControl(ControlElement element, const QStyleOption *opt, QPainter *p,
                                    const QWidget *w) const
{
    m_style->drawControl(element, opt, p, w);
}

//---------------------------------------------------------------------------------------------------------------------
auto VApplicationStyle::subElementRect(SubElement subElement, const QStyleOption *option, const QWidget *widget) const
    -> QRect
{
    return m_style->subElementRect(subElement, option, widget);
}

//---------------------------------------------------------------------------------------------------------------------
void VApplicationStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                                           const QWidget *widget) const
{
    m_style->drawComplexControl(cc, opt, p, widget);
}

//---------------------------------------------------------------------------------------------------------------------
auto VApplicationStyle::hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, const QPoint &pt,
                                              const QWidget *widget) const -> SubControl
{
    return m_style->hitTestComplexControl(cc, opt, pt, widget);
}

//---------------------------------------------------------------------------------------------------------------------
auto VApplicationStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc,
                                       const QWidget *widget) const -> QRect
{
    return m_style->subControlRect(cc, opt, sc, widget);
}

//---------------------------------------------------------------------------------------------------------------------
auto VApplicationStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const -> int
{
    return m_style->pixelMetric(metric, option, widget);
}

//---------------------------------------------------------------------------------------------------------------------
auto VApplicationStyle::sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &contentsSize,
                                         const QWidget *w) const -> QSize
{
    return m_style->sizeFromContents(ct, opt, contentsSize, w);
}

//---------------------------------------------------------------------------------------------------------------------
auto VApplicationStyle::styleHint(StyleHint stylehint, const QStyleOption *opt, const QWidget *widget,
                                  QStyleHintReturn *returnData) const -> int
{
    return m_style->styleHint(stylehint, opt, widget, returnData);
}

//---------------------------------------------------------------------------------------------------------------------
auto VApplicationStyle::standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt,
                                       const QWidget *widget) const -> QPixmap
{
    return m_style->standardPixmap(standardPixmap, opt, widget);
}

//---------------------------------------------------------------------------------------------------------------------
auto VApplicationStyle::standardIcon(StandardPixmap standardIcon, const QStyleOption *option,
                                     const QWidget *widget) const -> QIcon
{
    return StyleIcon(standardIcon, option, widget);
}

//---------------------------------------------------------------------------------------------------------------------
auto VApplicationStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const
    -> QPixmap
{
    return m_style->generatedIconPixmap(iconMode, pixmap, opt);
}

//---------------------------------------------------------------------------------------------------------------------
auto VApplicationStyle::layoutSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2,
                                      Qt::Orientation orientation, const QStyleOption *option,
                                      const QWidget *widget) const -> int
{
    return m_style->layoutSpacing(control1, control2, orientation, option, widget);
}

//---------------------------------------------------------------------------------------------------------------------
auto VApplicationStyle::StyleIcon(StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const
    -> QIcon
{
    if (VTheme::ThemeStylesheet() == "native"_L1)
    {
        return m_style->standardIcon(standardIcon, option, widget);
    }

    return StylesheetIcon(standardIcon, option, widget);
}

//---------------------------------------------------------------------------------------------------------------------
auto VApplicationStyle::StylesheetIcon(StandardPixmap standardIcon, const QStyleOption *option,
                                       const QWidget *widget) const -> QIcon
{
    static QHash<QStyle::StandardPixmap, QString> icons = StandardIconPaths();
    QString resource = QStringLiteral("://%1/%2").arg(VTheme::ThemeStylesheet(), icons.value(standardIcon));
    if (QFileInfo::exists(resource))
    {
        return QIcon(resource);
    }

    return m_style->standardIcon(standardIcon, option, widget);
}
