/************************************************************************
 **
 **  @file   tmainwindow.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 7, 2015
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

#include "tmainwindow.h"
#include "ui_tmainwindow.h"
#include "dialogs/dialogabouttape.h"
#include "dialogs/dialognewmeasurements.h"
#include "dialogs/dialogmdatabase.h"
#include "dialogs/dialogtapepreferences.h"
#include "dialogs/dialogsetupmultisize.h"
#include "dialogs/dialogrestrictdimension.h"
#include "dialogs/dialogdimensionlabels.h"
#include "dialogs/dialogmeasurementscsvcolumns.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/calculator.h"
#include "../vpatterndb/pmsystems.h"
#include "../vpatterndb/measurements.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vvitconverter.h"
#include "../ifc/xml/vvstconverter.h"
#include "../ifc/xml/vpatternconverter.h"
#include "../vmisc/vsysexits.h"
#include "../vmisc/qxtcsvmodel.h"
#include "../vmisc/dialogs/dialogexporttocsv.h"
#include "../vmisc/compatibility.h"
#include "vlitepattern.h"
#include "../qmuparser/qmudef.h"
#include "../vtools/dialogs/support/dialogeditwrongformula.h"
#include "version.h"
#include "mapplication.h" // Should be last because of definning qApp

#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
#include "../vmisc/backport/qscopeguard.h"
#else
#include <QScopeGuard>
#endif

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QComboBox>
#include <QProcess>
#include <QtNumeric>
#include <QTextCodec>
#include <QTimer>

#if defined(Q_OS_MAC)
#include <QMimeData>
#include <QDrag>
#endif //defined(Q_OS_MAC)

#define DIALOG_MAX_FORMULA_HEIGHT 64

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(tMainWindow, "t.mainwindow")

QT_WARNING_POP

namespace
{
enum class MUnits : qint8 { Table, Degrees};
}

// We need this enum in case we will add or delete a column. And also make code more readable.
enum {
    ColumnName = 0,
    ColumnFullName = 1,
    ColumnCalcValue = 2,
    ColumnFormula = 3,
    ColumnBaseValue = 4,
    ColumnShiftA = 5,
    ColumnShiftB = 6,
    ColumnShiftC = 7,
    ColumnCorrection = 8
};

//---------------------------------------------------------------------------------------------------------------------
TMainWindow::TMainWindow(QWidget *parent)
    : VAbstractMainWindow(parent),
      ui(new Ui::TMainWindow),
      formulaBaseHeight(0),
      gradation(new QTimer(this))
{
    ui->setupUi(this);

    VAbstractApplication::VApp()->Settings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    ui->lineEditFind->setClearButtonEnabled(true);
    ui->lineEditName->setClearButtonEnabled(true);
    ui->lineEditFullName->setClearButtonEnabled(true);
    ui->lineEditCustomerName->setClearButtonEnabled(true);
    ui->lineEditEmail->setClearButtonEnabled(true);

    ui->lineEditFind->installEventFilter(this);
    ui->plainTextEditFormula->installEventFilter(this);

    search = QSharedPointer<VTableSearch>(new VTableSearch(ui->tableWidget));
    ui->tabWidget->setVisible(false);

    ui->mainToolBar->setContextMenuPolicy(Qt::PreventContextMenu);
    ui->toolBarGradation->setContextMenuPolicy(Qt::PreventContextMenu);

    m_recentFileActs.fill(nullptr);

    connect(gradation, &QTimer::timeout, this, &TMainWindow::GradationChanged);

    SetupMenu();
    UpdateWindowTitle();
    ReadSettings();

#if defined(Q_OS_MAC)
    // On Mac deafault icon size is 32x32.
    ui->toolBarGradation->setIconSize(QSize(24, 24));

    ui->pushButtonShowInExplorer->setText(tr("Show in Finder"));

    // Mac OS Dock Menu
    QMenu *menu = new QMenu(this);
    connect(menu, &QMenu::aboutToShow, this, &TMainWindow::AboutToShowDockMenu);
    AboutToShowDockMenu();
    menu->setAsDockMenu();
#endif //defined(Q_OS_MAC)
}

//---------------------------------------------------------------------------------------------------------------------
TMainWindow::~TMainWindow()
{
    delete data;
    delete m;
    qDeleteAll(hackedWidgets);
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
QString TMainWindow::CurrentFile() const
{
    return curFile;
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::RetranslateTable()
{
    if (m != nullptr)
    {
        const int row = ui->tableWidget->currentRow();
        RefreshTable();
        ui->tableWidget->selectRow(row);
        search->RefreshList(ui->lineEditFind->text());
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool TMainWindow::SetDimensionABase(int base)
{
    const QList<MeasurementDimension_p> dimensions = m->Dimensions().values();

    if (dimensions.isEmpty())
    {
        qCCritical(tMainWindow, "%s\n", qPrintable(tr("The table doesn't provide dimensions")));
        return false;
    }

    const qint32 i = gradationDimensionA->findData(base);
    if (i != -1)
    {
        gradationDimensionA->setCurrentIndex(i);
    }

    if (not VFuzzyComparePossibleNulls(base, currentDimensionA))
    {
        qCCritical(tMainWindow, "%s\n", qPrintable(tr("Invalid base value for dimension A")));
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool TMainWindow::SetDimensionBBase(int base)
{
    const QList<MeasurementDimension_p> dimensions = m->Dimensions().values();

    if (dimensions.size() <= 1)
    {
        qCCritical(tMainWindow, "%s\n", qPrintable(tr("The table doesn't support dimension B")));
        return false;
    }

    const qint32 i = gradationDimensionB->findData(base);
    if (i != -1)
    {
        gradationDimensionB->setCurrentIndex(i);
    }

    if (not VFuzzyComparePossibleNulls(base, currentDimensionB))
    {
        qCCritical(tMainWindow, "%s\n", qPrintable(tr("Invalid base value for dimension B")));
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool TMainWindow::SetDimensionCBase(int base)
{
    const QList<MeasurementDimension_p> dimensions = m->Dimensions().values();

    if (dimensions.size() <= 2)
    {
        qCCritical(tMainWindow, "%s\n", qPrintable(tr("The table doesn't support dimension C")));
        return false;
    }

    const qint32 i = gradationDimensionC->findData(base);
    if (i != -1)
    {
        gradationDimensionC->setCurrentIndex(i);
    }

    if (not VFuzzyComparePossibleNulls(base, currentDimensionC))
    {
        qCCritical(tMainWindow, "%s\n", qPrintable(tr("Invalid base value for dimension C")));
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SetPUnit(Unit unit)
{
    pUnit = unit;
    SetCurrentPatternUnit();
    UpdatePatternUnit();
}

//---------------------------------------------------------------------------------------------------------------------
bool TMainWindow::LoadFile(const QString &path)
{
    if (m == nullptr)
    {
        if (not QFileInfo::exists(path))
        {
            qCCritical(tMainWindow, "%s", qUtf8Printable(tr("File '%1' doesn't exist!").arg(path)));
            if (MApplication::VApp()->IsTestMode())
            {
                qApp->exit(V_EX_NOINPUT);
            }
            return false;
        }

        // Check if file already opened
        const QList<TMainWindow*> list = MApplication::VApp()->MainWindows();
        for (auto w : list)
        {
            if (w->CurrentFile() == path)
            {
                w->activateWindow();
                close();
                return false;
            }
        }

        VlpCreateLock(lock, path);

        if (not lock->IsLocked())
        {
            if (not IgnoreLocking(lock->GetLockError(), path))
            {
                return false;
            }
        }

        try
        {
            data = new VContainer(VAbstractApplication::VApp()->TrVars(), &mUnit, VContainer::UniqueNamespace());

            m = new VMeasurements(data);
            m->setXMLContent(path);

            mType = m->Type();

            if (mType == MeasurementsType::Unknown)
            {
                throw VException(tr("File has unknown format."));
            }

            if (mType == MeasurementsType::Multisize)
            {
                VVSTConverter converter(path);
                m_curFileFormatVersion = converter.GetCurrentFormatVersion();
                m_curFileFormatVersionStr = converter.GetFormatVersionStr();
                m->setXMLContent(converter.Convert());// Read again after conversion
            }
            else
            {
                VVITConverter converter(path);
                m_curFileFormatVersion = converter.GetCurrentFormatVersion();
                m_curFileFormatVersionStr = converter.GetFormatVersionStr();
                m->setXMLContent(converter.Convert());// Read again after conversion
            }

            if (not m->IsDefinedKnownNamesValid())
            {
                throw VException(tr("File contains invalid known measurement(s)."));
            }

            mUnit = m->Units();
            pUnit = mUnit;

            currentDimensionA = m->DimensionABase();
            currentDimensionB = m->DimensionBBase();
            currentDimensionC = m->DimensionCBase();

            ui->labelToolTip->setVisible(false);
            ui->tabWidget->setVisible(true);

            mIsReadOnly = m->IsReadOnly();
            UpdatePadlock(mIsReadOnly);

            SetCurrentFile(path);

            InitWindow();

            const bool freshCall = true;
            RefreshData(freshCall);

            if (ui->tableWidget->rowCount() > 0)
            {
                ui->tableWidget->selectRow(0);
            }

            MeasurementGUI();

            ui->actionImportFromCSV->setEnabled(true);
        }
        catch (VException &e)
        {
            qCCritical(tMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("File error.")),
                       qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
            ui->labelToolTip->setVisible(true);
            ui->tabWidget->setVisible(false);
            delete m;
            m = nullptr;
            delete data;
            data = nullptr;
            lock.reset();

            if (MApplication::VApp()->IsTestMode())
            {
                qApp->exit(V_EX_NOINPUT);
            }
            return false;
        }
    }
    else
    {
        return MApplication::VApp()->NewMainWindow()->LoadFile(path);
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ShowToolTip(const QString &toolTip)
{
    Q_UNUSED(toolTip)
    // do nothing
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::FileNew()
{
    if (m == nullptr)
    {
        DialogNewMeasurements measurements(this);
        if (measurements.exec() == QDialog::Rejected)
        {
            return;
        }

        mUnit = measurements.MUnit();
        pUnit = mUnit;
        mType = measurements.Type();

        if (mType == MeasurementsType::Multisize)
        {
            DialogSetupMultisize setup(mUnit, this);
            if (setup.exec() == QDialog::Rejected)
            {
                return;
            }

            data = new VContainer(VAbstractApplication::VApp()->TrVars(), &mUnit, VContainer::UniqueNamespace());

            m = new VMeasurements(mUnit, setup.Dimensions(), data);
            m->SetFullCircumference(setup.FullCircumference());
            m_curFileFormatVersion = VVSTConverter::MeasurementMaxVer;
            m_curFileFormatVersionStr = VVSTConverter::MeasurementMaxVerStr;

            SetCurrentDimensionValues();
        }
        else
        {
            data = new VContainer(VAbstractApplication::VApp()->TrVars(), &mUnit, VContainer::UniqueNamespace());

            m = new VMeasurements(mUnit, data);
            m_curFileFormatVersion = VVITConverter::MeasurementMaxVer;
            m_curFileFormatVersionStr = VVITConverter::MeasurementMaxVerStr;
        }

        mIsReadOnly = m->IsReadOnly();
        UpdatePadlock(mIsReadOnly);

        SetCurrentFile(QString());
        MeasurementsWereSaved(false);

        InitWindow();

        MeasurementGUI();

        ui->actionImportFromCSV->setEnabled(true);
    }
    else
    {
        MApplication::VApp()->NewMainWindow()->FileNew();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::OpenIndividual()
{
    const QString filter = tr("Individual measurements") + QStringLiteral(" (*.vit);;") + tr("Multisize measurements") +
            QStringLiteral(" (*.vst);;") + tr("All files") + QStringLiteral(" (*.*)");
    //Use standard path to individual measurements
    const QString pathTo = MApplication::VApp()->TapeSettings()->GetPathIndividualMeasurements();

    bool usedNotExistedDir = false;
    QDir directory(pathTo);
    if (not directory.exists())
    {
        usedNotExistedDir = directory.mkpath(QChar('.'));
    }

    Open(pathTo, filter);

    if (usedNotExistedDir)
    {
        QDir(pathTo).rmpath(QChar('.'));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::OpenMultisize()
{
    const QString filter = tr("Multisize measurements") + QStringLiteral(" (*.vst);;") + tr("Individual measurements") +
            QStringLiteral(" (*.vit);;") + tr("All files") + QStringLiteral(" (*.*)");
    //Use standard path to multisize measurements
    QString pathTo = MApplication::VApp()->TapeSettings()->GetPathMultisizeMeasurements();
    pathTo = VCommonSettings::PrepareMultisizeTables(pathTo);

    Open(pathTo, filter);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::OpenTemplate()
{
    const QString filter = tr("Measurements") + QStringLiteral(" (*.vst *.vit);;") + tr("All files") +
            QStringLiteral(" (*.*)");
    //Use standard path to template files
    QString pathTo = MApplication::VApp()->TapeSettings()->GetPathTemplate();
    pathTo = VCommonSettings::PrepareStandardTemplates(pathTo);
    Open(pathTo, filter);

    if (m != nullptr)
    {// The file was opened.
        SetCurrentFile(QString()); // Force user to to save new file
        lock.reset();// remove lock from template
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::CreateFromExisting()
{
    const QString filter = tr("Individual measurements") + QStringLiteral(" (*.vit)");
    //Use standard path to individual measurements
    const QString pathTo = MApplication::VApp()->TapeSettings()->GetPathIndividualMeasurements();

    bool usedNotExistedDir = false;
    QDir directory(pathTo);
    if (not directory.exists())
    {
        usedNotExistedDir = directory.mkpath(QChar('.'));
    }

    const QString mPath = QFileDialog::getOpenFileName(this, tr("Select file"), pathTo, filter, nullptr,
                                                       VAbstractApplication::VApp()->NativeFileDialog());

    if (not mPath.isEmpty())
    {
        if (m == nullptr)
        {
            LoadFromExistingFile(mPath);
        }
        else
        {
            MApplication::VApp()->NewMainWindow()->CreateFromExisting();
        }
    }

    if (usedNotExistedDir)
    {
        QDir(pathTo).rmpath(QChar('.'));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::Preferences()
{
    // Calling constructor of the dialog take some time. Because of this user have time to call the dialog twice.
    static QPointer<DialogTapePreferences> guard;// Prevent any second run
    if (guard.isNull())
    {
        QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        auto *preferences = new DialogTapePreferences(this);
        // QScopedPointer needs to be sure any exception will never block guard
        QScopedPointer<DialogTapePreferences> dlg(preferences);
        guard = preferences;
        // Must be first
        connect(dlg.data(), &DialogTapePreferences::UpdateProperties, this, &TMainWindow::WindowsLocale);
        connect(dlg.data(), &DialogTapePreferences::UpdateProperties, this, &TMainWindow::ToolBarStyles);
        QGuiApplication::restoreOverrideCursor();
        dlg->exec();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ToolBarStyles()
{
    ToolBarStyle(ui->toolBarGradation);
    ToolBarStyle(ui->mainToolBar);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::closeEvent(QCloseEvent *event)
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
void TMainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        VAbstractApplication::VApp()->Settings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);

        InitMeasurementUnits();

        if (mType == MeasurementsType::Multisize)
        {
            ui->labelMType->setText(tr("Multisize measurements"));

            InitDimensionsBaseValue();
            InitDimensionControls();
            InitDimesionShifts();
            RetranslateTableHeaders();
        }
        else
        {
            ui->labelMType->setText(tr("Individual measurements"));

            const qint32 index = ui->comboBoxGender->currentIndex();
            ui->comboBoxGender->blockSignals(true);
            ui->comboBoxGender->clear();
            InitGender(ui->comboBoxGender);
            ui->comboBoxGender->setCurrentIndex(index);
            ui->comboBoxGender->blockSignals(false);

            InitMeasurementDimension();
        }

        {
            const qint32 index = ui->comboBoxPMSystem->currentIndex();
            ui->comboBoxPMSystem->blockSignals(true);
            ui->comboBoxPMSystem->clear();
            InitPMSystems(ui->comboBoxPMSystem);
            ui->comboBoxPMSystem->setCurrentIndex(index);
            ui->comboBoxPMSystem->blockSignals(false);
        }

        {
            if (labelPatternUnit)
            {
                labelPatternUnit->setText(tr("Pattern unit:"));
            }

            if (comboBoxUnits)
            {
                const qint32 index = comboBoxUnits->currentIndex();
                comboBoxUnits->blockSignals(true);
                comboBoxUnits->clear();
                InitComboBoxUnits();
                comboBoxUnits->setCurrentIndex(index);
                comboBoxUnits->blockSignals(false);
            }
        }
    }

    // remember to call base class implementation
    QMainWindow::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
bool TMainWindow::eventFilter(QObject *object, QEvent *event)
{
    if (auto *plainTextEdit = qobject_cast<QPlainTextEdit *>(object))
    {
        if (event->type() == QEvent::KeyPress)
        {
            auto *keyEvent = static_cast<QKeyEvent *>(event);
            if ((keyEvent->key() == Qt::Key_Period) && ((keyEvent->modifiers() & Qt::KeypadModifier) != 0u))
            {
                if (VAbstractApplication::VApp()->Settings()->GetOsSeparator())
                {
                    plainTextEdit->insertPlainText(QLocale().decimalPoint());
                }
                else
                {
                    plainTextEdit->insertPlainText(QLocale::c().decimalPoint());
                }
                return true;
            }
        }
    }
    else if (auto *textEdit = qobject_cast<QLineEdit *>(object))
    {
        if (event->type() == QEvent::KeyPress)
        {
            auto *keyEvent = static_cast<QKeyEvent *>(event);
            if ((keyEvent->key() == Qt::Key_Period) && ((keyEvent->modifiers() & Qt::KeypadModifier) != 0u))
            {
                if (VAbstractApplication::VApp()->Settings()->GetOsSeparator())
                {
                    textEdit->insert(QLocale().decimalPoint());
                }
                else
                {
                    textEdit->insert(QLocale::c().decimalPoint());
                }
                return true;
            }
        }
    }

    // pass the event on to the parent class
    return QMainWindow::eventFilter(object, event);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ExportToCSVData(const QString &fileName, bool withHeader, int mib, const QChar &separator)
{
    QxtCsvModel csv;
    const int columns = ui->tableWidget->columnCount();
    {
        int colCount = 0;
        for (int column = 0; column < columns; ++column)
        {
            if (not ui->tableWidget->isColumnHidden(column))
            {
                csv.insertColumn(colCount++);
            }
        }
    }

    if (withHeader)
    {
        int colCount = 0;
        for (int column = 0; column < columns; ++column)
        {
            if (not ui->tableWidget->isColumnHidden(column))
            {
                QTableWidgetItem *header = ui->tableWidget->horizontalHeaderItem(column);
                csv.setHeaderText(colCount, header->text());
                ++colCount;
            }
        }
    }

    const int rows = ui->tableWidget->rowCount();
    for (int row = 0; row < rows; ++row)
    {
        csv.insertRow(row);
        int colCount = 0;
        for (int column = 0; column < columns; ++column)
        {
            if (not ui->tableWidget->isColumnHidden(column))
            {
                QTableWidgetItem *item = ui->tableWidget->item(row, column);
                csv.setText(row, colCount, item->text());
                ++colCount;
            }
        }
    }

    QString error;
    csv.toCSV(fileName, error, withHeader, separator, QTextCodec::codecForMib(mib));
}

//---------------------------------------------------------------------------------------------------------------------
QStringList TMainWindow::RecentFileList() const
{
    return MApplication::VApp()->TapeSettings()->GetRecentFileList();
}

//---------------------------------------------------------------------------------------------------------------------
bool TMainWindow::FileSave()
{
    if (curFile.isEmpty() || mIsReadOnly)
    {
        return FileSaveAs();
    }

    if (mType == MeasurementsType::Multisize
            && m_curFileFormatVersion < VVSTConverter::MeasurementMaxVer
            && not ContinueFormatRewrite(m_curFileFormatVersionStr, VVSTConverter::MeasurementMaxVerStr))
    {
        return false;
    }

    if (mType == MeasurementsType::Individual
             && m_curFileFormatVersion < VVITConverter::MeasurementMaxVer
             && not ContinueFormatRewrite(m_curFileFormatVersionStr, VVITConverter::MeasurementMaxVerStr))
    {
        return false;
    }

#ifdef Q_OS_WIN32
        qt_ntfs_permission_lookup++; // turn checking on
#endif /*Q_OS_WIN32*/
    const bool isFileWritable = QFileInfo(curFile).isWritable();
