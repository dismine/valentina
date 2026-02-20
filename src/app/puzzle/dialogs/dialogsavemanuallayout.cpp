/************************************************************************
 **
 **  @file   dialogsavemanuallayout.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 9, 2021
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2021 Valentina project
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

#include "dialogsavemanuallayout.h"
#include "../vlayout/vlayoutexporter.h"
#include "../vmisc/exception/vexception.h"
#include "../vpapplication.h"
#include "ui_dialogsavemanuallayout.h"

#include <QMessageBox>
#include <QShowEvent>
#include <QtDebug>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

#ifndef Q_OS_WIN
Q_GLOBAL_STATIC_WITH_ARGS(const QString, baseFilenameRegExp, ("^[^\\/]+$"_L1)) // NOLINT
#else
Q_GLOBAL_STATIC_WITH_ARGS(const QString, baseFilenameRegExp, ("^[^\\:?\"*|\\/<>]+$"_L1)) // NOLINT
#endif

QT_WARNING_POP
} // namespace

//---------------------------------------------------------------------------------------------------------------------
DialogSaveManualLayout::DialogSaveManualLayout(vsizetype count, bool consoleExport, const QString &fileName,
                                               QWidget *parent)
  : VAbstractLayoutDialog(parent),
    ui(std::make_unique<Ui::DialogSaveManualLayout>()),
    m_count(count),
    m_consoleExport(consoleExport)
{
    ui->setupUi(this);

#if defined(Q_OS_MAC)
    setWindowFlags(Qt::Window);
#endif

    VPSettings *settings = VPApplication::VApp()->PuzzleSettings();
    settings->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    if (settings->GetSingleLineFonts() || settings->GetSingleStrokeOutlineFont())
    {
        ui->checkBoxTextAsPaths->setChecked(true);
        ui->checkBoxTextAsPaths->setDisabled(true);
    }

    QPushButton *bOk = ui->buttonBox->button(QDialogButtonBox::Ok);
    SCASSERT(bOk != nullptr)
    bOk->setEnabled(false);

    ui->lineEditFileName->setValidator(new QRegularExpressionValidator(QRegularExpression(*baseFilenameRegExp), this));

    const QString mask = m_count > 1 ? fileName + '_'_L1 : fileName;
    if (not m_consoleExport)
    {
        ui->lineEditFileName->setText(mask);
    }
    else
    {
        if (QRegularExpression(*baseFilenameRegExp).match(mask).hasMatch())
        {
            ui->lineEditFileName->setText(mask);
        }
        else
        {
            throw VException(tr("The base filename does not match a regular expression."));
        }
    }

    InitFileFormats();
    connect(ui->comboBoxFormatType,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DialogSaveManualLayout::ShowExample);
    connect(ui->comboBoxFormat,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            [this](int index)
            {
                if (index == -1)
                {
                    ui->comboBoxFormatType->clear();
                    return;
                }

                {
                    const QSignalBlocker blocker(ui->comboBoxFormatType);
                    InitFileFormatTypes(static_cast<LayoutExportFileFormat>(ui->comboBoxFormat->currentData().toInt()));
                    ui->comboBoxFormatType->setCurrentIndex(-1);
                }

                ui->comboBoxFormatType->setCurrentIndex(0);
            });

    InitDxfCompatibility();

    connect(bOk, &QPushButton::clicked, this, &DialogSaveManualLayout::Save);
    connect(ui->lineEditFileName, &QLineEdit::textChanged, this, &DialogSaveManualLayout::ShowExample);
    connect(ui->comboBoxFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogSaveManualLayout::ShowExample);

    connect(ui->pushButtonBrowse,
            &QPushButton::clicked,
            this,
            [this]()
            {
                const QString dirPath = VPApplication::VApp()->PuzzleSettings()->GetPathLayoutExport();

                const QString dir = QFileDialog::getExistingDirectory(
                    this, tr("Select folder"), dirPath,
                    VAbstractApplication::VApp()->NativeFileDialog(QFileDialog::ShowDirsOnly |
                                                                   QFileDialog::DontResolveSymlinks));
                if (not dir.isEmpty())
                { // If paths equal the signal will not be called, we will do this manually
                    dir == ui->lineEditPath->text() ? PathChanged(dir) : ui->lineEditPath->setText(dir);

                    VPApplication::VApp()->PuzzleSettings()->SetPathLayoutExport(dir);
                }
            });
    connect(ui->lineEditPath, &QLineEdit::textChanged, this, &DialogSaveManualLayout::PathChanged);

    ui->lineEditPath->setText(VPApplication::VApp()->PuzzleSettings()->GetPathLayoutExport());

    ReadSettings();

    ShowExample(); // Show example for current format.
}

//---------------------------------------------------------------------------------------------------------------------
DialogSaveManualLayout::~DialogSaveManualLayout() = default;

//---------------------------------------------------------------------------------------------------------------------
auto DialogSaveManualLayout::Path() const -> QString
{
    return ui->lineEditPath->text();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSaveManualLayout::FileName() const -> QString
{
    return ui->lineEditFileName->text();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSaveManualLayout::Format() const -> LayoutExportFormats
{
    return static_cast<LayoutExportFormats>(ui->comboBoxFormatType->currentData().toInt());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveManualLayout::SelectFormat(LayoutExportFormats format)
{
    if (static_cast<int>(format) < 0 || format >= LayoutExportFormats::COUNT)
    {
        throw VException(tr("Tried to use out of range file format type number."));
    }

    const LayoutExportFileFormat fileFormat = VLayoutExporter::LayoutExportFileFormat(format);

    int i = ui->comboBoxFormat->findData(static_cast<int>(fileFormat));
    if (i < 0)
    {
        throw VException(tr("Selected not present file format."));
    }

    {
        const QSignalBlocker blocker(ui->comboBoxFormat);
        ui->comboBoxFormat->setCurrentIndex(i);
    }

    {
        const QSignalBlocker blocker(ui->comboBoxFormatType);
        InitFileFormatTypes(fileFormat);
        ui->comboBoxFormatType->setCurrentIndex(-1);
    }

    i = ui->comboBoxFormatType->findData(static_cast<int>(format));
    if (i < 0)
    {
        throw VException(tr("Selected not present file format type."));
    }
    ui->comboBoxFormatType->setCurrentIndex(i);

    SetExportUnified(m_count > 1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveManualLayout::SetBinaryDXFFormat(bool binary)
{
    switch (Format())
    {
        case LayoutExportFormats::DXF_AC1006_Flat:
        case LayoutExportFormats::DXF_AC1009_Flat:
        case LayoutExportFormats::DXF_AC1012_Flat:
        case LayoutExportFormats::DXF_AC1014_Flat:
        case LayoutExportFormats::DXF_AC1015_Flat:
        case LayoutExportFormats::DXF_AC1018_Flat:
        case LayoutExportFormats::DXF_AC1021_Flat:
        case LayoutExportFormats::DXF_AC1024_Flat:
        case LayoutExportFormats::DXF_AC1027_Flat:
        case LayoutExportFormats::DXF_AAMA:
        case LayoutExportFormats::DXF_ASTM:
            ui->checkBoxBinaryDXF->setChecked(binary);
            break;
        default:
            ui->checkBoxBinaryDXF->setChecked(false);
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSaveManualLayout::IsBinaryDXFFormat() const -> bool
{
    switch (Format())
    {
        case LayoutExportFormats::DXF_AC1006_Flat:
        case LayoutExportFormats::DXF_AC1009_Flat:
        case LayoutExportFormats::DXF_AC1012_Flat:
        case LayoutExportFormats::DXF_AC1014_Flat:
        case LayoutExportFormats::DXF_AC1015_Flat:
        case LayoutExportFormats::DXF_AC1018_Flat:
        case LayoutExportFormats::DXF_AC1021_Flat:
        case LayoutExportFormats::DXF_AC1024_Flat:
        case LayoutExportFormats::DXF_AC1027_Flat:
        case LayoutExportFormats::DXF_AAMA:
        case LayoutExportFormats::DXF_ASTM:
            return ui->checkBoxBinaryDXF->isChecked();
        default:
            return false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSaveManualLayout::DxfCompatibility() const -> DXFApparelCompatibility
{
    switch (Format())
    {
        case LayoutExportFormats::DXF_AAMA:
        case LayoutExportFormats::DXF_ASTM:
            return static_cast<DXFApparelCompatibility>(ui->comboBoxDxfCompatibility->currentData().toInt());
        default:
            return DXFApparelCompatibility::STANDARD;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveManualLayout::SetDxfCompatibility(DXFApparelCompatibility type)
{
    switch (Format())
    {
        case LayoutExportFormats::DXF_AAMA:
        case LayoutExportFormats::DXF_ASTM:
        {
            if (static_cast<int>(type) < 0 || type >= DXFApparelCompatibility::COUNT)
            {
                break;
            }

            const int i = ui->comboBoxDxfCompatibility->findData(static_cast<int>(type));
            if (i < 0)
            {
                break;
            }
            ui->comboBoxDxfCompatibility->setCurrentIndex(i);
            break;
        }
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveManualLayout::SetShowGrainline(bool show)
{
    switch (Format())
    {
        case LayoutExportFormats::SVG:
        case LayoutExportFormats::PDF:
        case LayoutExportFormats::PNG:
        case LayoutExportFormats::PS:
        case LayoutExportFormats::EPS:
        case LayoutExportFormats::PDFTiled:
        case LayoutExportFormats::TIF:
        case LayoutExportFormats::DXF_AC1006_Flat:
        case LayoutExportFormats::DXF_AC1009_Flat:
        case LayoutExportFormats::DXF_AC1012_Flat:
        case LayoutExportFormats::DXF_AC1014_Flat:
        case LayoutExportFormats::DXF_AC1015_Flat:
        case LayoutExportFormats::DXF_AC1018_Flat:
        case LayoutExportFormats::DXF_AC1021_Flat:
        case LayoutExportFormats::DXF_AC1024_Flat:
        case LayoutExportFormats::DXF_AC1027_Flat:
        case LayoutExportFormats::HPGL:
        case LayoutExportFormats::HPGL2:
        case LayoutExportFormats::HPGL_PLT:
        case LayoutExportFormats::HPGL2_PLT:
            ui->checkBoxShowGrainline->setChecked(show);
            break;
        default:
            ui->checkBoxShowGrainline->setChecked(true);
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSaveManualLayout::IsShowGrainline() const -> bool
{
    switch (Format())
    {
        case LayoutExportFormats::SVG:
        case LayoutExportFormats::PDF:
        case LayoutExportFormats::PNG:
        case LayoutExportFormats::PS:
        case LayoutExportFormats::EPS:
        case LayoutExportFormats::PDFTiled:
        case LayoutExportFormats::TIF:
        case LayoutExportFormats::DXF_AC1006_Flat:
        case LayoutExportFormats::DXF_AC1009_Flat:
        case LayoutExportFormats::DXF_AC1012_Flat:
        case LayoutExportFormats::DXF_AC1014_Flat:
        case LayoutExportFormats::DXF_AC1015_Flat:
        case LayoutExportFormats::DXF_AC1018_Flat:
        case LayoutExportFormats::DXF_AC1021_Flat:
        case LayoutExportFormats::DXF_AC1024_Flat:
        case LayoutExportFormats::DXF_AC1027_Flat:
        case LayoutExportFormats::HPGL:
        case LayoutExportFormats::HPGL2:
        case LayoutExportFormats::HPGL_PLT:
        case LayoutExportFormats::HPGL2_PLT:
            return ui->checkBoxShowGrainline->isChecked();
        default:
            return true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveManualLayout::SetDestinationPath(const QString &cmdDestinationPath)
{
    QString path;
    if (cmdDestinationPath.isEmpty())
    {
        path = QDir::currentPath();
    }
    else if (QDir(cmdDestinationPath).isAbsolute())
    {
        path = cmdDestinationPath;
    }
    else
    {
        QDir dir;
        if (not dir.cd(cmdDestinationPath))
        {
            throw VException(tr("The destination directory doesn't exists or is not readable."));
        }
        path = dir.absolutePath();
    }

    qDebug() << "Output full path: " << path << "\n";
    ui->lineEditPath->setText(path);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSaveManualLayout::IsTextAsPaths() const -> bool
{
    return ui->checkBoxTextAsPaths->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveManualLayout::SetTextAsPaths(bool textAsPaths)
{
    if (const VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
        settings->GetSingleLineFonts() || settings->GetSingleStrokeOutlineFont())
    {
        ui->checkBoxTextAsPaths->setChecked(true);
    }
    else
    {
        ui->checkBoxTextAsPaths->setChecked(textAsPaths);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveManualLayout::SetExportUnified(bool value)
{
    switch (Format())
    {
        case LayoutExportFormats::PDF:
        case LayoutExportFormats::PDFTiled:
        case LayoutExportFormats::PS:
        case LayoutExportFormats::EPS:
            ui->checkBoxExportUnified->setChecked(value);
            break;
        default:
            ui->checkBoxExportUnified->setChecked(false);
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSaveManualLayout::IsExportUnified() const -> bool
{
    switch (Format())
    {
        case LayoutExportFormats::PDF:
        case LayoutExportFormats::PDFTiled:
        case LayoutExportFormats::PS:
        case LayoutExportFormats::EPS:
            return ui->checkBoxExportUnified->isChecked();
        default:
            return false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveManualLayout::SetTilesScheme(bool value)
{
    switch (Format())
    {
        case LayoutExportFormats::PDFTiled:
            ui->checkBoxTilesScheme->setChecked(value);
            break;
        default:
            ui->checkBoxTilesScheme->setChecked(false);
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSaveManualLayout::IsTilesScheme() const -> bool
{
    switch (Format())
    {
        case LayoutExportFormats::PDFTiled:
            return ui->checkBoxTilesScheme->isChecked();
        default:
            return false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveManualLayout::SetHideRuler(bool value)
{
    ui->checkBoxHideRuler->setChecked(Format() == LayoutExportFormats::PDFTiled ? value : false);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSaveManualLayout::IsHideRuler() const -> bool
{
    return Format() == LayoutExportFormats::PDFTiled && ui->checkBoxHideRuler->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveManualLayout::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    if (event->spontaneous())
    {
        return;
    }

    if (m_isInitialized)
    {
        return;
    }
    // do your init stuff here

    setFixedHeight(size().height());

    m_isInitialized = true; // first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveManualLayout::Save()
{
    WriteSettings();

    for (int i = 0; i < m_count; ++i)
    {
        QString name;
        if (m_count > 1 && not IsExportUnified())
        {
            name = Path() + '/' + FileName() + QString::number(i + 1) + VLayoutExporter::ExportFormatSuffix(Format());
        }
        else
        {
            name = Path() + '/' + FileName() + VLayoutExporter::ExportFormatSuffix(Format());
        }

        if (QFile::exists(name))
        {
            if (QMessageBox::StandardButton const res = QMessageBox::question(
                    this, tr("Name conflict"),
                    tr("Folder already contain file with name %1. Rewrite all conflict file names?").arg(name),
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                res == QMessageBox::No)
            {
                reject();
                return;
            }

            break;
        }
    }
    accept();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveManualLayout::PathChanged(const QString &text)
{
    QPushButton *bOk = ui->buttonBox->button(QDialogButtonBox::Ok);
    SCASSERT(bOk != nullptr)

    QPalette palette = ui->lineEditPath->palette();

    QDir dir(text);
    dir.setPath(text);
    if (not text.isEmpty() && dir.exists(text))
    {
        bOk->setEnabled(true);
        const QPalette defaultPalette = QApplication::palette(ui->lineEditPath);
        palette.setColor(ui->lineEditPath->foregroundRole(), defaultPalette.color(QPalette::Text));
    }
    else
    {
        bOk->setEnabled(false);
        palette.setColor(ui->lineEditPath->foregroundRole(), Qt::red);
    }

    ui->lineEditPath->setPalette(palette);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveManualLayout::ShowExample()
{
    const bool hasValues = ui->comboBoxFormatType->currentIndex() != -1;
    ui->groupBoxFormatOptions->setEnabled(hasValues);

    QPushButton *bOk = ui->buttonBox->button(QDialogButtonBox::Ok);
    SCASSERT(bOk != nullptr)
    bOk->setEnabled(hasValues);

    if (!hasValues)
    {
        return;
    }

    const LayoutExportFormats currentFormat = Format();

    QString example;
    if (m_count > 1)
    {
        example = tr("Example:") + FileName() + '1'_L1 + VLayoutExporter::ExportFormatSuffix(currentFormat);
    }
    else
    {
        example = tr("Example:") + FileName() + VLayoutExporter::ExportFormatSuffix(currentFormat);
    }
    ui->labelExample->setText(example);

    const VCommonSettings *settings = VAbstractApplication::VApp()->Settings();

    ui->labelOptionsNotAvailable->setVisible(false);
    ui->checkBoxBinaryDXF->setVisible(false);
    ui->checkBoxTextAsPaths->setVisible(false);
    ui->checkBoxExportUnified->setVisible(false);
    ui->checkBoxTilesScheme->setVisible(false);
    ui->checkBoxShowGrainline->setVisible(false);
    ui->labelDxfCompatibility->setVisible(false);
    ui->comboBoxDxfCompatibility->setVisible(false);
    ui->checkBoxHideRuler->setVisible(false);

    const bool editableTextAsPaths = !settings->GetSingleLineFonts() && !settings->GetSingleStrokeOutlineFont();

    switch (currentFormat)
    {
        case LayoutExportFormats::DXF_AAMA:
        case LayoutExportFormats::DXF_ASTM:
            ui->checkBoxBinaryDXF->setVisible(true);
            ui->labelDxfCompatibility->setVisible(true);
            ui->comboBoxDxfCompatibility->setVisible(true);
            break;
        case LayoutExportFormats::PDFTiled:
            ui->checkBoxTextAsPaths->setVisible(editableTextAsPaths);
            ui->checkBoxTilesScheme->setVisible(true);
            ui->checkBoxExportUnified->setVisible(m_count > 1);
            ui->checkBoxShowGrainline->setVisible(true);
            ui->checkBoxHideRuler->setVisible(true);
            break;
        case LayoutExportFormats::PDF:
        case LayoutExportFormats::PS:
        case LayoutExportFormats::EPS:
            ui->checkBoxTextAsPaths->setVisible(editableTextAsPaths);
            ui->checkBoxExportUnified->setVisible(m_count > 1);
            ui->checkBoxShowGrainline->setVisible(true);
            break;
        case LayoutExportFormats::HPGL:
        case LayoutExportFormats::HPGL2:
        case LayoutExportFormats::HPGL_PLT:
        case LayoutExportFormats::HPGL2_PLT:
            ui->checkBoxShowGrainline->setVisible(true);
            break;
        case LayoutExportFormats::SVG:
        case LayoutExportFormats::PNG:
        case LayoutExportFormats::TIF:
            ui->checkBoxTextAsPaths->setVisible(editableTextAsPaths);
            ui->checkBoxShowGrainline->setVisible(true);
            break;
        case LayoutExportFormats::DXF_AC1006_Flat:
        case LayoutExportFormats::DXF_AC1009_Flat:
        case LayoutExportFormats::DXF_AC1012_Flat:
        case LayoutExportFormats::DXF_AC1014_Flat:
        case LayoutExportFormats::DXF_AC1015_Flat:
        case LayoutExportFormats::DXF_AC1018_Flat:
        case LayoutExportFormats::DXF_AC1021_Flat:
        case LayoutExportFormats::DXF_AC1024_Flat:
        case LayoutExportFormats::DXF_AC1027_Flat:
            ui->checkBoxBinaryDXF->setVisible(true);
            ui->checkBoxTextAsPaths->setVisible(editableTextAsPaths);
            ui->checkBoxShowGrainline->setVisible(true);
            break;
        default:
            ui->labelOptionsNotAvailable->setVisible(true);
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveManualLayout::ReadSettings()
{
    VPSettings *settings = VPApplication::VApp()->PuzzleSettings();
    try
    {
        SelectFormat(static_cast<LayoutExportFormats>(settings->GetLayoutExportFormat()));
    }
    catch (VException &e)
    {
        qDebug() << qUtf8Printable(e.ErrorMessage());
    }
    SetShowGrainline(settings->GetShowGrainline());
    SetDxfCompatibility(static_cast<DXFApparelCompatibility>(settings->GetDxfCompatibility()));
    SetHideRuler(settings->GetHideRuler());
    SetTilesScheme(settings->GetTilesScheme());
    SetTextAsPaths(settings->GetTextAsPaths());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveManualLayout::WriteSettings() const
{
    if (m_consoleExport)
    {
        return;
    }

    VPSettings *settings = VPApplication::VApp()->PuzzleSettings();
    settings->SetLayoutExportFormat(static_cast<qint8>(Format()));
    settings->SetShowGrainline(IsShowGrainline());
    settings->SetDxfCompatibility(static_cast<qint8>(DxfCompatibility()));
    settings->SetHideRuler(IsHideRuler());
    settings->SetTilesScheme(IsTilesScheme());
    settings->SetTextAsPaths(IsTextAsPaths());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveManualLayout::InitDxfCompatibility()
{
    ui->comboBoxDxfCompatibility->addItem(tr("By standard"),
                                          QVariant(static_cast<int>(DXFApparelCompatibility::STANDARD)));
    ui->comboBoxDxfCompatibility->addItem("Richpeace CAD V8"_L1,
                                          QVariant(static_cast<int>(DXFApparelCompatibility::RPCADV08)));
    ui->comboBoxDxfCompatibility->addItem("Richpeace CAD V9"_L1,
                                          QVariant(static_cast<int>(DXFApparelCompatibility::RPCADV09)));
    ui->comboBoxDxfCompatibility->addItem("Richpeace CAD V10"_L1,
                                          QVariant(static_cast<int>(DXFApparelCompatibility::RPCADV10)));
    ui->comboBoxDxfCompatibility->addItem("Clo3D"_L1, QVariant(static_cast<int>(DXFApparelCompatibility::CLO3D)));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveManualLayout::InitFileFormats()
{
    auto AddItem = [this](const QString &label, LayoutExportFileFormat format)
    { ui->comboBoxFormat->addItem(label, QVariant(static_cast<int>(format))); };

    Q_STATIC_ASSERT_X(static_cast<int>(LayoutExportFileFormat::COUNT) == 10, "Update to cover all cases.");

    AddItem("PDF"_L1, LayoutExportFileFormat::PDF);
    AddItem("SVG"_L1, LayoutExportFileFormat::SVG);
    AddItem("PNG"_L1, LayoutExportFileFormat::PNG);
    // AddItem("OBJ"_L1, LayoutExportFileFormat::OBJ); // Temporarily unavailable
    AddItem("DXF"_L1, LayoutExportFileFormat::DXF);
    // AddItem("NC"_L1, LayoutExportFileFormat::NC);// No support for now
    AddItem("PLT"_L1, LayoutExportFileFormat::PLT);
    AddItem("HPGL"_L1, LayoutExportFileFormat::HPGL);
    AddItem("TIF"_L1, LayoutExportFileFormat::TIF);
    AddItem("RLD"_L1, LayoutExportFileFormat::RLD);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveManualLayout::InitFileFormatTypes(LayoutExportFileFormat format)
{
    ui->comboBoxFormatType->clear();
    const QVector<LayoutExportFormats> types = VLayoutExporter::MapLayoutExportFormats(format);

    for (const auto &type : types)
    {
        ui->comboBoxFormatType->addItem(VLayoutExporter::ExportFormatDescription(type),
                                        QVariant(static_cast<int>(type)));
    }

#ifdef V_NO_ASSERT // Temporarily unavailable
    auto RemoveFormatFromList = [this](LayoutExportFormats exportFormat)
    {
        const int index = ui->comboBoxFormatType->findData(static_cast<int>(exportFormat));
        if (index != -1)
        {
            ui->comboBoxFormatType->removeItem(index);
        }
    };

    //    RemoveFormatFromList(LayoutExportFormats::NC); // No support for now
    RemoveFormatFromList(LayoutExportFormats::OBJ);
#endif
}
