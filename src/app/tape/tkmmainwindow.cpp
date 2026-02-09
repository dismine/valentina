/************************************************************************
 **
 **  @file   tkmmainwindow.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   31 10, 2023
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
#include "tkmmainwindow.h"
#include "../ifc/xml/utils.h"
#include "../ifc/xml/vknownmeasurementsconverter.h"
#include "../vformat/knownmeasurements/vknownmeasurement.h"
#include "../vformat/knownmeasurements/vknownmeasurementsdocument.h"
#include "../vganalytics/vganalytics.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/defglobal.h"
#include "../vmisc/dialogs/dialogaskcollectstatistic.h"
#include "../vmisc/dialogs/dialogexporttocsv.h"
#include "../vmisc/dialogs/dialogselectlanguage.h"
#include "../vmisc/exception/vexception.h"
#include "../vmisc/qxtcsvmodel.h"
#include "../vmisc/theme/themeDef.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vsysexits.h"
#include "dialogs/dialogabouttape.h"
#include "dialogs/dialogknownmeasurementscsvcolumns.h"
#include "knownmeasurements/vknownmeasurements.h"
#include "mapplication.h" // Should be last because of definning qApp
#include "ui_tkmmainwindow.h"

#include <qmimedata.h>
#include <QAbstractButton>
#include <QCloseEvent>
#include <QCompleter>
#include <QDesktopServices>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QFileInfo>
#include <QImageReader>
#include <QLineEdit>
#include <QMessageBox>
#include <QMimeType>
#include <QPainter>
#include <QPlainTextEdit>
#include <QStringListModel>
#include <QTimer>
#include <QUuid>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#ifdef WITH_TEXTCODEC
#include "../vmisc/codecs/qtextcodec.h"
#else
#include "../vmisc/vtextcodec.h"
using QTextCodec = VTextCodec;
#endif // WITH_TEXTCODEC
#else
#include <QTextCodec>
#endif // QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

using namespace std::chrono_literals;

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(kmMainWindow, "km.mainwindow") // NOLINT

QT_WARNING_POP

using namespace Qt::Literals::StringLiterals;

namespace
{
constexpr int DIALOG_MAX_FORMULA_HEIGHT = 64;

// We need this enum in case we will add or delete a column. And also make code more readable.
enum
{
    ColumnName = 0,
    ColumnFullName = 1,
    ColumnGroup = 2
};

enum class MUnits : qint8
{
    Table,
    Degrees
};
} // namespace

//---------------------------------------------------------------------------------------------------------------------
TKMMainWindow::TKMMainWindow(QWidget *parent)
  : VAbstractMainWindow(parent),
    ui(new Ui::TKMMainWindow),
    m_searchHistory(new QMenu(this))
{
    ui->setupUi(this);

    InitIcons();
    WindowsLocale();

    ui->actionImportFromCSV->setIcon(FromTheme(VThemeIcon::DocumentImport));
    ui->actionExportToCSV->setIcon(FromTheme(VThemeIcon::DocumentExport));

    ui->labelDiagram->setText(UnknownMeasurementImage());

    ui->lineEditFind->installEventFilter(this);
    ui->plainTextEditFormula->installEventFilter(this);
    ui->listWidget->installEventFilter(this);
    // QListView::setViewMode overwriting the acceptDrops property
    // https://forum.qt.io/post/688467
    ui->listWidget->setDragDropMode(QAbstractItemView::DropOnly);

    m_search = QSharedPointer<VTableSearch>(new VTableSearch(ui->tableWidget));
    ui->tabWidget->setVisible(false);

    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &TKMMainWindow::RefreshMeasurementImagePreview);

    ui->toolBar->setContextMenuPolicy(Qt::PreventContextMenu);

    m_recentFileActs.fill(nullptr);

    SetupMenu();
    ReadSettings();

#if defined(Q_OS_MAC)
    ui->pushButtonShowInExplorer->setText(QCoreApplication::translate("TKMMainWindow", "Show in Finder"));
#endif // defined(Q_OS_MAC)

    if (MApplication::VApp()->IsAppInGUIMode())
    {
        QTimer::singleShot(1s, this, &TKMMainWindow::AskDefaultSettings);
    }

    m_buttonShortcuts.insert(VShortcutAction::CaseSensitiveMatch, ui->toolButtonCaseSensitive);
    m_buttonShortcuts.insert(VShortcutAction::WholeWordMatch, ui->toolButtonWholeWord);
    m_buttonShortcuts.insert(VShortcutAction::RegexMatch, ui->toolButtonRegexp);
    m_buttonShortcuts.insert(VShortcutAction::SearchHistory, ui->pushButtonSearch);
    m_buttonShortcuts.insert(VShortcutAction::RegexMatchUnicodeProperties, ui->toolButtonUseUnicodeProperties);
    m_buttonShortcuts.insert(VShortcutAction::FindNext, ui->toolButtonFindNext);
    m_buttonShortcuts.insert(VShortcutAction::FindPrevious, ui->toolButtonFindNext);

    if (VAbstractShortcutManager *manager = VAbstractApplication::VApp()->GetShortcutManager())
    {
        connect(manager, &VAbstractShortcutManager::ShortcutsUpdated, this, &TKMMainWindow::UpdateShortcuts);
        UpdateShortcuts();
    }
}

//---------------------------------------------------------------------------------------------------------------------
TKMMainWindow::~TKMMainWindow()
{
    ui->lineEditFind->blockSignals(true); // prevents crash
    delete m_m;
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto TKMMainWindow::CurrentFile() const -> QString
{
    return m_curFile;
}

//---------------------------------------------------------------------------------------------------------------------
auto TKMMainWindow::LoadFile(const QString &path) -> bool
{
    if (m_m != nullptr)
    {
        return MApplication::VApp()->NewMainKMWindow()->LoadFile(path);
    }

    if (not QFileInfo::exists(path))
    {
        qCCritical(kmMainWindow, "%s", qUtf8Printable(tr("File '%1' doesn't exist!").arg(path)));
        if (MApplication::VApp()->IsTestMode())
        {
            QCoreApplication::exit(V_EX_NOINPUT);
        }
        return false;
    }

    // Check if file already opened
    const QList<TKMMainWindow *> list = MApplication::VApp()->MainKMWindows();
    if (auto w = std::find_if(list.begin(), list.end(),
                              [path](TKMMainWindow *window) { return window->CurrentFile() == path; });
        w != list.end())
    {
        (*w)->activateWindow();
        close();
        return false;
    }

    VlpCreateLock(m_lock, path);

    if (not m_lock->IsLocked() &&
        not IgnoreLocking(m_lock->GetLockError(), path, MApplication::VApp()->IsAppInGUIMode()))
    {
        return false;
    }

    try
    {
        VKnownMeasurementsConverter converter(path);
        m_curFileFormatVersion = converter.GetCurrentFormatVersion();
        m_curFileFormatVersionStr = converter.GetFormatVersionStr();
        m_m = new VKnownMeasurementsDocument();
        m_m->setXMLContent(converter.Convert());

        if (const VCommonSettings *settings = VAbstractApplication::VApp()->Settings(); settings->IsCollectStatistic())
        {
            auto *statistic = VGAnalytics::Instance();

            if (QString clientID = settings->GetClientID(); clientID.isEmpty())
            {
                clientID = QUuid::createUuid().toString();
                settings->SetClientID(clientID);
                statistic->SetClientID(clientID);
            }

            statistic->Enable(true);

            const qint64 uptime = VAbstractApplication::VApp()->AppUptime();
            statistic->SendMultisizeMeasurementsFormatVersion(uptime, m_curFileFormatVersionStr);
        }

        ui->labelToolTip->setVisible(false);
        ui->tabWidget->setVisible(true);

        m_mIsReadOnly = m_m->IsReadOnly();
        UpdatePadlock(m_mIsReadOnly);

        SetCurrentFile(path);

        InitWindow();

        RefreshTable();

        if (ui->tableWidget->rowCount() > 0)
        {
            ui->tableWidget->selectRow(0);
        }

        RefreshImages();

        if (ui->listWidget->count() > 0)
        {
            ui->listWidget->setCurrentRow(0);
        }

        Controls(); // Buttons remove, up, down

        ui->actionImportFromCSV->setEnabled(true);
    }
    catch (VException &e)
    {
        qCCritical(kmMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("File error.")), qUtf8Printable(e.ErrorMessage()),
                   qUtf8Printable(e.DetailedInformation()));
        ui->labelToolTip->setVisible(true);
        ui->tabWidget->setVisible(false);
        delete m_m;
        m_m = nullptr;
        m_lock.reset();

        if (MApplication::VApp()->IsTestMode())
        {
            QCoreApplication::exit(V_EX_NOINPUT);
        }
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::UpdateWindowTitle()
{
    QString showName;
    bool isFileWritable = true;
    if (not m_curFile.isEmpty())
    {
        // #ifdef Q_OS_WIN32
        //         qt_ntfs_permission_lookup++; // turn checking on
        // #endif /*Q_OS_WIN32*/
        isFileWritable = QFileInfo(m_curFile).isWritable();
        // #ifdef Q_OS_WIN32
        //         qt_ntfs_permission_lookup--; // turn it off again
        // #endif /*Q_OS_WIN32*/
        showName = QFileInfo(m_curFile).fileName();
    }
    else
    {
        showName = untitledIndex > 0 ? tr("untitled %1").arg(untitledIndex) : tr("untitled");
        showName += ".vkm"_L1;
    }

    showName += "[*]"_L1;

    if (m_mIsReadOnly || not isFileWritable)
    {
        showName += " ("_L1 + tr("read only") + ')'_L1;
    }

    setWindowTitle(showName);
    setWindowFilePath(m_curFile);

#if defined(Q_OS_MAC)
    static auto fileIcon = QIcon(QCoreApplication::applicationDirPath() + "/../Resources/measurements.icns"_L1);
    QIcon icon;
    if (not m_curFile.isEmpty())
    {
        if (not isWindowModified())
        {
            icon = fileIcon;
        }
        else
        {
            static QIcon darkIcon;

            if (darkIcon.isNull())
            {
                darkIcon = QIcon(darkenPixmap(fileIcon.pixmap(16, 16)));
            }
            icon = darkIcon;
        }
    }
    setWindowIcon(icon);
#endif // defined(Q_OS_MAC)
}

//---------------------------------------------------------------------------------------------------------------------
auto TKMMainWindow::IsUntitled() const -> bool
{
    return m_curFile.isEmpty();
}

