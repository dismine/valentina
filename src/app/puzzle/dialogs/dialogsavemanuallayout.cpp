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
#include "../ifc/exception/vexception.h"
#include "../vlayout/vlayoutexporter.h"
#include "../vpapplication.h"
#include "ui_dialogsavemanuallayout.h"
#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#include "../vmisc/backport/qoverload.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 7, 0)

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
    ui(new Ui::DialogSaveManualLayout),
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

    const QString mask = m_count > 1 ? fileName + '_' : fileName;
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

    for (auto &v : InitFormats())
    {
        ui->comboBoxFormat->addItem(v.first, QVariant(static_cast<int>(v.second)));
    }
#ifdef V_NO_ASSERT // Temporarily unavailable
    RemoveFormatFromList(LayoutExportFormats::OBJ);
#endif

    //    RemoveFormatFromList(LayoutExportFormats::NC); // No support for now

    connect(bOk, &QPushButton::clicked, this, &DialogSaveManualLayout::Save);
    connect(ui->lineEditFileName, &QLineEdit::textChanged, this, &DialogSaveManualLayout::ShowExample);
    connect(ui->comboBoxFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogSaveManualLayout::ShowExample);

    connect(ui->pushButtonBrowse, &QPushButton::clicked, this,
            [this]()
            {
                const QString dirPath = VPApplication::VApp()->PuzzleSettings()->GetPathManualLayouts();

                const QString dir = QFileDialog::getExistingDirectory(
                    this, tr("Select folder"), dirPath,
                    VAbstractApplication::VApp()->NativeFileDialog(QFileDialog::ShowDirsOnly |
                                                                   QFileDialog::DontResolveSymlinks));
                if (not dir.isEmpty())
                { // If paths equal the signal will not be called, we will do this manually
                    dir == ui->lineEditPath->text() ? PathChanged(dir) : ui->lineEditPath->setText(dir);

                    VPApplication::VApp()->PuzzleSettings()->SetPathManualLayouts(dir);
                }
            });
    connect(ui->lineEditPath, &QLineEdit::textChanged, this, &DialogSaveManualLayout::PathChanged);

    ui->lineEditPath->setText(VPApplication::VApp()->PuzzleSettings()->GetPathManualLayouts());

    ReadSettings();

    ShowExample(); // Show example for current format.
}

//---------------------------------------------------------------------------------------------------------------------
DialogSaveManualLayout::~DialogSaveManualLayout()
{
    delete ui;
}

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
    return static_cast<LayoutExportFormats>(ui->comboBoxFormat->currentData().toInt());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveManualLayout::SelectFormat(LayoutExportFormats format)
{
    if (static_cast<int>(format) < 0 || format >= LayoutExportFormats::COUNT)
    {
        throw VException(tr("Tried to use out of range format number."));
    }

    const int i = ui->comboBoxFormat->findData(static_cast<int>(format));
    if (i < 0)
    {
        throw VException(tr("Selected not present format."));
    }
    ui->comboBoxFormat->setCurrentIndex(i);
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
    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    if (settings->GetSingleLineFonts() || settings->GetSingleStrokeOutlineFont())
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
            QMessageBox::StandardButton res = QMessageBox::question(
                this, tr("Name conflict"),
                tr("Folder already contain file with name %1. Rewrite all conflict file names?").arg(name),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if (res == QMessageBox::No)
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
        palette.setColor(ui->lineEditPath->foregroundRole(), Qt::black);
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

    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();

    ui->labelOptionsNotAvailable->setVisible(false);
    ui->checkBoxBinaryDXF->setVisible(false);
    ui->checkBoxTextAsPaths->setVisible(false);
    ui->checkBoxExportUnified->setVisible(false);
    ui->checkBoxTilesScheme->setVisible(false);
    ui->checkBoxShowGrainline->setVisible(false);

    const bool editableTextAsPaths = !settings->GetSingleLineFonts() && !settings->GetSingleStrokeOutlineFont();

    switch (currentFormat)
    {
        case LayoutExportFormats::DXF_AAMA:
        case LayoutExportFormats::DXF_ASTM:
            ui->checkBoxBinaryDXF->setVisible(true);
            break;
        case LayoutExportFormats::PDFTiled:
            ui->checkBoxTextAsPaths->setVisible(editableTextAsPaths);
            ui->checkBoxTilesScheme->setVisible(true);
            ui->checkBoxExportUnified->setVisible(m_count > 1);
            ui->checkBoxShowGrainline->setVisible(true);
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
auto DialogSaveManualLayout::SupportPSTest() -> bool
{
    static bool havePdf = false;
    static bool tested = false;
    if (!tested)
    {
        havePdf = VLayoutExporter::SupportPDFConversion();
        tested = true;
    }
    return havePdf;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSaveManualLayout::InitFormats() -> QVector<std::pair<QString, LayoutExportFormats>>
{
    QVector<std::pair<QString, LayoutExportFormats>> list;

    auto InitFormat = [&list](LayoutExportFormats format)
    { list.append(std::make_pair(VLayoutExporter::ExportFormatDescription(format), format)); };

    InitFormat(LayoutExportFormats::SVG);
    InitFormat(LayoutExportFormats::PDF);
    InitFormat(LayoutExportFormats::PNG);
    InitFormat(LayoutExportFormats::OBJ);
    if (SupportPSTest())
    {
        InitFormat(LayoutExportFormats::PS);
        InitFormat(LayoutExportFormats::EPS);
    }
    InitFormat(LayoutExportFormats::DXF_AC1006_Flat);
    InitFormat(LayoutExportFormats::DXF_AC1009_Flat);
    InitFormat(LayoutExportFormats::DXF_AC1012_Flat);
    InitFormat(LayoutExportFormats::DXF_AC1014_Flat);
    InitFormat(LayoutExportFormats::DXF_AC1015_Flat);
    InitFormat(LayoutExportFormats::DXF_AC1018_Flat);
    InitFormat(LayoutExportFormats::DXF_AC1021_Flat);
    InitFormat(LayoutExportFormats::DXF_AC1024_Flat);
    InitFormat(LayoutExportFormats::DXF_AC1027_Flat);
    InitFormat(LayoutExportFormats::DXF_AAMA);
    InitFormat(LayoutExportFormats::DXF_ASTM);
    InitFormat(LayoutExportFormats::PDFTiled);
    //    InitFormat(LayoutExportFormats::NC);
    InitFormat(LayoutExportFormats::RLD);
    InitFormat(LayoutExportFormats::TIF);
    InitFormat(LayoutExportFormats::HPGL);
    InitFormat(LayoutExportFormats::HPGL2);

    return list;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveManualLayout::RemoveFormatFromList(LayoutExportFormats format)
{
    const int index = ui->comboBoxFormat->findData(static_cast<int>(format));
    if (index != -1)
    {
        ui->comboBoxFormat->removeItem(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveManualLayout::ReadSettings()
{
    VPSettings *settings = VPApplication::VApp()->PuzzleSettings();
    SelectFormat(static_cast<LayoutExportFormats>(settings->GetLayoutExportFormat()));
    SetShowGrainline(settings->GetShowGrainline());
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
}
