/************************************************************************
 **
 **  @file   watermarkwindow.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   24 12, 2019
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2019 Valentina project
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
#include "watermarkwindow.h"
#include "ui_watermarkwindow.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDialog>
#include <QFuture>
#include <QMessageBox>
#include <QStyle>
#include <QtConcurrent>
#include <memory>

#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#include "../vmisc/backport/qoverload.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 7, 0)

#include "../ifc/exception/vexception.h"
#include "../ifc/xml/vwatermarkconverter.h"
#include "../vformat/vwatermark.h"
#include "../vmisc/def.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpropertyexplorer/checkablemessagebox.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
WatermarkWindow::WatermarkWindow(const QString &patternPath, QWidget *parent)
  : QMainWindow(parent),
    ui(new Ui::WatermarkWindow),
    m_patternPath(patternPath)
{
    ui->setupUi(this);
    m_okPathColor = ui->lineEditPath->palette().color(ui->lineEditPath->foregroundRole());

    VAbstractApplication::VApp()->Settings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());
    UpdateWindowTitle();

    m_curFileFormatVersion = VWatermarkConverter::WatermarkMaxVer;
    m_curFileFormatVersionStr = VWatermarkConverter::WatermarkMaxVerStr;

    ToolBarStyle(ui->toolBar);

    connect(ui->spinBoxOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this]() { WatermarkChangesWereSaved(false); });

    connect(ui->lineEditText, &QLineEdit::textChanged, this, [this]() { WatermarkChangesWereSaved(false); });

    connect(ui->spinBoxTextRotation, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this]() { WatermarkChangesWereSaved(false); });

    connect(ui->toolButtonFont, &QToolButton::clicked, this,
            [this]()
            {
                bool ok;
                QFont font = QFontDialog::getFont(&ok, m_data.font, this);
                if (ok)
                {
                    WatermarkChangesWereSaved(false);
                    m_data.font = font;
                    ui->lineEditFontSample->setFont(font);
                }
            });

    connect(ui->lineEditPath, &QLineEdit::textChanged, this,
            [this]()
            {
                WatermarkChangesWereSaved(false);
                ValidatePath();
            });

    connect(ui->pushButtonBrowse, &QPushButton::clicked, this,
            [this]()
            {
                const QString filter = tr("Images") + " (*.png *.jpg *.jpeg *.bmp)"_L1;
                const QString fileName =
                    QFileDialog::getOpenFileName(this, tr("Watermark image"), QString(), filter, nullptr,
                                                 VAbstractApplication::VApp()->NativeFileDialog());
                if (not fileName.isEmpty())
                {
                    ui->lineEditPath->setText(fileName);
                }
            });

    connect(ui->spinBoxImageRotation, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this]() { WatermarkChangesWereSaved(false); });

    connect(ui->checkBoxGrayColor, &QCheckBox::stateChanged, this, [this]() { WatermarkChangesWereSaved(false); });

    connect(ui->groupBoxWatermarkText, &QGroupBox::toggled, this, [this]() { WatermarkChangesWereSaved(false); });
    connect(ui->groupBoxWatermarkImage, &QGroupBox::toggled, this, [this]() { WatermarkChangesWereSaved(false); });

    ui->pushButtonColorPicker->insertColor(Qt::black, tr("Black", "color"));
    ui->pushButtonColorPicker->insertColor(Qt::red, tr("Red", "color"));
    ui->pushButtonColorPicker->insertColor(Qt::darkRed, tr("Dark red", "color"));
    ui->pushButtonColorPicker->insertColor(Qt::green, tr("Green", "color"));
    ui->pushButtonColorPicker->insertColor(Qt::darkGreen, tr("Dark green", "color"));
    ui->pushButtonColorPicker->insertColor(Qt::blue, tr("Blue", "color"));
    ui->pushButtonColorPicker->insertColor(Qt::darkBlue, tr("Dark blue", "color"));
    ui->pushButtonColorPicker->insertColor(Qt::cyan, tr("Cyan", "color"));
    ui->pushButtonColorPicker->insertColor(Qt::darkCyan, tr("Dark cyan", "color"));
    ui->pushButtonColorPicker->insertColor(Qt::magenta, tr("Magenta", "color"));
    ui->pushButtonColorPicker->insertColor(Qt::darkMagenta, tr("Dark magenta", "color"));
    ui->pushButtonColorPicker->insertColor(Qt::yellow, tr("Yellow", "color"));
    ui->pushButtonColorPicker->insertColor(Qt::darkYellow, tr("Dark yellow", "color"));
    ui->pushButtonColorPicker->insertColor(Qt::gray, tr("Gray", "color"));
    ui->pushButtonColorPicker->insertColor(Qt::darkGray, tr("Dark gray", "color"));
    ui->pushButtonColorPicker->insertColor(Qt::lightGray, tr("Light gray", "color"));

    QVector<QColor> colors = VAbstractApplication::VApp()->Settings()->GetWatermarkCustomColors();
    for (const auto &color : colors)
    {
        ui->pushButtonColorPicker->insertColor(color);
    }

    connect(ui->pushButtonColorPicker, &QtColorPicker::colorChanged, this,
            [this]() { WatermarkChangesWereSaved(false); });
}

//---------------------------------------------------------------------------------------------------------------------
WatermarkWindow::~WatermarkWindow()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto WatermarkWindow::CurrentFile() const -> QString
{
    return m_curFile;
}

//---------------------------------------------------------------------------------------------------------------------
auto WatermarkWindow::Open(QString path) -> bool
{
    qDebug("Loading new watermark file %s.", qUtf8Printable(path));

    if (path.isEmpty())
    {
        qDebug("Path is empty");
        Clear();
        return false;
    }

    // Convert to absolute path if need
    path = AbsoluteMPath(m_patternPath, path);

    QFuture<VWatermarkConverter *> futureConverter = QtConcurrent::run(
        [path]()
        {
            std::unique_ptr<VWatermarkConverter> converter(new VWatermarkConverter(path));
            return converter.release();
        });

    // We have unsaved changes or load more then one file per time
    if (OpenNewEditor(path))
    {
        return false;
    }

    qDebug("Loking watermark file");
    VlpCreateLock(lock, path);

    if (lock->IsLocked())
    {
        qDebug("Watermark file %s was locked.", qUtf8Printable(path));
    }
    else
    {
        if (not IgnoreLocking(lock->GetLockError(), path))
        {
            return false;
        }
    }

    VWatermark doc;

    try
    {
        QScopedPointer<VWatermarkConverter> converter(futureConverter.result());
        m_curFileFormatVersion = converter->GetCurrentFormatVersion();
        m_curFileFormatVersionStr = converter->GetFormatVersionStr();
        doc.setXMLContent(converter->Convert());
    }
    catch (VException &e)
    {
        qCritical("%s\n\n%s\n\n%s", qUtf8Printable(tr("File error.")), qUtf8Printable(e.ErrorMessage()),
                  qUtf8Printable(e.DetailedInformation()));
        Clear();
        return false;
    }

    m_curFile = path;
    UpdateWindowTitle();
    m_data = doc.GetWatermark();
    ShowWatermark();

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void WatermarkWindow::closeEvent(QCloseEvent *event)
{
#if defined(Q_OS_MAC) && QT_VERSION < QT_VERSION_CHECK(5, 11, 1)
    // Workaround for Qt bug https://bugreports.qt.io/browse/QTBUG-43344
    static int numCalled = 0;
    if (numCalled++ >= 1)
    {
        return;
    }
#endif

    if (MaybeSave())
    {
        event->accept();
        deleteLater();
    }
    else
    {
        event->ignore();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void WatermarkWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);

        UpdateWindowTitle();
    }
    // remember to call base class implementation
    QMainWindow::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void WatermarkWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (event->spontaneous())
    {
        return;
    }

    if (m_isInitialized)
    {
        return;
    }
    // do your init stuff here

    QSize sz = VAbstractApplication::VApp()->Settings()->GetWatermarkEditorSize();
    if (not sz.isEmpty())
    {
        resize(sz);
    }

    m_isInitialized = true; // first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void WatermarkWindow::resizeEvent(QResizeEvent *event)
{
    // remember the size for the next time this window is opened, but only
    // if the window was already initialized, which rules out the resize at
    // window creating, which would
    if (m_isInitialized)
    {
        VAbstractApplication::VApp()->Settings()->SetWatermarkEditorSize(size());
    }
    QMainWindow::resizeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void WatermarkWindow::on_actionNew_triggered()
{
    emit New();
}

//---------------------------------------------------------------------------------------------------------------------
auto WatermarkWindow::on_actionSaveAs_triggered() -> bool
{
    QString filters(tr("Watermark files") + "(*.vwm)"_L1);
    QString dir;
    if (m_curFile.isEmpty())
    {
        dir = QDir::homePath();
    }
    else
    {
        dir = QFileInfo(m_curFile).absolutePath();
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save as"), dir + '/'_L1 + tr("watermark") + ".vwm"_L1,
                                                    filters, nullptr, VAbstractApplication::VApp()->NativeFileDialog());

    if (fileName.isEmpty())
    {
        return false;
    }

    QFileInfo f(fileName);
    if (f.suffix().isEmpty() && f.suffix() != "vwm"_L1)
    {
        fileName += ".vwm"_L1;
    }

    if (f.exists() && m_curFile != fileName)
    {
        // Temporary try to lock the file before saving
        VLockGuard<char> tmp(fileName);
        if (not tmp.IsLocked())
        {
            qCritical("%s", qUtf8Printable(tr("Failed to lock. This file already opened in another window.")));
            return false;
        }
    }

    QString error;
    const bool result = SaveWatermark(fileName, error);
    if (not result)
    {
        QMessageBox messageBox(this);
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.setInformativeText(tr("Could not save file"));
        messageBox.setDefaultButton(QMessageBox::Ok);
        messageBox.setDetailedText(error);
        messageBox.setStandardButtons(QMessageBox::Ok);
        messageBox.exec();

        return result;
    }

    if (m_curFile == fileName && not lock.isNull())
    {
        lock->Unlock();
    }
    VlpCreateLock(lock, fileName);

    if (lock->IsLocked())
    {
        qDebug("Watermark file %s was locked.", qUtf8Printable(fileName));
    }
    else
    {
        qDebug("Failed to lock %s", qUtf8Printable(fileName));
        qDebug("Error type: %d", lock->GetLockError());
        qCritical("%s", qUtf8Printable(tr("Failed to lock. This file already opened in another window. Expect "
                                          "collissions when run 2 copies of the program.")));
    }

    return result;
}

//---------------------------------------------------------------------------------------------------------------------
auto WatermarkWindow::on_actionSave_triggered() -> bool
{
    if (m_curFile.isEmpty())
    {
        return on_actionSaveAs_triggered();
    }

    if (m_curFileFormatVersion < VWatermarkConverter::WatermarkMaxVer &&
        not ContinueFormatRewrite(m_curFileFormatVersionStr, VWatermarkConverter::WatermarkMaxVerStr))
    {
        return false;
    }

    // #ifdef Q_OS_WIN32
    //         qt_ntfs_permission_lookup++; // turn checking on
    // #endif /*Q_OS_WIN32*/
    const bool isFileWritable = QFileInfo(m_curFile).isWritable();
    // #ifdef Q_OS_WIN32
    //         qt_ntfs_permission_lookup--; // turn it off again
    // #endif /*Q_OS_WIN32*/
    if (not isFileWritable)
    {
        QMessageBox messageBox(this);
        messageBox.setIcon(QMessageBox::Question);
        messageBox.setText(tr("The document has no write permissions."));
        messageBox.setInformativeText(tr("Do you want to change the permissions?"));
        messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        messageBox.setDefaultButton(QMessageBox::Yes);

        if (messageBox.exec() == QMessageBox::Yes)
        {
            // #ifdef Q_OS_WIN32
            //                 qt_ntfs_permission_lookup++; // turn checking on
            // #endif /*Q_OS_WIN32*/
            bool changed =
                QFile::setPermissions(m_curFile, QFileInfo(m_curFile).permissions() | QFileDevice::WriteUser);
            // #ifdef Q_OS_WIN32
            //                 qt_ntfs_permission_lookup--; // turn it off again
            // #endif /*Q_OS_WIN32*/

            if (not changed)
            {
                QMessageBox messageBoxWarning(this);
                messageBoxWarning.setIcon(QMessageBox::Warning);
                messageBoxWarning.setText(tr("Cannot set permissions for %1 to writable.").arg(m_curFile));
                messageBoxWarning.setInformativeText(tr("Could not save the file."));
                messageBoxWarning.setDefaultButton(QMessageBox::Ok);
                messageBoxWarning.setStandardButtons(QMessageBox::Ok);
                messageBoxWarning.exec();
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    QString error;
    bool result = SaveWatermark(m_curFile, error);
    if (result)
    {
        m_curFileFormatVersion = VWatermarkConverter::WatermarkMaxVer;
        m_curFileFormatVersionStr = VWatermarkConverter::WatermarkMaxVerStr;
    }
    else
    {
        QMessageBox messageBox(this);
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.setText(tr("Could not save the file"));
        messageBox.setDefaultButton(QMessageBox::Ok);
        messageBox.setDetailedText(error);
        messageBox.setStandardButtons(QMessageBox::Ok);
        messageBox.exec();
    }
    return result;
}

//---------------------------------------------------------------------------------------------------------------------
void WatermarkWindow::on_actionOpen_triggered()
{
    qDebug("Openning new watermark file.");
    const QString filter(tr("Watermark files") + " (*.vwm)"_L1);
    QString dir = QDir::homePath();
    qDebug("Run QFileDialog::getOpenFileName: dir = %s.", qUtf8Printable(dir));
    const QString filePath = QFileDialog::getOpenFileName(this, tr("Open file"), dir, filter, nullptr,
                                                          VAbstractApplication::VApp()->NativeFileDialog());
    if (filePath.isEmpty())
    {
        return;
    }
    Open(filePath);
}

//---------------------------------------------------------------------------------------------------------------------
void WatermarkWindow::on_actionExit_triggered()
{
    close();
}

//---------------------------------------------------------------------------------------------------------------------
void WatermarkWindow::WatermarkChangesWereSaved(bool saved)
{
    const bool state = /*doc->IsModified() ||*/ !saved;
    setWindowModified(state);
}

//---------------------------------------------------------------------------------------------------------------------
auto WatermarkWindow::MaybeSave() -> bool
{
    if (this->isWindowModified())
    {
        QScopedPointer<QMessageBox> messageBox(new QMessageBox(
            tr("Unsaved changes"), tr("The watermark has been modified. Do you want to save your changes?"),
            QMessageBox::Warning, QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel, this, Qt::Sheet));

        messageBox->setDefaultButton(QMessageBox::Yes);
        messageBox->setEscapeButton(QMessageBox::Cancel);

        messageBox->setButtonText(QMessageBox::Yes, m_curFile.isEmpty() ? tr("Save") : tr("Save as"));
        messageBox->setButtonText(QMessageBox::No, tr("Don't Save"));

        messageBox->setWindowModality(Qt::ApplicationModal);
        const auto ret = static_cast<QMessageBox::StandardButton>(messageBox->exec());

        switch (ret)
        {
            case QMessageBox::Yes:
                return m_curFile.isEmpty() ? on_actionSaveAs_triggered() : on_actionSave_triggered();
            case QMessageBox::No:
                return true;
            case QMessageBox::Cancel:
                return false;
            default:
                break;
        }
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void WatermarkWindow::UpdateWindowTitle()
{
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
    }

    if (isFileWritable)
    {
        setWindowTitle(GetWatermarkFileName());
    }
    else
    {
        setWindowTitle(GetWatermarkFileName() + " ("_L1 + tr("read only") + ')'_L1);
    }
    setWindowFilePath(m_curFile);

#if defined(Q_OS_MAC)
    static QIcon fileIcon = QIcon(QCoreApplication::applicationDirPath() + "/../Resources/Valentina.icns"_L1);
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
auto WatermarkWindow::GetWatermarkFileName() -> QString
{
    QString shownName = tr("untitled.vwm");
    if (not m_curFile.isEmpty())
    {
        shownName = StrippedName(m_curFile);
    }
    shownName += "[*]"_L1;
    return shownName;
}

//---------------------------------------------------------------------------------------------------------------------
auto WatermarkWindow::ContinueFormatRewrite(const QString &currentFormatVersion, const QString &maxFormatVersion)
    -> bool
{
    if (VAbstractApplication::VApp()->Settings()->GetConfirmFormatRewriting())
    {
        Utils::CheckableMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Confirm format rewriting"));
        msgBox.setText(tr("This file is using previous format version v%1. The current is v%2. "
                          "Saving the file with this app version will update the format version for this "
                          "file. This may prevent you from be able to open the file with older app versions. "
                          "Do you really want to continue?")
                           .arg(currentFormatVersion, maxFormatVersion));
        msgBox.setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No);
        msgBox.setDefaultButton(QDialogButtonBox::No);
        msgBox.setIconPixmap(QApplication::style()->standardIcon(QStyle::SP_MessageBoxQuestion).pixmap(32, 32));

        int dialogResult = msgBox.exec();

        if (dialogResult == QDialog::Accepted)
        {
            VAbstractApplication::VApp()->Settings()->SetConfirmFormatRewriting(not msgBox.isChecked());
            return true;
        }
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto WatermarkWindow::SaveWatermark(const QString &fileName, QString &error) -> bool
{
    m_data.opacity = ui->spinBoxOpacity->value();
    m_data.showText = ui->groupBoxWatermarkText->isChecked();
    m_data.text = ui->lineEditText->text();
    m_data.textRotation = ui->spinBoxTextRotation->value();
    m_data.showImage = ui->groupBoxWatermarkImage->isChecked();
    m_data.path = RelativeMPath(fileName, ui->lineEditPath->text());
    m_data.imageRotation = ui->spinBoxImageRotation->value();
    m_data.grayscale = ui->checkBoxGrayColor->isChecked();
    m_data.textColor = ui->pushButtonColorPicker->currentColor();

    VWatermark doc;
    doc.CreateEmptyWatermark();
    doc.SetWatermark(m_data);

    const bool result = doc.SaveDocument(fileName, error);
    if (result)
    {
        SetCurrentFile(fileName);
        statusBar()->showMessage(tr("File saved"), 5000);
        WatermarkChangesWereSaved(result);

        VAbstractApplication::VApp()->Settings()->SetWatermarkCustomColors(ui->pushButtonColorPicker->CustomColors());
    }
    return result;
}

//---------------------------------------------------------------------------------------------------------------------
void WatermarkWindow::SetCurrentFile(const QString &fileName)
{
    m_curFile = fileName;
    UpdateWindowTitle();
}

//---------------------------------------------------------------------------------------------------------------------
auto WatermarkWindow::OpenNewEditor(const QString &fileName) -> bool
{
    if (this->isWindowModified() || not m_curFile.isEmpty())
    {
        emit OpenAnother(fileName);
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void WatermarkWindow::Clear()
{
    m_curFile.clear();
    UpdateWindowTitle();
    setWindowModified(false);
    m_data = VWatermarkData();
    ShowWatermark();
}

//---------------------------------------------------------------------------------------------------------------------
auto WatermarkWindow::IgnoreLocking(int error, const QString &path) -> bool
{
    QMessageBox::StandardButton answer = QMessageBox::Abort;
    switch (error)
    {
        case QLockFile::LockFailedError:
            answer = QMessageBox::warning(this, tr("Locking file"),
                                          tr("This file already opened in another window. Ignore if you want "
                                             "to continue (not recommended, can cause a data corruption)."),
                                          QMessageBox::Abort | QMessageBox::Ignore, QMessageBox::Abort);
            break;
        case QLockFile::PermissionError:
            answer = QMessageBox::question(this, tr("Locking file"),
                                           tr("The lock file could not be created, for lack of permissions. "
                                              "Ignore if you want to continue (not recommended, can cause "
                                              "a data corruption)."),
                                           QMessageBox::Abort | QMessageBox::Ignore, QMessageBox::Abort);
            break;
        case QLockFile::UnknownError:
            answer = QMessageBox::question(this, tr("Locking file"),
                                           tr("Unknown error happened, for instance a full partition prevented "
                                              "writing out the lock file. Ignore if you want to continue (not "
                                              "recommended, can cause a data corruption)."),
                                           QMessageBox::Abort | QMessageBox::Ignore, QMessageBox::Abort);
            break;
        default:
            answer = QMessageBox::Abort;
            break;
    }

    if (answer == QMessageBox::Abort)
    {
        qDebug("Failed to lock %s", qUtf8Printable(path));
        qDebug("Error type: %d", error);
        Clear();
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void WatermarkWindow::ShowWatermark()
{
    ui->spinBoxOpacity->blockSignals(true);
    ui->spinBoxOpacity->setValue(m_data.opacity);
    ui->spinBoxOpacity->blockSignals(false);

    ui->groupBoxWatermarkText->blockSignals(true);
    ui->groupBoxWatermarkText->setChecked(m_data.showText);
    ui->groupBoxWatermarkText->blockSignals(false);

    ui->lineEditText->blockSignals(true);
    ui->lineEditText->setText(m_data.text);
    ui->lineEditText->blockSignals(false);

    ui->spinBoxTextRotation->blockSignals(true);
    ui->spinBoxTextRotation->setValue(m_data.textRotation);
    ui->spinBoxTextRotation->blockSignals(false);

    ui->lineEditFontSample->blockSignals(true);
    ui->lineEditFontSample->setFont(m_data.font);
    ui->lineEditFontSample->blockSignals(false);

    ui->groupBoxWatermarkImage->blockSignals(true);
    ui->groupBoxWatermarkImage->setChecked(m_data.showImage);
    ui->groupBoxWatermarkImage->blockSignals(false);

    ui->lineEditPath->blockSignals(true);
    ui->lineEditPath->setText(AbsoluteMPath(m_curFile, m_data.path));
    ValidatePath();
    ui->lineEditPath->blockSignals(false);

    ui->spinBoxImageRotation->blockSignals(true);
    ui->spinBoxImageRotation->setValue(m_data.imageRotation);
    ui->spinBoxImageRotation->blockSignals(false);

    ui->checkBoxGrayColor->blockSignals(true);
    ui->checkBoxGrayColor->setChecked(m_data.grayscale);
    ui->checkBoxGrayColor->blockSignals(false);

    ui->pushButtonColorPicker->blockSignals(true);
    ui->pushButtonColorPicker->setCurrentColor(m_data.textColor);
    ui->pushButtonColorPicker->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void WatermarkWindow::ValidatePath()
{
    QPalette palette = ui->lineEditPath->palette();
    const QString path = ui->lineEditPath->text();
    palette.setColor(ui->lineEditPath->foregroundRole(),
                     path.isEmpty() || QFileInfo::exists(path) ? m_okPathColor : Qt::red);
    ui->lineEditPath->setPalette(palette);
}

//---------------------------------------------------------------------------------------------------------------------
void WatermarkWindow::ToolBarStyle(QToolBar *bar)
{
    SCASSERT(bar != nullptr)
    if (VAbstractApplication::VApp()->Settings()->GetToolBarStyle())
    {
        bar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    }
    else
    {
        bar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    }
}
