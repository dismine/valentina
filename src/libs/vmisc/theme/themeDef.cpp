/************************************************************************
 **
 **  @file   themeDef.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 4, 2024
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2024 Valentina project
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

#include "themeDef.h"

#include <QHash>
#include <QIcon>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
using ThemeIconHash = QHash<VThemeIcon, QIcon::ThemeIcon>;
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const ThemeIconHash,
                          themeIconMapping,
                          ({
                              // {VThemeIcon::AddressBookNew, QIcon::ThemeIcon::AddressBookNew},
                              {VThemeIcon::ApplicationExit, QIcon::ThemeIcon::ApplicationExit},
                              // {VThemeIcon::AppointmentNew, QIcon::ThemeIcon::AppointmentNew},
                              // {VThemeIcon::CallStart, QIcon::ThemeIcon::CallStart},
                              // {VThemeIcon::CallStop, QIcon::ThemeIcon::CallStop},
                              // {VThemeIcon::ContactNew, QIcon::ThemeIcon::ContactNew},
                              {VThemeIcon::DocumentNew, QIcon::ThemeIcon::DocumentNew},
                              {VThemeIcon::DocumentOpen, QIcon::ThemeIcon::DocumentOpen},
                              // {VThemeIcon::DocumentOpenRecent, QIcon::ThemeIcon::DocumentOpenRecent},
                              // {VThemeIcon::DocumentPageSetup, QIcon::ThemeIcon::DocumentPageSetup},
                              {VThemeIcon::DocumentPrint, QIcon::ThemeIcon::DocumentPrint},
                              {VThemeIcon::DocumentPrintPreview, QIcon::ThemeIcon::DocumentPrintPreview},
                              {VThemeIcon::DocumentProperties, QIcon::ThemeIcon::DocumentProperties},
                              // {VThemeIcon::DocumentRevert, QIcon::ThemeIcon::DocumentRevert},
                              {VThemeIcon::DocumentSave, QIcon::ThemeIcon::DocumentSave},
                              {VThemeIcon::DocumentSaveAs, QIcon::ThemeIcon::DocumentSaveAs},
                              // {VThemeIcon::DocumentSend, QIcon::ThemeIcon::DocumentSend},
                              {VThemeIcon::EditClear, QIcon::ThemeIcon::EditClear},
                              // {VThemeIcon::EditCopy, QIcon::ThemeIcon::EditCopy},
                              // {VThemeIcon::EditCut, QIcon::ThemeIcon::EditCut},
                              {VThemeIcon::EditDelete, QIcon::ThemeIcon::EditDelete},
                              // {VThemeIcon::EditFind, QIcon::ThemeIcon::EditFind},
                              // {VThemeIcon::EditPaste, QIcon::ThemeIcon::EditPaste},
                              {VThemeIcon::EditRedo, QIcon::ThemeIcon::EditRedo},
                              // {VThemeIcon::EditSelectAll, QIcon::ThemeIcon::EditSelectAll},
                              {VThemeIcon::EditUndo, QIcon::ThemeIcon::EditUndo},
                              // {VThemeIcon::FolderNew, QIcon::ThemeIcon::FolderNew},
                              // {VThemeIcon::FormatIndentLess, QIcon::ThemeIcon::FormatIndentLess},
                              // {VThemeIcon::FormatIndentMore, QIcon::ThemeIcon::FormatIndentMore},
                              {VThemeIcon::FormatJustifyCenter, QIcon::ThemeIcon::FormatJustifyCenter},
                              // {VThemeIcon::FormatJustifyFill, QIcon::ThemeIcon::FormatJustifyFill},
                              {VThemeIcon::FormatJustifyLeft, QIcon::ThemeIcon::FormatJustifyLeft},
                              {VThemeIcon::FormatJustifyRight, QIcon::ThemeIcon::FormatJustifyRight},
                              // {VThemeIcon::FormatTextDirectionLtr, QIcon::ThemeIcon::FormatTextDirectionLtr},
                              // {VThemeIcon::FormatTextDirectionRtl, QIcon::ThemeIcon::FormatTextDirectionRtl},
                              {VThemeIcon::FormatTextBold, QIcon::ThemeIcon::FormatTextBold},
                              {VThemeIcon::FormatTextItalic, QIcon::ThemeIcon::FormatTextItalic},
                              // {VThemeIcon::FormatTextUnderline, QIcon::ThemeIcon::FormatTextUnderline},
                              {VThemeIcon::FormatTextStrikethrough, QIcon::ThemeIcon::FormatTextStrikethrough},
                              {VThemeIcon::GoDown, QIcon::ThemeIcon::GoDown},
                              // {VThemeIcon::GoHome, QIcon::ThemeIcon::GoHome},
                              {VThemeIcon::GoNext, QIcon::ThemeIcon::GoNext},
                              // {VThemeIcon::GoPrevious, QIcon::ThemeIcon::GoPrevious},
                              {VThemeIcon::GoUp, QIcon::ThemeIcon::GoUp},
                              {VThemeIcon::HelpAbout, QIcon::ThemeIcon::HelpAbout},
                              // {VThemeIcon::HelpFaq, QIcon::ThemeIcon::HelpFaq},
                              // {VThemeIcon::InsertImage, QIcon::ThemeIcon::InsertImage},
                              // {VThemeIcon::InsertLink, QIcon::ThemeIcon::InsertLink},
                              // {VThemeIcon::InsertText, QIcon::ThemeIcon::InsertText},
                              {VThemeIcon::ListAdd, QIcon::ThemeIcon::ListAdd},
                              {VThemeIcon::ListRemove, QIcon::ThemeIcon::ListRemove},
                              // {VThemeIcon::MailForward, QIcon::ThemeIcon::MailForward},
                              // {VThemeIcon::MailMarkImportant, QIcon::ThemeIcon::MailMarkImportant},
                              // {VThemeIcon::MailMarkRead, QIcon::ThemeIcon::MailMarkRead},
                              // {VThemeIcon::MailMarkUnread, QIcon::ThemeIcon::MailMarkUnread},
                              // {VThemeIcon::MailMessageNew, QIcon::ThemeIcon::MailMessageNew},
                              // {VThemeIcon::MailReplyAll, QIcon::ThemeIcon::MailReplyAll},
                              // {VThemeIcon::MailReplySender, QIcon::ThemeIcon::MailReplySender},
                              // {VThemeIcon::MailSend, QIcon::ThemeIcon::MailSend},
                              // {VThemeIcon::MediaEject, QIcon::ThemeIcon::MediaEject},
                              // {VThemeIcon::MediaPlaybackPause, QIcon::ThemeIcon::MediaPlaybackPause},
                              // {VThemeIcon::MediaPlaybackStart, QIcon::ThemeIcon::MediaPlaybackStart},
                              // {VThemeIcon::MediaPlaybackStop, QIcon::ThemeIcon::MediaPlaybackStop},
                              // {VThemeIcon::MediaRecord, QIcon::ThemeIcon::MediaRecord},
                              // {VThemeIcon::MediaSeekBackward, QIcon::ThemeIcon::MediaSeekBackward},
                              // {VThemeIcon::MediaSeekForward, QIcon::ThemeIcon::MediaSeekForward},
                              // {VThemeIcon::MediaSkipBackward, QIcon::ThemeIcon::MediaSkipBackward},
                              // {VThemeIcon::MediaSkipForward, QIcon::ThemeIcon::MediaSkipForward},
                              {VThemeIcon::ObjectRotateLeft, QIcon::ThemeIcon::ObjectRotateLeft},
                              {VThemeIcon::ObjectRotateRight, QIcon::ThemeIcon::ObjectRotateRight},
                              // {VThemeIcon::ProcessStop, QIcon::ThemeIcon::ProcessStop},
                              // {VThemeIcon::SystemLockScreen, QIcon::ThemeIcon::SystemLockScreen},
                              // {VThemeIcon::SystemLogOut, QIcon::ThemeIcon::SystemLogOut},
                              // {VThemeIcon::SystemSearch, QIcon::ThemeIcon::SystemSearch},
                              // {VThemeIcon::SystemReboot, QIcon::ThemeIcon::SystemReboot},
                              // {VThemeIcon::SystemShutdown, QIcon::ThemeIcon::SystemShutdown},
                              // {VThemeIcon::ToolsCheckSpelling, QIcon::ThemeIcon::ToolsCheckSpelling},
                              // {VThemeIcon::ViewFullscreen, QIcon::ThemeIcon::ViewFullscreen},
                              {VThemeIcon::ViewRefresh, QIcon::ThemeIcon::ViewRefresh},
                              // {VThemeIcon::ViewRestore, QIcon::ThemeIcon::ViewRestore},
                              // {VThemeIcon::WindowClose, QIcon::ThemeIcon::WindowClose},
                              // {VThemeIcon::WindowNew, QIcon::ThemeIcon::WindowNew},
                              {VThemeIcon::ZoomFitBest, QIcon::ThemeIcon::ZoomFitBest},
                              {VThemeIcon::ZoomIn, QIcon::ThemeIcon::ZoomIn},
                              {VThemeIcon::ZoomOut, QIcon::ThemeIcon::ZoomOut},

                              // {VThemeIcon::AudioCard, QIcon::ThemeIcon::AudioCard},
                              // {VThemeIcon::AudioInputMicrophone, QIcon::ThemeIcon::AudioInputMicrophone},
                              // {VThemeIcon::Battery, QIcon::ThemeIcon::Battery},
                              // {VThemeIcon::CameraPhoto, QIcon::ThemeIcon::CameraPhoto},
                              // {VThemeIcon::CameraVideo, QIcon::ThemeIcon::CameraVideo},
                              // {VThemeIcon::CameraWeb, QIcon::ThemeIcon::CameraWeb},
                              // {VThemeIcon::Computer, QIcon::ThemeIcon::Computer},
                              // {VThemeIcon::DriveHarddisk, QIcon::ThemeIcon::DriveHarddisk},
                              // {VThemeIcon::DriveOptical, QIcon::ThemeIcon::DriveOptical},
                              // {VThemeIcon::InputGaming, QIcon::ThemeIcon::InputGaming},
                              // {VThemeIcon::InputKeyboard, QIcon::ThemeIcon::InputKeyboard},
                              // {VThemeIcon::InputMouse, QIcon::ThemeIcon::InputMouse},
                              // {VThemeIcon::InputTablet, QIcon::ThemeIcon::InputTablet},
                              // {VThemeIcon::MediaFlash, QIcon::ThemeIcon::MediaFlash},
                              // {VThemeIcon::MediaOptical, QIcon::ThemeIcon::MediaOptical},
                              // {VThemeIcon::MediaTape, QIcon::ThemeIcon::MediaTape},
                              // {VThemeIcon::MultimediaPlayer, QIcon::ThemeIcon::MultimediaPlayer},
                              // {VThemeIcon::NetworkWired, QIcon::ThemeIcon::NetworkWired},
                              // {VThemeIcon::NetworkWireless, QIcon::ThemeIcon::NetworkWireless},
                              // {VThemeIcon::Phone, QIcon::ThemeIcon::Phone},
                              // {VThemeIcon::Printer, QIcon::ThemeIcon::Printer},
                              // {VThemeIcon::Scanner, QIcon::ThemeIcon::Scanner},
                              // {VThemeIcon::VideoDisplay, QIcon::ThemeIcon::VideoDisplay},

                              // {VThemeIcon::AppointmentMissed, QIcon::ThemeIcon::AppointmentMissed},
                              // {VThemeIcon::AppointmentSoon, QIcon::ThemeIcon::AppointmentSoon},
                              // {VThemeIcon::AudioVolumeHigh, QIcon::ThemeIcon::AudioVolumeHigh},
                              // {VThemeIcon::AudioVolumeLow, QIcon::ThemeIcon::AudioVolumeLow},
                              // {VThemeIcon::AudioVolumeMedium, QIcon::ThemeIcon::AudioVolumeMedium},
                              // {VThemeIcon::AudioVolumeMuted, QIcon::ThemeIcon::AudioVolumeMuted},
                              // {VThemeIcon::BatteryCaution, QIcon::ThemeIcon::BatteryCaution},
                              // {VThemeIcon::BatteryLow, QIcon::ThemeIcon::BatteryLow},
                              // {VThemeIcon::DialogError, QIcon::ThemeIcon::DialogError},
                              // {VThemeIcon::DialogInformation, QIcon::ThemeIcon::DialogInformation},
                              // {VThemeIcon::DialogPassword, QIcon::ThemeIcon::DialogPassword},
                              // {VThemeIcon::DialogQuestion, QIcon::ThemeIcon::DialogQuestion},
                              {VThemeIcon::DialogWarning, QIcon::ThemeIcon::DialogWarning},
                              // {VThemeIcon::FolderDragAccept, QIcon::ThemeIcon::FolderDragAccept},
                              // {VThemeIcon::FolderOpen, QIcon::ThemeIcon::FolderOpen},
                              // {VThemeIcon::FolderVisiting, QIcon::ThemeIcon::FolderVisiting},
                              // {VThemeIcon::ImageLoading, QIcon::ThemeIcon::ImageLoading},
                              // {VThemeIcon::ImageMissing, QIcon::ThemeIcon::ImageMissing},
                              // {VThemeIcon::MailAttachment, QIcon::ThemeIcon::MailAttachment},
                              // {VThemeIcon::MailUnread, QIcon::ThemeIcon::MailUnread},
                              // {VThemeIcon::MailRead, QIcon::ThemeIcon::MailRead},
                              // {VThemeIcon::MailReplied, QIcon::ThemeIcon::MailReplied},
                              // {VThemeIcon::MediaPlaylistRepeat, QIcon::ThemeIcon::MediaPlaylistRepeat},
                              // {VThemeIcon::MediaPlaylistShuffle, QIcon::ThemeIcon::MediaPlaylistShuffle},
                              // {VThemeIcon::NetworkOffline, QIcon::ThemeIcon::NetworkOffline},
                              // {VThemeIcon::PrinterPrinting, QIcon::ThemeIcon::PrinterPrinting},
                              // {VThemeIcon::SecurityHigh, QIcon::ThemeIcon::SecurityHigh},
                              // {VThemeIcon::SecurityLow, QIcon::ThemeIcon::SecurityLow},
                              // {VThemeIcon::SoftwareUpdateAvailable, QIcon::ThemeIcon::SoftwareUpdateAvailable},
                              // {VThemeIcon::SoftwareUpdateUrgent, QIcon::ThemeIcon::SoftwareUpdateUrgent},
                              // {VThemeIcon::SyncError, QIcon::ThemeIcon::SyncError},
                              // {VThemeIcon::SyncSynchronizing, QIcon::ThemeIcon::SyncSynchronizing},
                              // {VThemeIcon::UserAvailable, QIcon::ThemeIcon::UserAvailable},
                              // {VThemeIcon::UserOffline, QIcon::ThemeIcon::UserOffline},
                              // {VThemeIcon::WeatherClear, QIcon::ThemeIcon::WeatherClear},
                              // {VThemeIcon::WeatherClearNight, QIcon::ThemeIcon::WeatherClearNight},
                              // {VThemeIcon::WeatherFewClouds, QIcon::ThemeIcon::WeatherFewClouds},
                              // {VThemeIcon::WeatherFewCloudsNight, QIcon::ThemeIcon::WeatherFewCloudsNight},
                              // {VThemeIcon::WeatherFog, QIcon::ThemeIcon::WeatherFog},
                              // {VThemeIcon::WeatherShowers, QIcon::ThemeIcon::WeatherShowers},
                              // {VThemeIcon::WeatherSnow, QIcon::ThemeIcon::WeatherSnow},
                              // {VThemeIcon::WeatherStorm, QIcon::ThemeIcon::WeatherStorm}
                          }))
#endif
using ThemeIconStringHash = QHash<VThemeIcon, QString>;
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const ThemeIconStringHash,
                          themeIconStringMapping,
                          ({
                              // {VThemeIcon::AddressBookNew, "address-book-new"_L1},
                              {VThemeIcon::ApplicationExit, "application-exit"_L1},
                              // {VThemeIcon::AppointmentNew, "appointment-new"_L1},
                              // {VThemeIcon::CallStart, "call-start"_L1},
                              // {VThemeIcon::CallStop, "call-stop"_L1},
                              // {VThemeIcon::ContactNew, "contact-new"_L1},
                              {VThemeIcon::DocumentNew, "document-new"_L1},
                              {VThemeIcon::DocumentOpen, "document-open"_L1},
                              // {VThemeIcon::DocumentOpenRecent, "document-open-recent"_L1},
                              // {VThemeIcon::DocumentPageSetup, "document-page-setup"_L1},
                              {VThemeIcon::DocumentPrint, "document-print"_L1},
                              {VThemeIcon::DocumentPrintPreview, "document-print-preview"_L1},
                              {VThemeIcon::DocumentProperties, "document-properties"_L1},
                              // {VThemeIcon::DocumentRevert, "document-revert"_L1},
                              {VThemeIcon::DocumentSave, "document-save"_L1},
                              {VThemeIcon::DocumentSaveAs, "document-save-as"_L1},
                              // {VThemeIcon::DocumentSend, "document-send"_L1},
                              {VThemeIcon::EditClear, "edit-clear"_L1},
                              // {VThemeIcon::EditCopy, "edit-copy"_L1},
                              // {VThemeIcon::EditCut, "edit-cut"_L1},
                              {VThemeIcon::EditDelete, "edit-delete"_L1},
                              // {VThemeIcon::EditFind, "edit-find"_L1},
                              // {VThemeIcon::EditPaste, "edit-paste"_L1},
                              {VThemeIcon::EditRedo, "edit-redo"_L1},
                              // {VThemeIcon::EditSelectAll, "edit-select-all"_L1},
                              {VThemeIcon::EditUndo, "edit-undo"_L1},
                              // {VThemeIcon::FolderNew, "folder-new"_L1},
                              // {VThemeIcon::FormatIndentLess, "format-indent-less"_L1},
                              // {VThemeIcon::FormatIndentMore, "format-indent-more"_L1},
                              {VThemeIcon::FormatJustifyCenter, "format-justify-center"_L1},
                              // {VThemeIcon::FormatJustifyFill, "format-justify-fill"_L1},
                              {VThemeIcon::FormatJustifyLeft, "format-justify-left"_L1},
                              {VThemeIcon::FormatJustifyRight, "format-justify-right"_L1},
                              // {VThemeIcon::FormatTextDirectionLtr, "format-text-direction-ltr"_L1},
                              // {VThemeIcon::FormatTextDirectionRtl, "format-text-direction-rtl"_L1},
                              {VThemeIcon::FormatTextBold, "format-text-bold"_L1},
                              {VThemeIcon::FormatTextItalic, "format-text-italic"_L1},
                              // {VThemeIcon::FormatTextUnderline, "format-text-underline"_L1},
                              {VThemeIcon::FormatTextStrikethrough, "format-text-strikethrough"_L1},
                              {VThemeIcon::GoDown, "go-down"_L1},
                              // {VThemeIcon::GoHome, "go-home"_L1},
                              {VThemeIcon::GoNext, "go-next"_L1},
                              // {VThemeIcon::GoPrevious, "go-previous"_L1},
                              {VThemeIcon::GoUp, "go-up"_L1},
                              {VThemeIcon::HelpAbout, "help-about"_L1},
                              // {VThemeIcon::HelpFaq, "help-faq"_L1},
                              // {VThemeIcon::InsertImage, "insert-image"_L1},
                              // {VThemeIcon::InsertLink, "insert-link"_L1},
                              // {VThemeIcon::InsertText, "insert-text"_L1},
                              {VThemeIcon::ListAdd, "list-add"_L1},
                              {VThemeIcon::ListRemove, "list-remove"_L1},
                              // {VThemeIcon::MailForward, "mail-forward"_L1},
                              // {VThemeIcon::MailMarkImportant, "mail-mark-important"_L1},
                              // {VThemeIcon::MailMarkRead, "mail-mark-read"_L1},
                              // {VThemeIcon::MailMarkUnread, "mail-mark-unread"_L1},
                              // {VThemeIcon::MailMessageNew, "mail-message-new"_L1},
                              // {VThemeIcon::MailReplyAll, "mail-reply-all"_L1},
                              // {VThemeIcon::MailReplySender, "mail-reply-sender"_L1},
                              // {VThemeIcon::MailSend, "mail-send"_L1},
                              // {VThemeIcon::MediaEject, "media-eject"_L1},
                              // {VThemeIcon::MediaPlaybackPause, "media-playback-pause"_L1},
                              // {VThemeIcon::MediaPlaybackStart, "media-playback-start"_L1},
                              // {VThemeIcon::MediaPlaybackStop, "media-playback-stop"_L1},
                              // {VThemeIcon::MediaRecord, "media-record"_L1},
                              // {VThemeIcon::MediaSeekBackward, "media-seek-backward"_L1},
                              // {VThemeIcon::MediaSeekForward, "media-seek-forward"_L1},
                              // {VThemeIcon::MediaSkipBackward, "media-skip-backward"_L1},
                              // {VThemeIcon::MediaSkipForward, "media-skip-forward"_L1},
                              {VThemeIcon::ObjectRotateLeft, "object-rotate-left"_L1},
                              {VThemeIcon::ObjectRotateRight, "object-rotate-right"_L1},
                              // {VThemeIcon::ProcessStop, "process-stop"_L1},
                              // {VThemeIcon::SystemLockScreen, "system-lock-screen"_L1},
                              // {VThemeIcon::SystemLogOut, "system-log-out"_L1},
                              {VThemeIcon::SystemSearch, "system-search"_L1},
                              // {VThemeIcon::SystemReboot, "system-reboot"_L1},
                              // {VThemeIcon::SystemShutdown, "system-shutdown"_L1},
                              // {VThemeIcon::ToolsCheckSpelling, "tools-check-spelling"_L1},
                              // {VThemeIcon::ViewFullscreen, "view-fullscreen"_L1},
                              {VThemeIcon::ViewRefresh, "view-refresh"_L1},
                              // {VThemeIcon::ViewRestore, "view-restore"_L1},
                              // {VThemeIcon::WindowClose, "window-close"_L1},
                              // {VThemeIcon::WindowNew, "window-new"_L1},
                              {VThemeIcon::ZoomFitBest, "zoom-fit-best"_L1},
                              {VThemeIcon::ZoomIn, "zoom-in"_L1},
                              {VThemeIcon::ZoomOut, "zoom-out"_L1},

                              // {VThemeIcon::AudioCard, "audio-card"_L1},
                              // {VThemeIcon::AudioInputMicrophone, "audio-input-microphone"_L1},
                              // {VThemeIcon::Battery, "battery"_L1},
                              // {VThemeIcon::CameraPhoto, "camera-photo"_L1},
                              // {VThemeIcon::CameraVideo, "camera-video"_L1},
                              // {VThemeIcon::CameraWeb, "camera-web"_L1},
                              // {VThemeIcon::Computer, "computer"_L1},
                              // {VThemeIcon::DriveHarddisk, "drive-harddisk"},
                              // {VThemeIcon::DriveOptical, "drive-optical"_L1},
                              // {VThemeIcon::InputGaming, "input-gaming"_L1},
                              // {VThemeIcon::InputKeyboard, "input-keyboard"_L1},
                              // {VThemeIcon::InputMouse, "input-mouse"_L1},
                              // {VThemeIcon::InputTablet, "input-tablet"_L1},
                              // {VThemeIcon::MediaFlash, "media-flash"_L1},
                              // {VThemeIcon::MediaOptical, "media-optical"_L1},
                              // {VThemeIcon::MediaTape, "media-tape"_L1},
                              // {VThemeIcon::MultimediaPlayer, "multimedia-player"_L1},
                              // {VThemeIcon::NetworkWired, "network-wired"_L1},
                              // {VThemeIcon::NetworkWireless, "network-wireless"_L1},
                              // {VThemeIcon::Phone, "phone"_L1},
                              // {VThemeIcon::Printer, "printer"_L1},
                              // {VThemeIcon::Scanner, "scanner"_L1},
                              // {VThemeIcon::VideoDisplay, "video-display"_L1},

                              // {VThemeIcon::AppointmentMissed, "appointment-missed"_L1},
                              // {VThemeIcon::AppointmentSoon, "appointment-soon"_L1},
                              // {VThemeIcon::AudioVolumeHigh, "audio-volume-high"_L1},
                              // {VThemeIcon::AudioVolumeLow, "audio-volume-low"_L1},
                              // {VThemeIcon::AudioVolumeMedium, "audio-volume-medium"_L1},
                              // {VThemeIcon::AudioVolumeMuted, "audio-volume-muted"_L1},
                              // {VThemeIcon::BatteryCaution, "battery-caution"_L1},
                              // {VThemeIcon::BatteryLow, "battery-low"_L1},
                              // {VThemeIcon::DialogError, "dialog-error"_L1},
                              // {VThemeIcon::DialogInformation, "dialog-information"_L1},
                              // {VThemeIcon::DialogPassword, "dialog-password"_L1},
                              // {VThemeIcon::DialogQuestion, "dialog-question"_L1},
                              {VThemeIcon::DialogWarning, "dialog-warning"_L1},
                              // {VThemeIcon::FolderDragAccept, "folder-drag-accept"_L1},
                              // {VThemeIcon::FolderOpen, "folder-open"_L1},
                              // {VThemeIcon::FolderVisiting, "folder-visiting"_L1},
                              // {VThemeIcon::ImageLoading, "image-loading"_L1},
                              // {VThemeIcon::ImageMissing, "image-missing"_L1},
                              // {VThemeIcon::MailAttachment, "mail-attachment"_L1},
                              // {VThemeIcon::MailUnread, "mail-unread"_L1},
                              // {VThemeIcon::MailRead, "mail-read"_L1},
                              // {VThemeIcon::MailReplied, "mail-replied"_L1},
                              // {VThemeIcon::MediaPlaylistRepeat, "media-playlist-repeat"_L1},
                              // {VThemeIcon::MediaPlaylistShuffle, "media-playlist-shuffle"_L1},
                              // {VThemeIcon::NetworkOffline, "network-offline"_L1},
                              // {VThemeIcon::PrinterPrinting, "printer-printing"_L1},
                              // {VThemeIcon::SecurityHigh, "security-high"_L1},
                              // {VThemeIcon::SecurityLow, "security-low"_L1},
                              // {VThemeIcon::SoftwareUpdateAvailable, "software-update-available"_L1},
                              // {VThemeIcon::SoftwareUpdateUrgent, "software-update-urgent"_L1},
                              // {VThemeIcon::SyncError, "sync-error"_L1},
                              // {VThemeIcon::SyncSynchronizing, "sync-synchronizing"_L1},
                              // {VThemeIcon::UserAvailable, "user-available"_L1},
                              // {VThemeIcon::UserOffline, "user-offline"_L1},
                              // {VThemeIcon::WeatherClear, "weather-clear"_L1},
                              // {VThemeIcon::WeatherClearNight, "weather-clear-night"_L1},
                              // {VThemeIcon::WeatherFewClouds, "weather-few-clouds"_L1},
                              // {VThemeIcon::WeatherFewCloudsNight, "weather-few-clouds-night"_L1},
                              // {VThemeIcon::WeatherFog, "weather-fog"_L1},
                              // {VThemeIcon::WeatherShowers, "weather-showers"_L1},
                              // {VThemeIcon::WeatherSnow, "weather-snow"_L1},
                              // {VThemeIcon::WeatherStorm, "weather-storm"_L1},

                              {VThemeIcon::DocumentImport, "document-import"_L1},
                              {VThemeIcon::DocumentExport, "document-export"_L1},
                              {VThemeIcon::PreferencesOther, "preferences-other"_L1},
                              {VThemeIcon::GtkOk, "gtk-ok"_L1},
                              {VThemeIcon::GtkNo, "gtk-no"_L1},
                              {VThemeIcon::SnapNodesRotationCenter, "snap-nodes-rotation-center"_L1},
                              {VThemeIcon::DrawStar, "draw-star"_L1},
                              {VThemeIcon::AddPlacemark, "add-placemark"_L1},
                              {VThemeIcon::DuplicateNode, "duplicate-node"_L1},
                          }))
QT_WARNING_POP
} // namespace

//---------------------------------------------------------------------------------------------------------------------
auto FromTheme(VThemeIcon icon) -> QIcon
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    if (themeIconMapping->contains(icon))
    {
        return QIcon::fromTheme(themeIconMapping->value(icon));
    }
#endif
    return QIcon::fromTheme(themeIconStringMapping->value(icon));
}

//---------------------------------------------------------------------------------------------------------------------
auto FromTheme(VThemeIcon icon, const QIcon &fallback) -> QIcon
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    if (themeIconMapping->contains(icon))
    {
        return QIcon::fromTheme(themeIconMapping->value(icon), fallback);
    }
#endif
    return QIcon::fromTheme(themeIconStringMapping->value(icon), fallback);
}

//---------------------------------------------------------------------------------------------------------------------
auto HasThemeIcon(VThemeIcon icon) -> bool
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    if (themeIconMapping->contains(icon))
    {
        return QIcon::hasThemeIcon(themeIconMapping->value(icon));
    }
#endif
    return QIcon::hasThemeIcon(themeIconStringMapping->value(icon));
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
//---------------------------------------------------------------------------------------------------------------------
auto ThemeIconMappings() -> ThemeIconHash
{
    return *themeIconMapping;
}
#endif

//---------------------------------------------------------------------------------------------------------------------
auto ThemeIconStringMappings() -> ThemeIconStringHash
{
    return *themeIconStringMapping;
}
