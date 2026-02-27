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
#include "../ifc/ifcdef.h"
#include "../ifc/xml/utils.h"
#include "../ifc/xml/vpatternconverter.h"
#include "../ifc/xml/vpatternimage.h"
#include "../ifc/xml/vvitconverter.h"
#include "../ifc/xml/vvstconverter.h"
#include "../qmuparser/qmudef.h"
#include "../vformat/knownmeasurements/vknownmeasurement.h"
#include "../vformat/knownmeasurements/vknownmeasurementsdatabase.h"
#include "../vganalytics/vganalytics.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/def.h"
#include "../vmisc/defglobal.h"
#include "../vmisc/dialogs/dialogaskcollectstatistic.h"
#include "../vmisc/dialogs/dialogexporttocsv.h"
#include "../vmisc/dialogs/dialogselectlanguage.h"
#include "../vmisc/literals.h"
#include "../vmisc/qxtcsvmodel.h"
#include "../vmisc/theme/themeDef.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vsysexits.h"
#include "../vpatterndb/calculator.h"
#include "../vpatterndb/variables/vmeasurement.h"
#include "../vpatterndb/vcontainer.h"
#include "../vtools/dialogs/support/dialogeditwrongformula.h"
#include "../vwidgets/vaspectratiopixmaplabel.h"
#include "dialogs/dialogabouttape.h"
#include "dialogs/dialogdimensioncustomnames.h"
#include "dialogs/dialogdimensionlabels.h"
#include "dialogs/dialogmdatabase.h"
#include "dialogs/dialogmeasurementscsvcolumns.h"
#include "dialogs/dialognewmeasurements.h"
#include "dialogs/dialognoknownmeasurements.h"
#include "dialogs/dialogrestrictdimension.h"
#include "dialogs/dialogsetupmultisize.h"
#include "mapplication.h" // Should be last because of definning qApp
#include "tkmmainwindow.h"
#include "ui_tmainwindow.h"
#include "vlitepattern.h"
#include "vtapesettings.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include "../vmisc/vtextcodec.h"
#else
#include <QTextCodec>
#endif

#include <QComboBox>
#include <QCursor>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QMimeType>
#include <QPixmap>
#include <QProcess>
#include <QScopeGuard>
#include <QTimer>
#include <QUuid>
#include <QtNumeric>
#include <chrono>

using namespace std::chrono_literals;

#if defined(Q_OS_MAC)
#include <QDrag>
#include <QMimeData>
#endif // defined(Q_OS_MAC)

using namespace Qt::Literals::StringLiterals;

constexpr int DIALOG_MAX_FORMULA_HEIGHT = 64;

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(tMainWindow, "t.mainwindow") // NOLINT

QT_WARNING_POP

namespace
{
enum class MUnits : qint8
{
    Table,
    Degrees
};

struct IndividualMeasurement
{
    IndividualMeasurement() = default;