//---------------------------------------------------------------------------------------------------------------------
auto TKMMainWindow::GetUntitledIndex() const -> int
{
    return untitledIndex;
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::SetUntitledIndex(int newUntitledIndex)
{
    untitledIndex = newUntitledIndex;
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::closeEvent(QCloseEvent *event)
{
    if (MaybeSave())
    {
        WriteSettings();
        event->accept();
        deleteLater();
    }
    else
    {
        event->ignore();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        WindowsLocale();

        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);

        ui->lineEditFind->setPlaceholderText(m_search->SearchPlaceholder());
        UpdateSearchControlsTooltips();

        UpdateWindowTitle();

        InitMeasurementUnits();

        const QSignalBlocker blocker(ui->comboBoxDiagram);

        QUuid current;
        if (ui->comboBoxDiagram->currentIndex() != -1)
        {
            current = ui->comboBoxDiagram->currentData().toUuid();
        }

        InitMeasurementDiagramList();

        if (int const i = ui->comboBoxDiagram->findData(current); i != -1)
        {
            ui->comboBoxDiagram->setCurrentIndex(i);
        }
    }

    if (event->type() == QEvent::PaletteChange)
    {
        InitIcons();
    }

    if (event->type() == QEvent::ThemeChange)
    {
        VTheme::Instance()->ResetColorScheme();
    }

    // remember to call base class implementation
    QMainWindow::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
auto TKMMainWindow::eventFilter(QObject *object, QEvent *event) -> bool
{
    if (auto *plainTextEdit = qobject_cast<QPlainTextEdit *>(object))
    {
        return HandleKeyPress(plainTextEdit, event);
    }

    if (auto *textEdit = qobject_cast<QLineEdit *>(object))
    {
        return HandleKeyPress(textEdit, event);
    }

    if (const auto *listWidget = qobject_cast<QListWidget *>(object))
    {
        return HandleListWidgetEvent(listWidget, event);
    }

    // pass the event on to the parent class
    return QMainWindow::eventFilter(object, event);
}

//---------------------------------------------------------------------------------------------------------------------
auto TKMMainWindow::HandleKeyPress(QWidget *widget, QEvent *event) -> bool
{
    if (event->type() != QEvent::KeyPress)
    {
        return false;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    if (const auto *keyEvent = static_cast<QKeyEvent *>(event);
        keyEvent->key() != Qt::Key_Period || !(keyEvent->modifiers() & Qt::KeypadModifier))
    {
        return false;
    }

    const QString decimalSeparator = VAbstractApplication::VApp()->Settings()->GetOsSeparator()
                                         ? LocaleDecimalPoint(QLocale())
                                         : LocaleDecimalPoint(QLocale::c());

    if (auto *plainTextEdit = qobject_cast<QPlainTextEdit *>(widget))
    {
        plainTextEdit->insertPlainText(decimalSeparator);
        return true;
    }

    if (auto *lineEdit = qobject_cast<QLineEdit *>(widget))
    {
        lineEdit->insert(decimalSeparator);
        return true;
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto TKMMainWindow::HandleListWidgetEvent(const QListWidget *listWidget, QEvent *event) -> bool
{
    if (listWidget != ui->listWidget)
    {
        return false;
    }

    if (event->type() == QEvent::DragEnter || event->type() == QEvent::DragMove)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
        return HandleDragEnterMove(static_cast<QDragEnterEvent *>(event));
    }

    if (event->type() == QEvent::Drop)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
        return HandleDrop(static_cast<QDropEvent *>(event));
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto TKMMainWindow::HandleDragEnterMove(QDragEnterEvent *dragEvent) -> bool
{
    if (!dragEvent->mimeData()->hasUrls())
    {
        return false;
    }

    const QStringList formats = SupportedFormats();
    if (const auto urls = dragEvent->mimeData()->urls();
        std::all_of(urls.begin(),
                    urls.end(),
                    [formats](const QUrl &url)
                    { return url.isLocalFile() && formats.contains(QFileInfo(url.toLocalFile()).suffix().toLower()); }))
    {
        dragEvent->acceptProposedAction();
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto TKMMainWindow::HandleDrop(QDropEvent *dropEvent) -> bool
{
    if (!dropEvent->mimeData()->hasUrls())
    {
        return false;
    }

    const QStringList formats = SupportedFormats();
    const auto urls = dropEvent->mimeData()->urls();
    QStringList imagePaths;
    imagePaths.reserve(urls.size());
    for (const QUrl &url : urls)
    {
        if (url.isLocalFile())
        {
            if (const QString filePath = url.toLocalFile(); formats.contains(QFileInfo(filePath).suffix().toLower()))
            {
                imagePaths.append(filePath);
            }
        }
    }

    if (imagePaths.size() == urls.size())
    {
        AddMeasurementImages(imagePaths);
        dropEvent->acceptProposedAction();
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::ExportToCSVData(const QString &fileName, bool withHeader, int mib, const QChar &separator)
{
    QxtCsvModel csv;

    int const columns = 5;
    int colCount = 0;
    for (int column = 0; column <= columns; ++column)
    {
        csv.insertSingleColumn(colCount++);
    }

    if (withHeader)
    {
        for (int column = 0; column <= columns; ++column)
        {
            csv.setHeaderText(column, CSVColumnHeader(column));
        }
    }

    const QMap<int, VKnownMeasurement> orderedTable = m_known.OrderedMeasurements();
    int row = 0;
    for (auto iMap = orderedTable.constBegin(); iMap != orderedTable.constEnd(); ++iMap)
    {
        const VKnownMeasurement &m = iMap.value();

        csv.insertSingleRow(row);
        csv.setText(row, 0, m.name);
        csv.setText(row, 1, m.fullName);
        csv.setText(row, 2, m.group);
        csv.setText(row, 3, m.formula);
        csv.setText(row, 4, m.description);
        ++row;
    }

    QString error;
    csv.toCSV(fileName, error, withHeader, separator, QTextCodec::codecForMib(mib));
}

//---------------------------------------------------------------------------------------------------------------------
auto TKMMainWindow::RecentFileList() const -> QStringList
{
    return MApplication::VApp()->TapeSettings()->GetRecentKMFileList();
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::AddMeasurementImages(const QStringList &imagePaths)
{
    if (imagePaths.isEmpty())
    {
        return;
    }

    const VTapeSettings *settings = MApplication::VApp()->TapeSettings();

    for (const auto &filePath : imagePaths)
    {
        if (QFileInfo f(filePath); f.exists())
        {
            settings->SetPathCustomImage(f.absolutePath());
        }

        VPatternImage const image = VPatternImage::FromFile(filePath);
        if (not image.IsValid())
        {
            qCritical() << tr("Invalid image. Error: %1").arg(image.ErrorString());
            continue;
        }

        m_m->AddImage(image);

        MeasurementsWereSaved(false);
    }

    m_known = VKnownMeasurements();
    RefreshImages();

    ShowImageData();
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::FileNew()
{
    if (m_m != nullptr)
    {
        MApplication::VApp()->NewMainKMWindow()->FileNew();
        return;
    }

    m_m = new VKnownMeasurementsDocument(this);
    m_m->CreateEmptyFile();

    m_curFileFormatVersion = VKnownMeasurementsConverter::KnownMeasurementsMaxVer;
    m_curFileFormatVersionStr = VKnownMeasurementsConverter::KnownMeasurementsMaxVerStr;

    m_mIsReadOnly = m_m->IsReadOnly();
    UpdatePadlock(m_mIsReadOnly);

    SetCurrentFile(QString());
    MeasurementsWereSaved(false);

    InitWindow();

    ui->actionImportFromCSV->setEnabled(true);
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::OpenKnownMeasurements()
{
    const QString filter = tr("Known measurements") + " (*.vkm);;"_L1 + tr("All files") + " (*.*)"_L1;
    // Use standard path to known measurements
    QString const pathTo = MApplication::VApp()->TapeSettings()->GetPathKnownMeasurements();

    Open(pathTo, filter);
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::ToolBarStyles()
{
    ToolBarStyle(ui->toolBar);
}

//---------------------------------------------------------------------------------------------------------------------
auto TKMMainWindow::FileSave() -> bool
{
    if (m_curFile.isEmpty() || m_mIsReadOnly)
    {
        return FileSaveAs();
    }

    if (m_curFileFormatVersion < VKnownMeasurementsConverter::KnownMeasurementsMaxVer &&
        not ContinueFormatRewrite(m_curFileFormatVersionStr, VKnownMeasurementsConverter::KnownMeasurementsMaxVerStr))
    {
        return false;
    }

    if (not CheckFilePermissions(m_curFile, this))
    {
        return false;
    }

    if (QString error; not SaveKnownMeasurements(m_curFile, error))
    {
        QMessageBox messageBox;
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.setText(tr("Could not save the file"));
        messageBox.setDefaultButton(QMessageBox::Ok);
        messageBox.setDetailedText(error);
        messageBox.setStandardButtons(QMessageBox::Ok);
        messageBox.exec();
        return false;
    }

    m_curFileFormatVersion = VKnownMeasurementsConverter::KnownMeasurementsMaxVer;
    m_curFileFormatVersionStr = VKnownMeasurementsConverter::KnownMeasurementsMaxVerStr;

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto TKMMainWindow::FileSaveAs() -> bool
{
    QString const filters = tr("Known measurements") + QStringLiteral(" (*.vkm)");

    QString fName = tr("known measurements");
    auto const suffix = QStringLiteral("vkm");

    fName += '.'_L1 + suffix;

    VTapeSettings *settings = MApplication::VApp()->TapeSettings();
    const QString dir = settings->GetPathKnownMeasurements();

    if (QDir const directory(dir); not directory.exists())
    {
        directory.mkpath(QChar('.'));
    }

    if (not m_curFile.isEmpty())
    {
        fName = QFileInfo(m_curFile).fileName();
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save as"), dir + '/'_L1 + fName, filters, nullptr,
                                                    VAbstractApplication::VApp()->NativeFileDialog());

    if (fileName.isEmpty())
    {
        return false;
    }

    if (QFileInfo const f(fileName); f.suffix().isEmpty() && f.suffix() != suffix)
    {
        fileName += '.'_L1 + suffix;
    }

    if (QFileInfo::exists(fileName) && m_curFile != fileName)
    {
        // Temporary try to lock the file before saving
        VLockGuard<char> const tmp(fileName);
        if (not tmp.IsLocked())
        {
            qCCritical(kmMainWindow, "%s",
                       qUtf8Printable(tr("Failed to lock. This file already opened in another window.")));
            return false;
        }
    }

    // Need for restoring previous state in case of failure
    const bool readOnly = m_m->IsReadOnly();

    m_m->SetReadOnly(false);
    m_mIsReadOnly = false;

    QString error;
    if (bool const result = SaveKnownMeasurements(fileName, error); not result)
    {
        QMessageBox messageBox;
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.setInformativeText(tr("Could not save file"));
        messageBox.setDefaultButton(QMessageBox::Ok);
        messageBox.setDetailedText(error);
        messageBox.setStandardButtons(QMessageBox::Ok);
        messageBox.exec();

        // Restore previous state
        m_m->SetReadOnly(readOnly);
        m_mIsReadOnly = readOnly;
        return false;
    }

    m_curFileFormatVersion = VKnownMeasurementsConverter::KnownMeasurementsMaxVer;
    m_curFileFormatVersionStr = VKnownMeasurementsConverter::KnownMeasurementsMaxVerStr;

    UpdatePadlock(false);
    UpdateWindowTitle();

    if (m_curFile == fileName && not m_lock.isNull())
    {
        m_lock->Unlock();
    }
    VlpCreateLock(m_lock, fileName);
    if (not m_lock->IsLocked())
    {
        qCCritical(kmMainWindow, "%s",
                   qUtf8Printable(tr("Failed to lock. This file already opened in another window. "
                                     "Expect collissions when run 2 copies of the program.")));
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::AboutToShowWindowMenu()
{
    ui->menuWindow->clear();
    CreateWindowMenu(ui->menuWindow);
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::ShowWindow() const
{
    if (auto *action = qobject_cast<QAction *>(sender()))
    {
        const QVariant v = action->data();
        if (v.canConvert<int>())
        {
            const int offset = qvariant_cast<int>(v);
            const QList<TKMMainWindow *> windows = MApplication::VApp()->MainKMWindows();
            windows.at(offset)->raise();
            windows.at(offset)->activateWindow();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::ImportDataFromCSV()
{
    if (m_m == nullptr)
    {
        return;
    }

    const QString filters = tr("Comma-Separated Values") + QStringLiteral(" (*.csv)");
    const auto suffix = QStringLiteral("csv");

    QString fileName = QFileDialog::getOpenFileName(this, tr("Import from CSV"), QDir::homePath(), filters, nullptr,
                                                    VAbstractApplication::VApp()->NativeFileDialog());

    if (fileName.isEmpty())
    {
        return;
    }

    if (QFileInfo const f(fileName); f.suffix().isEmpty() && f.suffix() != suffix)
    {
        fileName += '.'_L1 + suffix;
    }

    DialogExportToCSV dialog(this);
    dialog.SetWithHeader(VAbstractApplication::VApp()->Settings()->GetCSVWithHeader());
    dialog.SetSelectedMib(VAbstractApplication::VApp()->Settings()->GetCSVCodec());
    dialog.SetSeparator(VAbstractApplication::VApp()->Settings()->GetCSVSeparator());
    dialog.ShowFilePreview(fileName);

    if (dialog.exec() == QDialog::Accepted)
    {
        VAbstractApplication::VApp()->Settings()->SetCSVSeparator(dialog.GetSeparator());
        VAbstractApplication::VApp()->Settings()->SetCSVCodec(dialog.GetSelectedMib());
        VAbstractApplication::VApp()->Settings()->SetCSVWithHeader(dialog.IsWithHeader());

        auto columns = QSharedPointer<DialogKnownMeasurementsCSVColumns>::create(fileName, this);
        columns->SetWithHeader(dialog.IsWithHeader());
        columns->SetSeparator(dialog.GetSeparator());
        columns->SetCodec(QTextCodec::codecForMib(dialog.GetSelectedMib()));

        if (columns->exec() == QDialog::Accepted)
        {
            QxtCsvModel const csv(fileName,
                                  nullptr,
                                  dialog.IsWithHeader(),
                                  dialog.GetSeparator(),
                                  QTextCodec::codecForMib(dialog.GetSelectedMib()));
            const QVector<int> map = columns->ColumnsMap();
            ImportKnownMeasurements(csv, map, dialog.IsWithHeader());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::SaveKnownMeasurementsName()
{
    if (m_m->Name() != ui->lineEditKMName->text())
    {
        m_m->SetName(ui->lineEditKMName->text());
        MeasurementsWereSaved(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::SaveKnownMeasurementsDescription()
{
    if (m_m->Description() != ui->plainTextEditKMDescription->toPlainText())
    {
        m_m->SetDescription(ui->plainTextEditKMDescription->toPlainText());
        MeasurementsWereSaved(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::RemoveMeasurement()
{
    ShowMDiagram(VPatternImage());
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), 0);
    m_m->RemoveMeasurement(nameField->data(Qt::UserRole).toString());

    MeasurementsWereSaved(false);

    m_search->RemoveRow(row);
    m_known = VKnownMeasurements();
    RefreshTable();
    m_search->RefreshList(ui->lineEditFind->text());

    if (ui->tableWidget->rowCount() > 0)
    {
        ui->tableWidget->selectRow(row >= ui->tableWidget->rowCount() ? ui->tableWidget->rowCount() - 1 : row);
    }
    else
    {
        MFields(false);

        ui->actionExportToCSV->setEnabled(false);

        {
            const QSignalBlocker blocker(ui->lineEditName);
            ui->lineEditName->setText(QString());
        }

        {
            const QSignalBlocker blocker(ui->plainTextEditDescription);
            ui->plainTextEditDescription->setPlainText(QString());
        }

        {
            const QSignalBlocker blocker(ui->lineEditFullName);
            ui->lineEditFullName->setText(QString());
        }

        const QSignalBlocker blocker(ui->comboBoxMUnits);
        ui->comboBoxMUnits->setCurrentIndex(-1);
    }

    ui->tableWidget->repaint(); // Force repain to fix paint artifacts on Mac OS X
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::MoveTop()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(row, ColumnName);
    m_m->MoveTop(nameField->data(Qt::UserRole).toString());
    MeasurementsWereSaved(false);
    m_known = VKnownMeasurements();
    RefreshTable();
    m_search->RefreshList(ui->lineEditFind->text());
    ui->tableWidget->selectRow(0);
    ui->tableWidget->repaint(); // Force repain to fix paint artifacts on Mac OS X
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::MoveUp()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(row, ColumnName);
    m_m->MoveUp(nameField->data(Qt::UserRole).toString());
    MeasurementsWereSaved(false);
    m_known = VKnownMeasurements();
    RefreshTable();
    m_search->RefreshList(ui->lineEditFind->text());
    ui->tableWidget->selectRow(row - 1);
    ui->tableWidget->repaint(); // Force repain to fix paint artifacts on Mac OS X
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::MoveDown()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(row, ColumnName);
    m_m->MoveDown(nameField->data(Qt::UserRole).toString());
    MeasurementsWereSaved(false);
    m_known = VKnownMeasurements();
    RefreshTable();
    m_search->RefreshList(ui->lineEditFind->text());
    ui->tableWidget->selectRow(row + 1);
    ui->tableWidget->repaint(); // Force repain to fix paint artifacts on Mac OS X
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::MoveBottom()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(row, ColumnName);
    m_m->MoveBottom(nameField->data(Qt::UserRole).toString());
    MeasurementsWereSaved(false);
    m_known = VKnownMeasurements();
    RefreshTable();
    m_search->RefreshList(ui->lineEditFind->text());
    ui->tableWidget->selectRow(ui->tableWidget->rowCount() - 1);
    ui->tableWidget->repaint(); // Force repain to fix paint artifacts on Mac OS X
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::AddImage()
{
    const VTapeSettings *settings = MApplication::VApp()->TapeSettings();
    const QStringList filePaths = QFileDialog::getOpenFileNames(this,
                                                                tr("Measurement images"),
                                                                settings->GetPathCustomImage(),
                                                                PrepareImageFilters(),
                                                                nullptr,
                                                                VAbstractApplication::VApp()->NativeFileDialog());

    AddMeasurementImages(filePaths);
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::RemoveImage()
{
    auto *item = ui->listWidget->currentItem();

    if (item == nullptr)
    {
        ui->toolButtonRemoveImage->setDisabled(true);
        return;
    }

    m_m->RemoveImage(item->data(Qt::UserRole).toUuid());

    MeasurementsWereSaved(false);

    m_known = VKnownMeasurements();
    RefreshImages();

    if (m_known.Images().isEmpty())
    {
        ui->toolButtonRemoveImage->setDisabled(true);
        ui->toolButtonSaveImage->setDisabled(true);
    }
    else
    {
        ui->listWidget->setCurrentRow(0);
    }

    const int row = ui->tableWidget->currentRow();
    RefreshTable();
    ui->tableWidget->selectRow(row);
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::SaveImage()
{
    auto *item = ui->listWidget->currentItem();

    if (item == nullptr)
    {
        ui->toolButtonSaveImage->setDisabled(true);
        return;
    }

    QMap<QUuid, VPatternImage> const images = m_known.Images();

    QUuid const id = item->data(Qt::UserRole).toUuid();
    if (!images.contains(id))
    {
        ui->toolButtonSaveImage->setDisabled(true);
        return;
    }

    const VPatternImage image = images.value(id);

    if (not image.IsValid())
    {
        qCritical() << tr("Unable to save image. Error: %1").arg(image.ErrorString());
        return;
    }

    VTapeSettings *settings = MApplication::VApp()->TapeSettings();

    QMimeType const mime = image.MimeTypeFromData();

    QString title = image.Title();
    if (title.isEmpty())
    {
        title = tr("untitled");
    }
    QString path = settings->GetPathCustomImage() + QDir::separator() + title;

    if (QStringList const suffixes = mime.suffixes(); not suffixes.isEmpty())
    {
        path += '.'_L1 + suffixes.at(0);
    }

    QString const filter = mime.filterString();
    QString const filename = QFileDialog::getSaveFileName(this, tr("Save Image"), path, filter, nullptr,
                                                          VAbstractApplication::VApp()->NativeFileDialog());
    if (not filename.isEmpty())
    {
        if (QFileInfo::exists(filename))
        {
            settings->SetPathCustomImage(QFileInfo(filename).absolutePath());
        }

        QFile file(filename);
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(QByteArray::fromBase64(image.ContentData()));
        }
        else
        {
            qCritical() << tr("Unable to save image. Error: %1").arg(file.errorString());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::ShowImage()
{
    VPatternImage image;

    if (lastSelectedTab == ui->tabWidget->indexOf(ui->tabMeasurements)
        || ui->tabWidget->currentIndex() == ui->tabWidget->indexOf(ui->tabMeasurements))
    {
        const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName); // name
        if (nameField == nullptr)
        {
            return;
        }

        VKnownMeasurement const m = m_known.Measurement(nameField->data(Qt::UserRole).toString());
        image = m_known.Image(m.diagram);
    }
    else if (lastSelectedTab == ui->tabWidget->indexOf(ui->tabImages)
             || ui->tabWidget->currentIndex() == ui->tabWidget->indexOf(ui->tabImages))
    {
        const QListWidgetItem *item = ui->listWidget->currentItem();
        if (item == nullptr)
        {
            return;
        }

        QMap<QUuid, VPatternImage> const images = m_known.Images();

        QUuid const id = item->data(Qt::UserRole).toUuid();
        if (!images.contains(id))
        {
            return;
        }

        image = images.value(id);
    }
    else
    {
        return;
    }

    if (not image.IsValid())
    {
        qCritical() << tr("Unable to show image. Error: %1").arg(image.ErrorString());
        return;
    }

    QMimeType const mime = image.MimeTypeFromData();
    QString name = QDir::tempPath() + QDir::separator() + "image.XXXXXX"_L1;

    if (QStringList const suffixes = mime.suffixes(); not suffixes.isEmpty())
    {
        name += '.'_L1 + suffixes.at(0);
    }

    delete m_tmpImage.data();
    m_tmpImage = new QTemporaryFile(name, this);
    if (m_tmpImage->open())
    {
        m_tmpImage->write(QByteArray::fromBase64(image.ContentData()));
        m_tmpImage->flush();
        QDesktopServices::openUrl(QUrl::fromLocalFile(m_tmpImage->fileName()));
    }
    else
    {
        qCritical() << "Unable to open temp file";
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::AddKnown()
{
    const QString name = GenerateMeasurementName();
    qint32 currentRow = -1;

    if (ui->tableWidget->currentRow() == -1)
    {
        currentRow = ui->tableWidget->rowCount();
        m_m->AddEmptyMeasurement(name);
    }
    else
    {
        currentRow = ui->tableWidget->currentRow() + 1;
        const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
        m_m->AddEmptyMeasurementAfter(nameField->data(Qt::UserRole).toString(), name);
    }

    m_search->AddRow(currentRow);
    m_known = VKnownMeasurements();
    RefreshTable();
    m_search->RefreshList(ui->lineEditFind->text());

    ui->tableWidget->selectRow(currentRow);

    ui->actionExportToCSV->setEnabled(true);

    MeasurementsWereSaved(false);
    ui->tableWidget->repaint(); // Force repain to fix paint artifacts on Mac OS X
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::ShowMData()
{
    if (ui->tableWidget->rowCount() <= 0)
    {
        MFields(false);
        return;
    }

    MFields(true);

    if (ui->tableWidget->currentRow() == -1)
    {
        const QSignalBlocker blocker(ui->tableWidget);
        ui->tableWidget->selectRow(0);
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName); // name
    SCASSERT(nameField != nullptr)

    VKnownMeasurement const m = m_known.Measurement(nameField->data(Qt::UserRole).toString());

    ShowMDiagram(m_known.Image(m.diagram));

    {
        const QSignalBlocker blocker(ui->plainTextEditDescription);
        ui->plainTextEditDescription->setPlainText(m.description);
    }

    // Don't block all signal for QLineEdit. Need for correct handle with clear button.
    disconnect(ui->lineEditName, &QLineEdit::textEdited, this, &TKMMainWindow::SaveMName);
    ui->lineEditName->setText(m.name);
    connect(ui->lineEditName, &QLineEdit::textEdited, this, &TKMMainWindow::SaveMName);

    disconnect(ui->lineEditFullName, &QLineEdit::editingFinished, this, &TKMMainWindow::SaveMFullName);
    ui->lineEditFullName->setText(m.fullName);
    connect(ui->lineEditFullName, &QLineEdit::editingFinished, this, &TKMMainWindow::SaveMFullName);

    disconnect(ui->lineEditGroup, &QLineEdit::editingFinished, this, &TKMMainWindow::SaveMGroup);
    ui->lineEditGroup->setText(m.group);
    connect(ui->lineEditGroup, &QLineEdit::editingFinished, this, &TKMMainWindow::SaveMGroup);

    {
        const QSignalBlocker blocker(ui->comboBoxMUnits);
        ui->comboBoxMUnits->setCurrentIndex(
            ui->comboBoxMUnits->findData(static_cast<int>(m.specialUnits ? MUnits::Degrees : MUnits::Table)));
    }

    {
        const QSignalBlocker blocker(ui->comboBoxDiagram);
        InitMeasurementDiagramList();
        ui->comboBoxDiagram->setCurrentIndex(ui->comboBoxDiagram->findData(m.diagram));
    }

    const QSignalBlocker blocker(ui->plainTextEditFormula);
    ui->plainTextEditFormula->setPlainText(m.formula);
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::ShowImageData()
{
    if (ui->listWidget->count() <= 0)
    {
        ImageFields(false);
        return;
    }

    if (ui->listWidget->currentRow() == -1)
    {
        const QSignalBlocker blocker(ui->listWidget);
        ui->listWidget->setCurrentRow(0);
    }

    ImageFields(true);

    const QListWidgetItem *activeImage = ui->listWidget->item(ui->listWidget->currentRow());
    QUuid const imageId = activeImage->data(Qt::UserRole).toUuid();
    VPatternImage const image = m_known.Image(imageId);

    ShowMDiagram(image);

    // Don't block all signal for QLineEdit. Need for correct handle with clear button.
    disconnect(ui->lineEditImageTitle, &QLineEdit::editingFinished, this, &TKMMainWindow::SaveImageTitle);
    ui->lineEditImageTitle->setText(image.Title());
    connect(ui->lineEditImageTitle, &QLineEdit::editingFinished, this, &TKMMainWindow::SaveImageTitle);

    const QSignalBlocker blocker(ui->doubleSpinBoxImageSize);
    ui->doubleSpinBoxImageSize->setValue(image.GetSizeScale());
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::DeployFormula()
{
    SCASSERT(ui->plainTextEditFormula != nullptr)
    SCASSERT(ui->pushButtonGrow != nullptr)

    const QTextCursor cursor = ui->plainTextEditFormula->textCursor();

    if (ui->plainTextEditFormula->height() < DIALOG_MAX_FORMULA_HEIGHT)
    {
        ui->plainTextEditFormula->setFixedHeight(DIALOG_MAX_FORMULA_HEIGHT);
        // Set icon from theme (internal for Windows system)
        ui->pushButtonGrow->setIcon(FromTheme(VThemeIcon::GoNext));
    }
    else
    {
        ui->plainTextEditFormula->setFixedHeight(m_formulaBaseHeight);
        // Set icon from theme (internal for Windows system)
        ui->pushButtonGrow->setIcon(FromTheme(VThemeIcon::GoDown));
    }

    // I found that after change size of formula field, it was filed for angle formula, field for formula became black.
    // This code prevent this.
    setUpdatesEnabled(false);
    repaint();
    setUpdatesEnabled(true);

    ui->plainTextEditFormula->setFocus();
    ui->plainTextEditFormula->setTextCursor(cursor);
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::SaveMName()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);

    QString newName = ui->lineEditName->text().isEmpty() ? GenerateMeasurementName() : ui->lineEditName->text();

    if (QHash<QString, VKnownMeasurement> const m = m_known.Measurements(); m.contains(newName))
    {
        qint32 num = 2;
        QString name = newName;
        do
        {
            name = name + '_'_L1 + QString::number(num);
            num++;
        } while (!m.contains(newName));
        newName = name;
    }

    m_m->SetMName(nameField->text(), newName);
    MeasurementsWereSaved(false);
    m_known = VKnownMeasurements();
    RefreshTable();
    m_search->RefreshList(ui->lineEditFind->text());

    const QSignalBlocker blocker(ui->tableWidget);
    ui->tableWidget->selectRow(row);
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::SaveMFormula()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(row, ColumnName);

    QString const formula = ui->plainTextEditFormula->toPlainText();
    m_m->SetMFormula(nameField->data(Qt::UserRole).toString(), formula);

    MeasurementsWereSaved(false);

    const QTextCursor cursor = ui->plainTextEditFormula->textCursor();

    m_known = VKnownMeasurements();
    RefreshTable();
    m_search->RefreshList(ui->lineEditFind->text());

    {
        const QSignalBlocker blocker(ui->tableWidget);
        ui->tableWidget->selectRow(row);
    }

    ui->plainTextEditFormula->setTextCursor(cursor);
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::SaveMDescription()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
    m_m->SetMDescription(nameField->data(Qt::UserRole).toString(), ui->plainTextEditDescription->toPlainText());

    MeasurementsWereSaved(false);

    const QTextCursor cursor = ui->plainTextEditDescription->textCursor();

    m_known = VKnownMeasurements();
    RefreshTable();

    {
        const QSignalBlocker blocker(ui->tableWidget);
        ui->tableWidget->selectRow(row);
    }

    ui->plainTextEditDescription->setTextCursor(cursor);
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::SaveMFullName()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);

    m_m->SetMFullName(nameField->data(Qt::UserRole).toString(), ui->lineEditFullName->text());

    MeasurementsWereSaved(false);

    m_known = VKnownMeasurements();
    RefreshTable();

    const QSignalBlocker blocker(ui->tableWidget);
    ui->tableWidget->selectRow(row);
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::SaveMUnits()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
    const auto units = static_cast<MUnits>(ui->comboBoxMUnits->currentData().toInt());
    m_m->SetMSpecialUnits(nameField->data(Qt::UserRole).toString(), units == MUnits::Degrees);

    MeasurementsWereSaved(false);

    m_known = VKnownMeasurements();
    RefreshTable();
    m_search->RefreshList(ui->lineEditFind->text());

    const QSignalBlocker blocker(ui->tableWidget);
    ui->tableWidget->selectRow(row);
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::SaveMGroup()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);

    m_m->SetMGroup(nameField->data(Qt::UserRole).toString(), ui->lineEditGroup->text());

    MeasurementsWereSaved(false);

    m_known = VKnownMeasurements();
    RefreshTable();

    const QSignalBlocker blocker(ui->tableWidget);
    ui->tableWidget->selectRow(row);
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::SaveMDiagram()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
    const QUuid id = ui->comboBoxDiagram->currentData().toUuid();
    m_m->SetMImage(nameField->data(Qt::UserRole).toString(), id);

    MeasurementsWereSaved(false);

    m_known = VKnownMeasurements();
    RefreshTable();
    m_search->RefreshList(ui->lineEditFind->text());

    {
        const QSignalBlocker blocker(ui->tableWidget);
        ui->tableWidget->selectRow(row);
    }

    ShowMDiagram(m_known.Image(id));
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::SaveImageTitle()
{
    auto *item = ui->listWidget->currentItem();
    int const row = ui->listWidget->currentRow();

    if (item == nullptr)
    {
        return;
    }

    m_m->SetImageTitle(item->data(Qt::UserRole).toUuid(), ui->lineEditImageTitle->text());

    MeasurementsWereSaved(false);

    m_known = VKnownMeasurements();
    RefreshImages();

    {
        const QSignalBlocker blocker(ui->listWidget);
        ui->listWidget->setCurrentRow(row);
    }

    ShowMData();
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::SaveImageSizeScale()
{
    auto *item = ui->listWidget->currentItem();
    int const row = ui->listWidget->currentRow();

    if (item == nullptr)
    {
        return;
    }

    m_m->SetImageSizeScale(item->data(Qt::UserRole).toUuid(), ui->doubleSpinBoxImageSize->value());

    MeasurementsWereSaved(false);

    m_known = VKnownMeasurements();
    RefreshImages();

    {
        const QSignalBlocker blocker(ui->listWidget);
        ui->listWidget->setCurrentRow(row);
    }

    ShowMData();
}

//---------------------------------------------------------------------------------------------------------------------
#if defined(Q_OS_MAC)
//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::OpenAt(QAction *where)
{
    const QString path = m_curFile.left(m_curFile.indexOf(where->text())) + where->text();
    if (path == m_curFile)
    {
        return;
    }
    QProcess process;
    process.start(QStringLiteral("/usr/bin/open"), QStringList() << path, QIODevice::ReadOnly);
    process.waitForFinished();
}
#endif // defined(Q_OS_MAC)

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::AskDefaultSettings()
{
    if (!MApplication::VApp()->IsAppInGUIMode())
    {
        return;
    }

    VTapeSettings *settings = MApplication::VApp()->TapeSettings();
    if (not settings->IsLocaleSelected())
    {
        QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        DialogSelectLanguage dialog(this);
        QGuiApplication::restoreOverrideCursor();
        dialog.setWindowModality(Qt::WindowModal);
        if (dialog.exec() == QDialog::Accepted)
        {
            QString const locale = dialog.Locale();
            settings->SetLocale(locale);
            VAbstractApplication::VApp()->LoadTranslation(locale);
        }
    }

    if (settings->IsAskCollectStatistic() || settings->IsAskSendCrashReport())
    {
        if (DialogAskCollectStatistic dialog(this); dialog.exec() == QDialog::Accepted)
        {
            settings->SetCollectStatistic(dialog.CollectStatistic());
#if defined(CRASH_REPORTING)
            settings->SeSendCrashReport(dialog.SendCrashReport());
            settings->SetCrashEmail(dialog.UserEmail());
#endif
        }

        settings->SetAskCollectStatistic(false);
        settings->SetAskSendCrashReport(false);
    }

    if (settings->IsCollectStatistic())
    {
        auto *statistic = VGAnalytics::Instance();
        statistic->SetGUILanguage(settings->GetLocale());

        bool freshID = false;
        if (QString clientID = settings->GetClientID(); clientID.isEmpty())
        {
            clientID = QUuid::createUuid().toString();
            settings->SetClientID(clientID);
            statistic->SetClientID(clientID);
            freshID = true;
        }

        statistic->Enable(true);

        const qint64 uptime = MApplication::VApp()->AppUptime();
        freshID ? statistic->SendAppFreshInstallEvent(uptime) : statistic->SendAppStartEvent(uptime);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::UpdateShortcuts()
{
    if (VAbstractShortcutManager *manager = VAbstractApplication::VApp()->GetShortcutManager())
    {
        manager->UpdateButtonShortcut(m_buttonShortcuts);
        manager->UpdateActionShortcuts(m_actionShortcuts);
        UpdateSearchControlsTooltips();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::RefreshMeasurementImagePreview(int index)
{
    if (index == -1)
    {
        return;
    }

    if (index == ui->tabWidget->indexOf(ui->tabMeasurements))
    {
        ShowMData();
        lastSelectedTab = ui->tabWidget->indexOf(ui->tabMeasurements);
        ui->actionAddKnown->setEnabled(true);
    }
    else if (index == ui->tabWidget->indexOf(ui->tabImages))
    {
        ShowImageData();
        lastSelectedTab = ui->tabWidget->indexOf(ui->tabImages);
        ui->actionAddKnown->setEnabled(false);
    }
    else if (index == ui->tabWidget->indexOf(ui->tabInformation))
    {
        ui->actionAddKnown->setEnabled(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::SetupMenu()
{
    // File
    connect(ui->actionNew, &QAction::triggered, this, &TKMMainWindow::FileNew);
    m_actionShortcuts.insert(VShortcutAction::New, ui->actionNew);

    connect(ui->actionOpen, &QAction::triggered, this, &TKMMainWindow::OpenKnownMeasurements);
    m_actionShortcuts.insert(VShortcutAction::Open, ui->actionOpen);

    connect(ui->actionSave, &QAction::triggered, this, &TKMMainWindow::FileSave);
    m_actionShortcuts.insert(VShortcutAction::Save, ui->actionSave);

    connect(ui->actionSaveAs, &QAction::triggered, this, &TKMMainWindow::FileSaveAs);
    m_actionShortcuts.insert(VShortcutAction::SaveAs, ui->actionSaveAs);

    connect(ui->actionExportToCSV, &QAction::triggered, this, &TKMMainWindow::ExportDataToCSV);
    connect(ui->actionImportFromCSV, &QAction::triggered, this, &TKMMainWindow::ImportDataFromCSV);
    connect(ui->actionReadOnly, &QAction::triggered, this,
            [this](bool ro)
            {
                if (not m_mIsReadOnly)
                {
                    m_m->SetReadOnly(ro);
                    MeasurementsWereSaved(false);
                    UpdatePadlock(ro);
                    UpdateWindowTitle();
                }
                else
                {
                    if (auto *action = qobject_cast<QAction *>(this->sender()))
                    {
                        action->setChecked(true);
                    }
                }
            });
    connect(ui->actionPreferences, &QAction::triggered, this, [this]() { MApplication::VApp()->Preferences(this); });

    for (auto &recentFileAct : m_recentFileActs)
    {
        auto *action = new QAction(this);
        recentFileAct = action;
        connect(action, &QAction::triggered, this,
                [this]()
                {
                    if (auto *senderAction = qobject_cast<QAction *>(sender()))
                    {
                        const QString filePath = senderAction->data().toString();
                        if (not filePath.isEmpty())
                        {
                            LoadFile(filePath);
                        }
                    }
                });
        ui->menuFile->insertAction(ui->actionPreferences, recentFileAct);
        recentFileAct->setVisible(false);
    }

    m_separatorAct = new QAction(this);
    m_separatorAct->setSeparator(true);
    m_separatorAct->setVisible(false);
    ui->menuFile->insertAction(ui->actionPreferences, m_separatorAct);

    connect(ui->actionQuit, &QAction::triggered, this, &TKMMainWindow::close);
    m_actionShortcuts.insert(VShortcutAction::Quit, ui->actionQuit);

    // Measurements
    connect(ui->actionAddKnown, &QAction::triggered, this, &TKMMainWindow::AddKnown);

    // Window
    connect(ui->menuWindow, &QMenu::aboutToShow, this, &TKMMainWindow::AboutToShowWindowMenu);
    AboutToShowWindowMenu();

    // Help
    connect(ui->actionSettingsDirectory,
            &QAction::triggered,
            this,
            []() { ShowInGraphicalShell(MApplication::VApp()->TapeSettings()->fileName()); });
    connect(ui->actionAboutQt, &QAction::triggered, this, [this]() { QMessageBox::aboutQt(this, tr("About Qt")); });
    connect(ui->actionAboutTape, &QAction::triggered, this,
            [this]()
            {
                auto *aboutDialog = new DialogAboutTape(this);
                aboutDialog->setAttribute(Qt::WA_DeleteOnClose, true);
                aboutDialog->show();
            });

    // Actions for recent files loaded by a tape window application.
    UpdateRecentFileActions();
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::InitWindow()
{
    SCASSERT(m_m != nullptr)
    ui->labelToolTip->setVisible(false);
    ui->tabWidget->setVisible(true);
    ui->tabWidget->setCurrentIndex(0); // measurements
    ui->tableWidget->resizeRowsToContents();
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

    connect(ui->tableWidget, &QTableWidget::itemSelectionChanged, this, &TKMMainWindow::ShowMData);

    // Tab measurements
    m_formulaBaseHeight = ui->plainTextEditFormula->height();
    connect(ui->plainTextEditFormula, &QPlainTextEdit::textChanged, this, &TKMMainWindow::SaveMFormula);

    InitSearch();

    ui->actionAddKnown->setEnabled(true);
    ui->actionSaveAs->setEnabled(true);

    ui->lineEditName->setValidator(new QRegularExpressionValidator(
        QRegularExpression("^$|"_L1 + NameRegExp(VariableRegex::KnownMeasurement)), this));

    connect(ui->toolButtonRemoveMeasurement, &QToolButton::clicked, this, &TKMMainWindow::RemoveMeasurement);
    connect(ui->toolButtonTop, &QToolButton::clicked, this, &TKMMainWindow::MoveTop);
    connect(ui->toolButtonUp, &QToolButton::clicked, this, &TKMMainWindow::MoveUp);
    connect(ui->toolButtonDown, &QToolButton::clicked, this, &TKMMainWindow::MoveDown);
    connect(ui->toolButtonBottom, &QToolButton::clicked, this, &TKMMainWindow::MoveBottom);

    connect(ui->pushButtonGrow, &QPushButton::clicked, this, &TKMMainWindow::DeployFormula);
    connect(ui->lineEditName, &QLineEdit::textEdited, this, &TKMMainWindow::SaveMName);
    connect(ui->plainTextEditDescription, &QPlainTextEdit::textChanged, this, &TKMMainWindow::SaveMDescription);
    connect(ui->lineEditFullName, &QLineEdit::editingFinished, this, &TKMMainWindow::SaveMFullName);
    connect(ui->lineEditGroup, &QLineEdit::editingFinished, this, &TKMMainWindow::SaveMGroup);

    InitMeasurementUnits();

    {
        const QSignalBlocker blocker(ui->comboBoxMUnits);
        ui->comboBoxMUnits->setCurrentIndex(-1);
    }

    connect(ui->comboBoxMUnits, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TKMMainWindow::SaveMUnits);

    {
        const QSignalBlocker blocker(ui->comboBoxDiagram);
        ui->comboBoxDiagram->setCurrentIndex(-1);
    }

    connect(ui->comboBoxDiagram, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &TKMMainWindow::SaveMDiagram);

    m_groupCompleter = new QCompleter(m_known.Groups(), this);
    m_groupCompleter->setCompletionMode(QCompleter::PopupCompletion);
    m_groupCompleter->setModelSorting(QCompleter::UnsortedModel);
    m_groupCompleter->setFilterMode(Qt::MatchContains);
    m_groupCompleter->setCaseSensitivity(Qt::CaseInsensitive);

    ui->lineEditGroup->setCompleter(m_groupCompleter);

    // Tab Images
    ui->toolButtonRemoveImage->setDisabled(true);
    ui->toolButtonSaveImage->setDisabled(true);

    connect(ui->toolButtonAddImage, &QToolButton::clicked, this, &TKMMainWindow::AddImage);
    connect(ui->toolButtonRemoveImage, &QToolButton::clicked, this, &TKMMainWindow::RemoveImage);
    connect(ui->toolButtonSaveImage, &QToolButton::clicked, this, &TKMMainWindow::SaveImage);

    connect(ui->listWidget, &QListWidget::itemSelectionChanged, this, &TKMMainWindow::ShowImageData);

    connect(ui->lineEditImageTitle, &QLineEdit::editingFinished, this, &TKMMainWindow::SaveImageTitle);
    connect(ui->doubleSpinBoxImageSize, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &TKMMainWindow::SaveImageSizeScale);

    // Tab info
    ui->plainTextEditKMDescription->setEnabled(true);
    ui->plainTextEditKMDescription->setPlainText(m_m->Description());
    connect(ui->plainTextEditKMDescription, &QPlainTextEdit::textChanged, this,
            &TKMMainWindow::SaveKnownMeasurementsDescription);

    ui->lineEditKMName->setEnabled(true);
    ui->lineEditKMName->setText(m_m->Name());
    connect(ui->lineEditKMName, &QLineEdit::editingFinished, this, &TKMMainWindow::SaveKnownMeasurementsName);

    connect(ui->pushButtonShowInExplorer, &QPushButton::clicked, this, [this]() { ShowInGraphicalShell(m_curFile); });
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::InitSearch()
{
    VTapeSettings *settings = MApplication::VApp()->TapeSettings();
    m_search->SetUseUnicodePreperties(settings->GetKMSearchOptionUseUnicodeProperties());
    m_search->SetMatchWord(settings->GetKMSearchOptionWholeWord());
    m_search->SetMatchRegexp(settings->GetKMSearchOptionRegexp());
    m_search->SetMatchCase(settings->GetKMSearchOptionMatchCase());

    ui->lineEditFind->setPlaceholderText(m_search->SearchPlaceholder());

    UpdateSearchControlsTooltips();

    connect(ui->lineEditFind, &QLineEdit::textChanged, this, [this](const QString &term) { m_search->Find(term); });
    connect(ui->lineEditFind, &QLineEdit::editingFinished, this,
            [this]()
            {
                SaveSearchRequest();
                InitSearchHistory();
                m_search->Find(ui->lineEditFind->text());
            });
    connect(ui->toolButtonFindPrevious, &QToolButton::clicked, this,
            [this]()
            {
                SaveSearchRequest();
                InitSearchHistory();
                m_search->FindPrevious();
                ui->labelResults->setText(
                    QStringLiteral("%1/%2").arg(m_search->MatchIndex() + 1).arg(m_search->MatchCount()));
            });
    connect(ui->toolButtonFindNext, &QToolButton::clicked, this,
            [this]()
            {
                SaveSearchRequest();
                InitSearchHistory();
                m_search->FindNext();
                ui->labelResults->setText(
                    QStringLiteral("%1/%2").arg(m_search->MatchIndex() + 1).arg(m_search->MatchCount()));
            });

    connect(m_search.data(), &VTableSearch::HasResult, this,
            [this](bool state)
            {
                ui->toolButtonFindPrevious->setEnabled(state);
                ui->toolButtonFindNext->setEnabled(state);

                if (state)
                {
                    ui->labelResults->setText(
                        QStringLiteral("%1/%2").arg(m_search->MatchIndex() + 1).arg(m_search->MatchCount()));
                }
                else
                {
                    ui->labelResults->setText(tr("0 results"));
                }

                QPalette palette;

                if (not state && not ui->lineEditFind->text().isEmpty())
                {
                    palette.setColor(QPalette::Text, Qt::red);
                    ui->lineEditFind->setPalette(palette);

                    palette.setColor(QPalette::Active, ui->labelResults->foregroundRole(), Qt::red);
                    palette.setColor(QPalette::Inactive, ui->labelResults->foregroundRole(), Qt::red);
                    ui->labelResults->setPalette(palette);
                }
                else
                {
                    ui->lineEditFind->setPalette(palette);
                    ui->labelResults->setPalette(palette);
                }
            });

    connect(ui->toolButtonCaseSensitive, &QToolButton::toggled, this,
            [this](bool checked)
            {
                m_search->SetMatchCase(checked);
                m_search->Find(ui->lineEditFind->text());
                ui->lineEditFind->setPlaceholderText(m_search->SearchPlaceholder());
            });

    connect(ui->toolButtonWholeWord, &QToolButton::toggled, this,
            [this](bool checked)
            {
                m_search->SetMatchWord(checked);
                m_search->Find(ui->lineEditFind->text());
                ui->lineEditFind->setPlaceholderText(m_search->SearchPlaceholder());
            });

    connect(ui->toolButtonRegexp, &QToolButton::toggled, this,
            [this](bool checked)
            {
                m_search->SetMatchRegexp(checked);

                if (checked)
                {
                    {
                        const QSignalBlocker blocker(ui->toolButtonWholeWord);
                        ui->toolButtonWholeWord->setChecked(false);
                        ui->toolButtonWholeWord->blockSignals(false);
                    }

                    ui->toolButtonUseUnicodeProperties->setEnabled(true);
                }
                else
                {
                    ui->toolButtonWholeWord->setEnabled(true);

                    const QSignalBlocker blocker(ui->toolButtonUseUnicodeProperties);
                    ui->toolButtonUseUnicodeProperties->setChecked(false);
                    ui->toolButtonUseUnicodeProperties->blockSignals(false);
                }
                m_search->Find(ui->lineEditFind->text());
                ui->lineEditFind->setPlaceholderText(m_search->SearchPlaceholder());
            });

    connect(ui->toolButtonUseUnicodeProperties, &QToolButton::toggled, this,
            [this](bool checked)
            {
                m_search->SetUseUnicodePreperties(checked);
                m_search->Find(ui->lineEditFind->text());
            });

    m_searchHistory->setStyleSheet(QStringLiteral("QMenu { menu-scrollable: 1; }"));
    InitSearchHistory();
    ui->pushButtonSearch->setMenu(m_searchHistory);
    ui->pushButtonSearch->setProperty("hasMenu", true);
    ui->pushButtonSearch->style()->unpolish(ui->pushButtonSearch);
    ui->pushButtonSearch->style()->polish(ui->pushButtonSearch);
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::MeasurementsWereSaved(bool saved)
{
    setWindowModified(!saved);
    not m_mIsReadOnly ? ui->actionSave->setEnabled(!saved) : ui->actionSave->setEnabled(false);
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::SetCurrentFile(const QString &fileName)
{
    m_curFile = fileName;
    if (m_curFile.isEmpty())
    {
        ui->lineEditPathToFile->setText('<'_L1 + tr("Empty") + '>'_L1);
        ui->lineEditPathToFile->setToolTip(tr("File was not saved yet."));
        ui->lineEditPathToFile->setCursorPosition(0);
        ui->pushButtonShowInExplorer->setEnabled(false);
    }
    else
    {
        ui->lineEditPathToFile->setText(QDir::toNativeSeparators(m_curFile));
        ui->lineEditPathToFile->setToolTip(QDir::toNativeSeparators(m_curFile));
        ui->lineEditPathToFile->setCursorPosition(0);
        ui->pushButtonShowInExplorer->setEnabled(true);
        auto *settings = MApplication::VApp()->TapeSettings();
        QStringList files = settings->GetRecentKMFileList();
        files.removeAll(fileName);
        files.prepend(fileName);
        while (files.size() > MaxRecentFiles)
        {
            files.removeLast();
        }
        settings->SetRecentKMFileList(files);
        UpdateRecentFileActions();
    }

    UpdateWindowTitle();
}

//---------------------------------------------------------------------------------------------------------------------
auto TKMMainWindow::SaveKnownMeasurements(const QString &fileName, QString &error) -> bool
{
    const bool result = m_m->SaveDocument(fileName, error);
    if (result)
    {
        SetCurrentFile(fileName);
        MeasurementsWereSaved(result);
    }
    return result;
}

//---------------------------------------------------------------------------------------------------------------------
auto TKMMainWindow::MaybeSave() -> bool
{
    if (!isWindowModified())
    {
        return true;
    }

    if (m_curFile.isEmpty() && ui->tableWidget->rowCount() == 0)
    {
        return true; // Don't ask if file was created without modifications.
    }

    QScopedPointer<QMessageBox> const messageBox(
        new QMessageBox(QMessageBox::Warning, tr("Unsaved changes"),
                        tr("Measurements have been modified. Do you want to save your changes?"),
                        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, this, Qt::Sheet));

    messageBox->setDefaultButton(QMessageBox::Yes);
    messageBox->setEscapeButton(QMessageBox::Cancel);

    if (QAbstractButton *button = messageBox->button(QMessageBox::Yes))
    {
        button->setText(m_curFile.isEmpty() || m_mIsReadOnly ? tr("Save…") : tr("Save"));
    }

    if (QAbstractButton *button = messageBox->button(QMessageBox::No))
    {
        button->setText(tr("Don't Save"));
    }

    messageBox->setWindowModality(Qt::ApplicationModal);
    messageBox->setFixedSize(300, 85);

    switch (static_cast<QMessageBox::StandardButton>(messageBox->exec()))
    {
        case QMessageBox::Yes:
            if (m_mIsReadOnly)
            {
                return FileSaveAs();
            }
            else
            {
                return FileSave();
            }
        case QMessageBox::No:
            return true;
        case QMessageBox::Cancel:
            return false;
        default:
            break;
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::UpdatePadlock(bool ro)
{
    ui->actionReadOnly->setChecked(ro);
    ui->actionReadOnly->setIcon(ro ? QIcon("://tapeicon/24x24/padlock_locked.png")
                                   : QIcon("://tapeicon/24x24/padlock_opened.png"));
    ui->actionReadOnly->setDisabled(m_mIsReadOnly);
}

//---------------------------------------------------------------------------------------------------------------------
auto TKMMainWindow::AddCell(const QString &text, int row, int column, int aligment) -> QTableWidgetItem *
{
    auto *item = new QTableWidgetItem(text);
    SetTextAlignment(item, static_cast<Qt::Alignment>(aligment));
    item->setToolTip(text);

    // set the item non-editable (view only), and non-selectable
    Qt::ItemFlags flags = item->flags();
    flags &= ~(Qt::ItemIsEditable); // reset/clear the flag
    item->setFlags(flags);

    ui->tableWidget->setItem(row, column, item);

    return item;
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::ReadSettings()
{
    VTapeSettings *settings = MApplication::VApp()->TapeSettings();

    if (settings->status() == QSettings::NoError)
    {
        restoreGeometry(settings->GetKMGeometry());
        restoreState(settings->GetKMToolbarsState(), static_cast<int>(AppVersion()));

        // Text under tool buton icon
        ToolBarStyles();

        settings->RestoreKMMainWindowColumnWidths(ui->tableWidget);

        // Stack limit
        // VAbstractApplication::VApp()->getUndoStack()->setUndoLimit(settings->GetUndoCount());

        if (VAbstractShortcutManager *manager = VAbstractApplication::VApp()->GetShortcutManager())
        {
            manager->UpdateShortcuts();
        }
    }
    else
    {
        qWarning() << tr("Cannot read settings from a malformed .INI file.");
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::WriteSettings()
{
    VTapeSettings *settings = MApplication::VApp()->TapeSettings();
    settings->SetKMGeometry(saveGeometry());
    settings->SetKMToolbarsState(saveState(static_cast<int>(AppVersion())));

    settings->SetKMSearchOptionMatchCase(m_search->IsMatchCase());
    settings->SetKMSearchOptionWholeWord(m_search->IsMatchWord());
    settings->SetKMSearchOptionRegexp(m_search->IsMatchRegexp());
    settings->SetKMSearchOptionUseUnicodeProperties(m_search->IsUseUnicodePreperties());

    settings->SaveKMMainWindowColumnWidths(ui->tableWidget);

    settings->sync();
    if (settings->status() == QSettings::AccessError)
    {
        qWarning() << tr("Cannot save settings. Access denied.");
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::InitIcons()
{
    auto const iconResource = QStringLiteral("icon");
    ui->toolButtonAddImage->setIcon(VTheme::GetIconResource(iconResource, QStringLiteral("16x16/insert-image.png")));
    ui->toolButtonRemoveImage->setIcon(VTheme::GetIconResource(iconResource, QStringLiteral("16x16/remove-image.png")));

    if (int const index = ui->tabWidget->indexOf(ui->tabImages); index != -1)
    {
        ui->tabWidget->setTabIcon(index, VTheme::GetIconResource(iconResource, QStringLiteral("16x16/viewimage.png")));
    }

    auto const tapeIconResource = QStringLiteral("tapeicon");
    ui->actionMeasurementDiagram->setIcon(
        VTheme::GetIconResource(tapeIconResource, QStringLiteral("24x24/mannequin.png")));
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::InitSearchHistory()
{
    QStringList const searchHistory = MApplication::VApp()->TapeSettings()->GetTapeSearchHistory();
    m_searchHistory->clear();

    if (searchHistory.isEmpty())
    {
        QAction *action = m_searchHistory->addAction('<'_L1 + tr("Empty", "list") + '>'_L1);
        action->setDisabled(true);
        return;
    }

    for (const auto &term : searchHistory)
    {
        QAction *action = m_searchHistory->addAction(term);
        action->setData(term);
        connect(action, &QAction::triggered, this,
                [this]()
                {
                    auto *action = qobject_cast<QAction *>(sender());
                    if (action != nullptr)
                    {
                        QString const term = action->data().toString();
                        ui->lineEditFind->setText(term);
                        m_search->Find(term);
                        ui->lineEditFind->setFocus();
                    }
                });
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::SaveSearchRequest()
{
    QStringList searchHistory = MApplication::VApp()->TapeSettings()->GetKMSearchHistory();
    QString const term = ui->lineEditFind->text();
    if (term.isEmpty())
    {
        return;
    }

    searchHistory.removeAll(term);
    searchHistory.prepend(term);
    while (searchHistory.size() > VTableSearch::MaxHistoryRecords)
    {
        searchHistory.removeLast();
    }
    MApplication::VApp()->TapeSettings()->SetKMSearchHistory(searchHistory);
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::UpdateSearchControlsTooltips()
{
    auto UpdateToolTip = [this](QAbstractButton *button)
    {
        if (button->toolTip().contains("%1"_L1))
        {
            m_serachButtonTooltips.insert(button, button->toolTip());
            button->setToolTip(button->toolTip().arg(button->shortcut().toString(QKeySequence::NativeText)));
        }
        else if (m_serachButtonTooltips.contains(button))
        {
            QString const tooltip = m_serachButtonTooltips.value(button);
            button->setToolTip(tooltip.arg(button->shortcut().toString(QKeySequence::NativeText)));
        }
    };

    UpdateToolTip(ui->toolButtonCaseSensitive);
    UpdateToolTip(ui->toolButtonWholeWord);
    UpdateToolTip(ui->toolButtonRegexp);
    UpdateToolTip(ui->toolButtonUseUnicodeProperties);
    UpdateToolTip(ui->pushButtonSearch);
    UpdateToolTip(ui->toolButtonFindPrevious);
    UpdateToolTip(ui->toolButtonFindNext);
}

//---------------------------------------------------------------------------------------------------------------------
auto TKMMainWindow::UnknownMeasurementImage() -> QString
{
    return u"<html><head/><body><p><span style=\" font-size:340pt;\">?</span></p>"
           u"<p align=\"center\">%1</p></body></html>"_s.arg(tr("Unknown measurement"));
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::CreateWindowMenu(QMenu *menu)
{
    SCASSERT(menu != nullptr)

    QAction *action = menu->addAction(tr("&New Window"));
    connect(action, &QAction::triggered, this, []() { MApplication::VApp()->NewMainKMWindow()->activateWindow(); });
    action->setMenuRole(QAction::NoRole);

    menu->addAction(ui->actionAlwaysOnTop);
    connect(ui->actionAlwaysOnTop, &QAction::triggered, this, &TKMMainWindow::AlwaysOnTop);

    menu->addSeparator();

    const QList<TKMMainWindow *> windows = MApplication::VApp()->MainKMWindows();
    for (int i = 0; i < windows.count(); ++i)
    {
        TKMMainWindow *window = windows.at(i);

        auto title = QStringLiteral("%1. %2").arg(i + 1).arg(window->windowTitle());
        if (const auto index = title.lastIndexOf("[*]"_L1); index != -1)
        {
            window->isWindowModified() ? title.replace(index, 3, '*'_L1) : title.replace(index, 3, QString());
        }

        QAction *action = menu->addAction(title, this, &TKMMainWindow::ShowWindow);
        action->setData(i);
        action->setCheckable(true);
        action->setMenuRole(QAction::NoRole);
        if (window->isActiveWindow())
        {
            action->setChecked(true);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::RefreshTable()
{
    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    {
        const QSignalBlocker blocker(ui->tableWidget);
        ui->tableWidget->clearContents();

        if (!m_known.IsValid())
        {
            m_known = m_m->KnownMeasurements();
        }

        const QMap<int, VKnownMeasurement> orderedTable = m_known.OrderedMeasurements();
        qint32 currentRow = -1;
        ui->tableWidget->setRowCount(static_cast<int>(orderedTable.size()));
        for (auto iMap = orderedTable.constBegin(); iMap != orderedTable.constEnd(); ++iMap)
        {
            const VKnownMeasurement &m = iMap.value();
            currentRow++;

            QTableWidgetItem *item = AddCell(m.name, currentRow, ColumnName, Qt::AlignVCenter); // name
            item->setData(Qt::UserRole, m.name);

            AddCell(m.fullName, currentRow, ColumnFullName, Qt::AlignVCenter);
            AddCell(m.group, currentRow, ColumnGroup, Qt::AlignVCenter);
        }
    }

    ui->actionExportToCSV->setEnabled(ui->tableWidget->rowCount() > 0);

    m_groupCompleter->setModel(new QStringListModel(m_known.Groups(), m_groupCompleter));

    QGuiApplication::restoreOverrideCursor();
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::RefreshImages()
{
    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    int const row = ui->listWidget->currentRow();

    const QSignalBlocker blocker(ui->listWidget);
    ui->listWidget->clear();

    if (!m_known.IsValid())
    {
        m_known = m_m->KnownMeasurements();
    }
    QMap<QUuid, VPatternImage> const images = m_known.Images();

    int index = 1;
    for (auto i = images.cbegin(), end = images.cend(); i != end; ++i)
    {
        if (i.key().isNull())
        {
            continue;
        }

        auto *item = new QListWidgetItem(ui->listWidget);
        item->setTextAlignment(Qt::AlignCenter);

        if (i.value().IsValid())
        {
            QSize const size = i.value().Size();
            QSize const targetSize = ui->listWidget->iconSize();

            auto const scalingFactorWidth = static_cast<double>(targetSize.width()) / size.width();
            auto const scalingFactorHeight = static_cast<double>(targetSize.height()) / size.height();

            int newWidth = 0;
            int newHeight = 0;

            if (scalingFactorWidth < scalingFactorHeight)
            {
                newWidth = targetSize.width();
                newHeight = static_cast<int>(size.height() * scalingFactorWidth);
            }
            else
            {
                newWidth = static_cast<int>(size.width() * scalingFactorHeight);
                newHeight = targetSize.height();
            }

            QPixmap background(targetSize);
            background.fill(Qt::transparent);

            QPainter painter(&background);
            QPixmap const sourcePixmap = i.value().GetPixmap(newWidth, newHeight);

            // Calculate the position to center the source pixmap in the transparent pixmap
            int const x = (background.width() - sourcePixmap.width()) / 2;
            int const y = background.height() - sourcePixmap.height();

            painter.drawPixmap(x, y, sourcePixmap);
            painter.end();

            item->setIcon(background);
        }
        else
        {
            QImageReader imageReader(QStringLiteral("://icon/svg/broken_path.svg"));
            imageReader.setScaledSize(ui->listWidget->iconSize());
            QImage const image = imageReader.read();
            item->setIcon(QPixmap::fromImage(image));
        }

        QString title = i.value().Title();
        if (title.isEmpty())
        {
            title = tr("Unnamed image %1").arg(index);
            ++index;
        }

        item->setText(title);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        item->setData(Qt::UserRole, i.key());
    }

    ui->listWidget->setCurrentRow(row);

    QGuiApplication::restoreOverrideCursor();
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::ShowMDiagram(const VPatternImage &image)
{
    ui->labelDiagram->setResizedPixmap(QPixmap());
    ui->labelDiagram->setCursor(QCursor());

    disconnect(ui->labelDiagram, &VAspectRatioPixmapLabel::clicked, this, &TKMMainWindow::ShowImage);

    if (image.IsValid())
    {
        ui->labelDiagram->setCursor(Qt::PointingHandCursor);
        ui->labelDiagram->setResizedPixmap(image.GetPixmap());
        connect(ui->labelDiagram, &VAspectRatioPixmapLabel::clicked, this, &TKMMainWindow::ShowImage,
                Qt::UniqueConnection);
    }
    else
    {
        ui->labelDiagram->setText(UnknownMeasurementImage());
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto TKMMainWindow::Open(const QString &pathTo, const QString &filter) -> QString
{
    const QString mPath = QFileDialog::getOpenFileName(this, tr("Open file"), pathTo, filter, nullptr,
                                                       VAbstractApplication::VApp()->NativeFileDialog());

    if (not mPath.isEmpty())
    {
        if (m_m == nullptr)
        {
            LoadFile(mPath);
        }
        else
        {
            MApplication::VApp()->NewMainKMWindow()->LoadFile(mPath);
        }
    }

    return mPath;
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::Controls()
{
    ui->toolButtonRemoveMeasurement->setEnabled(ui->tableWidget->rowCount() > 0);

    if (ui->tableWidget->rowCount() >= 2)
    {
        if (ui->tableWidget->currentRow() == 0)
        {
            ui->toolButtonTop->setEnabled(false);
            ui->toolButtonUp->setEnabled(false);
            ui->toolButtonDown->setEnabled(true);
            ui->toolButtonBottom->setEnabled(true);
        }
        else if (ui->tableWidget->currentRow() == ui->tableWidget->rowCount() - 1)
        {
            ui->toolButtonTop->setEnabled(true);
            ui->toolButtonUp->setEnabled(true);
            ui->toolButtonDown->setEnabled(false);
            ui->toolButtonBottom->setEnabled(false);
        }
        else
        {
            ui->toolButtonTop->setEnabled(true);
            ui->toolButtonUp->setEnabled(true);
            ui->toolButtonDown->setEnabled(true);
            ui->toolButtonBottom->setEnabled(true);
        }
    }
    else
    {
        ui->toolButtonTop->setEnabled(false);
        ui->toolButtonUp->setEnabled(false);
        ui->toolButtonDown->setEnabled(false);
        ui->toolButtonBottom->setEnabled(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::MFields(bool enabled)
{
    ui->lineEditName->setEnabled(enabled);
    ui->plainTextEditDescription->setEnabled(enabled);
    ui->lineEditFullName->setEnabled(enabled);
    ui->comboBoxMUnits->setEnabled(enabled);
    ui->comboBoxDiagram->setEnabled(enabled);
    ui->pushButtonGrow->setEnabled(enabled);
    ui->plainTextEditFormula->setEnabled(enabled);
    ui->lineEditGroup->setEnabled(enabled);

    ui->lineEditFind->setEnabled(enabled);
    if (enabled && not ui->lineEditFind->text().isEmpty())
    {
        ui->toolButtonFindPrevious->setEnabled(enabled);
        ui->toolButtonFindNext->setEnabled(enabled);
    }
    else
    {
        ui->toolButtonFindPrevious->setEnabled(false);
        ui->toolButtonFindNext->setEnabled(false);
    }

    Controls();
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::ImageFields(bool enabled)
{
    ui->lineEditImageTitle->setEnabled(enabled);
    ui->toolButtonRemoveImage->setEnabled(enabled);
    ui->toolButtonSaveImage->setEnabled(enabled);
    ui->doubleSpinBoxImageSize->setEnabled(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
auto TKMMainWindow::GenerateMeasurementName() const -> QString
{
    QHash<QString, VKnownMeasurement> const m = m_known.Measurements();
    qint32 num = 1;
    QString name;
    do
    {
        name = VAbstractApplication::VApp()->TrVars()->InternalVarToUser(measurement_) + QString::number(num);
        num++;
    } while (m.contains(name));

    return name;
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::InitMeasurementUnits()
{
    const QSignalBlocker blocker(ui->comboBoxMUnits);

    int current = -1;
    if (ui->comboBoxMUnits->currentIndex() != -1)
    {
        current = ui->comboBoxMUnits->currentData().toInt();
    }

    ui->comboBoxMUnits->clear();
    ui->comboBoxMUnits->addItem(tr("Length units"), QVariant(static_cast<int>(MUnits::Table)));
    ui->comboBoxMUnits->addItem(tr("Degrees"), QVariant(static_cast<int>(MUnits::Degrees)));

    if (int const i = ui->comboBoxMUnits->findData(current); i != -1)
    {
        ui->comboBoxMUnits->setCurrentIndex(i);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::InitMeasurementDiagramList()
{
    ui->comboBoxDiagram->clear();

    QMap<QUuid, VPatternImage> const images = m_known.Images();

    ui->comboBoxDiagram->addItem(tr("None"), QUuid());

    int index = 1;
    for (auto i = images.cbegin(), end = images.cend(); i != end; ++i)
    {
        QString title = i.value().Title();
        if (title.isEmpty())
        {
            title = tr("Unnamed image %1").arg(index);
            ++index;
        }

        ui->comboBoxDiagram->addItem(title, i.key());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::ShowError(const QString &text)
{
    QMessageBox messageBox(this);
    messageBox.setIcon(QMessageBox::Critical);
    messageBox.setText(text);
    messageBox.setStandardButtons(QMessageBox::Ok);
    messageBox.setDefaultButton(QMessageBox::Ok);
    messageBox.exec();
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::RefreshDataAfterImport()
{
    const int currentRow = ui->tableWidget->currentRow();
    m_search->AddRow(currentRow);
    m_known = VKnownMeasurements();
    RefreshTable();
    m_search->RefreshList(ui->lineEditFind->text());
    ui->tableWidget->selectRow(currentRow);
    ui->actionExportToCSV->setEnabled(ui->tableWidget->rowCount() > 0);
    MeasurementsWereSaved(false);
}

//---------------------------------------------------------------------------------------------------------------------
auto TKMMainWindow::CheckMName(const QString &name, const QSet<QString> &importedNames) const -> QString
{
    if (name.isEmpty())
    {
        throw VException(tr("Measurement name is empty."));
    }

    if (importedNames.contains(name))
    {
        throw VException(tr("Imported file must not contain the same name twice."));
    }

    if (QRegularExpression const rx(NameRegExp(VariableRegex::KnownMeasurement)); not rx.match(name).hasMatch())
    {
        throw VException(tr("Measurement '%1' doesn't match regex pattern.").arg(name));
    }

    if (m_known.Measurements().contains(name))
    {
        throw VException(tr("Measurement '%1' already used in the file.").arg(name));
    }

    return name;
}

//---------------------------------------------------------------------------------------------------------------------
auto TKMMainWindow::CSVColumnHeader(int column) const -> QString
{
    switch (column)
    {
        case 0: // name
            return tr("Name", "measurement column");
        case 1: // full name
            return tr("Full name", "measurement column");
        case 2: // group
            return tr("Group", "measurement column");
        case 3: // formula
            return tr("Formula", "measurement column");
        case 4: // description
            return tr("Description", "measurement column");
        default:
            return {};
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto TKMMainWindow::ReadCSV(const QxtCsvModel &csv, const QVector<int> &map, bool withHeader)
    -> QVector<VKnownMeasurement>
{
    QVector<VKnownMeasurement> measurements;
    QSet<QString> importedNames;

    const int rows = csv.rowCount();

    for (int i = 0; i < rows; ++i)
    {
        try
        {
            const int nameColumn = map.at(static_cast<int>(KnownMeasurementsColumns::Name));
            const QString name = csv.text(i, nameColumn).simplified();
            if (name.isEmpty())
            {
                ShowError(
                    QApplication::translate("TKMMainWindow", "Error in row %1. The measurement name is empty.").arg(i));
                continue;
            }

            VKnownMeasurement measurement;
            const QString mName = CheckMName(name, importedNames);
            importedNames.insert(mName);
            measurement.name = mName;

            if (const int nameGroup = map.at(static_cast<int>(KnownMeasurementsColumns::Group)); nameGroup >= 0)
            {
                measurement.group = csv.text(i, nameGroup).simplified();
            }

            if (const int nameFullName = map.at(static_cast<int>(KnownMeasurementsColumns::FullName));
                nameFullName >= 0)
            {
                measurement.fullName = csv.text(i, nameFullName);
            }

            if (const int nameFormula = map.at(static_cast<int>(KnownMeasurementsColumns::Formula)); nameFormula >= 0)
            {
                measurement.formula = csv.text(i, nameFormula);
            }

            if (const int nameDescription = map.at(static_cast<int>(KnownMeasurementsColumns::Description));
                nameDescription >= 0)
            {
                measurement.description = csv.text(i, nameDescription);
            }

            measurements.append(measurement);
        }
        catch (VException &e)
        {
            int rowIndex = i + 1;
            if (withHeader)
            {
                ++rowIndex;
            }
            ShowError(
                QApplication::translate("TKMMainWindow", "Error in row %1. %2").arg(rowIndex).arg(e.ErrorMessage()));
            return {};
        }
    }

    return measurements;
}

//---------------------------------------------------------------------------------------------------------------------
void TKMMainWindow::ImportKnownMeasurements(const QxtCsvModel &csv, const QVector<int> &map, bool withHeader)
{
    if (csv.columnCount() < 2)
    {
        ShowError(tr("Individual measurements require at least 2 columns."));
        return;
    }

    QVector<VKnownMeasurement> measurements = ReadCSV(csv, map, withHeader);
    if (measurements.isEmpty())
    {
        return;
    }

    for (const auto &im : std::as_const(measurements))
    {
        m_m->AddEmptyMeasurement(im.name);

        if (not im.group.isEmpty())
        {
            m_m->SetMGroup(im.name, im.group);
        }

        if (not im.fullName.isEmpty())
        {
            m_m->SetMFullName(im.name, im.fullName);
        }

        if (not im.formula.isEmpty())
        {
            m_m->SetMFormula(im.name, im.formula);
        }

        if (not im.description.isEmpty())
        {
            m_m->SetMDescription(im.name, im.description);
        }
    }

    RefreshDataAfterImport();
}