#ifdef Q_OS_WIN32
        qt_ntfs_permission_lookup--; // turn it off again
#endif /*Q_OS_WIN32*/
    if (not isFileWritable)
    {
        QMessageBox messageBox(this);
        messageBox.setIcon(QMessageBox::Question);
        messageBox.setText(tr("The measurements document has no write permissions."));
        messageBox.setInformativeText(tr("Do you want to change the premissions?"));
        messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        messageBox.setDefaultButton(QMessageBox::Yes);

        if (messageBox.exec() == QMessageBox::Yes)
        {
#ifdef Q_OS_WIN32
            qt_ntfs_permission_lookup++; // turn checking on
#endif /*Q_OS_WIN32*/
            bool changed = QFile::setPermissions(curFile,
                                                 QFileInfo(curFile).permissions() | QFileDevice::WriteUser);
#ifdef Q_OS_WIN32
            qt_ntfs_permission_lookup--; // turn it off again
#endif /*Q_OS_WIN32*/

            if (not changed)
            {
                messageBox.setIcon(QMessageBox::Warning);
                messageBox.setText(tr("Cannot set permissions for %1 to writable.").arg(curFile));
                messageBox.setInformativeText(tr("Could not save the file."));
                messageBox.setStandardButtons(QMessageBox::Ok);
                messageBox.setDefaultButton(QMessageBox::Ok);
                messageBox.exec();
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    QString error;
    if (not SaveMeasurements(curFile, error))
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

    if (mType == MeasurementsType::Multisize)
    {
        m_curFileFormatVersion = VVSTConverter::MeasurementMaxVer;
        m_curFileFormatVersionStr = VVSTConverter::MeasurementMaxVerStr;
    }
    else
    {
        m_curFileFormatVersion = VVITConverter::MeasurementMaxVer;
        m_curFileFormatVersionStr = VVITConverter::MeasurementMaxVerStr;
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool TMainWindow::FileSaveAs()
{
    QString filters;
    QString fName = tr("measurements");
    QString suffix;
    if (mType == MeasurementsType::Individual)
    {
        filters = tr("Individual measurements") + QStringLiteral(" (*.vit)");
        suffix = QStringLiteral("vit");
        fName += QChar('.') + suffix;
    }
    else
    {
        filters = tr("Multisize measurements") + QStringLiteral(" (*.vst)");
        suffix = QStringLiteral("vst");
        fName += QChar('.') + suffix;
    }

    QString dir;
    if (curFile.isEmpty())
    {
        if (mType == MeasurementsType::Individual)
        {
            dir = MApplication::VApp()->TapeSettings()->GetPathIndividualMeasurements();
        }
        else
        {
            dir = MApplication::VApp()->TapeSettings()->GetPathMultisizeMeasurements();
            dir = VCommonSettings::PrepareMultisizeTables(dir);
        }
    }
    else
    {
        dir = QFileInfo(curFile).absolutePath();
    }

    bool usedNotExistedDir = false;
    QDir directory(dir);
    if (not directory.exists())
    {
        usedNotExistedDir = directory.mkpath(QChar('.'));
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save as"), dir + QChar('/') + fName, filters, nullptr,
                                                    VAbstractApplication::VApp()->NativeFileDialog());

    auto RemoveTempDir = qScopeGuard([usedNotExistedDir, dir]()
    {
        if (usedNotExistedDir)
        {
            QDir(dir).rmpath(QChar('.'));
        }
    });

    if (fileName.isEmpty())
    {
        return false;
    }

    QFileInfo f( fileName );
    if (f.suffix().isEmpty() && f.suffix() != suffix)
    {
        fileName += QChar('.') + suffix;
    }

    if (QFileInfo::exists(fileName) && curFile != fileName)
    {
        // Temporary try to lock the file before saving
        VLockGuard<char> tmp(fileName);
        if (not tmp.IsLocked())
        {
            qCCritical(tMainWindow, "%s",
                       qUtf8Printable(tr("Failed to lock. This file already opened in another window.")));
            return false;
        }
    }

    // Need for restoring previous state in case of failure
    const bool readOnly = m->IsReadOnly();

    m->SetReadOnly(false);
    mIsReadOnly = false;

    QString error;
    bool result = SaveMeasurements(fileName, error);
    if (not result)
    {
        QMessageBox messageBox;
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.setInformativeText(tr("Could not save file"));
        messageBox.setDefaultButton(QMessageBox::Ok);
        messageBox.setDetailedText(error);
        messageBox.setStandardButtons(QMessageBox::Ok);
        messageBox.exec();

        // Restore previous state
        m->SetReadOnly(readOnly);
        mIsReadOnly = readOnly;
        return false;
    }

    if (mType == MeasurementsType::Multisize)
    {
        m_curFileFormatVersion = VVSTConverter::MeasurementMaxVer;
        m_curFileFormatVersionStr = VVSTConverter::MeasurementMaxVerStr;
    }
    else
    {
        m_curFileFormatVersion = VVITConverter::MeasurementMaxVer;
        m_curFileFormatVersionStr = VVITConverter::MeasurementMaxVerStr;
    }

    UpdatePadlock(false);
    UpdateWindowTitle();

    if (curFile == fileName && not lock.isNull())
    {
        lock->Unlock();
    }
    VlpCreateLock(lock, fileName);
    if (not lock->IsLocked())
    {
        qCCritical(tMainWindow, "%s", qUtf8Printable(tr("Failed to lock. This file already opened in another window. "
                                                        "Expect collissions when run 2 copies of the program.")));
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::AboutToShowWindowMenu()
{
    ui->menuWindow->clear();
    CreateWindowMenu(ui->menuWindow);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ShowWindow() const
{
    if (auto *action = qobject_cast<QAction*>(sender()))
    {
        const QVariant v = action->data();
        if (v.canConvert<int>())
        {
            const int offset = qvariant_cast<int>(v);
            const QList<TMainWindow*> windows = MApplication::VApp()->MainWindows();
            windows.at(offset)->raise();
            windows.at(offset)->activateWindow();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ImportDataFromCSV()
{
    if (m == nullptr || m->Type() == MeasurementsType::Unknown)
    {
        return;
    }

    const QString filters = tr("Comma-Separated Values") + QStringLiteral(" (*.csv)");
    const QString suffix = QStringLiteral("csv");

    QString fileName = QFileDialog::getOpenFileName(this, tr("Import from CSV"), QDir::homePath(), filters, nullptr,
                                                    VAbstractApplication::VApp()->NativeFileDialog());

    if (fileName.isEmpty())
    {
        return;
    }

    QFileInfo f( fileName );
    if (f.suffix().isEmpty() && f.suffix() != suffix)
    {
        fileName += QChar('.') + suffix;
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

        QSharedPointer<DialogMeasurementsCSVColumns> columns;
        if (m->Type() == MeasurementsType::Multisize)
        {
            const QList<MeasurementDimension_p> dimensions = m->Dimensions().values();
            columns = QSharedPointer<DialogMeasurementsCSVColumns>::create(fileName, m->Type(), dimensions, this);
        }
        else
        {
            columns = QSharedPointer<DialogMeasurementsCSVColumns>::create(fileName, m->Type(), this);
        }
        columns->SetWithHeader(dialog.IsWithHeader());
        columns->SetSeparator(dialog.GetSeparator());
        columns->SetCodec(QTextCodec::codecForMib(dialog.GetSelectedMib()));

        if (columns->exec() == QDialog::Accepted)
        {
            QxtCsvModel csv(fileName, nullptr, dialog.IsWithHeader(), dialog.GetSeparator(),
                            QTextCodec::codecForMib(dialog.GetSelectedMib()));
            const QVector<int> map = columns->ColumnsMap();

            if (m->Type() == MeasurementsType::Individual)
            {
                ImportIndividualMeasurements(csv, map);
            }
            else
            {
                ImportMultisizeMeasurements(csv, map);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
#if defined(Q_OS_MAC)
void TMainWindow::AboutToShowDockMenu()
{
    if (QMenu *menu = qobject_cast<QMenu *>(sender()))
    {
        menu->clear();
        CreateWindowMenu(menu);

        menu->addSeparator();

        menu->addAction(ui->actionOpenIndividual);
        menu->addAction(ui->actionOpenMultisize);
        menu->addAction(ui->actionOpenTemplate);

        menu->addSeparator();

        QAction *actionPreferences = menu->addAction(tr("Preferences"));
        actionPreferences->setMenuRole(QAction::NoRole);
        connect(actionPreferences, &QAction::triggered, this, &TMainWindow::Preferences);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::OpenAt(QAction *where)
{
    const QString path = curFile.left(curFile.indexOf(where->text())) + where->text();
    if (path == curFile)
    {
        return;
    }
    QProcess process;
    process.start(QStringLiteral("/usr/bin/open"), QStringList() << path, QIODevice::ReadOnly);
    process.waitForFinished();
}
#endif //defined(Q_OS_MAC)

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveCustomerName()
{
    if (m->Customer() != ui->lineEditCustomerName->text())
    {
        m->SetCustomer(ui->lineEditCustomerName->text());
        MeasurementsWereSaved(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveEmail()
{
    if (m->Email() != ui->lineEditEmail->text())
    {
        m->SetEmail(ui->lineEditEmail->text());
        MeasurementsWereSaved(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveGender(int index)
{
    const GenderType type = static_cast<GenderType>(ui->comboBoxGender->itemData(index).toInt());
    if (m->Gender() != type)
    {
        m->SetGender(type);
        MeasurementsWereSaved(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveBirthDate(const QDate &date)
{
    if (m->BirthDate() != date)
    {
        m->SetBirthDate(date);
        MeasurementsWereSaved(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveNotes()
{
    if (m->Notes() != ui->plainTextEditNotes->toPlainText())
    {
        m->SetNotes(ui->plainTextEditNotes->toPlainText());
        MeasurementsWereSaved(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SavePMSystem(int index)
{
    QString system = ui->comboBoxPMSystem->itemData(index).toString();
    system.remove(0, 1);// clear p

    if (m->PMSystem() != system)
    {
        m->SetPMSystem(system);
        MeasurementsWereSaved(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::Remove()
{
    ShowMDiagram(QString());
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), 0);
    m->Remove(nameField->data(Qt::UserRole).toString());

    MeasurementsWereSaved(false);

    search->RemoveRow(row);
    RefreshData();
    search->RefreshList(ui->lineEditFind->text());

    if (ui->tableWidget->rowCount() > 0)
    {
        ui->tableWidget->selectRow(row >= ui->tableWidget->rowCount() ? ui->tableWidget->rowCount() - 1 : row);
    }
    else
    {
        MFields(false);

        ui->actionExportToCSV->setEnabled(false); 

        ui->lineEditName->blockSignals(true);
        ui->lineEditName->setText(QString());
        ui->lineEditName->blockSignals(false);

        ui->plainTextEditDescription->blockSignals(true);
        ui->plainTextEditDescription->setPlainText(QString());
        ui->plainTextEditDescription->blockSignals(false);

        ui->lineEditFullName->blockSignals(true);
        ui->lineEditFullName->setText(QString());
        ui->lineEditFullName->blockSignals(false);

        ui->comboBoxMUnits->blockSignals(true);
        ui->comboBoxMUnits->setCurrentIndex(-1);
        ui->comboBoxMUnits->blockSignals(false);

        if (mType == MeasurementsType::Multisize)
        {
            ui->labelCalculatedValue->blockSignals(true);
            ui->doubleSpinBoxBaseValue->blockSignals(true);
            ui->doubleSpinBoxCorrection->blockSignals(true);
            ui->doubleSpinBoxShiftA->blockSignals(true);
            ui->doubleSpinBoxShiftB->blockSignals(true);
            ui->doubleSpinBoxShiftC->blockSignals(true);

            ui->labelCalculatedValue->setText(QString());
            ui->doubleSpinBoxBaseValue->setValue(0);
            ui->doubleSpinBoxCorrection->setValue(0);
            ui->doubleSpinBoxShiftA->setValue(0);
            ui->doubleSpinBoxShiftB->setValue(0);
            ui->doubleSpinBoxShiftC->setValue(0);

            ui->labelCalculatedValue->blockSignals(false);
            ui->doubleSpinBoxBaseValue->blockSignals(false);
            ui->doubleSpinBoxCorrection->blockSignals(false);
            ui->doubleSpinBoxShiftA->blockSignals(false);
            ui->doubleSpinBoxShiftB->blockSignals(false);
            ui->doubleSpinBoxShiftC->blockSignals(false);
        }
        else
        {
            ui->labelCalculatedValue->blockSignals(true);
            ui->labelCalculatedValue->setText(QString());
            ui->labelCalculatedValue->blockSignals(false);

            ui->plainTextEditFormula->blockSignals(true);
            ui->plainTextEditFormula->setPlainText(QString());
            ui->plainTextEditFormula->blockSignals(false);
        }
    }

    ui->tableWidget->repaint(); // Force repain to fix paint artifacts on Mac OS X
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::MoveTop()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(row, ColumnName);
    m->MoveTop(nameField->data(Qt::UserRole).toString());
    MeasurementsWereSaved(false);
    RefreshData();
    search->RefreshList(ui->lineEditFind->text());
    ui->tableWidget->selectRow(0);
    ui->tableWidget->repaint(); // Force repain to fix paint artifacts on Mac OS X
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::MoveUp()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(row, ColumnName);
    m->MoveUp(nameField->data(Qt::UserRole).toString());
    MeasurementsWereSaved(false);
    RefreshData();
    search->RefreshList(ui->lineEditFind->text());
    ui->tableWidget->selectRow(row-1);
    ui->tableWidget->repaint(); // Force repain to fix paint artifacts on Mac OS X
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::MoveDown()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(row, ColumnName);
    m->MoveDown(nameField->data(Qt::UserRole).toString());
    MeasurementsWereSaved(false);
    RefreshData();
    search->RefreshList(ui->lineEditFind->text());
    ui->tableWidget->selectRow(row+1);
    ui->tableWidget->repaint(); // Force repain to fix paint artifacts on Mac OS X
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::MoveBottom()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(row, ColumnName);
    m->MoveBottom(nameField->data(Qt::UserRole).toString());
    MeasurementsWereSaved(false);
    RefreshData();
    search->RefreshList(ui->lineEditFind->text());
    ui->tableWidget->selectRow(ui->tableWidget->rowCount()-1);
    ui->tableWidget->repaint(); // Force repain to fix paint artifacts on Mac OS X
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::Fx()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(row, ColumnName);

    QSharedPointer<VMeasurement> meash;

    try
    {
       // Translate to internal look.
       meash = data->GetVariable<VMeasurement>(nameField->data(Qt::UserRole).toString());
    }
    catch(const VExceptionBadId & e)
    {
        qCCritical(tMainWindow, "%s\n\n%s\n\n%s",
                   qUtf8Printable(tr("Can't find measurement '%1'.").arg(nameField->text())),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        return;
    }

    auto *dialog = new DialogEditWrongFormula(meash->GetData(), NULL_ID, this);
    dialog->setWindowTitle(tr("Edit measurement"));
    dialog->SetMeasurementsMode();
    dialog->SetFormula(VAbstractApplication::VApp()->TrVars()
                       ->TryFormulaFromUser(ui->plainTextEditFormula->toPlainText(), true));
    const QString postfix = UnitsToStr(mUnit, true);//Show unit in dialog lable (cm, mm or inch)
    dialog->setPostfix(postfix);

    if (dialog->exec() == QDialog::Accepted)
    {
        // Fix the bug #492. https://bitbucket.org/dismine/valentina/issues/492/valentina-crashes-when-add-an-increment
        // Because of the bug need to take QTableWidgetItem twice time. Previous update "killed" the pointer.
        nameField = ui->tableWidget->item(row, ColumnName);
        m->SetMValue(nameField->data(Qt::UserRole).toString(), dialog->GetFormula());

        MeasurementsWereSaved(false);

        RefreshData();

        search->RefreshList(ui->lineEditFind->text());

        ui->tableWidget->selectRow(row);
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::AddCustom()
{
    const QString name = GetCustomName();
    qint32 currentRow = -1;

    if (ui->tableWidget->currentRow() == -1)
    {
        currentRow  = ui->tableWidget->rowCount();
        m->AddEmpty(name);
    }
    else
    {
        currentRow  = ui->tableWidget->currentRow()+1;
        const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
        m->AddEmptyAfter(nameField->data(Qt::UserRole).toString(), name);
    }

    search->AddRow(currentRow);
    RefreshData();
    search->RefreshList(ui->lineEditFind->text());

    ui->tableWidget->selectRow(currentRow);

    ui->actionExportToCSV->setEnabled(true);

    MeasurementsWereSaved(false);
    ui->tableWidget->repaint(); // Force repain to fix paint artifacts on Mac OS X
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::AddKnown()
{
    QScopedPointer<DialogMDataBase> dialog (new DialogMDataBase(m->ListKnown(), this));
    if (dialog->exec() == QDialog::Accepted)
    {
        qint32 currentRow;

        const QStringList list = dialog->GetNewNames();
        if (ui->tableWidget->currentRow() == -1)
        {
            currentRow  = ui->tableWidget->rowCount() + list.size() - 1;
            for (auto &name : list)
            {
                if (mType == MeasurementsType::Individual)
                {
                    m->AddEmpty(name, VAbstractApplication::VApp()->TrVars()->MFormula(name));
                }
                else
                {
                    m->AddEmpty(name);
                }

                search->AddRow(currentRow);
            }
        }
        else
        {
            currentRow  = ui->tableWidget->currentRow() + list.size();
            const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
            QString after = nameField->data(Qt::UserRole).toString();
            for (auto &name : list)
            {
                if (mType == MeasurementsType::Individual)
                {
                    m->AddEmptyAfter(after, name, VAbstractApplication::VApp()->TrVars()->MFormula(name));
                }
                else
                {
                    m->AddEmptyAfter(after, name);
                }
                search->AddRow(currentRow);
                after = name;
            }
        }

        RefreshData();
        search->RefreshList(ui->lineEditFind->text());

        ui->tableWidget->selectRow(currentRow);

        ui->actionExportToCSV->setEnabled(true);

        MeasurementsWereSaved(false);
    }
    ui->tableWidget->repaint(); // Force repain to fix paint artifacts on Mac OS X
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ImportFromPattern()
{
    if (m == nullptr)
    {
        return;
    }

    const QString filter(tr("Pattern files (*.val)"));
    //Use standard path to individual measurements
    QString pathTo = MApplication::VApp()->TapeSettings()->GetPathPattern();

    const QString mPath = QFileDialog::getOpenFileName(this, tr("Import from a pattern"), pathTo, filter, nullptr,
                                                       VAbstractApplication::VApp()->NativeFileDialog());
    if (mPath.isEmpty())
    {
        return;
    }

    VLockGuard<char> tmp(mPath);
    if (not tmp.IsLocked())
    {
        qCCritical(tMainWindow, "%s", qUtf8Printable(tr("This file already opened in another window.")));
        return;
    }

    QStringList measurements;
    try
    {
        VPatternConverter converter(mPath);
        QScopedPointer<VLitePattern> doc(new VLitePattern());
        doc->setXMLContent(converter.Convert());
        measurements = doc->ListMeasurements();
    }
    catch (VException &e)
    {
        qCCritical(tMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("File error.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        return;
    }

    measurements = FilterMeasurements(measurements, m->ListAll());

    qint32 currentRow;

    if (ui->tableWidget->currentRow() == -1)
    {
        currentRow  = ui->tableWidget->rowCount() + measurements.size() - 1;
        for (auto &mName : measurements)
        {
            m->AddEmpty(mName);
        }
    }
    else
    {
        currentRow  = ui->tableWidget->currentRow() + measurements.size();
        const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
        QString after = nameField->data(Qt::UserRole).toString();
        for (auto &mName : measurements)
        {
            m->AddEmptyAfter(after, mName);
            after = mName;
        }
    }

    RefreshData();

    search->RefreshList(ui->lineEditFind->text());

    ui->tableWidget->selectRow(currentRow);

    MeasurementsWereSaved(false);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::DimensionABaseChanged()
{
    currentDimensionA = gradationDimensionA->currentData().toDouble();

    const QList<MeasurementDimension_p> dimensions = m->Dimensions().values();
    if (dimensions.size() > 1)
    {
        MeasurementDimension_p dimension = dimensions.at(1);
        InitDimensionGradation(1, dimension, gradationDimensionB);

        if (dimensions.size() > 2)
        {
            dimension = dimensions.at(2);
            InitDimensionGradation(2, dimension, gradationDimensionC);
        }
    }

    gradation->start();
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::DimensionBBaseChanged()
{
    currentDimensionB = gradationDimensionB->currentData().toDouble();

    const QList<MeasurementDimension_p> dimensions = m->Dimensions().values();

    if (dimensions.size() > 2)
    {
        MeasurementDimension_p dimension = dimensions.at(2);
        InitDimensionGradation(2, dimension, gradationDimensionC);
    }

    gradation->start();
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::DimensionCBaseChanged()
{
    currentDimensionC = gradationDimensionC->currentData().toDouble();
    gradation->start();
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::GradationChanged()
{
    gradation->stop();
    const int row = ui->tableWidget->currentRow();
    RefreshData();
    search->RefreshList(ui->lineEditFind->text());
    ui->tableWidget->selectRow(row);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ShowMData()
{
    ShowNewMData(true);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ShowNewMData(bool fresh)
{
    if (ui->tableWidget->rowCount() > 0)
    {
        MFields(true);

        if (ui->tableWidget->currentRow() == -1)
        {
            ui->tableWidget->blockSignals(true);
            ui->tableWidget->selectRow(0);
            ui->tableWidget->blockSignals(false);
        }

        const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName); // name
        SCASSERT(nameField != nullptr)
        QSharedPointer<VMeasurement> meash;

        try
        {
            // Translate to internal look.
            meash = data->GetVariable<VMeasurement>(nameField->data(Qt::UserRole).toString());
        }
        catch(const VExceptionBadId &e)
        {
            Q_UNUSED(e)
            MFields(false);
            return;
        }

        ShowMDiagram(meash->GetName());

        // Don't block all signal for QLineEdit. Need for correct handle with clear button.
        disconnect(ui->lineEditName, &QLineEdit::textEdited, this, &TMainWindow::SaveMName);
        ui->plainTextEditDescription->blockSignals(true);
        if (meash->IsCustom())
        {
            ui->plainTextEditDescription->setPlainText(meash->GetDescription());
            ui->lineEditFullName->setText(meash->GetGuiText());
            ui->lineEditName->setText(ClearCustomName(meash->GetName()));
        }
        else
        {
            //Show known
            ui->plainTextEditDescription->setPlainText(
                        VAbstractApplication::VApp()->TrVars()->Description(meash->GetName()));
            ui->lineEditFullName->setText(VAbstractApplication::VApp()->TrVars()->GuiText(meash->GetName()));
            ui->lineEditName->setText(nameField->text());
        }
        connect(ui->lineEditName, &QLineEdit::textEdited, this, &TMainWindow::SaveMName);
        ui->plainTextEditDescription->blockSignals(false);

        ui->comboBoxMUnits->blockSignals(true);
        ui->comboBoxMUnits->setCurrentIndex(
            ui->comboBoxMUnits->findData(static_cast<int>(meash->IsSpecialUnits() ? MUnits::Degrees : MUnits::Table)));
        ui->comboBoxMUnits->blockSignals(false);

        if (mType == MeasurementsType::Multisize)
        {
            ui->labelCalculatedValue->blockSignals(true);
            ui->doubleSpinBoxBaseValue->blockSignals(true);
            ui->doubleSpinBoxCorrection->blockSignals(true);
            ui->doubleSpinBoxShiftA->blockSignals(true);
            ui->doubleSpinBoxShiftB->blockSignals(true);
            ui->doubleSpinBoxShiftC->blockSignals(true);

            QString calculatedValue;

            if (meash->IsSpecialUnits())
            {
                const qreal value = *data->DataVariables()->value(meash->GetName())->GetValue();
                calculatedValue = VAbstractApplication::VApp()->LocaleToString(value) + QChar(QChar::Space) +
                        degreeSymbol;
            }
            else
            {
                const QString postfix = UnitsToStr(pUnit);//Show unit in dialog lable (cm, mm or inch)
                const qreal value = UnitConvertor(*data->DataVariables()->value(meash->GetName())->GetValue(), mUnit,
                                                  pUnit);
                calculatedValue = VAbstractApplication::VApp()->LocaleToString(value) + QChar(QChar::Space) + postfix;
            }
            ui->labelCalculatedValue->setText(calculatedValue);

            if (fresh)
            {
                ui->doubleSpinBoxBaseValue->setValue(meash->GetBase());
                ui->doubleSpinBoxCorrection->setValue(
                    meash->GetCorrection(currentDimensionA, currentDimensionB, currentDimensionC));
                ui->doubleSpinBoxShiftA->setValue(meash->GetShiftA());
                ui->doubleSpinBoxShiftB->setValue(meash->GetShiftB());
                ui->doubleSpinBoxShiftC->setValue(meash->GetShiftC());
            }

            ui->labelCalculatedValue->blockSignals(false);
            ui->doubleSpinBoxBaseValue->blockSignals(false);
            ui->doubleSpinBoxCorrection->blockSignals(false);
            ui->doubleSpinBoxShiftA->blockSignals(false);
            ui->doubleSpinBoxShiftB->blockSignals(false);
            ui->doubleSpinBoxShiftC->blockSignals(false);
        }
        else
        {
            EvalFormula(meash->GetFormula(), false, meash->GetData(), ui->labelCalculatedValue,
                        meash->IsSpecialUnits());

            ui->plainTextEditFormula->blockSignals(true);

            QString formula =
                    VTranslateVars::TryFormulaToUser(meash->GetFormula(),
                                                     VAbstractApplication::VApp()->Settings()->GetOsSeparator());

            ui->plainTextEditFormula->setPlainText(formula);
            ui->plainTextEditFormula->blockSignals(false);

            ui->comboBoxDimension->blockSignals(true);
            ui->comboBoxDimension->setCurrentIndex(
                ui->comboBoxDimension->findData(static_cast<int>(meash->GetDimension())));
            ui->comboBoxDimension->blockSignals(false);
        }

        MeasurementGUI();
    }
    else
    {
        MFields(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ShowMDiagram(const QString &name)
{
    const VTranslateVars *trv = VAbstractApplication::VApp()->TrVars();
    const QString number = trv->MNumber(name);

    if (number.isEmpty())
    {
        ui->labelDiagram->setText(tr("<html><head/><body><p><span style=\" font-size:340pt;\">?</span></p>"
                                     "<p align=\"center\">Unknown measurement</p></body></html>"));
    }
    else
    {
        ui->labelDiagram->setText(QString("<html><head/><body><p align=\"center\">%1</p>"
                                          "<p align=\"center\"><b>%2</b>. <i>%3</i></p></body></html>")
                                          .arg(DialogMDataBase::ImgTag(number), number, trv->GuiText(name)));
    }
    // This part is very ugly, can't find better way to resize dockWidget.
    ui->labelDiagram->adjustSize();
    // And also those 50 px. DockWidget has some border. And i can't find how big it is.
    // Can lead to problem in future.
    ui->dockWidgetDiagram->setMaximumWidth(ui->labelDiagram->width()+50);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::DeployFormula()
{
    SCASSERT(ui->plainTextEditFormula != nullptr)
    SCASSERT(ui->pushButtonGrow != nullptr)

    const QTextCursor cursor = ui->plainTextEditFormula->textCursor();

    if (ui->plainTextEditFormula->height() < DIALOG_MAX_FORMULA_HEIGHT)
    {
        ui->plainTextEditFormula->setFixedHeight(DIALOG_MAX_FORMULA_HEIGHT);
        //Set icon from theme (internal for Windows system)
        ui->pushButtonGrow->setIcon(QIcon::fromTheme(QStringLiteral("go-next"),
                                                     QIcon(":/icons/win.icon.theme/16x16/actions/go-next.png")));
    }
    else
    {
       ui->plainTextEditFormula->setFixedHeight(formulaBaseHeight);
       //Set icon from theme (internal for Windows system)
       ui->pushButtonGrow->setIcon(QIcon::fromTheme(QStringLiteral("go-down"),
                                                    QIcon(":/icons/win.icon.theme/16x16/actions/go-down.png")));
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
void TMainWindow::SaveMName(const QString &text)
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);

    QSharedPointer<VMeasurement> meash;

    try
    {
        // Translate to internal look.
        meash = data->GetVariable<VMeasurement>(nameField->data(Qt::UserRole).toString());
    }
    catch(const VExceptionBadId &e)
    {
        qCWarning(tMainWindow, "%s\n\n%s\n\n%s",
                  qUtf8Printable(tr("Can't find measurement '%1'.").arg(nameField->text())),
                  qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        return;
    }

    QString newName = text;

    if (meash->IsCustom())
    {
        newName.isEmpty() ? newName = GetCustomName() : newName = CustomMSign + newName;

        if (not data->IsUnique(newName))
        {
            qint32 num = 2;
            QString name = newName;
            do
            {
                name = name + QChar('_') + QString::number(num);
                num++;
            } while (not data->IsUnique(name));
            newName = name;
        }

        m->SetMName(nameField->text(), newName);
        MeasurementsWereSaved(false);
        RefreshData();
        search->RefreshList(ui->lineEditFind->text());

        ui->tableWidget->blockSignals(true);
        ui->tableWidget->selectRow(row);
        ui->tableWidget->blockSignals(false);
    }
    else
    {
        qCWarning(tMainWindow, "%s", qUtf8Printable(tr("The name of known measurement forbidden to change.")));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveMValue()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(row, ColumnName);

    QString text = ui->plainTextEditFormula->toPlainText();

    QTableWidgetItem *formulaField = ui->tableWidget->item(row, ColumnFormula);
    if (formulaField->text() == text)
    {
        QTableWidgetItem *result = ui->tableWidget->item(row, ColumnCalcValue);
        const QString postfix = UnitsToStr(mUnit);//Show unit in dialog lable (cm, mm or inch)
        ui->labelCalculatedValue->setText(result->text() + QChar(QChar::Space) +postfix);
        return;
    }

    if (text.isEmpty())
    {
        const QString postfix = UnitsToStr(mUnit);//Show unit in dialog lable (cm, mm or inch)
        ui->labelCalculatedValue->setText(tr("Error") + " (" + postfix + "). " + tr("Empty field."));
        return;
    }

    QSharedPointer<VMeasurement> meash;
    try
    {
        // Translate to internal look.
        meash = data->GetVariable<VMeasurement>(nameField->data(Qt::UserRole).toString());
    }
    catch(const VExceptionBadId & e)
    {
        qCWarning(tMainWindow, "%s\n\n%s\n\n%s",
                  qUtf8Printable(tr("Can't find measurement '%1'.").arg(nameField->text())),
                  qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        return;
    }

    if (not EvalFormula(text, true, meash->GetData(), ui->labelCalculatedValue, meash->IsSpecialUnits()))
    {
        return;
    }

    try
    {
        const QString formula = VAbstractApplication::VApp()->TrVars()
                ->FormulaFromUser(text, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
        m->SetMValue(nameField->data(Qt::UserRole).toString(), formula);
    }
    catch (qmu::QmuParserError &e) // Just in case something bad will happen
    {
        Q_UNUSED(e)
        return;
    }

    MeasurementsWereSaved(false);

    const QTextCursor cursor = ui->plainTextEditFormula->textCursor();

    RefreshData();
    search->RefreshList(ui->lineEditFind->text());

    ui->tableWidget->blockSignals(true);
    ui->tableWidget->selectRow(row);
    ui->tableWidget->blockSignals(false);

    ui->plainTextEditFormula->setTextCursor(cursor);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveMBaseValue(double value)
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
    m->SetMBaseValue(nameField->data(Qt::UserRole).toString(), value);

    MeasurementsWereSaved(false);

    RefreshData();
    search->RefreshList(ui->lineEditFind->text());

    ui->tableWidget->blockSignals(true);
    ui->tableWidget->selectRow(row);
    ui->tableWidget->blockSignals(false);

    ShowNewMData(false);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveMShiftA(double value)
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
    m->SetMShiftA(nameField->data(Qt::UserRole).toString(), value);

    MeasurementsWereSaved(false);

    RefreshData();
    search->RefreshList(ui->lineEditFind->text());

    ui->tableWidget->blockSignals(true);
    ui->tableWidget->selectRow(row);
    ui->tableWidget->blockSignals(false);

    ShowNewMData(false);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveMShiftB(double value)
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
    m->SetMShiftB(nameField->data(Qt::UserRole).toString(), value);

    MeasurementsWereSaved(false);

    RefreshData();
    search->RefreshList(ui->lineEditFind->text());

    ui->tableWidget->blockSignals(true);
    ui->tableWidget->selectRow(row);
    ui->tableWidget->blockSignals(false);

    ShowNewMData(false);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveMShiftC(double value)
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
    m->SetMShiftC(nameField->data(Qt::UserRole).toString(), value);

    MeasurementsWereSaved(false);

    RefreshData();
    search->RefreshList(ui->lineEditFind->text());

    ui->tableWidget->blockSignals(true);
    ui->tableWidget->selectRow(row);
    ui->tableWidget->blockSignals(false);

    ShowNewMData(false);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveMCorrectionValue(double value)
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
    m->SetMCorrectionValue(nameField->data(Qt::UserRole).toString(),
                           currentDimensionA, currentDimensionB, currentDimensionC, value);

    MeasurementsWereSaved(false);

    RefreshData();
    search->RefreshList(ui->lineEditFind->text());

    ui->tableWidget->blockSignals(true);
    ui->tableWidget->selectRow(row);
    ui->tableWidget->blockSignals(false);

    ShowNewMData(false);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveMDescription()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
    m->SetMDescription(nameField->data(Qt::UserRole).toString(), ui->plainTextEditDescription->toPlainText());

    MeasurementsWereSaved(false);

    const QTextCursor cursor = ui->plainTextEditDescription->textCursor();

    RefreshData();

    ui->tableWidget->blockSignals(true);
    ui->tableWidget->selectRow(row);
    ui->tableWidget->blockSignals(false);

    ui->plainTextEditDescription->setTextCursor(cursor);
}


//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveMFullName()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);

    QSharedPointer<VMeasurement> meash;

    try
    {
        // Translate to internal look.
        meash = data->GetVariable<VMeasurement>(nameField->data(Qt::UserRole).toString());
    }
    catch(const VExceptionBadId &e)
    {
        qCWarning(tMainWindow, "%s\n\n%s\n\n%s",
                  qUtf8Printable(tr("Can't find measurement '%1'.").arg(nameField->text())),
                  qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        return;
    }

    if (meash->IsCustom())
    {
        m->SetMFullName(nameField->data(Qt::UserRole).toString(), ui->lineEditFullName->text());

        MeasurementsWereSaved(false);

        RefreshData();

        ui->tableWidget->blockSignals(true);
        ui->tableWidget->selectRow(row);
        ui->tableWidget->blockSignals(false);
    }
    else
    {
        qCWarning(tMainWindow, "%s", qUtf8Printable(tr("The full name of known measurement forbidden to change.")));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveMUnits()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
    const MUnits units = static_cast<MUnits>(ui->comboBoxMUnits->currentData().toInt());
    m->SetMSpecialUnits(nameField->data(Qt::UserRole).toString(), units == MUnits::Degrees);

    MeasurementsWereSaved(false);

    RefreshData();
    search->RefreshList(ui->lineEditFind->text());

    ui->tableWidget->blockSignals(true);
    ui->tableWidget->selectRow(row);
    ui->tableWidget->blockSignals(false);

    ShowNewMData(false);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveMDimension()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
    const IMD dimension = static_cast<IMD>(ui->comboBoxDimension->currentData().toInt());
    m->SetMDimension(nameField->data(Qt::UserRole).toString(), dimension);

    MeasurementsWereSaved(false);

    RefreshData();
    search->RefreshList(ui->lineEditFind->text());

    ui->tableWidget->blockSignals(true);
    ui->tableWidget->selectRow(row);
    ui->tableWidget->blockSignals(false);

    ShowNewMData(false);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::FullCircumferenceChanged(bool checked)
{
    m->SetFullCircumference(checked);
    MeasurementsWereSaved(false);
    InitDimensionsBaseValue();
    InitDimensionControls();
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ExportToIndividual()
{
    QString dir;
    if (curFile.isEmpty())
    {
        dir = MApplication::VApp()->TapeSettings()->GetPathIndividualMeasurements();
    }
    else
    {
        dir = QFileInfo(curFile).absolutePath();
    }

    bool usedNotExistedDir = false;
    QDir directory(dir);
    if (not directory.exists())
    {
        usedNotExistedDir = directory.mkpath(QChar('.'));
    }

    QString filters = tr("Individual measurements") + QStringLiteral(" (*.vit)");
    QString fName = tr("measurements.vit");
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export to individual"), dir + QChar('/') + fName,
                                                    filters, nullptr, VAbstractApplication::VApp()->NativeFileDialog());

    auto RemoveTempDir = qScopeGuard([usedNotExistedDir, dir]()
                                     {
                                         if (usedNotExistedDir)
                                         {
                                             QDir(dir).rmpath(QChar('.'));
                                         }
                                     });

    if (fileName.isEmpty())
    {
        return;
    }

    QString suffix = QStringLiteral("vit");
    QFileInfo f( fileName );
    if (f.suffix().isEmpty() && f.suffix() != suffix)
    {
        fileName += QChar('.') + suffix;
    }

    QScopedPointer<VContainer> tmpData(
                new VContainer(VAbstractApplication::VApp()->TrVars(), &mUnit, VContainer::UniqueNamespace()));

    VMeasurements individualMeasurements(mUnit, tmpData.data());

    const QMap<int, QSharedPointer<VMeasurement> > orderedTable = OrderedMeasurments();
    QMap<int, QSharedPointer<VMeasurement> >::const_iterator iMap;
    for (iMap = orderedTable.constBegin(); iMap != orderedTable.constEnd(); ++iMap)
    {
        const QSharedPointer<VMeasurement> &meash = iMap.value();
        individualMeasurements.AddEmpty(meash->GetName());
        individualMeasurements.SetMValue(meash->GetName(), QString::number(*meash->GetValue()));
        individualMeasurements.SetMSpecialUnits(meash->GetName(), meash->IsSpecialUnits());
        if (meash->IsCustom())
        {
            individualMeasurements.SetMDescription(meash->GetName(), meash->GetDescription());
            individualMeasurements.SetMFullName(meash->GetName(), meash->GetGuiText());
        }
    }

    QString error;
    const bool result = individualMeasurements.SaveDocument(fileName, error);
    if (not result)
    {
        QMessageBox messageBox;
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.setInformativeText(tr("Could not save the file"));
        messageBox.setDefaultButton(QMessageBox::Ok);
        messageBox.setDetailedText(error);
        messageBox.setStandardButtons(QMessageBox::Ok);
        messageBox.exec();
        return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::RestrictFirstDimesion()
{
    const QList<MeasurementDimension_p> dimensions = m->Dimensions().values();
    const QMap<QString, VDimensionRestriction> restrictions = m->GetRestrictions();
    bool fullCircumference = m->IsFullCircumference();

    DialogRestrictDimension dialog(dimensions, restrictions, RestrictDimension::First, fullCircumference, this);
    if (dialog.exec() == QDialog::Rejected)
    {
        return;
    }

    m->SetRestrictions(dialog.Restrictions());
    MeasurementsWereSaved(false);

    if (not dimensions.isEmpty())
    {
        InitDimensionGradation(0, dimensions.at(0), gradationDimensionA);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::RestrictSecondDimesion()
{
    const QList<MeasurementDimension_p> dimensions = m->Dimensions().values();
    const QMap<QString, VDimensionRestriction> restrictions = m->GetRestrictions();
    bool fullCircumference = m->IsFullCircumference();

    DialogRestrictDimension dialog(dimensions, restrictions, RestrictDimension::Second, fullCircumference, this);
    if (dialog.exec() == QDialog::Rejected)
    {
        return;
    }

    m->SetRestrictions(dialog.Restrictions());
    MeasurementsWereSaved(false);
    DimensionABaseChanged(); // trigger refresh
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::RestrictThirdDimesion()
{
    const QList<MeasurementDimension_p> dimensions = m->Dimensions().values();
    const QMap<QString, VDimensionRestriction> restrictions = m->GetRestrictions();
    bool fullCircumference = m->IsFullCircumference();

    DialogRestrictDimension dialog(dimensions, restrictions, RestrictDimension::Third, fullCircumference, this);
    if (dialog.exec() == QDialog::Rejected)
    {
        return;
    }

    m->SetRestrictions(dialog.Restrictions());
    MeasurementsWereSaved(false);
    DimensionABaseChanged(); // trigger refresh
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::EditDimensionLabels()
{
    const QMap<MeasurementDimension, MeasurementDimension_p > dimensions = m->Dimensions();

    DialogDimensionLabels dialog(dimensions, m->IsFullCircumference(), this);
    if (dialog.exec() == QDialog::Rejected)
    {
        return;
    }

    m->SetDimensionLabels(dialog.Labels());

    MeasurementsWereSaved(false);

    InitDimensionsBaseValue();
    InitDimensionControls();
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SetupMenu()
{
    // File
    connect(ui->actionNew, &QAction::triggered, this, &TMainWindow::FileNew);
    ui->actionNew->setShortcuts(QKeySequence::New);

    connect(ui->actionOpenIndividual, &QAction::triggered, this, &TMainWindow::OpenIndividual);
    connect(ui->actionOpenMultisize, &QAction::triggered, this, &TMainWindow::OpenMultisize);
    connect(ui->actionOpenTemplate, &QAction::triggered, this, &TMainWindow::OpenTemplate);
    connect(ui->actionCreateFromExisting, &QAction::triggered, this, &TMainWindow::CreateFromExisting);

    connect(ui->actionSave, &QAction::triggered, this, &TMainWindow::FileSave);
    ui->actionSave->setShortcuts(QKeySequence::Save);

    connect(ui->actionSaveAs, &QAction::triggered, this, &TMainWindow::FileSaveAs);
    ui->actionSaveAs->setShortcuts(QKeySequence::SaveAs);

    connect(ui->actionExportToCSV, &QAction::triggered, this, &TMainWindow::ExportDataToCSV);
    connect(ui->actionImportFromCSV, &QAction::triggered, this, &TMainWindow::ImportDataFromCSV);
    connect(ui->actionReadOnly, &QAction::triggered, this, [this](bool ro)
    {
        if (not mIsReadOnly)
        {
            m->SetReadOnly(ro);
            MeasurementsWereSaved(false);
            UpdatePadlock(ro);
            UpdateWindowTitle();
        }
        else
        {
            if (auto *action = qobject_cast< QAction * >(this->sender()))
            {
                action->setChecked(true);
            }
        }
    });
    connect(ui->actionPreferences, &QAction::triggered, this, &TMainWindow::Preferences);

    for (auto & recentFileAct : m_recentFileActs)
    {
        auto *action = new QAction(this);
        recentFileAct = action;
        connect(action, &QAction::triggered, this, [this]()
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
    ui->menuFile->insertAction(ui->actionPreferences, m_separatorAct );


    connect(ui->actionQuit, &QAction::triggered, this, &TMainWindow::close);
    ui->actionQuit->setShortcuts(QKeySequence::Quit);

    // Measurements
    connect(ui->actionAddCustom, &QAction::triggered, this, &TMainWindow::AddCustom);
    connect(ui->actionAddKnown, &QAction::triggered, this, &TMainWindow::AddKnown);
    connect(ui->actionDatabase, &QAction::triggered, MApplication::VApp(), &MApplication::ShowDataBase);
    connect(ui->actionImportFromPattern, &QAction::triggered, this, &TMainWindow::ImportFromPattern);

    // Window
    connect(ui->menuWindow, &QMenu::aboutToShow, this, &TMainWindow::AboutToShowWindowMenu);
    AboutToShowWindowMenu();

    // Help
    connect(ui->actionAboutQt, &QAction::triggered, this, [this]()
    {
        QMessageBox::aboutQt(this, tr("About Qt"));
    });
    connect(ui->actionAboutTape, &QAction::triggered, this, [this]()
    {
        auto *aboutDialog = new DialogAboutTape(this);
        aboutDialog->setAttribute(Qt::WA_DeleteOnClose, true);
        aboutDialog->show();
    });

    //Actions for recent files loaded by a tape window application.
    UpdateRecentFileActions();

}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::InitWindow()
{
    SCASSERT(m != nullptr)
    ui->labelToolTip->setVisible(false);
    ui->tabWidget->setVisible(true);
    ui->tabWidget->setCurrentIndex(0); // measurements

    ui->plainTextEditNotes->setEnabled(true);
    ui->toolBarGradation->setVisible(true);

    if (mType == MeasurementsType::Multisize)
    {
        InitMenu();
        ui->labelMType->setText(tr("Multisize measurements"));

        InitDimensionsBaseValue();
        HackDimensionBaseValue();

        // Because Qt Designer doesn't know about our deleting we will create empty objects for correct
        // working the retranslation UI
        // Tab Measurements
        HackWidget(&ui->horizontalLayoutValue);
        HackWidget(&ui->plainTextEditFormula);
        HackWidget(&ui->toolButtonExpr);
        HackWidget(&ui->labelFormula);
        HackWidget(&ui->pushButtonGrow);
        HackWidget(&ui->labelDimension);
        HackWidget(&ui->comboBoxDimension);

        // Tab Information
        HackWidget(&ui->lineEditCustomerName);
        HackWidget(&ui->comboBoxGender);
        HackWidget(&ui->lineEditEmail);
        HackWidget(&ui->labelCustomerName);
        HackWidget(&ui->labelBirthDate);
        HackWidget(&ui->dateEditBirthDate);
        HackWidget(&ui->labelGender);
        HackWidget(&ui->labelEmail);

        InitDimensionControls();
        ShowDimensionControls();
        SetDimensionBases();

        InitDimesionShifts();

        connect(ui->doubleSpinBoxBaseValue, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &TMainWindow::SaveMBaseValue);
        connect(ui->doubleSpinBoxCorrection, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &TMainWindow::SaveMCorrectionValue);

        connect(ui->doubleSpinBoxShiftA, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &TMainWindow::SaveMShiftA);
        connect(ui->doubleSpinBoxShiftB, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &TMainWindow::SaveMShiftB);
        connect(ui->doubleSpinBoxShiftC, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &TMainWindow::SaveMShiftC);

        HackDimensionShifts();

        SetDecimals();
    }
    else
    {
        ui->labelMType->setText(tr("Individual measurements"));

        ui->lineEditCustomerName->setEnabled(true);
        ui->dateEditBirthDate->setEnabled(true);
        ui->comboBoxGender->setEnabled(true);
        ui->lineEditEmail->setEnabled(true);

        // Tab Measurements
        HackWidget(&ui->doubleSpinBoxBaseValue);
        HackWidget(&ui->doubleSpinBoxCorrection);
        HackWidget(&ui->labelBaseValue);
        HackWidget(&ui->labelCorrection);
        HackDimensionShifts();

        // Tab Information
        HackDimensionBaseValue();
        HackWidget(&ui->frameBaseValue);
        HackWidget(&ui->labelBaseValues);

        ui->lineEditCustomerName->setText(m->Customer());

        ui->comboBoxGender->clear();
        InitGender(ui->comboBoxGender);
        const qint32 index = ui->comboBoxGender->findData(static_cast<int>(m->Gender()));
        ui->comboBoxGender->setCurrentIndex(index);

        {
            const QLocale dateLocale = QLocale(VAbstractApplication::VApp()->Settings()->GetLocale());
            ui->dateEditBirthDate->setLocale(dateLocale);
            ui->dateEditBirthDate->setDisplayFormat(dateLocale.dateFormat());
            ui->dateEditBirthDate->setDate(m->BirthDate());
        }

        ui->lineEditEmail->setText(m->Email());

        connect(ui->lineEditCustomerName, &QLineEdit::editingFinished, this, &TMainWindow::SaveCustomerName);
        connect(ui->lineEditEmail, &QLineEdit::editingFinished, this, &TMainWindow::SaveEmail);
        connect(ui->comboBoxGender, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                &TMainWindow::SaveGender);
        connect(ui->dateEditBirthDate, &QDateEdit::dateChanged, this, &TMainWindow::SaveBirthDate);
        connect(ui->pushButtonGrow, &QPushButton::clicked, this, &TMainWindow::DeployFormula);

        this->formulaBaseHeight = ui->plainTextEditFormula->height();
        connect(ui->plainTextEditFormula, &QPlainTextEdit::textChanged, this, &TMainWindow::SaveMValue,
                Qt::UniqueConnection);

        connect(ui->toolButtonExpr, &QToolButton::clicked, this, &TMainWindow::Fx);

        InitMeasurementDimension();
        connect(ui->comboBoxDimension, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                &TMainWindow::SaveMDimension);
    }

    ui->comboBoxPMSystem->setEnabled(true);
    ui->comboBoxPMSystem->clear();
    InitPMSystems(ui->comboBoxPMSystem);
    const qint32 index = ui->comboBoxPMSystem->findData(QLatin1Char('p')+m->PMSystem());
    ui->comboBoxPMSystem->setCurrentIndex(index);
    connect(ui->comboBoxPMSystem, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &TMainWindow::SavePMSystem);

    connect(ui->lineEditFind, &QLineEdit::textChanged, this, [this] (const QString &term){search->Find(term);});
    connect(ui->toolButtonFindPrevious, &QToolButton::clicked, this, [this] (){search->FindPrevious();});
    connect(ui->toolButtonFindNext, &QToolButton::clicked, this, [this] (){search->FindNext();});

    connect(search.data(), &VTableSearch::HasResult, this, [this] (bool state)
    {
        ui->toolButtonFindPrevious->setEnabled(state);
    });
    connect(search.data(), &VTableSearch::HasResult, this, [this] (bool state)
    {
        ui->toolButtonFindNext->setEnabled(state);
    });

    ui->plainTextEditNotes->setPlainText(m->Notes());
    connect(ui->plainTextEditNotes, &QPlainTextEdit::textChanged, this, &TMainWindow::SaveNotes);

    ui->actionAddCustom->setEnabled(true);
    ui->actionAddKnown->setEnabled(true);
    ui->actionImportFromPattern->setEnabled(true);
    ui->actionSaveAs->setEnabled(true);

    ui->lineEditName->setValidator(new QRegularExpressionValidator(QRegularExpression(
                                                                       QStringLiteral("^$|")+NameRegExp()),
                                                                   this));

    connect(ui->toolButtonRemove, &QToolButton::clicked, this, &TMainWindow::Remove);
    connect(ui->toolButtonTop, &QToolButton::clicked, this, &TMainWindow::MoveTop);
    connect(ui->toolButtonUp, &QToolButton::clicked, this, &TMainWindow::MoveUp);
    connect(ui->toolButtonDown, &QToolButton::clicked, this, &TMainWindow::MoveDown);
    connect(ui->toolButtonBottom, &QToolButton::clicked, this, &TMainWindow::MoveBottom);

    connect(ui->lineEditName, &QLineEdit::textEdited, this, &TMainWindow::SaveMName);
    connect(ui->plainTextEditDescription, &QPlainTextEdit::textChanged, this, &TMainWindow::SaveMDescription);
    connect(ui->lineEditFullName, &QLineEdit::textEdited, this, &TMainWindow::SaveMFullName);

    connect(ui->pushButtonShowInExplorer, &QPushButton::clicked, this, [this]()
    {
        ShowInGraphicalShell(curFile);
    });

    InitPatternUnits();
    InitMeasurementUnits();

    ui->comboBoxMUnits->blockSignals(true);
    ui->comboBoxMUnits->setCurrentIndex(-1);
    ui->comboBoxMUnits->blockSignals(false);

    connect(ui->comboBoxMUnits, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TMainWindow::SaveMUnits);

    InitTable();
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::InitMenu()
{
    if (mType == MeasurementsType::Multisize)
    {
        // Measurements
        ui->actionUseFullCircumference->setVisible(true);
        ui->actionUseFullCircumference->setEnabled(true);
        ui->actionUseFullCircumference->setChecked(m->IsFullCircumference());
        connect(ui->actionUseFullCircumference, &QAction::triggered, this, &TMainWindow::FullCircumferenceChanged);

        QAction *separator = new QAction(this);
        separator->setSeparator(true);
        ui->menuMeasurements->insertAction(ui->actionUseFullCircumference, separator);

        const QList<MeasurementDimension_p> dimensions = m->Dimensions().values();
        if (dimensions.size() > 0)
        {
            ui->actionRestrictFirstDimension->setVisible(true);
            ui->actionRestrictFirstDimension->setEnabled(true);
            connect(ui->actionRestrictFirstDimension, &QAction::triggered, this, &TMainWindow::RestrictFirstDimesion);

            if (dimensions.size() > 1)
            {
                ui->actionRestrictSecondDimension->setVisible(true);
                ui->actionRestrictSecondDimension->setEnabled(true);
                connect(ui->actionRestrictSecondDimension, &QAction::triggered, this,
                        &TMainWindow::RestrictSecondDimesion);

                if (dimensions.size() > 2)
                {
                    ui->actionRestrictThirdDimension->setVisible(true);
                    ui->actionRestrictThirdDimension->setEnabled(true);
                    connect(ui->actionRestrictThirdDimension, &QAction::triggered, this,
                            &TMainWindow::RestrictThirdDimesion);
                }
            }
        }

        ui->actionDimensionLabels->setVisible(true);
        ui->actionDimensionLabels->setEnabled(true);
        connect(ui->actionDimensionLabels, &QAction::triggered, this, &TMainWindow::EditDimensionLabels);

        // File
        ui->actionExportToIndividual->setVisible(true);
        ui->actionExportToIndividual->setEnabled(true);
        connect(ui->actionExportToIndividual, &QAction::triggered, this, &TMainWindow::ExportToIndividual);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::InitDimensionsBaseValue()
{
    const QList<MeasurementDimension_p> dimensions = m->Dimensions().values();
    const QString unit = UnitsToStr(m->Units(), true);
    const bool fc = m->IsFullCircumference();

    auto DimensionsBaseValue = [this, dimensions, unit, fc](int index, QLabel *name, QLabel *base)
    {
        SCASSERT(name != nullptr)
        SCASSERT(base != nullptr)

        if (dimensions.size() > index)
        {
            MeasurementDimension_p dimension = dimensions.at(index);
            name->setText(VAbstartMeasurementDimension::DimensionName(dimension->Type())+QChar(':'));
            name->setToolTip(VAbstartMeasurementDimension::DimensionToolTip(dimension->Type(),
                                                                            dimension->IsCircumference(),
                                                                            m->IsFullCircumference()));

            DimesionLabels labels = dimension->Labels();

            if (labels.contains(dimension->BaseValue()) && not labels.value(dimension->BaseValue()).isEmpty())
            {
                base->setText(labels.value(dimension->BaseValue()));
            }
            else
            {
                if (dimension->IsCircumference() || dimension->Type() == MeasurementDimension::X)
                {
                    if (dimension->Type() != MeasurementDimension::X && fc)
                    {
                        base->setText(QString("%1 %2").arg(dimension->BaseValue()*2).arg(unit));
                    }
                    else
                    {
                        base->setText(QString("%1 %2").arg(dimension->BaseValue()).arg(unit));
                    }
                }
                else
                {
                    base->setText(QString::number(dimension->BaseValue()));
                }
            }
        }
    };

    DimensionsBaseValue(0, ui->labelDimensionA, ui->labelDimensionABase);
    DimensionsBaseValue(1, ui->labelDimensionB, ui->labelDimensionBBase);
    DimensionsBaseValue(2, ui->labelDimensionC, ui->labelDimensionCBase);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::InitDimensionGradation(int index, const MeasurementDimension_p &dimension, QComboBox *control)
{
    SCASSERT(control != nullptr)

    const bool fc = m->IsFullCircumference();
    const QString unit = UnitsToStr(m->Units(), true);

    qreal current = -1;
    if (control->currentIndex() != -1)
    {
        current = control->currentData().toDouble();
    }

    control->blockSignals(true);
    control->clear();

    const QVector<qreal> bases = DimensionRestrictedValues(index, dimension);
    const DimesionLabels labels = dimension->Labels();

    if (dimension->Type() == MeasurementDimension::X)
    {
        for(auto base : bases)
        {
            if (labels.contains(base) && not labels.value(base).isEmpty())
            {
                control->addItem(labels.value(base), base);
            }
            else
            {
                control->addItem(QString("%1 %2").arg(base).arg(unit), base);
            }
        }
    }
    else if (dimension->Type() == MeasurementDimension::Y)
    {
        for(auto base : bases)
        {
            if (labels.contains(base) && not labels.value(base).isEmpty())
            {
                control->addItem(labels.value(base), base);
            }
            else
            {
                if (dimension->IsCircumference())
                {
                    control->addItem(QString("%1 %2").arg(fc ? base*2 : base).arg(unit), base);
                }
                else
                {
                    control->addItem(QString::number(base), base);
                }
            }
        }
    }
    else if (dimension->Type() == MeasurementDimension::W || dimension->Type() == MeasurementDimension::Z)
    {
        for(auto base : bases)
        {
            if (labels.contains(base) && not labels.value(base).isEmpty())
            {
                control->addItem(labels.value(base), base);
            }
            else
            {
                control->addItem(QString("%1 %2").arg(fc ? base*2 : base).arg(unit), base);
            }
        }
    }

    // after initialization the current index is 0. The signal for changing the index will not be triggered if not make
    // it invalid first
    control->setCurrentIndex(-1);

    int i = control->findData(current);
    if (i != -1)
    {
        control->setCurrentIndex(i);
        control->blockSignals(false);
    }
    else
    {
        control->blockSignals(false);
        control->setCurrentIndex(0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::InitDimensionControls()
{
    const QList<MeasurementDimension_p> dimensions = m->Dimensions().values();
    const QString unit = UnitsToStr(m->Units(), true);

    auto InitControl = [this, dimensions, unit](int index, QLabel *&name, QComboBox *&control)
    {
        if (dimensions.size() > index)
        {
            MeasurementDimension_p dimension = dimensions.at(index);

            if (name == nullptr)
            {
                name = new QLabel(VAbstartMeasurementDimension::DimensionName(dimension->Type())+QChar(':'));
            }
            else
            {
                name->setText(VAbstartMeasurementDimension::DimensionName(dimension->Type())+QChar(':'));
            }
            name->setToolTip(VAbstartMeasurementDimension::DimensionToolTip(dimension->Type(),
                                                                            dimension->IsCircumference(),
                                                                            m->IsFullCircumference()));

            if (control == nullptr)
            {
                control = new QComboBox;
            }

            InitDimensionGradation(index, dimension, control);
        }
    };

    InitControl(0, labelGradationDimensionA, gradationDimensionA);
    InitControl(1, labelGradationDimensionB, gradationDimensionB);
    InitControl(2, labelGradationDimensionC, gradationDimensionC);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::InitDimesionShifts()
{
    const QList<MeasurementDimension_p> dimensions = m->Dimensions().values();

    auto InitShift = [this, dimensions](int index, QLabel *name)
    {
        if (dimensions.size() > index)
        {
            MeasurementDimension_p dimension = dimensions.at(index);

            name->setText(tr("Shift (%1):").arg(VAbstartMeasurementDimension::DimensionName(dimension->Type())));
            name->setToolTip(VAbstartMeasurementDimension::DimensionToolTip(dimension->Type(),
                                                                            dimension->IsCircumference(),
                                                                            m->IsFullCircumference()));
        }
    };

    InitShift(0, ui->labelShiftA);
    InitShift(1, ui->labelShiftB);
    InitShift(2, ui->labelShiftC);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::InitTable()
{
    if (mType == MeasurementsType::Multisize)
    {
        ui->tableWidget->setColumnHidden( ColumnFormula, true );// formula

        RetranslateTableHeaders();
    }
    else
    {
        ui->tableWidget->setColumnHidden( ColumnBaseValue, true );// base value
        ui->tableWidget->setColumnHidden( ColumnShiftA, true );
        ui->tableWidget->setColumnHidden( ColumnShiftB, true );
        ui->tableWidget->setColumnHidden( ColumnShiftC, true );
        ui->tableWidget->setColumnHidden( ColumnCorrection, true );
    }

    connect(ui->tableWidget, &QTableWidget::itemSelectionChanged, this, &TMainWindow::ShowMData);

    ShowUnits();

    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ShowUnits()
{
    const QString unit = UnitsToStr(mUnit);

    ShowHeaderUnits(ui->tableWidget, ColumnCalcValue, UnitsToStr(pUnit));// calculated value
    ShowHeaderUnits(ui->tableWidget, ColumnFormula, unit);// formula
    ShowHeaderUnits(ui->tableWidget, ColumnBaseValue, unit);// base value
    ShowHeaderUnits(ui->tableWidget, ColumnShiftA, unit);
    ShowHeaderUnits(ui->tableWidget, ColumnShiftB, unit);
    ShowHeaderUnits(ui->tableWidget, ColumnShiftC, unit);
    ShowHeaderUnits(ui->tableWidget, ColumnCorrection, unit);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ShowHeaderUnits(QTableWidget *table, int column, const QString &unit)
{
    SCASSERT(table != nullptr)

    QString header = table->horizontalHeaderItem(column)->text();
    const int index = header.indexOf(QLatin1String("("));
    if (index != -1)
    {
        header.remove(index-1, 100);
    }
    const QString unitHeader = QStringLiteral("%1 (%2)").arg(header, unit);
    table->horizontalHeaderItem(column)->setText(unitHeader);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::MeasurementsWereSaved(bool saved)
{
    setWindowModified(!saved);
    not mIsReadOnly ? ui->actionSave->setEnabled(!saved): ui->actionSave->setEnabled(false);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SetCurrentFile(const QString &fileName)
{
    curFile = fileName;
    if (curFile.isEmpty())
    {
        ui->lineEditPathToFile->setText(QChar('<') + tr("Empty") + QChar('>'));
        ui->lineEditPathToFile->setToolTip(tr("File was not saved yet."));
        ui->lineEditPathToFile->setCursorPosition(0);
        ui->pushButtonShowInExplorer->setEnabled(false);
    }
    else
    {
        ui->lineEditPathToFile->setText(QDir::toNativeSeparators(curFile));
        ui->lineEditPathToFile->setToolTip(QDir::toNativeSeparators(curFile));
        ui->lineEditPathToFile->setCursorPosition(0);
        ui->pushButtonShowInExplorer->setEnabled(true);
        auto settings = MApplication::VApp()->TapeSettings();
        QStringList files = settings->GetRecentFileList();
        files.removeAll(fileName);
        files.prepend(fileName);
        while (files.size() > MaxRecentFiles)
        {
            files.removeLast();
        }
        settings->SetRecentFileList(files);
        UpdateRecentFileActions();
    }

    UpdateWindowTitle();
}

//---------------------------------------------------------------------------------------------------------------------
bool TMainWindow::SaveMeasurements(const QString &fileName, QString &error)
{
    const bool result = m->SaveDocument(fileName, error);
    if (result)
    {
        SetCurrentFile(fileName);
        MeasurementsWereSaved(result);
    }
    return result;
}

//---------------------------------------------------------------------------------------------------------------------
bool TMainWindow::MaybeSave()
{
    if (this->isWindowModified())
    {
        if (curFile.isEmpty() && ui->tableWidget->rowCount() == 0)
        {
            return true;// Don't ask if file was created without modifications.
        }

        QScopedPointer<QMessageBox> messageBox(new QMessageBox(tr("Unsaved changes"),
                                                               tr("Measurements have been modified.\n"
                                                                  "Do you want to save your changes?"),
                                                               QMessageBox::Warning, QMessageBox::Yes, QMessageBox::No,
                                                               QMessageBox::Cancel, this, Qt::Sheet));

        messageBox->setDefaultButton(QMessageBox::Yes);
        messageBox->setEscapeButton(QMessageBox::Cancel);

        messageBox->setButtonText(QMessageBox::Yes, curFile.isEmpty() || mIsReadOnly ? tr("Save…") : tr("Save"));
        messageBox->setButtonText(QMessageBox::No, tr("Don't Save"));

        messageBox->setWindowModality(Qt::ApplicationModal);
        const auto ret = static_cast<QMessageBox::StandardButton>(messageBox->exec());

        switch (ret)
        {
            case QMessageBox::Yes:
                if (mIsReadOnly)
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
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
QTableWidgetItem *TMainWindow::AddCell(const QString &text, int row, int column, int aligment, bool ok)
{
    auto *item = new QTableWidgetItem(text);
    item->setTextAlignment(aligment);
    item->setToolTip(text);

    // set the item non-editable (view only), and non-selectable
    Qt::ItemFlags flags = item->flags();
    flags &= ~(Qt::ItemIsEditable); // reset/clear the flag
    item->setFlags(flags);

    if (not ok)
    {
        QBrush brush = item->foreground();
        brush.setColor(Qt::red);
        item->setForeground(brush);
    }

    ui->tableWidget->setItem(row, column, item);

    return item;
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::RefreshData(bool freshCall)
{
    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    data->ClearUniqueNames();
    data->ClearVariables(VarType::Measurement);
    m->ReadMeasurements(currentDimensionA, currentDimensionB, currentDimensionC);

    RefreshTable(freshCall);

    QGuiApplication::restoreOverrideCursor();
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::RefreshTable(bool freshCall)
{
    ui->tableWidget->blockSignals(true);
    ui->tableWidget->clearContents();

    ShowUnits();

    const QMap<int, QSharedPointer<VMeasurement> > orderedTable = OrderedMeasurments();
    qint32 currentRow = -1;
    QMap<int, QSharedPointer<VMeasurement> >::const_iterator iMap;
    ui->tableWidget->setRowCount ( orderedTable.size() );
    for (iMap = orderedTable.constBegin(); iMap != orderedTable.constEnd(); ++iMap)
    {
        const QSharedPointer<VMeasurement> &meash = iMap.value();
        currentRow++;

        if (mType == MeasurementsType::Individual)
        {
            QTableWidgetItem *item = AddCell(VAbstractApplication::VApp()->TrVars()->MToUser(meash->GetName()),
                                             currentRow, ColumnName, Qt::AlignVCenter); // name
            item->setData(Qt::UserRole, meash->GetName());

            if (meash->IsCustom())
            {
                AddCell(meash->GetGuiText(), currentRow, ColumnFullName, Qt::AlignVCenter);
            }
            else
            {
                AddCell(VAbstractApplication::VApp()->TrVars()->GuiText(meash->GetName()), currentRow, ColumnFullName,
                        Qt::AlignVCenter);
            }

            QString calculatedValue;
            if (meash->IsSpecialUnits())
            {
                calculatedValue = locale().toString(*meash->GetValue()) + degreeSymbol;
            }
            else
            {
                calculatedValue = locale().toString(UnitConvertor(*meash->GetValue(), mUnit, pUnit));
            }

            AddCell(calculatedValue, currentRow, ColumnCalcValue, Qt::AlignHCenter | Qt::AlignVCenter,
                    meash->IsFormulaOk()); // calculated value

            QString formula =
                    VTranslateVars::TryFormulaToUser(meash->GetFormula(),
                                                     VAbstractApplication::VApp()->Settings()->GetOsSeparator());

            AddCell(formula, currentRow, ColumnFormula, Qt::AlignVCenter); // formula
        }
        else
        {
            QTableWidgetItem *item = AddCell(VAbstractApplication::VApp()->TrVars()->MToUser(meash->GetName()),
                                             currentRow, 0, Qt::AlignVCenter); // name
            item->setData(Qt::UserRole, meash->GetName());

            if (meash->IsCustom())
            {
                AddCell(meash->GetGuiText(), currentRow, ColumnFullName, Qt::AlignVCenter);
            }
            else
            {
                AddCell(VAbstractApplication::VApp()->TrVars()->GuiText(meash->GetName()), currentRow, ColumnFullName,
                        Qt::AlignVCenter);
            }

            QString calculatedValue;
            if (meash->IsSpecialUnits())
            {
                const qreal value = *data->DataVariables()->value(meash->GetName())->GetValue();
                calculatedValue = locale().toString(value) + degreeSymbol;
            }
            else
            {
                const qreal value = UnitConvertor(*data->DataVariables()->value(meash->GetName())->GetValue(), mUnit,
                                                  pUnit);
                calculatedValue = locale().toString(value);
            }

            AddCell(calculatedValue, currentRow, ColumnCalcValue,
                    Qt::AlignHCenter | Qt::AlignVCenter, meash->IsFormulaOk()); // calculated value

            AddCell(locale().toString(meash->GetBase()), currentRow, ColumnBaseValue,
                    Qt::AlignHCenter | Qt::AlignVCenter); // base value

            AddCell(locale().toString(meash->GetShiftA()), currentRow, ColumnShiftA,
                    Qt::AlignHCenter | Qt::AlignVCenter);

            AddCell(locale().toString(meash->GetShiftB()), currentRow, ColumnShiftB,
                    Qt::AlignHCenter | Qt::AlignVCenter);

            AddCell(locale().toString(meash->GetShiftC()), currentRow, ColumnShiftC,
                    Qt::AlignHCenter | Qt::AlignVCenter);

            AddCell(locale().toString(meash->GetCorrection(currentDimensionA, currentDimensionB, currentDimensionC)),
                    currentRow, ColumnCorrection, Qt::AlignHCenter | Qt::AlignVCenter);
        }
    }

    if (freshCall)
    {
        ui->tableWidget->resizeColumnsToContents();
        ui->tableWidget->resizeRowsToContents();
    }
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->blockSignals(false);

    ui->actionExportToCSV->setEnabled(ui->tableWidget->rowCount() > 0);
}

//---------------------------------------------------------------------------------------------------------------------
QString TMainWindow::GetCustomName() const
{
    qint32 num = 1;
    QString name;
    do
    {
        name = CustomMSign + VAbstractApplication::VApp()->TrVars()->InternalVarToUser(measurement_) +
                QString::number(num);
        num++;
    } while (not data->IsUnique(name));

    return name;
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::Controls()
{
    if (ui->tableWidget->rowCount() > 0)
    {
        ui->toolButtonRemove->setEnabled(true);
    }
    else
    {
        ui->toolButtonRemove->setEnabled(false);
    }

    if (ui->tableWidget->rowCount() >= 2)
    {
        if (ui->tableWidget->currentRow() == 0)
        {
            ui->toolButtonTop->setEnabled(false);
            ui->toolButtonUp->setEnabled(false);
            ui->toolButtonDown->setEnabled(true);
            ui->toolButtonBottom->setEnabled(true);
        }
        else if (ui->tableWidget->currentRow() == ui->tableWidget->rowCount()-1)
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
void TMainWindow::MFields(bool enabled)
{
    ui->lineEditName->setEnabled(enabled);
    ui->plainTextEditDescription->setEnabled(enabled);
    ui->lineEditFullName->setEnabled(enabled);
    ui->comboBoxMUnits->setEnabled(enabled);

    if (mType == MeasurementsType::Multisize)
    {
        ui->doubleSpinBoxBaseValue->setEnabled(enabled);
        ui->doubleSpinBoxShiftA->setEnabled(enabled);
        ui->doubleSpinBoxShiftB->setEnabled(enabled);
        ui->doubleSpinBoxShiftC->setEnabled(enabled);
        ui->doubleSpinBoxCorrection->setEnabled(enabled);
    }
    else
    {
        ui->plainTextEditFormula->setEnabled(enabled);
        ui->pushButtonGrow->setEnabled(enabled);
        ui->toolButtonExpr->setEnabled(enabled);
        ui->comboBoxDimension->setEnabled(enabled);
    }

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
void TMainWindow::UpdateWindowTitle()
{
    QString showName;
    bool isFileWritable = true;
    if (not curFile.isEmpty())
    {
#ifdef Q_OS_WIN32
        qt_ntfs_permission_lookup++; // turn checking on
#endif /*Q_OS_WIN32*/
        isFileWritable = QFileInfo(curFile).isWritable();
#ifdef Q_OS_WIN32
        qt_ntfs_permission_lookup--; // turn it off again
#endif /*Q_OS_WIN32*/
        showName = StrippedName(curFile);
    }
    else
    {
        showName = tr("untitled %1").arg(MApplication::VApp()->MainWindows().size()+1);
        mType == MeasurementsType::Multisize ? showName += QLatin1String(".vst") : showName += QLatin1String(".vit");
    }

    showName += QLatin1String("[*]");

    if (mIsReadOnly || not isFileWritable)
    {
        showName += QStringLiteral(" (") + tr("read only") + QChar(')');
    }

    setWindowTitle(showName);
    setWindowFilePath(curFile);

#if defined(Q_OS_MAC)
    static QIcon fileIcon = QIcon(QCoreApplication::applicationDirPath() +
                                  QLatin1String("/../Resources/measurements.icns"));
    QIcon icon;
    if (not curFile.isEmpty())
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
#endif //defined(Q_OS_MAC)
}

//---------------------------------------------------------------------------------------------------------------------
QString TMainWindow::ClearCustomName(const QString &name) const
{
    QString clear = name;
    const int index = clear.indexOf(CustomMSign);
    if (index == 0)
    {
        clear.remove(0, 1);
    }
    return clear;
}

//---------------------------------------------------------------------------------------------------------------------
bool TMainWindow::EvalFormula(const QString &formula, bool fromUser, VContainer *data, QLabel *label,
                              bool specialUnits)
{
    const QString postfix = specialUnits ? degreeSymbol : UnitsToStr(pUnit);

    if (formula.isEmpty())
    {
        label->setText(tr("Error") + QStringLiteral(" (") + postfix + QStringLiteral("). ") + tr("Empty field."));
        label->setToolTip(tr("Empty field"));
        return false;
    }

    try
    {
        // Replace line return character with spaces for calc if exist
        QString f;
        if (fromUser)
        {
            f = VAbstractApplication::VApp()->TrVars()
                    ->FormulaFromUser(formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
        }
        else
        {
            f = formula;
        }
        QScopedPointer<Calculator> cal(new Calculator());
        qreal result = cal->EvalFormula(data->DataVariables(), f);

        if (qIsInf(result) || qIsNaN(result))
        {
            label->setText(tr("Error") + " (" + postfix + ").");
            label->setToolTip(tr("Invalid result. Value is infinite or NaN. Please, check your calculations."));
            return false;
        }

        if (not specialUnits)
        {
            result = UnitConvertor(result, mUnit, pUnit);
        }

        label->setText(VAbstractApplication::VApp()->LocaleToString(result) + QChar(QChar::Space) +postfix);
        label->setToolTip(tr("Value"));
        return true;
    }
    catch (qmu::QmuParserError &e)
    {
        label->setText(tr("Error") + " (" + postfix + "). " + tr("Parser error: %1").arg(e.GetMsg()));
        label->setToolTip(tr("Parser error: %1").arg(e.GetMsg()));
        return false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::Open(const QString &pathTo, const QString &filter)
{
    const QString mPath = QFileDialog::getOpenFileName(this, tr("Open file"), pathTo, filter, nullptr,
                                                       VAbstractApplication::VApp()->NativeFileDialog());

    if (not mPath.isEmpty())
    {
        if (m == nullptr)
        {
            LoadFile(mPath);
        }
        else
        {
            MApplication::VApp()->NewMainWindow()->LoadFile(mPath);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::UpdatePadlock(bool ro)
{
    ui->actionReadOnly->setChecked(ro);
    if (ro)
    {
        ui->actionReadOnly->setIcon(QIcon("://tapeicon/24x24/padlock_locked.png"));
    }
    else
    {
        ui->actionReadOnly->setIcon(QIcon("://tapeicon/24x24/padlock_opened.png"));
    }

    ui->actionReadOnly->setDisabled(mIsReadOnly);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::MeasurementGUI()
{
    if (const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName))
    {
        const bool isCustom = not (nameField->text().indexOf(CustomMSign) == 0);
        ui->lineEditName->setReadOnly(isCustom);
        ui->plainTextEditDescription->setReadOnly(isCustom);
        ui->lineEditFullName->setReadOnly(isCustom);

        // Need to block signals for QLineEdit in readonly mode because it still emits
        // QLineEdit::editingFinished signal.
        ui->lineEditName->blockSignals(isCustom);
        ui->lineEditFullName->blockSignals(isCustom);

        Controls(); // Buttons remove, up, down
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ReadSettings()
{
    const VTapeSettings *settings = MApplication::VApp()->TapeSettings();

    if (settings->status() == QSettings::NoError)
    {
        restoreGeometry(settings->GetGeometry());
        restoreState(settings->GetWindowState());
        restoreState(settings->GetToolbarsState(), APP_VERSION);

        // Text under tool buton icon
        ToolBarStyles();

        // Stack limit
        //VAbstractApplication::VApp()->getUndoStack()->setUndoLimit(settings->GetUndoCount());
    }
    else
    {
        qWarning() << tr("Cannot read settings from a malformed .INI file.");
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::WriteSettings()
{
    VTapeSettings *settings = MApplication::VApp()->TapeSettings();
    settings->SetGeometry(saveGeometry());
    settings->SetWindowState(saveState());
    settings->SetToolbarsState(saveState(APP_VERSION));

    settings->sync();
    if (settings->status() == QSettings::AccessError)
    {
        qWarning() << tr("Cannot save settings. Access denied.");
    }
}

//---------------------------------------------------------------------------------------------------------------------
QStringList TMainWindow::FilterMeasurements(const QStringList &mNew, const QStringList &mFilter)
{
    return ConvertToList(ConvertToSet<QString>(mNew).subtract(ConvertToSet<QString>(mFilter)));
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::UpdatePatternUnit()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    RefreshTable();

    search->RefreshList(ui->lineEditFind->text());

    ui->tableWidget->selectRow(row);
}

//---------------------------------------------------------------------------------------------------------------------
bool TMainWindow::LoadFromExistingFile(const QString &path)
{
    if (m == nullptr)
    {
        if (not QFileInfo::exists(path))
        {
            qCCritical(tMainWindow, "%s", qUtf8Printable(tr("File '%1' doesn't exist!").arg(path)));
            if (MApplication::VApp()->IsTestMode())
            {
                qApp->exit(V_EX_NOINPUT);
            }
            return false;
        }

        // Check if file already opened
        const QList<TMainWindow*> list = MApplication::VApp()->MainWindows();
        for (auto w : list)
        {
            if (w->CurrentFile() == path)
            {
                w->activateWindow();
                close();
                return false;
            }
        }

        VlpCreateLock(lock, path);

        if (not lock->IsLocked())
        {
            if (not IgnoreLocking(lock->GetLockError(), path))
            {
                return false;
            }
        }

        try
        {
            data = new VContainer(VAbstractApplication::VApp()->TrVars(), &mUnit, VContainer::UniqueNamespace());

            m = new VMeasurements(data);
            m->setXMLContent(path);

            mType = m->Type();

            if (mType == MeasurementsType::Unknown)
            {
                throw VException(tr("File has unknown format."));
            }

            if (mType == MeasurementsType::Multisize)
            {
                throw VException (tr("Export from multisize measurements is not supported."));
            }

            VVITConverter converter(path);
            m_curFileFormatVersion = converter.GetCurrentFormatVersion();
            m_curFileFormatVersionStr = converter.GetFormatVersionStr();
            m->setXMLContent(converter.Convert());// Read again after conversion

            if (not m->IsDefinedKnownNamesValid())
            {
                throw VException(tr("File contains invalid known measurement(s)."));
            }

            mUnit = m->Units();
            pUnit = mUnit;

            currentDimensionA = m->DimensionABase();
            currentDimensionB = m->DimensionBBase();
            currentDimensionC = m->DimensionCBase();

            ui->labelToolTip->setVisible(false);
            ui->tabWidget->setVisible(true);

            InitWindow();

            m->ClearForExport();
            const bool freshCall = true;
            RefreshData(freshCall);

            if (ui->tableWidget->rowCount() > 0)
            {
                ui->tableWidget->selectRow(0);
            }

            lock.reset();// Now we can unlock the file

            mIsReadOnly = m->IsReadOnly();
            UpdatePadlock(mIsReadOnly);
            MeasurementGUI();
        }
        catch (VException &e)
        {
            qCCritical(tMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("File error.")),
                       qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
            ui->labelToolTip->setVisible(true);
            ui->tabWidget->setVisible(false);
            delete m;
            m = nullptr;
            delete data;
            data = nullptr;
            lock.reset();

            if (MApplication::VApp()->IsTestMode())
            {
                qApp->exit(V_EX_NOINPUT);
            }
            return false;
        }
    }
    else
    {
        return MApplication::VApp()->NewMainWindow()->LoadFile(path);
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::CreateWindowMenu(QMenu *menu)
{
    SCASSERT(menu != nullptr)

    QAction *action = menu->addAction(tr("&New Window"));
    connect(action, &QAction::triggered, this, []()
    {
        MApplication::VApp()->NewMainWindow()->activateWindow();
    });
    action->setMenuRole(QAction::NoRole);
    menu->addSeparator();

    const QList<TMainWindow*> windows = MApplication::VApp()->MainWindows();
    for (int i = 0; i < windows.count(); ++i)
    {
        TMainWindow *window = windows.at(i);

        QString title = QStringLiteral("%1. %2").arg(i+1).arg(window->windowTitle());
        const int index = title.lastIndexOf(QLatin1String("[*]"));
        if (index != -1)
        {
            window->isWindowModified() ? title.replace(index, 3, QChar('*')) : title.replace(index, 3, QString());
        }

        QAction *action = menu->addAction(title, this, SLOT(ShowWindow()));
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
bool TMainWindow::IgnoreLocking(int error, const QString &path)
{
    QMessageBox::StandardButton answer = QMessageBox::Abort;
    if (not MApplication::VApp()->IsTestMode())
    {
        switch(error)
        {
            case QLockFile::LockFailedError:
                answer = QMessageBox::warning(this, tr("Locking file"),
                                              tr("This file already opened in another window. Ignore if you want "
                                                 "to continue (not recommended, can cause a data corruption)."),
                                              QMessageBox::Abort|QMessageBox::Ignore, QMessageBox::Abort);
                break;
            case QLockFile::PermissionError:
                answer = QMessageBox::question(this, tr("Locking file"),
                                               tr("The lock file could not be created, for lack of permissions. "
                                                  "Ignore if you want to continue (not recommended, can cause "
                                                  "a data corruption)."),
                                               QMessageBox::Abort|QMessageBox::Ignore, QMessageBox::Abort);
                break;
            case QLockFile::UnknownError:
                answer = QMessageBox::question(this, tr("Locking file"),
                                               tr("Unknown error happened, for instance a full partition "
                                                  "prevented writing out the lock file. Ignore if you want to "
                                                  "continue (not recommended, can cause a data corruption)."),
                                               QMessageBox::Abort|QMessageBox::Ignore, QMessageBox::Abort);
                break;
            default:
                answer = QMessageBox::Abort;
                break;
        }
    }

    if (answer == QMessageBox::Abort)
    {
        qCDebug(tMainWindow, "Failed to lock %s", qUtf8Printable(path));
        qCDebug(tMainWindow, "Error type: %d", error);
        if (MApplication::VApp()->IsTestMode())
        {
            switch(error)
            {
                case QLockFile::LockFailedError:
                    qCCritical(tMainWindow, "%s",
                               qUtf8Printable(tr("This file already opened in another window.")));
                    break;
                case QLockFile::PermissionError:
                    qCCritical(tMainWindow, "%s",
                               qUtf8Printable(tr("The lock file could not be created, for lack of permissions.")));
                    break;
                case QLockFile::UnknownError:
                    qCCritical(tMainWindow, "%s",
                               qUtf8Printable(tr("Unknown error happened, for instance a full partition "
                                                 "prevented writing out the lock file.")));
                    break;
                default:
                    break;
            }

            qApp->exit(V_EX_NOINPUT);
        }
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::HackDimensionBaseValue()
{
    const QList<MeasurementDimension_p> dimensions = m->Dimensions().values();

    auto DimensionsBaseValue = [this, dimensions](int index, QLabel *name, QLabel *base)
    {
        SCASSERT(name != nullptr)
        SCASSERT(base != nullptr)

        if (dimensions.size() <= index)
        {
            HackWidget(&name);
            HackWidget(&base);
        }
    };

    DimensionsBaseValue(0, ui->labelDimensionA, ui->labelDimensionABase);
    DimensionsBaseValue(1, ui->labelDimensionB, ui->labelDimensionBBase);
    DimensionsBaseValue(2, ui->labelDimensionC, ui->labelDimensionCBase);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::HackDimensionShifts()
{
    const QList<MeasurementDimension_p> dimensions = m->Dimensions().values();

    auto HackShift = [this, dimensions](int index, QLabel *&name, QDoubleSpinBox *&shift)
    {
        SCASSERT(name != nullptr)
        SCASSERT(shift != nullptr)

        if (dimensions.size() <= index)
        {
            HackWidget(&name);
            HackWidget(&shift);
        }
    };

    HackShift(0, ui->labelShiftA, ui->doubleSpinBoxShiftA);
    HackShift(1, ui->labelShiftB, ui->doubleSpinBoxShiftB);
    HackShift(2, ui->labelShiftC, ui->doubleSpinBoxShiftC);
}

//---------------------------------------------------------------------------------------------------------------------
QString TMainWindow::CheckMName(const QString &name, const QSet<QString> &importedNames) const
{
    if (name.isEmpty())
    {
        throw VException(tr("Measurement name in is empty."));
    }

    if (importedNames.contains(name))
    {
        throw VException(tr("Imported file must not contain the same name twice."));
    }

    if (name.indexOf(CustomMSign) == 0)
    {
        QRegularExpression rx(NameRegExp());
        if (not rx.match(name).hasMatch())
        {
            throw VException(tr("Merasurement '%1' doesn't match regex pattern.").arg(name));
        }

        if (not data->IsUnique(name))
        {
            throw VException(tr("Merasurement '%1' already used in file.").arg(name));
        }
    }
    else
    {
        if (not ConvertToSet<QString>(AllGroupNames()).contains(name))
        {
            throw VException(tr("Measurement '%1' is not one of known measurements.").arg(name));
        }

        if (not data->IsUnique(name))
        {
            throw VException(tr("Merasurement '%1' already used in file.").arg(name));
        }
    }

    return name;
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ShowError(const QString &text)
{
    QMessageBox messageBox(this);
    messageBox.setIcon(QMessageBox::Critical);
    messageBox.setText(text);
    messageBox.setStandardButtons(QMessageBox::Ok);
    messageBox.setDefaultButton(QMessageBox::Ok);
    messageBox.exec();
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::RefreshDataAfterImport()
{
    const int currentRow = ui->tableWidget->currentRow();
    search->AddRow(currentRow);
    RefreshData();
    search->RefreshList(ui->lineEditFind->text());

    ui->tableWidget->selectRow(currentRow);
    ui->actionExportToCSV->setEnabled(true);
    MeasurementsWereSaved(false);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ImportIndividualMeasurements(const QxtCsvModel &csv, const QVector<int> &map)
{
    const int columns = csv.columnCount();
    const int rows = csv.rowCount();

    if (columns < 2)
    {
        ShowError(tr("Individual measurements require at least 2 columns."));
        return;
    }

    struct IndividualMeasurement
    {
        IndividualMeasurement()
            : name(),
              value('0'),
              fullName(),
              description()
        {}

        QString name;
        QString value;
        QString fullName;
        QString description;
    };

    QVector<IndividualMeasurement> measurements;
    QSet<QString> importedNames;

    for(int i=0; i < rows; ++i)
    {
        try
        {
            const int nameColumn = map.at(static_cast<int>(IndividualMeasurementsColumns::Name));
            const QString name = csv.text(i, nameColumn).simplified();
            if (name.isEmpty())
            {
                ShowError(tr("Error in row %1. Measurement name is empty.").arg(i));
                continue;
            }

            IndividualMeasurement measurement;
            const QString mName = CheckMName(VAbstractApplication::VApp()->TrVars()->MFromUser(name), importedNames);
            importedNames.insert(mName);
            measurement.name = mName;

            const int valueColumn = map.at(static_cast<int>(IndividualMeasurementsColumns::Value));
            measurement.value =
                    VTranslateVars::TryFormulaFromUser(csv.text(i, valueColumn),
                                                       VAbstractApplication::VApp()->Settings()->GetOsSeparator());

            const bool custom = name.startsWith(CustomMSign);
            if (columns > 2 && custom)
            {
                const int fullNameColumn = map.at(static_cast<int>(IndividualMeasurementsColumns::FullName));
                if (fullNameColumn >= 0)
                {
                    measurement.fullName = csv.text(i, fullNameColumn).simplified();
                }
            }

            if (columns > 3 && custom)
            {
                const int descriptionColumn = map.at(static_cast<int>(IndividualMeasurementsColumns::Description));
                if (descriptionColumn >= 0)
                {
                    measurement.description = csv.text(i, descriptionColumn).simplified();
                }
            }

            measurements.append(measurement);
        }
        catch (VException &e)
        {
            ShowError(tr("Error in row %1.").arg(i) + QLatin1Char(' ') + e.ErrorMessage());
            return;
        }
    }

    for(auto &im : qAsConst(measurements))
    {
        m->AddEmpty(im.name, im.value);

        if (not im.fullName.isEmpty())
        {
            m->SetMFullName(im.name, im.fullName);
        }

        if (not im.description.isEmpty())
        {
            m->SetMDescription(im.name, im.description);
        }
    }

    RefreshDataAfterImport();
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ImportMultisizeMeasurements(const QxtCsvModel &csv, const QVector<int> &map)
{
    const int columns = csv.columnCount();
    const int rows = csv.rowCount();

    if (columns < 4)
    {
        ShowError(tr("Multisize measurements require at least 4 columns."));
        return;
    }

    auto ConverToDouble = [](QString text, const QString &error)
    {
        text.replace(" ", QString());
        text = VTranslateVars::TryFormulaFromUser(text, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
        bool ok = false;

        const qreal value = QLocale::c().toDouble(text, &ok);
        if (not ok)
        {
            throw VException(error);
        }
        return value;
    };

    struct MultisizeMeasurement
    {
        MultisizeMeasurement()
            : name(),
              fullName(),
              description()
        {}

        QString name;
        qreal base{0};
        qreal shiftA{0};
        qreal shiftB{0};
        qreal shiftC{0};
        QString fullName;
        QString description;
    };

    QVector<MultisizeMeasurement> measurements;
    QSet<QString> importedNames;

    const QMap<MeasurementDimension, MeasurementDimension_p > dimensions = m->Dimensions();

    for(int i=0; i < rows; ++i)
    {
        try
        {
            const int nameColumn = map.at(static_cast<int>(MultisizeMeasurementsColumns::Name));
            const QString name = csv.text(i, nameColumn).simplified();
            if (name.isEmpty())
            {
                ShowError(tr("Error in row %1. Measurement name is empty.").arg(i));
                continue;
            }

            MultisizeMeasurement measurement;
            const QString mName = CheckMName(VAbstractApplication::VApp()->TrVars()->MFromUser(name), importedNames);
            importedNames.insert(mName);
            measurement.name = mName;

            const int baseValueColumn = map.at(static_cast<int>(MultisizeMeasurementsColumns::BaseValue));
            measurement.base = ConverToDouble(csv.text(i, baseValueColumn),
                                              tr("Cannot convert base value to double in column 2."));

            const int shiftAColumn = map.at(static_cast<int>(MultisizeMeasurementsColumns::ShiftA));
            measurement.shiftA = ConverToDouble(csv.text(i, shiftAColumn),
                                                tr("Cannot convert shift value to double in column %1.")
                                                    .arg(shiftAColumn));

            if (dimensions.size() > 1)
            {
                const int shiftBColumn = map.at(static_cast<int>(MultisizeMeasurementsColumns::ShiftB));
                measurement.shiftB = ConverToDouble(csv.text(i, shiftBColumn),
                                                    tr("Cannot convert shift value to double in column %1.")
                                                        .arg(shiftBColumn));
            }

            if (dimensions.size() > 2)
            {
                const int shiftCColumn = map.at(static_cast<int>(MultisizeMeasurementsColumns::ShiftC));
                measurement.shiftC = ConverToDouble(csv.text(i, shiftCColumn),
                                                    tr("Cannot convert shift value to double in column %1.")
                                                        .arg(shiftCColumn));
            }

            const bool custom = name.startsWith(CustomMSign);
            if (columns > 4 && custom)
            {
                const int fullNameColumn = map.at(static_cast<int>(MultisizeMeasurementsColumns::FullName));
                if (fullNameColumn >= 0)
                {
                    measurement.fullName = csv.text(i, fullNameColumn).simplified();
                }
            }

            if (columns > 5 && custom)
            {
                const int descriptionColumn = map.at(static_cast<int>(MultisizeMeasurementsColumns::Description));
                if (descriptionColumn >= 0)
                {
                    measurement.description = csv.text(i, descriptionColumn).simplified();
                }
            }

            measurements.append(measurement);
        }
        catch (VException &e)
        {
            ShowError(tr("Error in row %1.").arg(i) + QLatin1Char(' ') + e.ErrorMessage());
            return;
        }
    }

    for(auto &mm : qAsConst(measurements))
    {
        m->AddEmpty(mm.name);
        m->SetMBaseValue(mm.name, mm.base);
        m->SetMShiftA(mm.name, mm.shiftA);

        if (dimensions.size() > 1)
        {
            m->SetMShiftB(mm.name, mm.shiftB);
        }

        if (dimensions.size() > 2)
        {
            m->SetMShiftC(mm.name, mm.shiftC);
        }

        if (not mm.fullName.isEmpty())
        {
            m->SetMFullName(mm.name, mm.fullName);
        }

        if (not mm.description.isEmpty())
        {
            m->SetMDescription(mm.name, mm.description);
        }
    }

    RefreshDataAfterImport();
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SetCurrentPatternUnit()
{
    if (comboBoxUnits)
    {
        comboBoxUnits->blockSignals(true);
        const qint32 indexUnit = comboBoxUnits->findData(static_cast<int>(pUnit));
        if (indexUnit != -1)
        {
            comboBoxUnits->setCurrentIndex(indexUnit);
        }
        comboBoxUnits->blockSignals(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ShowDimensionControls()
{
    const QList<MeasurementDimension_p> dimensions = m->Dimensions().values();

    auto ShowControl = [this, dimensions](int index, QLabel *name, QComboBox *control)
    {
        if (dimensions.size() > index)
        {
            SCASSERT(name != nullptr)
            SCASSERT(control != nullptr)

            ui->toolBarGradation->addWidget(name);
            ui->toolBarGradation->addWidget(control);
        }
    };

    ShowControl(0, labelGradationDimensionA, gradationDimensionA);
    ShowControl(1, labelGradationDimensionB, gradationDimensionB);
    ShowControl(2, labelGradationDimensionC, gradationDimensionC);

    if (gradationDimensionA)
    {
        connect(gradationDimensionA, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &TMainWindow::DimensionABaseChanged);
    }

    if (gradationDimensionB)
    {
        connect(gradationDimensionB, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &TMainWindow::DimensionBBaseChanged);
    }

    if (gradationDimensionC)
    {
        connect(gradationDimensionC, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &TMainWindow::DimensionCBaseChanged);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SetDimensionBases()
{
    const QList<MeasurementDimension_p> dimensions = m->Dimensions().values();

    if (dimensions.size() > 0)
    {
        MeasurementDimension_p dimension = dimensions.at(0);
        currentDimensionA = dimension->BaseValue();
    }

    if (dimensions.size() > 1)
    {
        MeasurementDimension_p dimension = dimensions.at(1);
        currentDimensionB = dimension->BaseValue();
    }

    if (dimensions.size() > 2)
    {
        MeasurementDimension_p dimension = dimensions.at(2);
        currentDimensionC = dimension->BaseValue();
    }

    auto SetBase = [dimensions](int index, QComboBox *control, qreal &value)
    {
        if (dimensions.size() > index)
        {
            SCASSERT(control != nullptr)

            MeasurementDimension_p dimension = dimensions.at(index);

            const qint32 i = control->findData(value);
            if (i != -1)
            {
                control->setCurrentIndex(i);
            }
            else
            {
                value = control->currentData().toDouble();
            }
        }
    };

    SetBase(0, gradationDimensionA, currentDimensionA);
    SetBase(1, gradationDimensionB, currentDimensionB);
    SetBase(2, gradationDimensionC, currentDimensionC);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SetCurrentDimensionValues()
{
    const QList<MeasurementDimension_p> dimensions = m->Dimensions().values();

    auto SetDimensionValue = [dimensions](int index, qreal &value)
    {
        if (dimensions.size() > index)
        {
            MeasurementDimension_p dimension = dimensions.at(index);
            value = dimension->BaseValue();
        }
    };

    SetDimensionValue(0, currentDimensionA);
    SetDimensionValue(1, currentDimensionB);
    SetDimensionValue(2, currentDimensionC);
}

//---------------------------------------------------------------------------------------------------------------------
QVector<qreal> TMainWindow::DimensionRestrictedValues(int index, const MeasurementDimension_p &dimension)
{
    VDimensionRestriction restriction;
    if (index == 0)
    {
        restriction = m->Restriction(0);
    }
    else if (index == 1)
    {
        restriction = m->Restriction(currentDimensionA);
    }
    else
    {
        restriction = m->Restriction(currentDimensionA, currentDimensionB);
    }

    const QVector<qreal> bases = dimension->ValidBases();

    qreal min = bases.indexOf(restriction.GetMin()) != -1 ? restriction.GetMin() : dimension->MinValue();
    qreal max = bases.indexOf(restriction.GetMax()) != -1 ? restriction.GetMax() : dimension->MaxValue();

    if (min > max)
    {
        min = dimension->MinValue();
        max = dimension->MaxValue();
    }

    return VAbstartMeasurementDimension::ValidBases(min, max, dimension->Step(), restriction.GetExcludeValues());
}

//---------------------------------------------------------------------------------------------------------------------
QMap<int, QSharedPointer<VMeasurement> > TMainWindow::OrderedMeasurments() const
{
    const QMap<QString, QSharedPointer<VMeasurement> > table = data->DataMeasurements();
    QMap<int, QSharedPointer<VMeasurement> > orderedTable;
    QMap<QString, QSharedPointer<VMeasurement> >::const_iterator iterMap;
    for (iterMap = table.constBegin(); iterMap != table.constEnd(); ++iterMap)
    {
        const QSharedPointer<VMeasurement> &meash = iterMap.value();
        orderedTable.insert(meash->Index(), meash);
    }

    return orderedTable;
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SetDecimals()
{
    switch (mUnit)
    {
        case Unit::Cm:
            ui->doubleSpinBoxBaseValue->setDecimals(1);
            ui->doubleSpinBoxBaseValue->setSingleStep(0.1);

            ui->doubleSpinBoxCorrection->setDecimals(1);
            ui->doubleSpinBoxCorrection->setSingleStep(0.1);

            ui->doubleSpinBoxShiftA->setDecimals(1);
            ui->doubleSpinBoxShiftA->setSingleStep(0.1);

            ui->doubleSpinBoxShiftB->setDecimals(1);
            ui->doubleSpinBoxShiftB->setSingleStep(0.1);

            ui->doubleSpinBoxShiftC->setDecimals(1);
            ui->doubleSpinBoxShiftC->setSingleStep(0.1);
            break;
        case Unit::Mm:
            ui->doubleSpinBoxBaseValue->setDecimals(0);
            ui->doubleSpinBoxBaseValue->setSingleStep(1);

            ui->doubleSpinBoxCorrection->setDecimals(0);
            ui->doubleSpinBoxCorrection->setSingleStep(1);

            ui->doubleSpinBoxShiftA->setDecimals(0);
            ui->doubleSpinBoxShiftA->setSingleStep(1);

            ui->doubleSpinBoxShiftB->setDecimals(0);
            ui->doubleSpinBoxShiftB->setSingleStep(1);

            ui->doubleSpinBoxShiftC->setDecimals(0);
            ui->doubleSpinBoxShiftC->setSingleStep(1);
            break;
        case Unit::Inch:
            ui->doubleSpinBoxBaseValue->setDecimals(5);
            ui->doubleSpinBoxBaseValue->setSingleStep(0.00001);

            ui->doubleSpinBoxCorrection->setDecimals(5);
            ui->doubleSpinBoxCorrection->setSingleStep(0.00001);

            ui->doubleSpinBoxShiftA->setDecimals(5);
            ui->doubleSpinBoxShiftA->setSingleStep(0.00001);

            ui->doubleSpinBoxShiftB->setDecimals(5);
            ui->doubleSpinBoxShiftB->setSingleStep(0.00001);

            ui->doubleSpinBoxShiftC->setDecimals(5);
            ui->doubleSpinBoxShiftC->setSingleStep(0.00001);
            break;
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::InitPatternUnits()
{
    labelPatternUnit = new QLabel(tr("Pattern unit:"));
    ui->toolBarGradation->addWidget(labelPatternUnit);

    comboBoxUnits = new QComboBox(this);
    InitComboBoxUnits();
    SetCurrentPatternUnit();

    connect(comboBoxUnits, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index)
    {
        pUnit = static_cast<Unit>(comboBoxUnits->itemData(index).toInt());

        UpdatePatternUnit();
    });

    ui->toolBarGradation->addWidget(comboBoxUnits);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::InitComboBoxUnits()
{
    SCASSERT(comboBoxUnits != nullptr)
    comboBoxUnits->addItem(UnitsToStr(Unit::Cm, true), QVariant(static_cast<int>(Unit::Cm)));
    comboBoxUnits->addItem(UnitsToStr(Unit::Mm, true), QVariant(static_cast<int>(Unit::Mm)));
    comboBoxUnits->addItem(UnitsToStr(Unit::Inch, true), QVariant(static_cast<int>(Unit::Inch)));
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::InitMeasurementUnits()
{
    if (not m)
    {
        return;
    }

    ui->comboBoxMUnits->blockSignals(true);

    int current = -1;
    if (ui->comboBoxMUnits->currentIndex() != -1)
    {
        current = ui->comboBoxMUnits->currentData().toInt();
    }

    QString units;
    switch (m->Units())
    {
        case Unit::Mm:
            units = tr("Millimeters");
            break;
        case Unit::Inch:
            units = tr("Inches");
            break;
        case Unit::Cm:
            units = tr("Centimeters");
            break;
        default:
            units = "<Invalid>";
            break;
    }

    ui->comboBoxMUnits->clear();
    ui->comboBoxMUnits->addItem(units, QVariant(static_cast<int>(MUnits::Table)));
    ui->comboBoxMUnits->addItem(tr("Degrees"), QVariant(static_cast<int>(MUnits::Degrees)));

    int i = ui->comboBoxMUnits->findData(current);
    if (i != -1)
    {
        ui->comboBoxMUnits->setCurrentIndex(i);
    }

    ui->comboBoxMUnits->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::InitGender(QComboBox *gender)
{
    SCASSERT(gender != nullptr)
    gender->addItem(tr("unknown", "gender"), QVariant(static_cast<int>(GenderType::Unknown)));
    gender->addItem(tr("male", "gender"), QVariant(static_cast<int>(GenderType::Male)));
    gender->addItem(tr("female", "gender"), QVariant(static_cast<int>(GenderType::Female)));
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::InitMeasurementDimension()
{
    ui->comboBoxDimension->blockSignals(true);

    int current = -1;
    if (ui->comboBoxDimension->currentIndex() != -1)
    {
        current = ui->comboBoxDimension->currentData().toInt();
    }

    ui->comboBoxDimension->clear();

    ui->comboBoxDimension->addItem(VMeasurements::IMDName(IMD::N), QVariant(static_cast<int>(IMD::N)));
    ui->comboBoxDimension->addItem(VMeasurements::IMDName(IMD::X), QVariant(static_cast<int>(IMD::X)));
    ui->comboBoxDimension->addItem(VMeasurements::IMDName(IMD::Y), QVariant(static_cast<int>(IMD::Y)));
    ui->comboBoxDimension->addItem(VMeasurements::IMDName(IMD::W), QVariant(static_cast<int>(IMD::W)));
    ui->comboBoxDimension->addItem(VMeasurements::IMDName(IMD::Z), QVariant(static_cast<int>(IMD::Z)));

    int i = ui->comboBoxDimension->findData(current);
    if (i != -1)
    {
        ui->comboBoxDimension->setCurrentIndex(i);
    }

    ui->comboBoxDimension->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::RetranslateTableHeaders()
{
    if (mType == MeasurementsType::Multisize)
    {
        const QList< MeasurementDimension_p > dimensions = m->Dimensions().values();

        if (not dimensions.isEmpty())
        {
            const MeasurementDimension_p& dimension = dimensions.at(0);
            ui->tableWidget->horizontalHeaderItem(ColumnShiftA)->setText(
                tr("%1 shift").arg(VAbstartMeasurementDimension::DimensionName(dimension->Type())));
        }

        if (dimensions.size() < 2)
        {
            ui->tableWidget->setColumnHidden( ColumnShiftB, true );
        }
        else
        {
            const MeasurementDimension_p &dimension = dimensions.at(1);
            ui->tableWidget->horizontalHeaderItem(ColumnShiftB)->setText(
                tr("%1 shift").arg(VAbstartMeasurementDimension::DimensionName(dimension->Type())));
        }

        if (dimensions.size() < 3)
        {
            ui->tableWidget->setColumnHidden( ColumnShiftC, true );
        }
        else
        {
            const MeasurementDimension_p &dimension = dimensions.at(2);
            ui->tableWidget->horizontalHeaderItem(ColumnShiftC)->setText(
                tr("%1 shift").arg(VAbstartMeasurementDimension::DimensionName(dimension->Type())));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class T>
void TMainWindow::HackWidget(T **widget)
{
    delete *widget;
    *widget = new T();
    hackedWidgets.append(*widget);
}