    QString name{};           // NOLINT(misc-non-private-member-variables-in-classes)
    QString value{'0'};       // NOLINT(misc-non-private-member-variables-in-classes)
    QString fullName{};       // NOLINT(misc-non-private-member-variables-in-classes)
    QString description{};    // NOLINT(misc-non-private-member-variables-in-classes)
    bool specialUnits{false}; // NOLINT(misc-non-private-member-variables-in-classes)
};

//---------------------------------------------------------------------------------------------------------------------
auto SaveDirPath(const QString &curFile, MeasurementsType mType) -> QString
{
    QString dir;
    if (curFile.isEmpty())
    {
        VTapeSettings *settings = MApplication::VApp()->TapeSettings();
        dir = (mType == MeasurementsType::Individual ? settings->GetPathIndividualMeasurements()
                                                     : settings->GetPathMultisizeMeasurements());
    }
    else
    {
        dir = QFileInfo(curFile).absolutePath();
    }

    return dir;
}

//---------------------------------------------------------------------------------------------------------------------
void InitDimensionXItems(const QVector<qreal> &bases, const DimesionLabels &labels, QComboBox *control,
                         const QString &unit)
{
    SCASSERT(control != nullptr)

    for (auto base : bases)
    {
        if (VFuzzyContains(labels, base) && not VFuzzyValue(labels, base).isEmpty())
        {
            control->addItem(VFuzzyValue(labels, base), base);
        }
        else
        {
            control->addItem(QStringLiteral("%1 %2").arg(base).arg(unit), base);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void InitDimensionYWZItems(const QVector<qreal> &bases, const DimesionLabels &labels, QComboBox *control,
                           const QString &unit, bool bodyMeasurement, bool fc)
{
    for (auto base : bases)
    {
        if (VFuzzyContains(labels, base) && not VFuzzyValue(labels, base).isEmpty())
        {
            control->addItem(VFuzzyValue(labels, base), base);
        }
        else
        {
            if (bodyMeasurement)
            {
                control->addItem(QStringLiteral("%1 %2").arg(fc ? base * 2 : base).arg(unit), base);
            }
            else
            {
                control->addItem(QString::number(base), base);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto ConverToDouble(QString text, const QString &error) -> qreal
{
    text.replace(QStringLiteral(" "), QString());
    text = VTranslateVars::TryFormulaFromUser(text, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    bool ok = false;

    const qreal value = QLocale::c().toDouble(text, &ok);
    if (not ok)
    {
        throw VException(error);
    }
    return value;
}

//---------------------------------------------------------------------------------------------------------------------
void SetIndividualMeasurementFullName(int i, const QString &name, const QxtCsvModel &csv, const QVector<int> &map,
                                      IndividualMeasurement &measurement)
{
    const int columns = csv.columnCount();
    const bool custom = name.startsWith(CustomMSign);
    if (columns > 2 && custom)
    {
        const int fullNameColumn = map.at(static_cast<int>(IndividualMeasurementsColumns::FullName));
        if (fullNameColumn >= 0)
        {
            measurement.fullName = csv.text(i, fullNameColumn).simplified();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void SetIndividualMeasurementDescription(int i, const QString &name, const QxtCsvModel &csv, const QVector<int> &map,
                                         IndividualMeasurement &measurement)
{
    const int columns = csv.columnCount();
    const bool custom = name.startsWith(CustomMSign);
    if (columns > 3 && custom)
    {
        const int descriptionColumn = map.at(static_cast<int>(IndividualMeasurementsColumns::Description));
        if (descriptionColumn >= 0)
        {
            measurement.description = csv.text(i, descriptionColumn).simplified();
        }
    }
}

// We need this enum in case we will add or delete a column. And also make code more readable.
enum
{
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
} // namespace

//---------------------------------------------------------------------------------------------------------------------
TMainWindow::TMainWindow(QWidget *parent)
  : VAbstractMainWindow(parent),
    ui(new Ui::TMainWindow),
    m_gradation(new QTimer(this)),
    m_searchHistory(new QMenu(this))
{
    ui->setupUi(this);

    InitIcons();

    VAbstractApplication::VApp()->Settings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    ui->labelDiagram->setText(UnknownMeasurementImage());

    ui->lineEditFind->installEventFilter(this);
    ui->plainTextEditFormula->installEventFilter(this);

    m_search = QSharedPointer<VTableSearch>(new VTableSearch(ui->tableWidget));

    connect(ui->tabWidget,
            &QTabWidget::currentChanged,
            this,
            [this](int index)
            {
                if (index == -1)
                {
                    return;
                }

                if (index == ui->tabWidget->indexOf(ui->tabMeasurements))
                {
                    ui->actionAddCustom->setEnabled(true);
                    ui->actionAddKnown->setEnabled(true);
                    ui->actionAddSeparator->setEnabled(true);
                }
                else if (index == ui->tabWidget->indexOf(ui->tabInformation))
                {
                    ui->actionAddCustom->setEnabled(false);
                    ui->actionAddKnown->setEnabled(false);
                    ui->actionAddSeparator->setEnabled(false);
                }
            });
    ui->tabWidget->setVisible(false);

    ui->mainToolBar->setContextMenuPolicy(Qt::PreventContextMenu);
    ui->toolBarGradation->setContextMenuPolicy(Qt::PreventContextMenu);

    m_recentFileActs.fill(nullptr);

    connect(m_gradation, &QTimer::timeout, this, &TMainWindow::GradationChanged);

    SetupMenu();

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
#endif // defined(Q_OS_MAC)

    if (MApplication::VApp()->IsAppInGUIMode())
    {
        QTimer::singleShot(1s, this, &TMainWindow::AskDefaultSettings);
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
        connect(manager, &VAbstractShortcutManager::ShortcutsUpdated, this, &TMainWindow::UpdateShortcuts);
        UpdateShortcuts();
    }
}

//---------------------------------------------------------------------------------------------------------------------
TMainWindow::~TMainWindow()
{
    ui->lineEditFind->blockSignals(true); // prevents crash
    delete m_data;
    delete m_m;
    qDeleteAll(m_hackedWidgets);
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto TMainWindow::CurrentFile() const -> QString
{
    return m_curFile;
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SetDimensionABase(qreal base)
{
    if (const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values(); dimensions.isEmpty())
    {
        qCWarning(tMainWindow, "%s\n", qPrintable(tr("The table doesn't provide dimensions")));
        return;
    }

    if (const qint32 i = m_gradationDimensionA->findData(base); i != -1)
    {
        m_gradationDimensionA->setCurrentIndex(i);
    }

    if (not VFuzzyComparePossibleNulls(base, m_currentDimensionA))
    {
        qCWarning(tMainWindow, "%s\n", qPrintable(tr("Invalid base value for dimension A")));
        return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SetDimensionBBase(qreal base)
{
    if (const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values(); dimensions.size() <= 1)
    {
        qCWarning(tMainWindow, "%s\n", qPrintable(tr("The table doesn't support dimension B")));
        return;
    }

    if (const qint32 i = m_gradationDimensionB->findData(base); i != -1)
    {
        m_gradationDimensionB->setCurrentIndex(i);
    }

    if (not VFuzzyComparePossibleNulls(base, m_currentDimensionB))
    {
        qCWarning(tMainWindow, "%s\n", qPrintable(tr("Invalid base value for dimension B")));
        return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SetDimensionCBase(qreal base)
{
    if (const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values(); dimensions.size() <= 2)
    {
        qCWarning(tMainWindow, "%s\n", qPrintable(tr("The table doesn't support dimension C")));
        return;
    }

    if (const qint32 i = m_gradationDimensionC->findData(base); i != -1)
    {
        m_gradationDimensionC->setCurrentIndex(i);
    }

    if (not VFuzzyComparePossibleNulls(base, m_currentDimensionC))
    {
        qCWarning(tMainWindow, "%s\n", qPrintable(tr("Invalid base value for dimension C")));
        return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SetPUnit(Unit unit)
{
    m_pUnit = unit;
    SetCurrentPatternUnit();
    UpdatePatternUnit();
}

//---------------------------------------------------------------------------------------------------------------------
auto TMainWindow::LoadFile(const QString &path) -> bool
{
    if (m_m != nullptr)
    {
        return MApplication::VApp()->NewMainTapeWindow()->LoadFile(path);
    }

    if (not QFileInfo::exists(path))
    {
        qCCritical(tMainWindow, "%s", qUtf8Printable(tr("File '%1' doesn't exist!").arg(path)));
        if (MApplication::VApp()->IsTestMode())
        {
            QCoreApplication::exit(V_EX_NOINPUT);
        }
        return false;
    }

    // Check if file already opened
    const QList<TMainWindow *> list = MApplication::VApp()->MainTapeWindows();
    if (auto w = std::find_if(list.begin(), list.end(),
                              [path](const TMainWindow *window) { return window->CurrentFile() == path; });
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
        m_data = new VContainer(VAbstractApplication::VApp()->TrVars(), &m_mUnit, VContainer::UniqueNamespace());

        m_m = new VMeasurements(m_data);
        m_m->setXMLContent(path);

        m_mType = m_m->Type();

        if (m_mType == MeasurementsType::Unknown)
        {
            throw VException(tr("File has unknown format."));
        }

        if (m_mType == MeasurementsType::Multisize)
        {
            VVSTConverter converter(path);
            m_curFileFormatVersion = converter.GetCurrentFormatVersion();
            m_curFileFormatVersionStr = converter.GetFormatVersionStr();
            m_m->setXMLContent(converter.Convert()); // Read again after conversion

            VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
            if (settings->IsCollectStatistic())
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
        }
        else
        {
            VVITConverter converter(path);
            m_curFileFormatVersion = converter.GetCurrentFormatVersion();
            m_curFileFormatVersionStr = converter.GetFormatVersionStr();
            m_m->setXMLContent(converter.Convert()); // Read again after conversion

            VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
            if (settings->IsCollectStatistic())
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
                statistic->SendIndividualMeasurementsFormatVersion(uptime, m_curFileFormatVersionStr);
            }
        }

        m_mUnit = m_m->Units();
        m_pUnit = m_mUnit;

        m_currentDimensionA = m_m->DimensionABase();
        m_currentDimensionB = m_m->DimensionBBase();
        m_currentDimensionC = m_m->DimensionCBase();

        ui->labelToolTip->setVisible(false);
        ui->tabWidget->setVisible(true);

        m_mIsReadOnly = m_m->IsReadOnly();
        UpdatePadlock(m_mIsReadOnly);

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
        qCCritical(tMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("File error.")), qUtf8Printable(e.ErrorMessage()),
                   qUtf8Printable(e.DetailedInformation()));
        ui->labelToolTip->setVisible(true);
        ui->tabWidget->setVisible(false);
        delete m_m;
        m_m = nullptr;
        delete m_data;
        m_data = nullptr;
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
void TMainWindow::FileNew()
{
    if (m_m != nullptr)
    {
        MApplication::VApp()->NewMainTapeWindow()->FileNew();
        return;
    }

    DialogNewMeasurements measurements(this);
    if (measurements.exec() == QDialog::Rejected)
    {
        return;
    }

    m_mUnit = measurements.MUnit();
    m_pUnit = m_mUnit;
    m_mType = measurements.Type();

    if (m_mType == MeasurementsType::Multisize)
    {
        DialogSetupMultisize setup(m_mUnit, this);
        if (setup.exec() == QDialog::Rejected)
        {
            m_mUnit = Unit::Cm;
            m_pUnit = m_mUnit;
            m_mType = MeasurementsType::Individual;
            return;
        }

        m_data = new VContainer(VAbstractApplication::VApp()->TrVars(), &m_mUnit, VContainer::UniqueNamespace());

        m_m = new VMeasurements(m_mUnit, setup.Dimensions(), m_data);
        m_m->SetFullCircumference(setup.FullCircumference());
        m_curFileFormatVersion = VVSTConverter::MeasurementMaxVer;
        m_curFileFormatVersionStr = VVSTConverter::MeasurementMaxVerStr;

        SetCurrentDimensionValues();
    }
    else
    {
        m_data = new VContainer(VAbstractApplication::VApp()->TrVars(), &m_mUnit, VContainer::UniqueNamespace());

        m_m = new VMeasurements(m_mUnit, m_data);
        m_curFileFormatVersion = VVITConverter::MeasurementMaxVer;
        m_curFileFormatVersionStr = VVITConverter::MeasurementMaxVerStr;
    }

    m_m->SetKnownMeasurements(MApplication::VApp()->TapeSettings()->GetKnownMeasurementsId());

    m_mIsReadOnly = m_m->IsReadOnly();
    UpdatePadlock(m_mIsReadOnly);

    SetCurrentFile(QString());
    MeasurementsWereSaved(false);

    InitWindow();

    MeasurementGUI();

    ui->actionImportFromCSV->setEnabled(true);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::OpenIndividual()
{
    const QString filter = tr("Individual measurements") + " (*.vit);;"_L1 + tr("Multisize measurements") +
                           " (*.vst);;"_L1 + tr("All files") + " (*.*)"_L1;
    // Use standard path to individual measurements
    QString pathTo = MApplication::VApp()->TapeSettings()->GetPathIndividualMeasurements();

    pathTo = Open(pathTo, filter);

    if (!pathTo.isEmpty())
    {
        MApplication::VApp()->TapeSettings()->SetPathIndividualMeasurements(pathTo);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::OpenMultisize()
{
    const QString filter = tr("Multisize measurements") + QStringLiteral(" (*.vst);;") + tr("Individual measurements") +
                           QStringLiteral(" (*.vit);;") + tr("All files") + QStringLiteral(" (*.*)");
    // Use standard path to multisize measurements
    QString pathTo = MApplication::VApp()->TapeSettings()->GetPathMultisizeMeasurements();

    pathTo = Open(pathTo, filter);

    if (!pathTo.isEmpty())
    {
        MApplication::VApp()->TapeSettings()->SetPathMultisizeMeasurements(pathTo);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::OpenTemplate()
{
    const QString filter =
        tr("Measurements") + QStringLiteral(" (*.vst *.vit);;") + tr("All files") + QStringLiteral(" (*.*)");
    // Use standard path to template files
    QString pathTo = MApplication::VApp()->TapeSettings()->GetPathTemplate();
    pathTo = Open(pathTo, filter);

    if (!pathTo.isEmpty())
    {
        MApplication::VApp()->TapeSettings()->SetPathTemplate(pathTo);
    }

    if (m_m != nullptr)
    {                              // The file was opened.
        SetCurrentFile(QString()); // Force user to to save new file
        m_lock.reset();            // remove lock from template
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::CreateFromExisting()
{
    const QString filter =
        tr("Measurements") + QStringLiteral(" (*.vst *.vit);;") + tr("All files") + QStringLiteral(" (*.*)");

    const QString mPath = QFileDialog::getOpenFileName(this, tr("Select file"), QDir::homePath(), filter, nullptr,
                                                       VAbstractApplication::VApp()->NativeFileDialog());

    if (not mPath.isEmpty())
    {
        if (m_m == nullptr)
        {
            LoadFromExistingFile(mPath);
        }
        else
        {
            MApplication::VApp()->NewMainTapeWindow()->CreateFromExisting();
        }
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
        WindowsLocale();

        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);

        ui->lineEditFind->setPlaceholderText(m_search->SearchPlaceholder());
        UpdateSearchControlsTooltips();

        UpdateWindowTitle();

        InitMeasurementUnits();

        RetranslateMDiagram();

        if (m_mType == MeasurementsType::Multisize)
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
            const qint32 index = ui->comboBoxKnownMeasurements->currentIndex();
            ui->comboBoxKnownMeasurements->blockSignals(true);
            ui->comboBoxKnownMeasurements->clear();
            InitKnownMeasurements(ui->comboBoxKnownMeasurements);
            ui->comboBoxKnownMeasurements->setCurrentIndex(index);
            ui->comboBoxKnownMeasurements->blockSignals(false);
        }

        {
            if (m_labelPatternUnit)
            {
                m_labelPatternUnit->setText(tr("Pattern unit:"));
            }

            if (m_comboBoxUnits)
            {
                const qint32 index = m_comboBoxUnits->currentIndex();
                m_comboBoxUnits->blockSignals(true);
                m_comboBoxUnits->clear();
                InitComboBoxUnits();
                m_comboBoxUnits->setCurrentIndex(index);
                m_comboBoxUnits->blockSignals(false);
            }
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
auto TMainWindow::eventFilter(QObject *object, QEvent *event) -> bool
{
    if (auto *plainTextEdit = qobject_cast<QPlainTextEdit *>(object))
    {
        if (event->type() == QEvent::KeyPress)
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            if (const auto *keyEvent = static_cast<QKeyEvent *>(event);
                (keyEvent->key() == Qt::Key_Period) && ((keyEvent->modifiers() & Qt::KeypadModifier) != 0U))
            {
                if (VAbstractApplication::VApp()->Settings()->GetOsSeparator())
                {
                    plainTextEdit->insertPlainText(LocaleDecimalPoint(QLocale()));
                }
                else
                {
                    plainTextEdit->insertPlainText(LocaleDecimalPoint(QLocale::c()));
                }
                return true;
            }
        }
    }
    else if (auto *textEdit = qobject_cast<QLineEdit *>(object))
    {
        if (event->type() == QEvent::KeyPress)
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            if (const auto *keyEvent = static_cast<QKeyEvent *>(event);
                (keyEvent->key() == Qt::Key_Period) && ((keyEvent->modifiers() & Qt::KeypadModifier) != 0U))
            {
                if (VAbstractApplication::VApp()->Settings()->GetOsSeparator())
                {
                    textEdit->insert(LocaleDecimalPoint(QLocale()));
                }
                else
                {
                    textEdit->insert(LocaleDecimalPoint(QLocale::c()));
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

    int columns = 2;
    if (m_mType == MeasurementsType::Individual)
    {
        columns = 5;
    }
    else
    {
        columns = 5;

        if (const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values(); dimensions.size() > 1)
        {
            columns += qMin(static_cast<int>(dimensions.size()), 2);
        }
    }

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

    VKnownMeasurementsDatabase const *db = MApplication::VApp()->KnownMeasurementsDatabase();
    VKnownMeasurements const knownDB = db->KnownMeasurements(m_m->KnownMeasurements());

    const QMap<int, QSharedPointer<VMeasurement>> orderedTable = OrderedMeasurements();
    int row = 0;
    for (auto iMap = orderedTable.constBegin(); iMap != orderedTable.constEnd(); ++iMap)
    {
        ExportRowToCSV(csv, row, iMap.value(), knownDB);
        ++row;
    }

    QString error;
    csv.toCSV(fileName, error, withHeader, separator, VTextCodec::codecForMib(mib));
}

//---------------------------------------------------------------------------------------------------------------------
auto TMainWindow::RecentFileList() const -> QStringList
{
    return MApplication::VApp()->TapeSettings()->GetRecentFileList();
}

//---------------------------------------------------------------------------------------------------------------------
auto TMainWindow::FileSave() -> bool
{
    if (m_curFile.isEmpty() || m_mIsReadOnly)
    {
        return FileSaveAs();
    }

    if (m_mType == MeasurementsType::Multisize && m_curFileFormatVersion < VVSTConverter::MeasurementMaxVer &&
        not ContinueFormatRewrite(m_curFileFormatVersionStr, VVSTConverter::MeasurementMaxVerStr))
    {
        return false;
    }

    if (m_mType == MeasurementsType::Individual && m_curFileFormatVersion < VVITConverter::MeasurementMaxVer &&
        not ContinueFormatRewrite(m_curFileFormatVersionStr, VVITConverter::MeasurementMaxVerStr))
    {
        return false;
    }

    if (not CheckFilePermissions(m_curFile, this))
    {
        return false;
    }

    if (QString error; not SaveMeasurements(m_curFile, error))
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

    if (m_mType == MeasurementsType::Multisize)
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
auto TMainWindow::FileSaveAs() -> bool
{
    QString filters;
    QString fName = tr("measurements");
    QString suffix;
    if (m_mType == MeasurementsType::Individual)
    {
        filters = tr("Individual measurements") + QStringLiteral(" (*.vit)");
        suffix = QStringLiteral("vit");
    }
    else
    {
        filters = tr("Multisize measurements") + QStringLiteral(" (*.vst)");
        suffix = QStringLiteral("vst");
    }

    fName += '.'_L1 + suffix;

    const QString dir = SaveDirPath(m_curFile, m_mType);

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

    if (m_curFile.isEmpty())
    {
        VTapeSettings *settings = MApplication::VApp()->TapeSettings();
        m_mType == MeasurementsType::Individual
            ? settings->SetPathIndividualMeasurements(QFileInfo(fileName).absolutePath())
            : settings->SetPathMultisizeMeasurements(QFileInfo(fileName).absolutePath());
    }

    if (QFileInfo::exists(fileName) && m_curFile != fileName)
    {
        // Temporary try to lock the file before saving
        VLockGuard<char> const tmp(fileName);
        if (not tmp.IsLocked())
        {
            qCCritical(tMainWindow, "%s",
                       qUtf8Printable(tr("Failed to lock. This file already opened in another window.")));
            return false;
        }
    }

    // Need for restoring previous state in case of failure
    const bool readOnly = m_m->IsReadOnly();

    m_m->SetReadOnly(false);
    m_mIsReadOnly = false;

    QString error;
    if (bool const result = SaveMeasurements(fileName, error); not result)
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

    if (m_mType == MeasurementsType::Multisize)
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

    if (m_curFile == fileName && not m_lock.isNull())
    {
        m_lock->Unlock();
    }
    VlpCreateLock(m_lock, fileName);
    if (not m_lock->IsLocked())
    {
        qCCritical(tMainWindow, "%s",
                   qUtf8Printable(tr("Failed to lock. This file already opened in another window. "
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
    if (auto *action = qobject_cast<QAction *>(sender()))
    {
        const QVariant v = action->data();
        if (v.canConvert<int>())
        {
            const int offset = qvariant_cast<int>(v);
            const QList<TMainWindow *> windows = MApplication::VApp()->MainTapeWindows();
            windows.at(offset)->raise();
            windows.at(offset)->activateWindow();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ImportDataFromCSV()
{
    if (m_m == nullptr || m_m->Type() == MeasurementsType::Unknown)
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

        QSharedPointer<DialogMeasurementsCSVColumns> columns;
        if (m_m->Type() == MeasurementsType::Multisize)
        {
            const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values();
            columns = QSharedPointer<DialogMeasurementsCSVColumns>::create(fileName, m_m->Type(), dimensions, this);
        }
        else
        {
            columns = QSharedPointer<DialogMeasurementsCSVColumns>::create(fileName, m_m->Type(), this);
        }
        columns->SetWithHeader(dialog.IsWithHeader());
        columns->SetSeparator(dialog.GetSeparator());
        columns->SetCodec(VTextCodec::codecForMib(dialog.GetSelectedMib()));

        if (columns->exec() == QDialog::Accepted)
        {
            QxtCsvModel const csv(fileName, nullptr, dialog.IsWithHeader(), dialog.GetSeparator(),
                                  VTextCodec::codecForMib(dialog.GetSelectedMib()));
            const QVector<int> map = columns->ColumnsMap();

            if (m_m->Type() == MeasurementsType::Individual)
            {
                ImportIndividualMeasurements(csv, map, dialog.IsWithHeader());
            }
            else
            {
                ImportMultisizeMeasurements(csv, map, dialog.IsWithHeader());
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
        connect(actionPreferences, &QAction::triggered, this, [this]() { MApplication::VApp()->Preferences(this); });
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::OpenAt(QAction *where)
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
void TMainWindow::SaveCustomerName()
{
    if (m_m->Customer() != ui->lineEditCustomerName->text())
    {
        m_m->SetCustomer(ui->lineEditCustomerName->text());
        MeasurementsWereSaved(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveEmail()
{
    if (m_m->Email() != ui->lineEditEmail->text())
    {
        m_m->SetEmail(ui->lineEditEmail->text());
        MeasurementsWereSaved(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveGender(int index)
{
    const auto type = static_cast<GenderType>(ui->comboBoxGender->itemData(index).toInt());
    if (m_m->Gender() != type)
    {
        m_m->SetGender(type);
        MeasurementsWereSaved(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveBirthDate(const QDate &date)
{
    if (m_m->BirthDate() != date)
    {
        m_m->SetBirthDate(date);
        MeasurementsWereSaved(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveNotes()
{
    if (m_m->Notes() != ui->plainTextEditNotes->toPlainText())
    {
        m_m->SetNotes(ui->plainTextEditNotes->toPlainText());
        MeasurementsWereSaved(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveKnownMeasurements(int index)
{
    QUuid const known = ui->comboBoxKnownMeasurements->itemData(index).toUuid();

    ui->actionEditCurrentKnownMeasurements->setEnabled(KnownMeasurementsRegistred(known));

    if (m_m->KnownMeasurements() != known)
    {
        m_m->SetKnownMeasurements(known);
        MeasurementsWereSaved(false);
        SyncKnownMeasurements();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::Remove()
{
    ShowMDiagram(QSharedPointer<VMeasurement>());
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), 0);
    m_m->Remove(nameField->data(Qt::UserRole).toString());

    MeasurementsWereSaved(false);

    m_search->RemoveRow(row);
    RefreshData();
    m_search->RefreshList(ui->lineEditFind->text());

    if (ui->tableWidget->rowCount() > 0)
    {
        ui->tableWidget->selectRow(row >= ui->tableWidget->rowCount() ? ui->tableWidget->rowCount() - 1 : row);
    }
    else
    {
        MFields(false);

        ui->toolButtonAddImage->setEnabled(false);
        ui->toolButtonRemoveImage->setEnabled(false);
        ui->toolButtonSaveImage->setEnabled(false);

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

        if (m_mType == MeasurementsType::Multisize)
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
    m_m->MoveTop(nameField->data(Qt::UserRole).toString());
    MeasurementsWereSaved(false);
    RefreshData();
    m_search->RefreshList(ui->lineEditFind->text());
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
    m_m->MoveUp(nameField->data(Qt::UserRole).toString());
    MeasurementsWereSaved(false);
    RefreshData();
    m_search->RefreshList(ui->lineEditFind->text());
    ui->tableWidget->selectRow(row - 1);
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
    m_m->MoveDown(nameField->data(Qt::UserRole).toString());
    MeasurementsWereSaved(false);
    RefreshData();
    m_search->RefreshList(ui->lineEditFind->text());
    ui->tableWidget->selectRow(row + 1);
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
    m_m->MoveBottom(nameField->data(Qt::UserRole).toString());
    MeasurementsWereSaved(false);
    RefreshData();
    m_search->RefreshList(ui->lineEditFind->text());
    ui->tableWidget->selectRow(ui->tableWidget->rowCount() - 1);
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
        meash = m_data->GetVariable<VMeasurement>(nameField->data(Qt::UserRole).toString());
    }
    catch (const VExceptionBadId &e)
    {
        qCCritical(tMainWindow, "%s\n\n%s\n\n%s",
                   qUtf8Printable(tr("Can't find measurement '%1'.").arg(nameField->text())),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        return;
    }

    auto *dialog = new DialogEditWrongFormula(meash->GetData(), NULL_ID, this);
    dialog->setWindowTitle(tr("Edit measurement"));
    dialog->SetMeasurementsMode();
    dialog->SetFormula(VTranslateVars::TryFormulaFromUser(ui->plainTextEditFormula->toPlainText(), true));
    const QString postfix = UnitsToStr(m_mUnit, true); // Show unit in dialog lable (cm, mm or inch)
    dialog->setPostfix(postfix);

    if (dialog->exec() == QDialog::Accepted)
    {
        // Fix the bug #492. https://bitbucket.org/dismine/valentina/issues/492/valentina-crashes-when-add-an-increment
        // Because of the bug need to take QTableWidgetItem twice time. Previous update "killed" the pointer.
        nameField = ui->tableWidget->item(row, ColumnName);
        m_m->SetMValue(nameField->data(Qt::UserRole).toString(), dialog->GetFormula());

        MeasurementsWereSaved(false);

        RefreshData();

        m_search->RefreshList(ui->lineEditFind->text());

        ui->tableWidget->selectRow(row);
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::AddImage()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        ui->toolButtonAddImage->setDisabled(true);
        return;
    }

    VTapeSettings *settings = MApplication::VApp()->TapeSettings();

    const QString filePath =
        QFileDialog::getOpenFileName(this, tr("Measurement image"), settings->GetPathCustomImage(),
                                     PrepareImageFilters(), nullptr, VAbstractApplication::VApp()->NativeFileDialog());

    if (!filePath.isEmpty())
    {
        if (QFileInfo::exists(filePath))
        {
            settings->SetPathCustomImage(QFileInfo(filePath).absolutePath());
        }

        VPatternImage const image = VPatternImage::FromFile(filePath);

        if (not image.IsValid())
        {
            qCritical() << tr("Invalid image. Error: %1").arg(image.ErrorString());
            return;
        }

        const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
        m_m->SetMImage(nameField->data(Qt::UserRole).toString(), image);

        MeasurementsWereSaved(false);

        RefreshData();
        m_search->RefreshList(ui->lineEditFind->text());

        ui->tableWidget->blockSignals(true);
        ui->tableWidget->selectRow(row);
        ui->tableWidget->blockSignals(false);

        ShowNewMData(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::RemoveImage()
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        ui->toolButtonRemoveImage->setDisabled(true);
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
    m_m->SetMImage(nameField->data(Qt::UserRole).toString(), VPatternImage());

    MeasurementsWereSaved(false);

    RefreshData();
    m_search->RefreshList(ui->lineEditFind->text());

    ui->tableWidget->blockSignals(true);
    ui->tableWidget->selectRow(row);
    ui->tableWidget->blockSignals(false);

    ShowNewMData(false);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveImage()
{
    if (const int row = ui->tableWidget->currentRow(); row == -1)
    {
        ui->toolButtonSaveImage->setDisabled(true);
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName); // name
    SCASSERT(nameField != nullptr)
    QSharedPointer<VMeasurement> meash;

    try
    {
        // Translate to internal look.
        meash = m_data->GetVariable<VMeasurement>(nameField->data(Qt::UserRole).toString());
    }
    catch (const VExceptionBadId &e)
    {
        Q_UNUSED(e)
        qCritical() << "Unable to get measurement";
        return;
    }

    const VPatternImage image = meash->GetImage();

    if (not image.IsValid())
    {
        qCritical() << tr("Unable to save image. Error: %1").arg(image.ErrorString());
        return;
    }

    VTapeSettings *settings = MApplication::VApp()->TapeSettings();

    QMimeType const mime = image.MimeTypeFromData();
    QString path = settings->GetPathCustomImage() + QDir::separator() + tr("untitled");

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
void TMainWindow::ShowImage()
{
    if (const int row = ui->tableWidget->currentRow(); row == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName); // name
    SCASSERT(nameField != nullptr)
    QSharedPointer<VMeasurement> meash;

    try
    {
        // Translate to internal look.
        meash = m_data->GetVariable<VMeasurement>(nameField->data(Qt::UserRole).toString());
    }
    catch (const VExceptionBadId &e)
    {
        Q_UNUSED(e)
        qCritical() << "Unable to get measurement";
        return;
    }

    const VPatternImage image = meash->GetImage();

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
void TMainWindow::AddCustom()
{
    const QString name = GetCustomName();
    qint32 currentRow = -1;

    if (ui->tableWidget->currentRow() == -1)
    {
        currentRow = ui->tableWidget->rowCount();
        m_m->AddEmpty(name);
    }
    else
    {
        currentRow = ui->tableWidget->currentRow() + 1;
        const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
        m_m->AddEmptyAfter(nameField->data(Qt::UserRole).toString(), name);
    }

    m_search->AddRow(currentRow);
    RefreshData();
    m_search->RefreshList(ui->lineEditFind->text());

    ui->tableWidget->selectRow(currentRow);

    ui->actionExportToCSV->setEnabled(true);

    MeasurementsWereSaved(false);
    ui->tableWidget->repaint(); // Force repain to fix paint artifacts on Mac OS X
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::AddKnown()
{
    if (m_m->KnownMeasurements().isNull())
    {
        if (DialogNoKnownMeasurements dialog(this); dialog.exec() == QDialog::Accepted)
        {
            MApplication::VApp()->NewMainKMWindow();
        }
        return;
    }

    QScopedPointer<DialogMDataBase> const dialog(new DialogMDataBase(m_m->KnownMeasurements(), m_m->ListKnown(), this));
    if (dialog->exec() == QDialog::Rejected)
    {
        return;
    }

    vsizetype currentRow = -1;
    const QStringList list = dialog->GetNewNames();
    VKnownMeasurementsDatabase *db = MApplication::VApp()->KnownMeasurementsDatabase();
    VKnownMeasurements const knownDB = db->KnownMeasurements(m_m->KnownMeasurements());

    auto AddMeasurement = [this, knownDB, &currentRow](const QString &name)
    {
        VKnownMeasurement const known = knownDB.Measurement(name);

        if (m_mType == MeasurementsType::Individual)
        {
            m_m->AddEmpty(name, known.formula, known.specialUnits);
        }
        else
        {
            m_m->AddEmpty(name, QString(), known.specialUnits);
        }

        m_search->AddRow(currentRow);
    };

    if (ui->tableWidget->currentRow() == -1)
    {
        currentRow = ui->tableWidget->rowCount() + list.size() - 1;
        for (const auto &name : list)
        {
            AddMeasurement(name);
        }
    }
    else
    {
        currentRow = ui->tableWidget->currentRow() + list.size();
        const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
        QString after = nameField->data(Qt::UserRole).toString();
        for (const auto &name : list)
        {
            AddMeasurement(name);
            after = name;
        }
    }

    RefreshData();
    m_search->RefreshList(ui->lineEditFind->text());

    ui->tableWidget->selectRow(static_cast<int>(currentRow));

    ui->actionExportToCSV->setEnabled(true);

    MeasurementsWereSaved(false);

    ui->tableWidget->repaint(); // Force repain to fix paint artifacts on Mac OS X
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::AddSeparator()
{
    const QString name = GetCustomName();
    qint32 currentRow = -1;

    if (ui->tableWidget->currentRow() == -1)
    {
        currentRow = ui->tableWidget->rowCount();
        m_m->AddSeparator(name);
    }
    else
    {
        currentRow = ui->tableWidget->currentRow() + 1;
        const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
        m_m->AddSeparatorAfter(nameField->data(Qt::UserRole).toString(), name);
    }

    m_search->AddRow(currentRow);
    RefreshData();
    m_search->RefreshList(ui->lineEditFind->text());

    ui->tableWidget->selectRow(currentRow);

    ui->actionExportToCSV->setEnabled(true);

    MeasurementsWereSaved(false);
    ui->tableWidget->repaint(); // Force repain to fix paint artifacts on Mac OS X
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ImportFromPattern()
{
    if (m_m == nullptr)
    {
        return;
    }

    const QString filter(tr("Pattern files (*.val)"));
    // Use standard path to individual measurements
    QString const pathTo = MApplication::VApp()->TapeSettings()->GetPathPattern();

    const QString mPath = QFileDialog::getOpenFileName(this, tr("Import from a pattern"), pathTo, filter, nullptr,
                                                       VAbstractApplication::VApp()->NativeFileDialog());
    if (mPath.isEmpty())
    {
        return;
    }

    if (VLockGuard<char> const tmp(mPath); not tmp.IsLocked())
    {
        qCCritical(tMainWindow, "%s", qUtf8Printable(tr("This file already opened in another window.")));
        return;
    }

    QStringList measurements;
    try
    {
        VPatternConverter converter(mPath);
        QScopedPointer<VLitePattern> const doc(new VLitePattern());
        doc->setXMLContent(converter.Convert());
        measurements = doc->ListMeasurements();

        VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
        if (settings->IsCollectStatistic())
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
            statistic->SendPatternFormatVersion(uptime, converter.GetFormatVersionStr());
        }
    }
    catch (VException &e)
    {
        qCCritical(tMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("File error.")), qUtf8Printable(e.ErrorMessage()),
                   qUtf8Printable(e.DetailedInformation()));
        return;
    }

    measurements = FilterMeasurements(measurements, m_m->ListAll());

    vsizetype currentRow;

    if (ui->tableWidget->currentRow() == -1)
    {
        currentRow = ui->tableWidget->rowCount() + measurements.size() - 1;
        for (auto &mName : measurements)
        {
            m_m->AddEmpty(mName);
        }
    }
    else
    {
        currentRow = ui->tableWidget->currentRow() + measurements.size();
        const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
        QString after = nameField->data(Qt::UserRole).toString();
        for (auto &mName : measurements)
        {
            m_m->AddEmptyAfter(after, mName);
            after = mName;
        }
    }

    RefreshData();

    m_search->RefreshList(ui->lineEditFind->text());

    ui->tableWidget->selectRow(static_cast<int>(currentRow));

    MeasurementsWereSaved(false);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::DimensionABaseChanged()
{
    m_currentDimensionA = m_gradationDimensionA->currentData().toDouble();

    if (const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values(); dimensions.size() > 1)
    {
        MeasurementDimension_p dimension = dimensions.at(1);
        InitDimensionGradation(1, dimension, m_gradationDimensionB);

        if (dimensions.size() > 2)
        {
            dimension = dimensions.at(2);
            InitDimensionGradation(2, dimension, m_gradationDimensionC);
        }
    }

    m_gradation->start();
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::DimensionBBaseChanged()
{
    m_currentDimensionB = m_gradationDimensionB->currentData().toDouble();

    if (const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values(); dimensions.size() > 2)
    {
        const MeasurementDimension_p &dimension = dimensions.at(2);
        InitDimensionGradation(2, dimension, m_gradationDimensionC);
    }

    m_gradation->start();
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::DimensionCBaseChanged()
{
    m_currentDimensionC = m_gradationDimensionC->currentData().toDouble();
    m_gradation->start();
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::GradationChanged()
{
    m_gradation->stop();
    const int row = ui->tableWidget->currentRow();
    RefreshData();
    m_search->RefreshList(ui->lineEditFind->text());
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
    if (ui->tableWidget->rowCount() <= 0)
    {
        MFields(false);
        return;
    }

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
        meash = m_data->GetVariable<VMeasurement>(nameField->data(Qt::UserRole).toString());
    }
    catch (const VExceptionBadId &e)
    {
        Q_UNUSED(e)
        MFields(false);
        return;
    }

    ShowMDiagram(meash);

    ui->toolButtonAddImage->setEnabled(meash->IsCustom());
    ui->toolButtonRemoveImage->setEnabled(meash->IsCustom() && !meash->GetCustomImage().IsNull());
    ui->toolButtonSaveImage->setEnabled(!meash->GetImage().IsNull());

    ui->labelFullName->setVisible(meash->GetType() == VarType::Measurement);
    ui->lineEditFullName->setVisible(meash->GetType() == VarType::Measurement);

    ui->labelAlias->setVisible(meash->GetType() == VarType::Measurement);
    ui->lineEditAlias->setVisible(meash->GetType() == VarType::Measurement);

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
        // Show known
        VKnownMeasurementsDatabase *db = MApplication::VApp()->KnownMeasurementsDatabase();
        VKnownMeasurements const knownDB = db->KnownMeasurements(meash->GetKnownMeasurementsId());
        VKnownMeasurement const known = knownDB.Measurement(meash->GetName());

        ui->plainTextEditDescription->setPlainText(known.description);
        ui->lineEditFullName->setText(known.fullName);
        ui->lineEditName->setText(nameField->text());
    }
    connect(ui->lineEditName, &QLineEdit::textEdited, this, &TMainWindow::SaveMName);
    ui->plainTextEditDescription->blockSignals(false);

    ui->labelMUnits->setVisible(meash->GetType() == VarType::Measurement);
    ui->comboBoxMUnits->setVisible(meash->GetType() == VarType::Measurement);

    ui->comboBoxMUnits->blockSignals(true);
    ui->comboBoxMUnits->setCurrentIndex(
        ui->comboBoxMUnits->findData(static_cast<int>(meash->IsSpecialUnits() ? MUnits::Degrees : MUnits::Table)));
    ui->comboBoxMUnits->blockSignals(false);

    if (m_mType == MeasurementsType::Multisize)
    {
        ui->labelCalculated->setVisible(meash->GetType() == VarType::Measurement);
        ui->labelCalculatedValue->setVisible(meash->GetType() == VarType::Measurement);

        ui->labelBaseValue->setVisible(meash->GetType() == VarType::Measurement);
        ui->doubleSpinBoxBaseValue->setVisible(meash->GetType() == VarType::Measurement);

        ui->labelCorrection->setVisible(meash->GetType() == VarType::Measurement);
        ui->doubleSpinBoxCorrection->setVisible(meash->GetType() == VarType::Measurement);

        ui->labelShiftA->setVisible(meash->GetType() == VarType::Measurement);
        ui->doubleSpinBoxShiftA->setVisible(meash->GetType() == VarType::Measurement);

        const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values();

        if (dimensions.size() > 1)
        {
            ui->labelShiftB->setVisible(meash->GetType() == VarType::Measurement);
            ui->doubleSpinBoxShiftB->setVisible(meash->GetType() == VarType::Measurement);
        }

        if (dimensions.size() > 2)
        {
            ui->labelShiftC->setVisible(meash->GetType() == VarType::Measurement);
            ui->doubleSpinBoxShiftC->setVisible(meash->GetType() == VarType::Measurement);
        }

        ui->labelCalculatedValue->blockSignals(true);
        ui->doubleSpinBoxBaseValue->blockSignals(true);
        ui->doubleSpinBoxCorrection->blockSignals(true);
        ui->doubleSpinBoxShiftA->blockSignals(true);
        ui->doubleSpinBoxShiftB->blockSignals(true);
        ui->doubleSpinBoxShiftC->blockSignals(true);
        ui->lineEditAlias->blockSignals(true);

        ui->lineEditAlias->setText(meash->GetValueAlias(m_currentDimensionA, m_currentDimensionB, m_currentDimensionC));

        QString calculatedValue;

        if (meash->IsSpecialUnits())
        {
            const qreal value = *m_data->DataVariables()->value(meash->GetName())->GetValue();
            calculatedValue = VAbstractApplication::VApp()->LocaleToString(value) + QChar(QChar::Space) + degreeSymbol;
        }
        else
        {
            const QString postfix = UnitsToStr(m_pUnit); // Show unit in dialog lable (cm, mm or inch)
            const qreal value =
                UnitConvertor(*m_data->DataVariables()->value(meash->GetName())->GetValue(), m_mUnit, m_pUnit);
            calculatedValue = VAbstractApplication::VApp()->LocaleToString(value) + QChar(QChar::Space) + postfix;
        }

        ui->labelCalculatedValue->setText(calculatedValue);

        if (fresh)
        {
            ui->doubleSpinBoxBaseValue->setValue(meash->GetBase());
            ui->doubleSpinBoxCorrection->setValue(
                meash->GetCorrection(m_currentDimensionA, m_currentDimensionB, m_currentDimensionC));
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
        ui->lineEditAlias->blockSignals(false);
    }
    else
    {
        ui->labelCalculated->setVisible(meash->GetType() == VarType::Measurement);
        ui->labelCalculatedValue->setVisible(meash->GetType() == VarType::Measurement);

        ui->labelFormula->setVisible(meash->GetType() == VarType::Measurement);
        ui->plainTextEditFormula->setVisible(meash->GetType() == VarType::Measurement);
        ui->pushButtonGrow->setVisible(meash->GetType() == VarType::Measurement);
        ui->toolButtonExpr->setVisible(meash->GetType() == VarType::Measurement);

        ui->labelDimension->setVisible(meash->GetType() == VarType::Measurement);
        ui->comboBoxDimension->setVisible(meash->GetType() == VarType::Measurement);

        EvalFormula(meash->GetFormula(), false, meash->GetData(), ui->labelCalculatedValue, meash->IsSpecialUnits());

        ui->plainTextEditFormula->blockSignals(true);

        QString const formula = VTranslateVars::TryFormulaToUser(
            meash->GetFormula(), VAbstractApplication::VApp()->Settings()->GetOsSeparator());

        ui->plainTextEditFormula->setPlainText(formula);
        ui->plainTextEditFormula->blockSignals(false);

        ui->comboBoxDimension->blockSignals(true);
        ui->comboBoxDimension->setCurrentIndex(
            ui->comboBoxDimension->findData(static_cast<int>(meash->GetDimension())));
        ui->comboBoxDimension->blockSignals(false);

        ui->lineEditAlias->blockSignals(true);
        ui->lineEditAlias->setText(meash->GetValueAlias());
        ui->lineEditAlias->blockSignals(false);
    }

    MeasurementGUI();
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ShowMDiagram(const QSharedPointer<VMeasurement> &m)
{
    ui->labelDiagram->setResizedPixmap(QPixmap());
    ui->labelDiagram->setCursor(QCursor());

    disconnect(ui->labelDiagram, &VAspectRatioPixmapLabel::clicked, this, &TMainWindow::ShowImage);

    if (m.isNull())
    {
        ui->labelDiagram->setText(UnknownMeasurementImage());
        return;
    }

    if (VPatternImage const image = m->GetImage(); image.IsValid())
    {
        ui->labelDiagram->setCursor(Qt::PointingHandCursor);
        ui->labelDiagram->setResizedPixmap(image.GetPixmap());
        connect(ui->labelDiagram, &VAspectRatioPixmapLabel::clicked, this, &TMainWindow::ShowImage,
                Qt::UniqueConnection);
    }
    else
    {
        ui->labelDiagram->setText(UnknownMeasurementImage());
    }
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
        meash = m_data->GetVariable<VMeasurement>(nameField->data(Qt::UserRole).toString());
    }
    catch (const VExceptionBadId &e)
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

        if (not m_data->IsUnique(newName))
        {
            qint32 num = 2;
            QString name = newName;
            do
            {
                name = name + '_'_L1 + QString::number(num);
                num++;
            } while (not m_data->IsUnique(name));
            newName = name;
        }

        m_m->SetMName(nameField->text(), newName);
        MeasurementsWereSaved(false);
        RefreshData();
        m_search->RefreshList(ui->lineEditFind->text());

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
void TMainWindow::SaveMValueAlias(const QString &text)
{
    const int row = ui->tableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    if (const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
        m_mType == MeasurementsType::Multisize)
    {
        m_m->SetMValueAlias(nameField->data(Qt::UserRole).toString(),
                            m_currentDimensionA,
                            m_currentDimensionB,
                            m_currentDimensionC,
                            text);
    }
    else
    {
        m_m->SetMValueAlias(nameField->data(Qt::UserRole).toString(), text);
    }

    MeasurementsWereSaved(false);

    RefreshData();
    m_search->RefreshList(ui->lineEditFind->text());

    ui->tableWidget->blockSignals(true);
    ui->tableWidget->selectRow(row);
    ui->tableWidget->blockSignals(false);

    ShowNewMData(false);
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

    QString const text = ui->plainTextEditFormula->toPlainText();

    if (const QTableWidgetItem *formulaField = ui->tableWidget->item(row, ColumnFormula); formulaField->text() == text)
    {
        QTableWidgetItem *result = ui->tableWidget->item(row, ColumnCalcValue);
        const QString postfix = UnitsToStr(m_mUnit); // Show unit in dialog lable (cm, mm or inch)
        ui->labelCalculatedValue->setText(result->text() + QChar(QChar::Space) + postfix);
        return;
    }

    if (text.isEmpty())
    {
        const QString postfix = UnitsToStr(m_mUnit); // Show unit in dialog lable (cm, mm or inch)
        ui->labelCalculatedValue->setText(tr("Error") + " (" + postfix + "). " + tr("Empty field."));
        return;
    }

    QSharedPointer<VMeasurement> meash;
    try
    {
        // Translate to internal look.
        meash = m_data->GetVariable<VMeasurement>(nameField->data(Qt::UserRole).toString());
    }
    catch (const VExceptionBadId &e)
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
        const QString formula = VAbstractApplication::VApp()->TrVars()->FormulaFromUser(
            text, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
        m_m->SetMValue(nameField->data(Qt::UserRole).toString(), formula);
    }
    catch (qmu::QmuParserError &e) // Just in case something bad will happen
    {
        Q_UNUSED(e)
        return;
    }

    MeasurementsWereSaved(false);

    const QTextCursor cursor = ui->plainTextEditFormula->textCursor();

    RefreshData();
    m_search->RefreshList(ui->lineEditFind->text());

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
    m_m->SetMBaseValue(nameField->data(Qt::UserRole).toString(), value);

    MeasurementsWereSaved(false);

    RefreshData();
    m_search->RefreshList(ui->lineEditFind->text());

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
    m_m->SetMShiftA(nameField->data(Qt::UserRole).toString(), value);

    MeasurementsWereSaved(false);

    RefreshData();
    m_search->RefreshList(ui->lineEditFind->text());

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
    m_m->SetMShiftB(nameField->data(Qt::UserRole).toString(), value);

    MeasurementsWereSaved(false);

    RefreshData();
    m_search->RefreshList(ui->lineEditFind->text());

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
    m_m->SetMShiftC(nameField->data(Qt::UserRole).toString(), value);

    MeasurementsWereSaved(false);

    RefreshData();
    m_search->RefreshList(ui->lineEditFind->text());

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
    m_m->SetMCorrectionValue(nameField->data(Qt::UserRole).toString(), m_currentDimensionA, m_currentDimensionB,
                             m_currentDimensionC, value);

    MeasurementsWereSaved(false);

    RefreshData();
    m_search->RefreshList(ui->lineEditFind->text());

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
    m_m->SetMDescription(nameField->data(Qt::UserRole).toString(), ui->plainTextEditDescription->toPlainText());

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
        meash = m_data->GetVariable<VMeasurement>(nameField->data(Qt::UserRole).toString());
    }
    catch (const VExceptionBadId &e)
    {
        qCWarning(tMainWindow, "%s\n\n%s\n\n%s",
                  qUtf8Printable(tr("Can't find measurement '%1'.").arg(nameField->text())),
                  qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        return;
    }

    if (meash->IsCustom())
    {
        m_m->SetMFullName(nameField->data(Qt::UserRole).toString(), ui->lineEditFullName->text());

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
    const auto units = static_cast<MUnits>(ui->comboBoxMUnits->currentData().toInt());
    m_m->SetMSpecialUnits(nameField->data(Qt::UserRole).toString(), units == MUnits::Degrees);

    MeasurementsWereSaved(false);

    RefreshData();
    m_search->RefreshList(ui->lineEditFind->text());

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
    const auto dimension = static_cast<IMD>(ui->comboBoxDimension->currentData().toInt());
    m_m->SetMDimension(nameField->data(Qt::UserRole).toString(), dimension);

    MeasurementsWereSaved(false);

    RefreshData();
    m_search->RefreshList(ui->lineEditFind->text());

    ui->tableWidget->blockSignals(true);
    ui->tableWidget->selectRow(row);
    ui->tableWidget->blockSignals(false);

    ShowNewMData(false);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::FullCircumferenceChanged(bool checked)
{
    m_m->SetFullCircumference(checked);
    MeasurementsWereSaved(false);
    InitDimensionsBaseValue();
    InitDimensionControls();
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ExportToIndividual()
{
    QString dir;
    if (m_curFile.isEmpty())
    {
        dir = MApplication::VApp()->TapeSettings()->GetPathIndividualMeasurements();
    }
    else
    {
        dir = QFileInfo(m_curFile).absolutePath();
    }

    QString const filters = tr("Individual measurements") + QStringLiteral(" (*.vit)");
    QString const fName = tr("measurements.vit");
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export to individual"), dir + '/'_L1 + fName, filters,
                                                    nullptr, VAbstractApplication::VApp()->NativeFileDialog());

    if (fileName.isEmpty())
    {
        return;
    }

    auto const suffix = QStringLiteral("vit");
    if (QFileInfo const f(fileName); f.suffix().isEmpty() && f.suffix() != suffix)
    {
        fileName += '.'_L1 + suffix;
    }

    if (m_curFile.isEmpty())
    {
        MApplication::VApp()->TapeSettings()->SetPathIndividualMeasurements(QFileInfo(fileName).absolutePath());
    }

    QScopedPointer<VContainer> const tmpData(
        new VContainer(VAbstractApplication::VApp()->TrVars(), &m_mUnit, VContainer::UniqueNamespace()));

    VMeasurements individualMeasurements(m_mUnit, tmpData.data());

    const QMap<int, QSharedPointer<VMeasurement>> orderedTable = OrderedMeasurements();
    for (auto iMap = orderedTable.constBegin(); iMap != orderedTable.constEnd(); ++iMap)
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
    const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values();
    const QMap<QString, VDimensionRestriction> restrictions = m_m->GetRestrictions();
    bool const fullCircumference = m_m->IsFullCircumference();

    DialogRestrictDimension dialog(dimensions, restrictions, RestrictDimension::First, fullCircumference, this);
    if (dialog.exec() == QDialog::Rejected)
    {
        return;
    }

    m_m->SetRestrictions(dialog.Restrictions());
    MeasurementsWereSaved(false);

    if (not dimensions.isEmpty())
    {
        InitDimensionGradation(0, dimensions.at(0), m_gradationDimensionA);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::RestrictSecondDimesion()
{
    const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values();
    const QMap<QString, VDimensionRestriction> restrictions = m_m->GetRestrictions();
    bool const fullCircumference = m_m->IsFullCircumference();

    DialogRestrictDimension dialog(dimensions, restrictions, RestrictDimension::Second, fullCircumference, this);
    if (dialog.exec() == QDialog::Rejected)
    {
        return;
    }

    m_m->SetRestrictions(dialog.Restrictions());
    MeasurementsWereSaved(false);
    DimensionABaseChanged(); // trigger refresh
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::RestrictThirdDimesion()
{
    const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values();
    const QMap<QString, VDimensionRestriction> restrictions = m_m->GetRestrictions();
    bool const fullCircumference = m_m->IsFullCircumference();

    DialogRestrictDimension dialog(dimensions, restrictions, RestrictDimension::Third, fullCircumference, this);
    if (dialog.exec() == QDialog::Rejected)
    {
        return;
    }

    m_m->SetRestrictions(dialog.Restrictions());
    MeasurementsWereSaved(false);
    DimensionABaseChanged(); // trigger refresh
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::EditDimensionLabels()
{
    const QMap<MeasurementDimension, MeasurementDimension_p> dimensions = m_m->Dimensions();

    DialogDimensionLabels dialog(dimensions, m_m->IsFullCircumference(), this);
    if (dialog.exec() == QDialog::Rejected)
    {
        return;
    }

    m_m->SetDimensionLabels(dialog.Labels());

    MeasurementsWereSaved(false);

    InitDimensionsBaseValue();
    InitDimensionControls();
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::DimensionCustomNames()
{
    const QMap<MeasurementDimension, MeasurementDimension_p> dimensions = m_m->Dimensions();

    DialogDimensionCustomNames dialog(dimensions, this);
    if (dialog.exec() == QDialog::Rejected)
    {
        return;
    }

    m_m->SetDimensionCustomNames(dialog.CustomNames());

    MeasurementsWereSaved(false);
    InitDimensionControls();
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::AskDefaultSettings()
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
void TMainWindow::UpdateShortcuts()
{
    if (VAbstractShortcutManager *manager = VAbstractApplication::VApp()->GetShortcutManager())
    {
        manager->UpdateButtonShortcut(m_buttonShortcuts);
        manager->UpdateActionShortcuts(m_actionShortcuts);
        UpdateSearchControlsTooltips();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SetupMenu()
{
    // File
    connect(ui->actionNew, &QAction::triggered, this, &TMainWindow::FileNew);
    m_actionShortcuts.insert(VShortcutAction::New, ui->actionNew);

    connect(ui->actionOpenIndividual, &QAction::triggered, this, &TMainWindow::OpenIndividual);
    connect(ui->actionOpenMultisize, &QAction::triggered, this, &TMainWindow::OpenMultisize);
    connect(ui->actionOpenTemplate, &QAction::triggered, this, &TMainWindow::OpenTemplate);
    connect(ui->actionCreateFromExisting, &QAction::triggered, this, &TMainWindow::CreateFromExisting);

    connect(ui->actionSave, &QAction::triggered, this, &TMainWindow::FileSave);
    m_actionShortcuts.insert(VShortcutAction::Save, ui->actionSave);

    connect(ui->actionSaveAs, &QAction::triggered, this, &TMainWindow::FileSaveAs);
    m_actionShortcuts.insert(VShortcutAction::SaveAs, ui->actionSaveAs);

    connect(ui->actionExportToCSV, &QAction::triggered, this, &TMainWindow::ExportDataToCSV);
    connect(ui->actionImportFromCSV, &QAction::triggered, this, &TMainWindow::ImportDataFromCSV);
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

    connect(ui->actionQuit, &QAction::triggered, this, &TMainWindow::close);
    m_actionShortcuts.insert(VShortcutAction::Quit, ui->actionQuit);

    // Measurements
    connect(ui->actionAddCustom, &QAction::triggered, this, &TMainWindow::AddCustom);
    connect(ui->actionAddKnown, &QAction::triggered, this, &TMainWindow::AddKnown);
    connect(ui->actionAddSeparator, &QAction::triggered, this, &TMainWindow::AddSeparator);

    connect(ui->actionImportFromPattern, &QAction::triggered, this, &TMainWindow::ImportFromPattern);

    connect(ui->actionCreateKnownMeasurements, &QAction::triggered, this,
            []() { MApplication::VApp()->NewMainKMWindow(); });

    connect(ui->actionEditCurrentKnownMeasurements, &QAction::triggered, this,
            [this]()
            {
                QUuid const id = m_m->KnownMeasurements();
                if (id.isNull())
                {
                    ui->actionEditCurrentKnownMeasurements->setDisabled(true);
                    return;
                }

                VKnownMeasurementsDatabase *db = MApplication::VApp()->KnownMeasurementsDatabase();
                QHash<QUuid, VKnownMeasurementsHeader> const known = db->AllKnownMeasurements();
                if (!known.contains(id))
                {
                    qCritical() << tr("Unknown known measurements: %1").arg(id.toString());
                    ui->actionEditCurrentKnownMeasurements->setDisabled(true);
                    return;
                }

                MApplication::VApp()->MainKMWindow()->LoadFile(known.value(id).path);
            });

    // Window
    connect(ui->menuWindow, &QMenu::aboutToShow, this, &TMainWindow::AboutToShowWindowMenu);
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
void TMainWindow::InitWindow()
{
    SCASSERT(m_m != nullptr)
    ui->labelToolTip->setVisible(false);
    ui->tabWidget->setVisible(true);
    ui->tabWidget->setCurrentIndex(0); // measurements

    ui->plainTextEditNotes->setEnabled(true);
    ui->toolBarGradation->setVisible(true);

    if (m_mType == MeasurementsType::Multisize)
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

        connect(ui->doubleSpinBoxBaseValue, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                &TMainWindow::SaveMBaseValue);
        connect(ui->doubleSpinBoxCorrection, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                &TMainWindow::SaveMCorrectionValue);

        connect(ui->doubleSpinBoxShiftA, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                &TMainWindow::SaveMShiftA);
        connect(ui->doubleSpinBoxShiftB, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                &TMainWindow::SaveMShiftB);
        connect(ui->doubleSpinBoxShiftC, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                &TMainWindow::SaveMShiftC);

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

        ui->lineEditCustomerName->setText(m_m->Customer());

        ui->comboBoxGender->clear();
        InitGender(ui->comboBoxGender);
        const qint32 index = ui->comboBoxGender->findData(static_cast<int>(m_m->Gender()));
        ui->comboBoxGender->setCurrentIndex(index);

        {
            const auto dateLocale = QLocale(VAbstractApplication::VApp()->Settings()->GetLocale());
            ui->dateEditBirthDate->setLocale(dateLocale);
            ui->dateEditBirthDate->setDisplayFormat(dateLocale.dateFormat());
            ui->dateEditBirthDate->setDate(m_m->BirthDate());
        }

        ui->lineEditEmail->setText(m_m->Email());

        connect(ui->lineEditCustomerName, &QLineEdit::editingFinished, this, &TMainWindow::SaveCustomerName);
        connect(ui->lineEditEmail, &QLineEdit::editingFinished, this, &TMainWindow::SaveEmail);
        connect(ui->comboBoxGender, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                &TMainWindow::SaveGender);
        connect(ui->dateEditBirthDate, &QDateEdit::dateChanged, this, &TMainWindow::SaveBirthDate);
        connect(ui->pushButtonGrow, &QPushButton::clicked, this, &TMainWindow::DeployFormula);

        this->m_formulaBaseHeight = ui->plainTextEditFormula->height();
        connect(ui->plainTextEditFormula, &QPlainTextEdit::textChanged, this, &TMainWindow::SaveMValue,
                Qt::UniqueConnection);

        connect(ui->toolButtonExpr, &QToolButton::clicked, this, &TMainWindow::Fx);

        InitMeasurementDimension();
        connect(ui->comboBoxDimension, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                &TMainWindow::SaveMDimension);
    }

    ui->actionEditCurrentKnownMeasurements->setEnabled(!m_m->KnownMeasurements().isNull());
    ui->comboBoxKnownMeasurements->setEnabled(true);
    ui->comboBoxKnownMeasurements->clear();
    InitKnownMeasurements(ui->comboBoxKnownMeasurements);
    const qint32 index = ui->comboBoxKnownMeasurements->findData(m_m->KnownMeasurements());
    ui->comboBoxKnownMeasurements->setCurrentIndex(index);
    InitKnownMeasurementsDescription();
    connect(ui->comboBoxKnownMeasurements, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &TMainWindow::SaveKnownMeasurements);

    InitSearch();

    ui->plainTextEditNotes->setPlainText(m_m->Notes());
    connect(ui->plainTextEditNotes, &QPlainTextEdit::textChanged, this, &TMainWindow::SaveNotes);

    ui->actionAddCustom->setEnabled(true);
    ui->actionAddKnown->setEnabled(true);
    ui->actionAddSeparator->setEnabled(true);
    ui->actionImportFromPattern->setEnabled(true);
    ui->actionSaveAs->setEnabled(true);

    ui->lineEditName->setValidator(new QRegularExpressionValidator(QRegularExpression("^$|"_L1 + NameRegExp()), this));

    connect(ui->toolButtonRemove, &QToolButton::clicked, this, &TMainWindow::Remove);
    connect(ui->toolButtonTop, &QToolButton::clicked, this, &TMainWindow::MoveTop);
    connect(ui->toolButtonUp, &QToolButton::clicked, this, &TMainWindow::MoveUp);
    connect(ui->toolButtonDown, &QToolButton::clicked, this, &TMainWindow::MoveDown);
    connect(ui->toolButtonBottom, &QToolButton::clicked, this, &TMainWindow::MoveBottom);

    ui->toolButtonAddImage->setDisabled(true);
    ui->toolButtonRemoveImage->setDisabled(true);
    ui->toolButtonSaveImage->setDisabled(true);

    connect(ui->toolButtonAddImage, &QToolButton::clicked, this, &TMainWindow::AddImage);
    connect(ui->toolButtonRemoveImage, &QToolButton::clicked, this, &TMainWindow::RemoveImage);
    connect(ui->toolButtonSaveImage, &QToolButton::clicked, this, &TMainWindow::SaveImage);

    connect(ui->lineEditAlias, &QLineEdit::textEdited, this, &TMainWindow::SaveMValueAlias);
    connect(ui->lineEditName, &QLineEdit::textEdited, this, &TMainWindow::SaveMName);
    connect(ui->plainTextEditDescription, &QPlainTextEdit::textChanged, this, &TMainWindow::SaveMDescription);
    connect(ui->lineEditFullName, &QLineEdit::textEdited, this, &TMainWindow::SaveMFullName);

    connect(ui->pushButtonShowInExplorer, &QPushButton::clicked, this, [this]() { ShowInGraphicalShell(m_curFile); });

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
    if (m_mType == MeasurementsType::Multisize)
    {
        // Measurements
        ui->actionUseFullCircumference->setVisible(true);
        ui->actionUseFullCircumference->setEnabled(true);
        ui->actionUseFullCircumference->setChecked(m_m->IsFullCircumference());
        connect(ui->actionUseFullCircumference, &QAction::triggered, this, &TMainWindow::FullCircumferenceChanged);

        auto *separator = new QAction(this);
        separator->setSeparator(true);
        ui->menuMeasurements->insertAction(ui->actionUseFullCircumference, separator);

        if (const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values(); not dimensions.empty())
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

        ui->actionDimensionCustomNames->setVisible(true);
        ui->actionDimensionCustomNames->setEnabled(true);
        connect(ui->actionDimensionCustomNames, &QAction::triggered, this, &TMainWindow::DimensionCustomNames);

        // File
        ui->actionExportToIndividual->setVisible(true);
        ui->actionExportToIndividual->setEnabled(true);
        connect(ui->actionExportToIndividual, &QAction::triggered, this, &TMainWindow::ExportToIndividual);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::InitDimensionsBaseValue()
{
    const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values();
    const QString unit = UnitsToStr(m_m->Units(), true);
    const bool fc = m_m->IsFullCircumference();

    auto DimensionsBaseValue = [this, dimensions, unit, fc](int index, QLabel *name, QLabel *base)
    {
        SCASSERT(name != nullptr)
        SCASSERT(base != nullptr)

        if (dimensions.size() > index)
        {
            const MeasurementDimension_p &dimension = dimensions.at(index);
            name->setText(dimension->Name() + ':'_L1);
            name->setToolTip(VAbstartMeasurementDimension::DimensionToolTip(dimension, m_m->IsFullCircumference()));

            DimesionLabels const labels = dimension->Labels();

            if (VFuzzyContains(labels, dimension->BaseValue()) &&
                not VFuzzyValue(labels, dimension->BaseValue()).isEmpty())
            {
                base->setText(VFuzzyValue(labels, dimension->BaseValue()));
            }
            else
            {
                if (dimension->IsBodyMeasurement() || dimension->Type() == MeasurementDimension::X)
                {
                    if (dimension->Type() != MeasurementDimension::X && fc)
                    {
                        base->setText(QStringLiteral("%1 %2").arg(dimension->BaseValue() * 2).arg(unit));
                    }
                    else
                    {
                        base->setText(QStringLiteral("%1 %2").arg(dimension->BaseValue()).arg(unit));
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

    const bool fc = m_m->IsFullCircumference();
    const QString unit = UnitsToStr(m_m->Units(), true);

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
        InitDimensionXItems(bases, labels, control, unit);
    }
    else if (dimension->Type() == MeasurementDimension::Y || dimension->Type() == MeasurementDimension::W ||
             dimension->Type() == MeasurementDimension::Z)
    {
        InitDimensionYWZItems(bases, labels, control, unit, dimension->IsBodyMeasurement(), fc);
    }

    // Calculate the width of the largest item using QFontMetrics
    QFontMetrics const fontMetrics(control->font());
    int maxWidth = 0;
    for (int i = 0; i < control->count(); ++i)
    {
        int const itemWidth = fontMetrics.horizontalAdvance(control->itemText(i));
        if (itemWidth > maxWidth)
        {
            maxWidth = itemWidth;
        }
    }

    // Set the minimum width of the view to the largest item width
    control->view()->setMinimumWidth(maxWidth);

    // after initialization the current index is 0. The signal for changing the index will not be triggered if not make
    // it invalid first
    control->setCurrentIndex(-1);

    int const i = control->findData(current);
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
    const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values();

    auto InitControl = [this, dimensions](int index, QLabel *&name, QComboBox *&control)
    {
        if (dimensions.size() > index)
        {
            const MeasurementDimension_p &dimension = dimensions.at(index);

            if (name == nullptr)
            {
                name = new QLabel(dimension->Name() + ':'_L1);
            }
            else
            {
                name->setText(dimension->Name() + ':'_L1);
            }
            name->setToolTip(VAbstartMeasurementDimension::DimensionToolTip(dimension, m_m->IsFullCircumference()));

            if (control == nullptr)
            {
                control = new QComboBox;
                control->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            }

            InitDimensionGradation(index, dimension, control);
        }
    };

    InitControl(0, m_labelGradationDimensionA, m_gradationDimensionA);
    InitControl(1, m_labelGradationDimensionB, m_gradationDimensionB);
    InitControl(2, m_labelGradationDimensionC, m_gradationDimensionC);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::InitDimesionShifts()
{
    const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values();

    auto InitShift = [this, dimensions](int index, QLabel *name)
    {
        if (dimensions.size() > index)
        {
            const MeasurementDimension_p &dimension = dimensions.at(index);

            name->setText(tr("Shift (%1):").arg(dimension->Name()));
            name->setToolTip(VAbstartMeasurementDimension::DimensionToolTip(dimension, m_m->IsFullCircumference()));
        }
    };

    InitShift(0, ui->labelShiftA);
    InitShift(1, ui->labelShiftB);
    InitShift(2, ui->labelShiftC);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::InitTable()
{
    if (m_mType == MeasurementsType::Multisize)
    {
        ui->tableWidget->setColumnHidden(ColumnFormula, true); // formula

        RetranslateTableHeaders();
    }
    else
    {
        ui->tableWidget->setColumnHidden(ColumnBaseValue, true); // base value
        ui->tableWidget->setColumnHidden(ColumnShiftA, true);
        ui->tableWidget->setColumnHidden(ColumnShiftB, true);
        ui->tableWidget->setColumnHidden(ColumnShiftC, true);
        ui->tableWidget->setColumnHidden(ColumnCorrection, true);
    }

    connect(ui->tableWidget, &QTableWidget::itemSelectionChanged, this, &TMainWindow::ShowMData);

    ShowUnits();

    ui->tableWidget->resizeRowsToContents();
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ShowUnits()
{
    const QString unit = UnitsToStr(m_mUnit);

    ShowHeaderUnits(ui->tableWidget, ColumnCalcValue, UnitsToStr(m_pUnit)); // calculated value
    ShowHeaderUnits(ui->tableWidget, ColumnFormula, unit);                  // formula
    ShowHeaderUnits(ui->tableWidget, ColumnBaseValue, unit);                // base value
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
    if (const auto index = header.indexOf('('_L1); index != -1)
    {
        header.remove(index - 1, 100);
    }
    const auto unitHeader = QStringLiteral("%1 (%2)").arg(header, unit);
    table->horizontalHeaderItem(column)->setText(unitHeader);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::MeasurementsWereSaved(bool saved)
{
    setWindowModified(!saved);
    not m_mIsReadOnly ? ui->actionSave->setEnabled(!saved) : ui->actionSave->setEnabled(false);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SetCurrentFile(const QString &fileName)
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
auto TMainWindow::SaveMeasurements(const QString &fileName, QString &error) -> bool
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
auto TMainWindow::MaybeSave() -> bool
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
auto TMainWindow::AddCell(const QString &text, int row, int column, int aligment, bool ok) -> QTableWidgetItem *
{
    auto *item = new QTableWidgetItem(text);
    SetTextAlignment(item, static_cast<Qt::Alignment>(aligment));
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
auto TMainWindow::AddSeparatorCell(const QString &text, int row, int column, int aligment, bool ok)
    -> QTableWidgetItem *
{
    QTableWidgetItem *item = AddCell(text, row, column, aligment, ok);

    QFont itemFont = item->font();
    itemFont.setBold(true);
    itemFont.setItalic(true);
    item->setFont(itemFont);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::RefreshData(bool freshCall)
{
    if (m_m == nullptr)
    {
        return;
    }

    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    m_data->ClearUniqueNames();
    m_data->ClearVariables({VarType::Measurement, VarType::MeasurementSeparator});
    m_m->ReadMeasurements(m_currentDimensionA, m_currentDimensionB, m_currentDimensionC);
    RefreshTable(freshCall);

    QGuiApplication::restoreOverrideCursor();
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::RefreshTable(bool freshCall)
{
    ui->tableWidget->blockSignals(true);
    ui->tableWidget->setRowCount(0);

    ShowUnits();

    const QMap<int, QSharedPointer<VMeasurement>> orderedTable = OrderedMeasurements();
    qint32 currentRow = -1;
    ui->tableWidget->setRowCount(static_cast<int>(orderedTable.size()));
    for (auto iMap = orderedTable.constBegin(); iMap != orderedTable.constEnd(); ++iMap)
    {
        const QSharedPointer<VMeasurement> &meash = iMap.value();
        currentRow++;

        if (meash->GetType() == VarType::Measurement)
        {
            RefreshMeasurementData(meash, currentRow);
        }
        else if (meash->GetType() == VarType::MeasurementSeparator)
        {
            QTableWidgetItem *item = AddSeparatorCell(meash->GetName(), currentRow, ColumnName,
                                                      Qt::AlignVCenter); // name
            item->setData(Qt::UserRole, meash->GetName());
            AddCell(meash->GetDescription(), currentRow, ColumnFullName, Qt::AlignVCenter); // description

            if (m_mType == MeasurementsType::Individual)
            {
                ui->tableWidget->setSpan(currentRow, 1, 1, 3);
            }
            else if (m_mType == MeasurementsType::Multisize)
            {
                ui->tableWidget->setSpan(currentRow, 1, 1, 8);
            }
        }
    }

    Q_UNUSED(freshCall)
    //    if (freshCall)
    //    {
    //        ui->tableWidget->resizeColumnsToContents();
    //        ui->tableWidget->resizeRowsToContents();
    //    }
    //    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->blockSignals(false);

    ui->actionExportToCSV->setEnabled(ui->tableWidget->rowCount() > 0);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::RefreshMeasurementData(const QSharedPointer<VMeasurement> &meash, qint32 currentRow)
{
    VKnownMeasurementsDatabase const *db = MApplication::VApp()->KnownMeasurementsDatabase();
    VKnownMeasurements const knownDB = db->KnownMeasurements(meash->GetKnownMeasurementsId());
    VKnownMeasurement const known = knownDB.Measurement(meash->GetName());

    if (m_mType == MeasurementsType::Individual)
    {
        QTableWidgetItem *item = AddCell(meash->GetName(), currentRow, ColumnName, Qt::AlignVCenter); // name
        item->setData(Qt::UserRole, meash->GetName());

        if (meash->IsCustom())
        {
            AddCell(meash->GetGuiText(), currentRow, ColumnFullName, Qt::AlignVCenter);
        }
        else
        {
            AddCell(known.fullName, currentRow, ColumnFullName, Qt::AlignVCenter);
        }

        QString calculatedValue;
        if (meash->IsSpecialUnits())
        {
            calculatedValue = locale().toString(*meash->GetValue()) + degreeSymbol;
        }
        else
        {
            calculatedValue = locale().toString(UnitConvertor(*meash->GetValue(), m_mUnit, m_pUnit));
        }

        if (const QString valueAlias = meash->GetValueAlias(); !valueAlias.isEmpty())
        {
            calculatedValue = QStringLiteral("%1 (%2)").arg(calculatedValue, valueAlias);
        }

        AddCell(calculatedValue, currentRow, ColumnCalcValue, Qt::AlignHCenter | Qt::AlignVCenter,
                meash->IsFormulaOk()); // calculated value

        QString const formula = VTranslateVars::TryFormulaToUser(
            meash->GetFormula(), VAbstractApplication::VApp()->Settings()->GetOsSeparator());

        AddCell(formula, currentRow, ColumnFormula, Qt::AlignVCenter); // formula
    }
    else
    {
        QTableWidgetItem *item = AddCell(meash->GetName(), currentRow, 0, Qt::AlignVCenter); // name
        item->setData(Qt::UserRole, meash->GetName());

        if (meash->IsCustom())
        {
            AddCell(meash->GetGuiText(), currentRow, ColumnFullName, Qt::AlignVCenter);
        }
        else
        {
            AddCell(known.fullName, currentRow, ColumnFullName, Qt::AlignVCenter);
        }

        QString calculatedValue;
        if (meash->IsSpecialUnits())
        {
            const qreal value = *m_data->DataVariables()->value(meash->GetName())->GetValue();
            calculatedValue = locale().toString(value) + degreeSymbol;
        }
        else
        {
            const qreal value =
                UnitConvertor(*m_data->DataVariables()->value(meash->GetName())->GetValue(), m_mUnit, m_pUnit);
            calculatedValue = locale().toString(value);
        }

        if (const QString valueAlias = meash->GetValueAlias(m_currentDimensionA,
                                                            m_currentDimensionB,
                                                            m_currentDimensionC);
            !valueAlias.isEmpty())
        {
            calculatedValue = QStringLiteral("%1 (%2)").arg(calculatedValue, valueAlias);
        }

        AddCell(calculatedValue, currentRow, ColumnCalcValue, Qt::AlignHCenter | Qt::AlignVCenter,
                meash->IsFormulaOk()); // calculated value

        AddCell(locale().toString(meash->GetBase()), currentRow, ColumnBaseValue,
                Qt::AlignHCenter | Qt::AlignVCenter); // base value

        AddCell(locale().toString(meash->GetShiftA()), currentRow, ColumnShiftA, Qt::AlignHCenter | Qt::AlignVCenter);

        AddCell(locale().toString(meash->GetShiftB()), currentRow, ColumnShiftB, Qt::AlignHCenter | Qt::AlignVCenter);

        AddCell(locale().toString(meash->GetShiftC()), currentRow, ColumnShiftC, Qt::AlignHCenter | Qt::AlignVCenter);

        AddCell(locale().toString(meash->GetCorrection(m_currentDimensionA, m_currentDimensionB, m_currentDimensionC)),
                currentRow, ColumnCorrection, Qt::AlignHCenter | Qt::AlignVCenter);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto TMainWindow::GetCustomName() const -> QString
{
    qint32 num = 1;
    QString name;
    do
    {
        name = CustomMSign + VAbstractApplication::VApp()->TrVars()->InternalVarToUser(measurement_) +
               QString::number(num);
        num++;
    } while (not m_data->IsUnique(name));

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
void TMainWindow::MFields(bool enabled)
{
    ui->lineEditName->setEnabled(enabled);
    ui->plainTextEditDescription->setEnabled(enabled);
    ui->lineEditFullName->setEnabled(enabled);
    ui->comboBoxMUnits->setEnabled(enabled);
    ui->lineEditAlias->setEnabled(enabled);

    if (m_mType == MeasurementsType::Multisize)
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
        m_mType == MeasurementsType::Multisize ? showName += ".vst"_L1 : showName += ".vit"_L1;
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
void TMainWindow::SyncKnownMeasurements()
{
    if (m_m == nullptr)
    {
        return;
    }

    ui->comboBoxKnownMeasurements->blockSignals(true);
    ui->comboBoxKnownMeasurements->clear();
    InitKnownMeasurements(ui->comboBoxKnownMeasurements);
    const qint32 index = ui->comboBoxKnownMeasurements->findData(m_m->KnownMeasurements());
    ui->comboBoxKnownMeasurements->setCurrentIndex(index);
    ui->comboBoxKnownMeasurements->blockSignals(false);

    InitKnownMeasurementsDescription();

    const int row = ui->tableWidget->currentRow();

    RefreshData(false);

    if (row == -1)
    {
        return;
    }

    ui->tableWidget->blockSignals(true);
    ui->tableWidget->selectRow(row);
    ui->tableWidget->blockSignals(false);

    const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName); // name
    SCASSERT(nameField != nullptr)
    QSharedPointer<VMeasurement> meash;

    try
    {
        // Translate to internal look.
        meash = m_data->GetVariable<VMeasurement>(nameField->data(Qt::UserRole).toString());
    }
    catch (const VExceptionBadId &e)
    {
        Q_UNUSED(e)
        return;
    }

    if (meash->IsCustom())
    {
        return;
    }

    ShowMDiagram(meash);

    VKnownMeasurementsDatabase const *db = MApplication::VApp()->KnownMeasurementsDatabase();
    VKnownMeasurements const knownDB = db->KnownMeasurements(meash->GetKnownMeasurementsId());
    VKnownMeasurement const known = knownDB.Measurement(meash->GetName());

    ui->plainTextEditDescription->blockSignals(true);
    ui->plainTextEditDescription->setPlainText(known.description);
    ui->plainTextEditDescription->blockSignals(false);

    ui->lineEditFullName->blockSignals(true);
    ui->lineEditFullName->setText(known.fullName);
    ui->lineEditFullName->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
auto TMainWindow::IsUntitled() const -> bool
{
    return m_curFile.isEmpty();
}

//---------------------------------------------------------------------------------------------------------------------
auto TMainWindow::GetUntitledIndex() const -> int
{
    return untitledIndex;
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SetUntitledIndex(int newUntitledIndex)
{
    untitledIndex = newUntitledIndex;
}

//---------------------------------------------------------------------------------------------------------------------
auto TMainWindow::ClearCustomName(const QString &name) -> QString
{
    QString clear = name;
    if (const auto index = clear.indexOf(CustomMSign); index == 0)
    {
        clear.remove(0, 1);
    }
    return clear;
}

//---------------------------------------------------------------------------------------------------------------------
auto TMainWindow::EvalFormula(const QString &formula, bool fromUser, VContainer *data, QLabel *label, bool specialUnits)
    -> bool
{
    const QString postfix = specialUnits ? degreeSymbol : UnitsToStr(m_pUnit);

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
            f = VAbstractApplication::VApp()->TrVars()->FormulaFromUser(
                formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
        }
        else
        {
            f = formula;
        }
        QScopedPointer<Calculator> const cal(new Calculator());
        qreal result = cal->EvalFormula(data->DataVariables(), f);

        if (qIsInf(result) || qIsNaN(result))
        {
            label->setText(tr("Error") + " (" + postfix + ").");
            label->setToolTip(tr("Invalid result. Value is infinite or NaN. Please, check your calculations."));
            return false;
        }

        if (not specialUnits)
        {
            result = UnitConvertor(result, m_mUnit, m_pUnit);
        }

        label->setText(VAbstractApplication::VApp()->LocaleToString(result) + QChar(QChar::Space) + postfix);
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
auto TMainWindow::Open(const QString &pathTo, const QString &filter) -> QString
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
            MApplication::VApp()->NewMainTapeWindow()->LoadFile(mPath);
        }
    }

    return mPath;
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::UpdatePadlock(bool ro)
{
    ui->actionReadOnly->setChecked(ro);
    ui->actionReadOnly->setIcon(ro ? QIcon("://tapeicon/24x24/padlock_locked.png")
                                   : QIcon("://tapeicon/24x24/padlock_opened.png"));
    ui->actionReadOnly->setDisabled(m_mIsReadOnly);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::MeasurementGUI()
{
    if (const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName))
    {
        const bool isCustom = not(nameField->text().indexOf(CustomMSign) == 0);
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
    VTapeSettings *settings = MApplication::VApp()->TapeSettings();

    if (settings->status() == QSettings::NoError)
    {
        restoreGeometry(settings->GetGeometry());
        restoreState(settings->GetToolbarsState(), static_cast<int>(AppVersion()));

        // Text under tool buton icon
        ToolBarStyles();

        settings->RestoreMainWindowColumnWidths(ui->tableWidget);

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
void TMainWindow::WriteSettings()
{
    VTapeSettings *settings = MApplication::VApp()->TapeSettings();
    settings->SetGeometry(saveGeometry());
    settings->SetToolbarsState(saveState(static_cast<int>(AppVersion())));

    settings->SetTapeSearchOptionMatchCase(m_search->IsMatchCase());
    settings->SetTapeSearchOptionWholeWord(m_search->IsMatchWord());
    settings->SetTapeSearchOptionRegexp(m_search->IsMatchRegexp());
    settings->SetTapeSearchOptionUseUnicodeProperties(m_search->IsUseUnicodePreperties());

    settings->SaveMainWindowColumnWidths(ui->tableWidget);

    settings->sync();
    if (settings->status() == QSettings::AccessError)
    {
        qWarning() << tr("Cannot save settings. Access denied.");
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto TMainWindow::FilterMeasurements(const QStringList &mNew, const QStringList &mFilter) -> QStringList
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

    m_search->RefreshList(ui->lineEditFind->text());

    ui->tableWidget->selectRow(row);
}

//---------------------------------------------------------------------------------------------------------------------
auto TMainWindow::LoadFromExistingFile(const QString &path) -> bool
{
    if (m_m != nullptr)
    {
        return MApplication::VApp()->NewMainTapeWindow()->LoadFile(path);
    }

    if (not QFileInfo::exists(path))
    {
        qCCritical(tMainWindow, "%s", qUtf8Printable(tr("File '%1' doesn't exist!").arg(path)));
        if (MApplication::VApp()->IsTestMode())
        {
            QCoreApplication::exit(V_EX_NOINPUT);
        }
        return false;
    }

    // Check if file already opened
    const QList<TMainWindow *> list = MApplication::VApp()->MainTapeWindows();
    if (auto w = std::find_if(list.begin(), list.end(),
                              [path](const TMainWindow *window) { return window->CurrentFile() == path; });
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
        m_data = new VContainer(VAbstractApplication::VApp()->TrVars(), &m_mUnit, VContainer::UniqueNamespace());

        m_m = new VMeasurements(m_data);
        m_m->setXMLContent(path);

        m_mType = m_m->Type();

        if (m_mType == MeasurementsType::Unknown)
        {
            throw VException(tr("File has unknown format."));
        }

        QScopedPointer<VAbstractMConverter> const converter(
            (m_mType == MeasurementsType::Individual) ? static_cast<VAbstractMConverter *>(new VVITConverter(path))
                                                      : static_cast<VAbstractMConverter *>(new VVSTConverter(path)));

        m_curFileFormatVersion = converter->GetCurrentFormatVersion();
        m_curFileFormatVersionStr = converter->GetFormatVersionStr();
        m_m->setXMLContent(converter->Convert()); // Read again after conversion

        m_m->SetKnownMeasurements(MApplication::VApp()->TapeSettings()->GetKnownMeasurementsId());

        m_mUnit = m_m->Units();
        m_pUnit = m_mUnit;

        m_currentDimensionA = m_m->DimensionABase();
        m_currentDimensionB = m_m->DimensionBBase();
        m_currentDimensionC = m_m->DimensionCBase();

        ui->labelToolTip->setVisible(false);
        ui->tabWidget->setVisible(true);

        InitWindow();

        m_m->ClearForExport();
        const bool freshCall = true;
        RefreshData(freshCall);

        if (ui->tableWidget->rowCount() > 0)
        {
            ui->tableWidget->selectRow(0);
        }

        m_lock.reset(); // Now we can unlock the file

        m_mIsReadOnly = m_m->IsReadOnly();
        UpdatePadlock(m_mIsReadOnly);
        MeasurementGUI();
    }
    catch (VException &e)
    {
        qCCritical(tMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("File error.")), qUtf8Printable(e.ErrorMessage()),
                   qUtf8Printable(e.DetailedInformation()));
        ui->labelToolTip->setVisible(true);
        ui->tabWidget->setVisible(false);
        delete m_m;
        m_m = nullptr;
        delete m_data;
        m_data = nullptr;
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
void TMainWindow::CreateWindowMenu(QMenu *menu)
{
    SCASSERT(menu != nullptr)

    QAction *action = menu->addAction(tr("&New Window"));
    connect(action, &QAction::triggered, this, []() { MApplication::VApp()->NewMainTapeWindow()->activateWindow(); });
    action->setMenuRole(QAction::NoRole);

    menu->addAction(ui->actionAlwaysOnTop);
    connect(ui->actionAlwaysOnTop, &QAction::triggered, this, &TMainWindow::AlwaysOnTop);

    menu->addSeparator();

    const QList<TMainWindow *> windows = MApplication::VApp()->MainTapeWindows();
    for (int i = 0; i < windows.count(); ++i)
    {
        TMainWindow *window = windows.at(i);

        auto title = QStringLiteral("%1. %2").arg(i + 1).arg(window->windowTitle());
        if (const auto index = title.lastIndexOf("[*]"_L1); index != -1)
        {
            window->isWindowModified() ? title.replace(index, 3, '*'_L1) : title.replace(index, 3, QString());
        }

        QAction *action = menu->addAction(title, this, &TMainWindow::ShowWindow);
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
void TMainWindow::HackDimensionBaseValue()
{
    const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values();

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
    const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values();

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
auto TMainWindow::CheckMName(const QString &name, const QSet<QString> &importedNames) const -> QString
{
    if (name.isEmpty())
    {
        throw VException(tr("Measurement name is empty."));
    }

    if (importedNames.contains(name))
    {
        throw VException(tr("Imported file must not contain the same name twice."));
    }

    if (QRegularExpression const rx(NameRegExp()); not rx.match(name).hasMatch())
    {
        throw VException(tr("Measurement '%1' doesn't match regex pattern.").arg(name));
    }

    if (not m_data->IsUnique(name))
    {
        throw VException(tr("Measurement '%1' already used in the file.").arg(name));
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
    m_search->AddRow(currentRow);
    RefreshData();
    m_search->RefreshList(ui->lineEditFind->text());

    ui->tableWidget->selectRow(currentRow);
    ui->actionExportToCSV->setEnabled(true);
    MeasurementsWereSaved(false);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ImportIndividualMeasurements(const QxtCsvModel &csv, const QVector<int> &map, bool withHeader)
{
    const int columns = csv.columnCount();
    const int rows = csv.rowCount();

    if (columns < 2)
    {
        ShowError(tr("Individual measurements require at least 2 columns."));
        return;
    }

    QVector<IndividualMeasurement> measurements;
    QSet<QString> importedNames;

    for (int i = 0; i < rows; ++i)
    {
        try
        {
            const int nameColumn = map.at(static_cast<int>(IndividualMeasurementsColumns::Name));
            const QString name = csv.text(i, nameColumn).simplified();
            if (name.isEmpty())
            {
                ShowError(tr("Error in row %1. The measurement name is empty.").arg(i));
                continue;
            }

            IndividualMeasurement measurement;
            const QString mName = CheckMName(name, importedNames);
            importedNames.insert(mName);
            measurement.name = mName;

            const int valueColumn = map.at(static_cast<int>(IndividualMeasurementsColumns::Value));
            QString rawValue = csv.text(i, valueColumn);

            if (rawValue.endsWith(degreeSymbol))
            {
                measurement.specialUnits = true;
                RemoveLast(rawValue);
            }

            measurement.value = VTranslateVars::TryFormulaFromUser(
                rawValue, VAbstractApplication::VApp()->Settings()->GetOsSeparator());

            SetIndividualMeasurementFullName(i, name, csv, map, measurement);
            SetIndividualMeasurementDescription(i, name, csv, map, measurement);

            measurements.append(measurement);
        }
        catch (VException &e)
        {
            int rowIndex = i + 1;
            if (withHeader)
            {
                ++rowIndex;
            }
            ShowError(tr("Error in row %1. %2").arg(rowIndex).arg(e.ErrorMessage()));
            return;
        }
    }

    for (const auto &im : qAsConst(measurements))
    {
        m_m->AddEmpty(im.name, im.value);

        if (not im.fullName.isEmpty())
        {
            m_m->SetMFullName(im.name, im.fullName);
        }

        if (im.specialUnits)
        {
            m_m->SetMSpecialUnits(im.name, im.specialUnits);
        }

        if (not im.description.isEmpty())
        {
            m_m->SetMDescription(im.name, im.description);
        }
    }

    RefreshDataAfterImport();
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ImportMultisizeMeasurements(const QxtCsvModel &csv, const QVector<int> &map, bool withHeader)
{
    if (const int columns = csv.columnCount(); columns < 4)
    {
        ShowError(tr("Multisize measurements require at least 4 columns."));
        return;
    }

    QVector<MultisizeMeasurement> measurements;
    QSet<QString> importedNames;

    const QMap<MeasurementDimension, MeasurementDimension_p> dimensions = m_m->Dimensions();
    const int rows = csv.rowCount();

    for (int i = 0; i < rows; ++i)
    {
        try
        {
            measurements.append(ImportMultisizeMeasurement(csv, i, map, dimensions.size(), importedNames));
        }
        catch (VException &e)
        {
            int rowIndex = i + 1;
            if (withHeader)
            {
                ++rowIndex;
            }
            ShowError(tr("Error in row %1. %2").arg(rowIndex).arg(e.ErrorMessage()));
            return;
        }
    }

    for (const auto &mm : qAsConst(measurements))
    {
        m_m->AddEmpty(mm.name);
        m_m->SetMBaseValue(mm.name, mm.base);
        m_m->SetMShiftA(mm.name, mm.shiftA);

        if (dimensions.size() > 1)
        {
            m_m->SetMShiftB(mm.name, mm.shiftB);
        }

        if (dimensions.size() > 2)
        {
            m_m->SetMShiftC(mm.name, mm.shiftC);
        }

        if (not mm.fullName.isEmpty())
        {
            m_m->SetMFullName(mm.name, mm.fullName);
        }

        if (mm.specialUnits)
        {
            m_m->SetMSpecialUnits(mm.name, mm.specialUnits);
        }

        if (not mm.description.isEmpty())
        {
            m_m->SetMDescription(mm.name, mm.description);
        }
    }

    RefreshDataAfterImport();
}

//---------------------------------------------------------------------------------------------------------------------
auto TMainWindow::ImportMultisizeMeasurement(const QxtCsvModel &csv, int i, const QVector<int> &map,
                                             vsizetype dimensionsCount, QSet<QString> &importedNames)
    -> TMainWindow::MultisizeMeasurement
{
    const auto nameColumn = map.at(static_cast<int>(MultisizeMeasurementsColumns::Name));
    const QString name = csv.text(i, nameColumn).simplified();
    if (name.isEmpty())
    {
        throw VException(tr("The measurement name is empty."));
    }

    MultisizeMeasurement measurement;

    const QString mName = CheckMName(name, importedNames);
    importedNames.insert(mName);
    measurement.name = mName;

    const auto baseValueColumn = map.at(static_cast<int>(MultisizeMeasurementsColumns::BaseValue));
    QString rawBaseValue = csv.text(i, baseValueColumn);

    if (rawBaseValue.endsWith(degreeSymbol))
    {
        measurement.specialUnits = true;
        RemoveLast(rawBaseValue);
    }

    measurement.base = ConverToDouble(rawBaseValue, tr("Cannot convert base value to double in column 2."));

    const auto shiftAColumn = map.at(static_cast<int>(MultisizeMeasurementsColumns::ShiftA));
    measurement.shiftA = ConverToDouble(csv.text(i, shiftAColumn),
                                        tr("Cannot convert shift value to double in column %1.").arg(shiftAColumn));

    if (dimensionsCount > 1)
    {
        const auto shiftBColumn = map.at(static_cast<int>(MultisizeMeasurementsColumns::ShiftB));
        measurement.shiftB = ConverToDouble(csv.text(i, shiftBColumn),
                                            tr("Cannot convert shift value to double in column %1.").arg(shiftBColumn));
    }

    if (dimensionsCount > 2)
    {
        const auto shiftCColumn = map.at(static_cast<int>(MultisizeMeasurementsColumns::ShiftC));
        measurement.shiftC = ConverToDouble(csv.text(i, shiftCColumn),
                                            tr("Cannot convert shift value to double in column %1.").arg(shiftCColumn));
    }

    const int columns = csv.columnCount();
    const bool custom = name.startsWith(CustomMSign);
    if (columns > 4 && custom)
    {
        const auto fullNameColumn = map.at(static_cast<int>(MultisizeMeasurementsColumns::FullName));
        if (fullNameColumn >= 0)
        {
            measurement.fullName = csv.text(i, fullNameColumn).simplified();
        }
    }

    if (columns > 5 && custom)
    {
        const auto descriptionColumn = map.at(static_cast<int>(MultisizeMeasurementsColumns::Description));
        if (descriptionColumn >= 0)
        {
            measurement.description = csv.text(i, descriptionColumn).simplified();
        }
    }

    return measurement;
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SetCurrentPatternUnit()
{
    if (m_comboBoxUnits)
    {
        m_comboBoxUnits->blockSignals(true);
        if (const qint32 indexUnit = m_comboBoxUnits->findData(static_cast<int>(m_pUnit)); indexUnit != -1)
        {
            m_comboBoxUnits->setCurrentIndex(indexUnit);
        }
        m_comboBoxUnits->blockSignals(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ShowDimensionControls()
{
    const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values();

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

    ShowControl(0, m_labelGradationDimensionA, m_gradationDimensionA);
    ShowControl(1, m_labelGradationDimensionB, m_gradationDimensionB);
    ShowControl(2, m_labelGradationDimensionC, m_gradationDimensionC);

    if (m_gradationDimensionA)
    {
        connect(m_gradationDimensionA, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                &TMainWindow::DimensionABaseChanged);
    }

    if (m_gradationDimensionB)
    {
        connect(m_gradationDimensionB, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                &TMainWindow::DimensionBBaseChanged);
    }

    if (m_gradationDimensionC)
    {
        connect(m_gradationDimensionC, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                &TMainWindow::DimensionCBaseChanged);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SetDimensionBases()
{
    const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values();

    if (not dimensions.empty())
    {
        const MeasurementDimension_p &dimension = dimensions.at(0);
        m_currentDimensionA = dimension->BaseValue();
    }

    if (dimensions.size() > 1)
    {
        const MeasurementDimension_p &dimension = dimensions.at(1);
        m_currentDimensionB = dimension->BaseValue();
    }

    if (dimensions.size() > 2)
    {
        const MeasurementDimension_p &dimension = dimensions.at(2);
        m_currentDimensionC = dimension->BaseValue();
    }

    auto SetBase = [dimensions](int index, QComboBox *control, qreal &value)
    {
        if (dimensions.size() > index)
        {
            SCASSERT(control != nullptr)

            if (const qint32 i = control->findData(value); i != -1)
            {
                control->setCurrentIndex(i);
            }
            else
            {
                value = control->currentData().toDouble();
            }
        }
    };

    SetBase(0, m_gradationDimensionA, m_currentDimensionA);
    SetBase(1, m_gradationDimensionB, m_currentDimensionB);
    SetBase(2, m_gradationDimensionC, m_currentDimensionC);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SetCurrentDimensionValues()
{
    const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values();

    auto SetDimensionValue = [dimensions](int index, qreal &value)
    {
        if (dimensions.size() > index)
        {
            const MeasurementDimension_p &dimension = dimensions.at(index);
            value = dimension->BaseValue();
        }
    };

    SetDimensionValue(0, m_currentDimensionA);
    SetDimensionValue(1, m_currentDimensionB);
    SetDimensionValue(2, m_currentDimensionC);
}

//---------------------------------------------------------------------------------------------------------------------
auto TMainWindow::DimensionRestrictedValues(int index, const MeasurementDimension_p &dimension) -> QVector<qreal>
{
    VDimensionRestriction restriction;
    if (index == 0)
    {
        restriction = m_m->Restriction(0);
    }
    else if (index == 1)
    {
        restriction = m_m->Restriction(m_currentDimensionA);
    }
    else
    {
        restriction = m_m->Restriction(m_currentDimensionA, m_currentDimensionB);
    }

    const QVector<qreal> bases = dimension->ValidBases();

    qreal min = VFuzzyIndexOf(bases, restriction.GetMin()) != -1 ? restriction.GetMin() : dimension->MinValue();
    qreal max = VFuzzyIndexOf(bases, restriction.GetMax()) != -1 ? restriction.GetMax() : dimension->MaxValue();

    if (min > max)
    {
        min = dimension->MinValue();
        max = dimension->MaxValue();
    }

    return VAbstartMeasurementDimension::ValidBases(min, max, dimension->Step(), restriction.GetExcludeValues());
}

//---------------------------------------------------------------------------------------------------------------------
auto TMainWindow::OrderedMeasurements() const -> QMap<int, QSharedPointer<VMeasurement>>
{
    const QMap<QString, QSharedPointer<VMeasurement>> table = m_data->DataMeasurementsWithSeparators();
    QMap<int, QSharedPointer<VMeasurement>> orderedTable;
    for (auto iterMap = table.constBegin(); iterMap != table.constEnd(); ++iterMap)
    {
        const QSharedPointer<VMeasurement> &meash = iterMap.value();
        orderedTable.insert(meash->Index(), meash);
    }

    return orderedTable;
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::InitIcons()
{
    auto const iconResource = QStringLiteral("icon");
    ui->toolButtonExpr->setIcon(VTheme::GetIconResource(iconResource, QStringLiteral("24x24/fx.png")));
    ui->toolButtonAddImage->setIcon(VTheme::GetIconResource(iconResource, QStringLiteral("16x16/insert-image.png")));
    ui->toolButtonRemoveImage->setIcon(VTheme::GetIconResource(iconResource, QStringLiteral("16x16/remove-image.png")));

    auto const tapeIconResource = QStringLiteral("tapeicon");
    ui->actionMeasurementDiagram->setIcon(
        VTheme::GetIconResource(tapeIconResource, QStringLiteral("24x24/mannequin.png")));
}

//---------------------------------------------------------------------------------------------------------------------
auto TMainWindow::UnknownMeasurementImage() -> QString
{
    return u"<html><head/><body><p><span style=\" font-size:340pt;\">?</span></p>"
           u"<p align=\"center\">%1</p></body></html>"_s.arg(tr("Unknown measurement"));
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::RetranslateMDiagram()
{
    if (ui->tableWidget->rowCount() <= 0 || ui->tableWidget->currentRow() == -1)
    {
        return;
    }

    const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName); // name
    SCASSERT(nameField != nullptr)
    QSharedPointer<VMeasurement> meash;

    try
    {
        // Translate to internal look.
        meash = m_data->GetVariable<VMeasurement>(nameField->data(Qt::UserRole).toString());
    }
    catch (const VExceptionBadId &e)
    {
        Q_UNUSED(e)
        return;
    }

    ShowMDiagram(meash);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::InitKnownMeasurements(QComboBox *combo)
{
    VKnownMeasurementsDatabase *db = MApplication::VApp()->KnownMeasurementsDatabase();
    QHash<QUuid, VKnownMeasurementsHeader> const known = db->AllKnownMeasurements();

    SCASSERT(combo != nullptr)
    combo->addItem(tr("None"), QUuid());

    if (m_m == nullptr)
    {
        return;
    }

    if (QUuid const kmId = m_m->KnownMeasurements(); !kmId.isNull() && !known.contains(kmId))
    {
        combo->addItem(tr("Invalid link"), kmId);
    }

    const QString rootPath = MApplication::VApp()->TapeSettings()->GetPathKnownMeasurements();

    for (auto i = known.constBegin(); i != known.constEnd(); ++i)
    {
        QString name = i.value().name;

        if (name.isEmpty())
        {
            QString fullPath = i.value().path;
            if (fullPath.startsWith(rootPath))
            {
                fullPath.remove(0, rootPath.length() + 1); // Remove root folder path
            }
            name = fullPath;
        }

        combo->addItem(name, i.key());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::InitKnownMeasurementsDescription()
{
    VKnownMeasurementsDatabase *db = MApplication::VApp()->KnownMeasurementsDatabase();
    QHash<QUuid, VKnownMeasurementsHeader> const known = db->AllKnownMeasurements();

    ui->plainTextEditKnownMeasurementsDescription->clear();
    QUuid const id = m_m->KnownMeasurements();
    if (!id.isNull() && known.contains(id))
    {
        ui->plainTextEditKnownMeasurementsDescription->setPlainText(known.value(id).description);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto TMainWindow::KnownMeasurementsRegistred(const QUuid &id) -> bool
{
    if (id.isNull())
    {
        return false;
    }

    VKnownMeasurementsDatabase *db = MApplication::VApp()->KnownMeasurementsDatabase();
    QHash<QUuid, VKnownMeasurementsHeader> const known = db->AllKnownMeasurements();
    return known.contains(id);
}

//---------------------------------------------------------------------------------------------------------------------
auto TMainWindow::CSVColumnHeader(int column) const -> QString
{
    if (m_mType == MeasurementsType::Individual)
    {
        switch (column)
        {
            case 0: // name
                return tr("Name", "measurement column");
            case 1: // full name
                return tr("Full name", "measurement column");
            case 2: // calculated value
                return tr("Calculated value", "measurement column") + " ("_L1 + UnitsToStr(m_pUnit) + ')'_L1;
            case 3: // formula
                return tr("Formula", "measurement column");
            case 4: // description
                return tr("Description", "measurement column");
            default:
                return {};
        }
    }

    if (column == 0)
    {
        return tr("Name", "measurement column");
    }

    if (column == 1)
    {
        return tr("Full name", "measurement column");
    }

    if (column == 2)
    {
        return tr("Calculated value", "measurement column") + " ("_L1 + UnitsToStr(m_pUnit) + ')'_L1;
    }

    if (column == 3)
    {
        return tr("Base value", "measurement column");
    }

    const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values();
    const auto suffix = QStringLiteral(" (%1):");

    if (column == 4)
    {
        if (not dimensions.empty())
        {
            const MeasurementDimension_p &dimension = dimensions.at(0);
            return tr("Shift", "measurement column") + suffix.arg(dimension->Name());
        }

        return QStringLiteral("Shift A");
    }

    if (column == 5 && dimensions.size() > 1)
    {
        const MeasurementDimension_p &dimension = dimensions.at(1);
        return tr("Shift", "measurement column") + suffix.arg(dimension->Name());
    }

    if (column == 6 && dimensions.size() > 2)
    {
        const MeasurementDimension_p &dimension = dimensions.at(2);
        return tr("Shift", "measurement column") + suffix.arg(dimension->Name());
    }

    if ((column == 5 && dimensions.size() <= 1) || (column == 6 && dimensions.size() <= 2) || column == 7)
    {
        return tr("Description", "measurement column");
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ExportRowToCSV(QxtCsvModel &csv, int row, const QSharedPointer<VMeasurement> &meash,
                                 const VKnownMeasurements &knownDB) const
{
    csv.insertSingleRow(row);

    VKnownMeasurement const known = knownDB.Measurement(meash->GetName());

    csv.setText(row, 0, meash->GetName());

    if (meash->IsCustom())
    {
        csv.setText(row, 1, meash->GetGuiText());
    }
    else
    {
        csv.setText(row, 1, known.fullName);
    }

    QString calculatedValue;
    if (meash->IsSpecialUnits())
    {
        calculatedValue = locale().toString(*meash->GetValue()) + degreeSymbol;
    }
    else
    {
        calculatedValue = locale().toString(UnitConvertor(*meash->GetValue(), m_mUnit, m_pUnit));
    }
    csv.setText(row, 2, calculatedValue);

    if (m_mType == MeasurementsType::Individual)
    {
        QString const formula = VTranslateVars::TryFormulaToUser(
            meash->GetFormula(), VAbstractApplication::VApp()->Settings()->GetOsSeparator());
        csv.setText(row, 3, formula);

        if (meash->IsCustom())
        {
            csv.setText(row, 4, meash->GetDescription());
        }
        else
        {
            csv.setText(row, 4, known.description);
        }
    }
    else
    {
        csv.setText(row, 3, locale().toString(meash->GetBase()));

        const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values();

        csv.setText(row, 4, locale().toString(meash->GetShiftA()));

        int column = 5;
        if (dimensions.size() > 1)
        {
            csv.setText(row, column, locale().toString(meash->GetShiftB()));
            ++column;
        }

        if (dimensions.size() > 2)
        {
            csv.setText(row, column, locale().toString(meash->GetShiftC()));
            ++column;
        }

        if (meash->IsCustom())
        {
            csv.setText(row, column, meash->GetDescription());
        }
        else
        {
            csv.setText(row, column, known.description);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SetDecimals()
{
    switch (m_mUnit)
    {
        case Unit::Cm:
            ui->doubleSpinBoxBaseValue->setDecimals(2);
            ui->doubleSpinBoxBaseValue->setSingleStep(0.1);

            ui->doubleSpinBoxCorrection->setDecimals(2);
            ui->doubleSpinBoxCorrection->setSingleStep(0.1);

            ui->doubleSpinBoxShiftA->setDecimals(2);
            ui->doubleSpinBoxShiftA->setSingleStep(0.1);

            ui->doubleSpinBoxShiftB->setDecimals(2);
            ui->doubleSpinBoxShiftB->setSingleStep(0.1);

            ui->doubleSpinBoxShiftC->setDecimals(2);
            ui->doubleSpinBoxShiftC->setSingleStep(0.1);
            break;
        case Unit::Mm:
            ui->doubleSpinBoxBaseValue->setDecimals(1);
            ui->doubleSpinBoxBaseValue->setSingleStep(1);

            ui->doubleSpinBoxCorrection->setDecimals(1);
            ui->doubleSpinBoxCorrection->setSingleStep(1);

            ui->doubleSpinBoxShiftA->setDecimals(1);
            ui->doubleSpinBoxShiftA->setSingleStep(1);

            ui->doubleSpinBoxShiftB->setDecimals(1);
            ui->doubleSpinBoxShiftB->setSingleStep(1);

            ui->doubleSpinBoxShiftC->setDecimals(1);
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
    m_labelPatternUnit = new QLabel(tr("Pattern unit:"));
    ui->toolBarGradation->addWidget(m_labelPatternUnit);

    m_comboBoxUnits = new QComboBox(this);
    InitComboBoxUnits();
    SetCurrentPatternUnit();

    connect(m_comboBoxUnits, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index)
            {
                m_pUnit = static_cast<Unit>(m_comboBoxUnits->itemData(index).toInt());

                UpdatePatternUnit();
            });

    ui->toolBarGradation->addWidget(m_comboBoxUnits);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::InitComboBoxUnits()
{
    SCASSERT(m_comboBoxUnits != nullptr)
    m_comboBoxUnits->addItem(UnitsToStr(Unit::Cm, true), QVariant(static_cast<int>(Unit::Cm)));
    m_comboBoxUnits->addItem(UnitsToStr(Unit::Mm, true), QVariant(static_cast<int>(Unit::Mm)));
    m_comboBoxUnits->addItem(UnitsToStr(Unit::Inch, true), QVariant(static_cast<int>(Unit::Inch)));
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::InitMeasurementUnits()
{
    if (not m_m)
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
    switch (m_m->Units())
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
            units = QStringLiteral("<Invalid>");
            break;
    }

    ui->comboBoxMUnits->clear();
    ui->comboBoxMUnits->addItem(units, QVariant(static_cast<int>(MUnits::Table)));
    ui->comboBoxMUnits->addItem(tr("Degrees"), QVariant(static_cast<int>(MUnits::Degrees)));

    if (int const i = ui->comboBoxMUnits->findData(current); i != -1)
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

    if (int const i = ui->comboBoxDimension->findData(current); i != -1)
    {
        ui->comboBoxDimension->setCurrentIndex(i);
    }

    ui->comboBoxDimension->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::InitSearch()
{
    VTapeSettings *settings = MApplication::VApp()->TapeSettings();
    m_search->SetUseUnicodePreperties(settings->GetTapeSearchOptionUseUnicodeProperties());
    m_search->SetMatchWord(settings->GetTapeSearchOptionWholeWord());
    m_search->SetMatchRegexp(settings->GetTapeSearchOptionRegexp());
    m_search->SetMatchCase(settings->GetTapeSearchOptionMatchCase());

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
                    ui->toolButtonWholeWord->blockSignals(true);
                    ui->toolButtonWholeWord->setChecked(false);
                    ui->toolButtonWholeWord->blockSignals(false);
                    ui->toolButtonWholeWord->setEnabled(false);

                    ui->toolButtonUseUnicodeProperties->setEnabled(true);
                }
                else
                {
                    ui->toolButtonWholeWord->setEnabled(true);
                    ui->toolButtonUseUnicodeProperties->blockSignals(true);
                    ui->toolButtonUseUnicodeProperties->setChecked(false);
                    ui->toolButtonUseUnicodeProperties->blockSignals(false);
                    ui->toolButtonUseUnicodeProperties->setEnabled(false);
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
void TMainWindow::InitSearchHistory()
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
void TMainWindow::SaveSearchRequest()
{
    QStringList searchHistory = MApplication::VApp()->TapeSettings()->GetTapeSearchHistory();
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
    MApplication::VApp()->TapeSettings()->SetTapeSearchHistory(searchHistory);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::UpdateSearchControlsTooltips()
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
void TMainWindow::RetranslateTableHeaders()
{
    if (m_mType == MeasurementsType::Multisize)
    {
        const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values();

        if (not dimensions.isEmpty())
        {
            const MeasurementDimension_p &dimension = dimensions.at(0);
            ui->tableWidget->horizontalHeaderItem(ColumnShiftA)->setText(tr("%1 shift").arg(dimension->Name()));
        }

        if (dimensions.size() < 2)
        {
            ui->tableWidget->setColumnHidden(ColumnShiftB, true);
        }
        else
        {
            const MeasurementDimension_p &dimension = dimensions.at(1);
            ui->tableWidget->horizontalHeaderItem(ColumnShiftB)->setText(tr("%1 shift").arg(dimension->Name()));
        }

        if (dimensions.size() < 3)
        {
            ui->tableWidget->setColumnHidden(ColumnShiftC, true);
        }
        else
        {
            const MeasurementDimension_p &dimension = dimensions.at(2);
            ui->tableWidget->horizontalHeaderItem(ColumnShiftC)->setText(tr("%1 shift").arg(dimension->Name()));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class T> void TMainWindow::HackWidget(T **widget)
{
    delete *widget;
    *widget = new T();
    m_hackedWidgets.append(*widget);
}
