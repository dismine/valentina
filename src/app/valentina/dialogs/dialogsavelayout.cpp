/************************************************************************
 **
 **  @file   dialogsavelayout.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 1, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#include "dialogsavelayout.h"
#include "ui_dialogsavelayout.h"
#include "../core/vapplication.h"
#include "../vmisc/vvalentinasettings.h"
#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#include "../vmisc/backport/qoverload.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#include "../ifc/exception/vexception.h"
#include "../vlayout/vlayoutexporter.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QtDebug>
#include <QRegularExpression>
#include <QtDebug>
#include <QGlobalStatic>

#ifndef Q_OS_WIN
    Q_GLOBAL_STATIC_WITH_ARGS(const QString, baseFilenameRegExp, (QLatin1String("^[^\\/]+$"))) // NOLINT
#else
    Q_GLOBAL_STATIC_WITH_ARGS(const QString, baseFilenameRegExp, (QLatin1String("^[^\\:?\"*|\\/<>]+$"))) // NOLINT
#endif

bool DialogSaveLayout::havePdf = false; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
bool DialogSaveLayout::tested  = false; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

//---------------------------------------------------------------------------------------------------------------------
DialogSaveLayout::DialogSaveLayout(int count, Draw mode, const QString &fileName, QWidget *parent)
    :  VAbstractLayoutDialog(parent),
      ui(new Ui::DialogSaveLAyout),
      m_count(count),
      m_mode(mode)
{
    ui->setupUi(this);

#if defined(Q_OS_MAC)
    setWindowFlags(Qt::Window);
#endif

    ui->lineEditPath->setClearButtonEnabled(true);
    ui->lineEditFileName->setClearButtonEnabled(true);

    VAbstractApplication::VApp()->Settings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    QPushButton *bOk = ui->buttonBox->button(QDialogButtonBox::Ok);
    SCASSERT(bOk != nullptr)
    bOk->setEnabled(false);

    ui->lineEditFileName->setValidator( new QRegularExpressionValidator(QRegularExpression(*baseFilenameRegExp), this));

    const QString mask = fileName + '_';
    if (VApplication::IsGUIMode())
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

    if (m_mode != Draw::Layout)
    {
        RemoveFormatFromList(LayoutExportFormats::PDFTiled);
    }
    else
    {
        RemoveFormatFromList(LayoutExportFormats::RLD);
        ui->checkBoxTextAsPaths->setVisible(false);
    }

    connect(bOk, &QPushButton::clicked, this, &DialogSaveLayout::Save);
    connect(ui->lineEditFileName, &QLineEdit::textChanged, this, &DialogSaveLayout::ShowExample);
    connect(ui->comboBoxFormat, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DialogSaveLayout::ShowExample);
    connect(ui->pushButtonBrowse, &QPushButton::clicked, this, [this]()
    {
        const QString dirPath = VAbstractValApplication::VApp()->ValentinaSettings()->GetPathLayout();
        bool usedNotExistedDir = false;
        QDir directory(dirPath);
        if (not directory.exists())
        {
            usedNotExistedDir = directory.mkpath(QChar('.'));
        }

        const QString dir = QFileDialog::getExistingDirectory(
            this, tr("Select folder"), dirPath,
            VAbstractApplication::VApp()->NativeFileDialog(QFileDialog::ShowDirsOnly |
                                                           QFileDialog::DontResolveSymlinks));
        if (not dir.isEmpty())
        {// If paths equal the signal will not be called, we will do this manually
            dir == ui->lineEditPath->text() ? PathChanged(dir) : ui->lineEditPath->setText(dir);
        }

        if (usedNotExistedDir)
        {
            QDir directory(dirPath);
            directory.rmpath(QChar('.'));
        }
    });
    connect(ui->lineEditPath, &QLineEdit::textChanged, this, &DialogSaveLayout::PathChanged);

    ui->lineEditPath->setText(VAbstractValApplication::VApp()->ValentinaSettings()->GetPathLayout());

    InitTileTemplates(ui->comboBoxTemplates);

    connect(ui->toolButtonScaleConnected, &QToolButton::clicked, this, &DialogSaveLayout::ToggleScaleConnection);

    connect(ui->doubleSpinBoxHorizontalScale, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &DialogSaveLayout::HorizontalScaleChanged);
    connect(ui->doubleSpinBoxVerticalScale, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &DialogSaveLayout::VerticalScaleChanged);

    ReadSettings();

    ShowExample();//Show example for current format.
}

//---------------------------------------------------------------------------------------------------------------------

void DialogSaveLayout::SelectFormat(LayoutExportFormats format)
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
void DialogSaveLayout::SetBinaryDXFFormat(bool binary)
{
    switch(Format())
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
        case LayoutExportFormats::DXF_AC1006_AAMA:
        case LayoutExportFormats::DXF_AC1009_AAMA:
        case LayoutExportFormats::DXF_AC1012_AAMA:
        case LayoutExportFormats::DXF_AC1014_AAMA:
        case LayoutExportFormats::DXF_AC1015_AAMA:
        case LayoutExportFormats::DXF_AC1018_AAMA:
        case LayoutExportFormats::DXF_AC1021_AAMA:
        case LayoutExportFormats::DXF_AC1024_AAMA:
        case LayoutExportFormats::DXF_AC1027_AAMA:
        case LayoutExportFormats::DXF_AC1006_ASTM:
        case LayoutExportFormats::DXF_AC1009_ASTM:
        case LayoutExportFormats::DXF_AC1012_ASTM:
        case LayoutExportFormats::DXF_AC1014_ASTM:
        case LayoutExportFormats::DXF_AC1015_ASTM:
        case LayoutExportFormats::DXF_AC1018_ASTM:
        case LayoutExportFormats::DXF_AC1021_ASTM:
        case LayoutExportFormats::DXF_AC1024_ASTM:
        case LayoutExportFormats::DXF_AC1027_ASTM:
            ui->checkBoxBinaryDXF->setChecked(binary);
            break;
        case LayoutExportFormats::SVG:
        case LayoutExportFormats::PDF:
        case LayoutExportFormats::PDFTiled:
        case LayoutExportFormats::PNG:
        case LayoutExportFormats::OBJ:
        case LayoutExportFormats::PS:
        case LayoutExportFormats::EPS:
        case LayoutExportFormats::NC:
        case LayoutExportFormats::RLD:
        case LayoutExportFormats::TIF:
        default:
            ui->checkBoxBinaryDXF->setChecked(false);
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSaveLayout::IsBinaryDXFFormat() const -> bool
{
    switch(Format())
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
        case LayoutExportFormats::DXF_AC1006_AAMA:
        case LayoutExportFormats::DXF_AC1009_AAMA:
        case LayoutExportFormats::DXF_AC1012_AAMA:
        case LayoutExportFormats::DXF_AC1014_AAMA:
        case LayoutExportFormats::DXF_AC1015_AAMA:
        case LayoutExportFormats::DXF_AC1018_AAMA:
        case LayoutExportFormats::DXF_AC1021_AAMA:
        case LayoutExportFormats::DXF_AC1024_AAMA:
        case LayoutExportFormats::DXF_AC1027_AAMA:
        case LayoutExportFormats::DXF_AC1006_ASTM:
        case LayoutExportFormats::DXF_AC1009_ASTM:
        case LayoutExportFormats::DXF_AC1012_ASTM:
        case LayoutExportFormats::DXF_AC1014_ASTM:
        case LayoutExportFormats::DXF_AC1015_ASTM:
        case LayoutExportFormats::DXF_AC1018_ASTM:
        case LayoutExportFormats::DXF_AC1021_ASTM:
        case LayoutExportFormats::DXF_AC1024_ASTM:
        case LayoutExportFormats::DXF_AC1027_ASTM:
            return ui->checkBoxBinaryDXF->isChecked();
        case LayoutExportFormats::SVG:
        case LayoutExportFormats::PDF:
        case LayoutExportFormats::PDFTiled:
        case LayoutExportFormats::PNG:
        case LayoutExportFormats::OBJ:
        case LayoutExportFormats::PS:
        case LayoutExportFormats::EPS:
        case LayoutExportFormats::NC:
        case LayoutExportFormats::RLD:
        case LayoutExportFormats::TIF:
        default:
            return false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveLayout::SetShowGrainline(bool show)
{
    switch(Format())
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
        case LayoutExportFormats::SVG:
        case LayoutExportFormats::PDF:
        case LayoutExportFormats::PDFTiled:
        case LayoutExportFormats::PNG:
        case LayoutExportFormats::PS:
        case LayoutExportFormats::EPS:
        case LayoutExportFormats::TIF:
            ui->checkBoxShowGrainline->setChecked(show);
            break;
        case LayoutExportFormats::DXF_AC1006_AAMA:
        case LayoutExportFormats::DXF_AC1009_AAMA:
        case LayoutExportFormats::DXF_AC1012_AAMA:
        case LayoutExportFormats::DXF_AC1014_AAMA:
        case LayoutExportFormats::DXF_AC1015_AAMA:
        case LayoutExportFormats::DXF_AC1018_AAMA:
        case LayoutExportFormats::DXF_AC1021_AAMA:
        case LayoutExportFormats::DXF_AC1024_AAMA:
        case LayoutExportFormats::DXF_AC1027_AAMA:
        case LayoutExportFormats::DXF_AC1006_ASTM:
        case LayoutExportFormats::DXF_AC1009_ASTM:
        case LayoutExportFormats::DXF_AC1012_ASTM:
        case LayoutExportFormats::DXF_AC1014_ASTM:
        case LayoutExportFormats::DXF_AC1015_ASTM:
        case LayoutExportFormats::DXF_AC1018_ASTM:
        case LayoutExportFormats::DXF_AC1021_ASTM:
        case LayoutExportFormats::DXF_AC1024_ASTM:
        case LayoutExportFormats::DXF_AC1027_ASTM:
        case LayoutExportFormats::RLD:
        case LayoutExportFormats::NC:
        case LayoutExportFormats::OBJ:
        default:
            ui->checkBoxShowGrainline->setChecked(true);
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSaveLayout::IsShowGrainline() const -> bool
{
    switch(Format())
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
        case LayoutExportFormats::SVG:
        case LayoutExportFormats::PDF:
        case LayoutExportFormats::PDFTiled:
        case LayoutExportFormats::PNG:
        case LayoutExportFormats::PS:
        case LayoutExportFormats::EPS:
        case LayoutExportFormats::TIF:
            return ui->checkBoxShowGrainline->isChecked();
        case LayoutExportFormats::DXF_AC1006_AAMA:
        case LayoutExportFormats::DXF_AC1009_AAMA:
        case LayoutExportFormats::DXF_AC1012_AAMA:
        case LayoutExportFormats::DXF_AC1014_AAMA:
        case LayoutExportFormats::DXF_AC1015_AAMA:
        case LayoutExportFormats::DXF_AC1018_AAMA:
        case LayoutExportFormats::DXF_AC1021_AAMA:
        case LayoutExportFormats::DXF_AC1024_AAMA:
        case LayoutExportFormats::DXF_AC1027_AAMA:
        case LayoutExportFormats::DXF_AC1006_ASTM:
        case LayoutExportFormats::DXF_AC1009_ASTM:
        case LayoutExportFormats::DXF_AC1012_ASTM:
        case LayoutExportFormats::DXF_AC1014_ASTM:
        case LayoutExportFormats::DXF_AC1015_ASTM:
        case LayoutExportFormats::DXF_AC1018_ASTM:
        case LayoutExportFormats::DXF_AC1021_ASTM:
        case LayoutExportFormats::DXF_AC1024_ASTM:
        case LayoutExportFormats::DXF_AC1027_ASTM:
        case LayoutExportFormats::RLD:
        case LayoutExportFormats::NC:
        case LayoutExportFormats::OBJ:
        default:
            return true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSaveLayout::MakeHelpFormatList() -> QString
{
   QString out(QStringLiteral("\n"));
   const auto formats = InitFormats();
   for(int i = 0; i < formats.size(); ++i)
   {
       out += QStringLiteral("\t* ") + formats.at(i).first + QStringLiteral(" = ")
               + QString::number(static_cast<int>(formats.at(i).second));

       if (i < formats.size() - 1)
       {
           out += QLatin1String(",\n");
       }
       else
       {
           out += QLatin1String(".\n");
       }
   }
   return out;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveLayout::SetDestinationPath(const QString &cmdDestinationPath)
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
auto DialogSaveLayout::Mode() const -> Draw
{
    return m_mode;
}

//---------------------------------------------------------------------------------------------------------------------
DialogSaveLayout::~DialogSaveLayout()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSaveLayout::Path() const -> QString
{
    return ui->lineEditPath->text();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSaveLayout::FileName() const -> QString
{
    return ui->lineEditFileName->text();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSaveLayout::Format() const -> LayoutExportFormats
{
    return static_cast<LayoutExportFormats>(ui->comboBoxFormat->currentData().toInt());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveLayout::Save()
{
    WriteSettings();

    for (int i=0; i < m_count; ++i)
    {
        const QString name = Path()+'/'+FileName()+QString::number(i+1)+VLayoutExporter::ExportFormatSuffix(Format());
        if (QFile::exists(name))
        {
            QMessageBox::StandardButton res = QMessageBox::question(this, tr("Name conflict"),
                                  tr("Folder already contain file with name %1. Rewrite all conflict file names?")
                                  .arg(name), QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
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
void DialogSaveLayout::PathChanged(const QString &text)
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
void DialogSaveLayout::ShowExample()
{
    const LayoutExportFormats currentFormat = Format();
    ui->labelExample->setText(tr("Example:") + FileName() + QLatin1Char('1') + VLayoutExporter::ExportFormatSuffix(currentFormat));

    ui->checkBoxBinaryDXF->setEnabled(false);
    ui->groupBoxPaperFormat->setEnabled(false);
    ui->groupBoxMargins->setEnabled(false);
    ui->checkBoxTextAsPaths->setEnabled(true);
    ui->checkBoxShowGrainline->setEnabled(m_mode == Draw::Layout);

    switch(currentFormat)
    {
        case LayoutExportFormats::DXF_AC1006_AAMA:
        case LayoutExportFormats::DXF_AC1009_AAMA:
        case LayoutExportFormats::DXF_AC1012_AAMA:
        case LayoutExportFormats::DXF_AC1014_AAMA:
        case LayoutExportFormats::DXF_AC1015_AAMA:
        case LayoutExportFormats::DXF_AC1018_AAMA:
        case LayoutExportFormats::DXF_AC1021_AAMA:
        case LayoutExportFormats::DXF_AC1024_AAMA:
        case LayoutExportFormats::DXF_AC1027_AAMA:
        case LayoutExportFormats::DXF_AC1006_ASTM:
        case LayoutExportFormats::DXF_AC1009_ASTM:
        case LayoutExportFormats::DXF_AC1012_ASTM:
        case LayoutExportFormats::DXF_AC1014_ASTM:
        case LayoutExportFormats::DXF_AC1015_ASTM:
        case LayoutExportFormats::DXF_AC1018_ASTM:
        case LayoutExportFormats::DXF_AC1021_ASTM:
        case LayoutExportFormats::DXF_AC1024_ASTM:
        case LayoutExportFormats::DXF_AC1027_ASTM:
            ui->checkBoxBinaryDXF->setEnabled(true);
            ui->checkBoxShowGrainline->setEnabled(false);
            break;
        case LayoutExportFormats::PDFTiled:
            ui->groupBoxPaperFormat->setEnabled(true);
            ui->groupBoxMargins->setEnabled(true);
            break;
        case LayoutExportFormats::RLD:
            ui->checkBoxTextAsPaths->setEnabled(false);
            ui->checkBoxShowGrainline->setEnabled(false);
            break;
        case LayoutExportFormats::NC:
        case LayoutExportFormats::OBJ:
            ui->checkBoxShowGrainline->setEnabled(false);
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
        case LayoutExportFormats::SVG:
        case LayoutExportFormats::PDF:
        case LayoutExportFormats::PNG:
        case LayoutExportFormats::PS:
        case LayoutExportFormats::EPS:
        case LayoutExportFormats::TIF:
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveLayout::ToggleScaleConnection()
{
    m_scaleConnected = not m_scaleConnected;

    QIcon icon;
    icon.addFile(m_scaleConnected ? QStringLiteral(":/icon/32x32/link.png")
                                  : QStringLiteral(":/icon/32x32/broken_link.png"));
    ui->toolButtonScaleConnected->setIcon(icon);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveLayout::HorizontalScaleChanged(double d)
{
    if (m_scaleConnected)
    {
        ui->doubleSpinBoxVerticalScale->blockSignals(true);
        ui->doubleSpinBoxVerticalScale->setValue(d);
        ui->doubleSpinBoxVerticalScale->blockSignals(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveLayout::VerticalScaleChanged(double d)
{
    if (m_scaleConnected)
    {
        ui->doubleSpinBoxHorizontalScale->blockSignals(true);
        ui->doubleSpinBoxHorizontalScale->setValue(d);
        ui->doubleSpinBoxHorizontalScale->blockSignals(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSaveLayout::IsTextAsPaths() const -> bool
{
    return ui->checkBoxTextAsPaths->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveLayout::SetTextAsPaths(bool textAsPaths)
{
    if (m_mode != Draw::Layout)
    {
        ui->checkBoxTextAsPaths->setChecked(textAsPaths);
    }
    else
    {
        ui->checkBoxTextAsPaths->setChecked(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveLayout::SetTiledExportMode(bool tiledExportMode)
{
    m_tiledExportMode = tiledExportMode;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveLayout::SetTiledMargins(QMarginsF margins)
{
    // read Margins top, right, bottom, left
    margins = UnitConvertor(margins, Unit::Mm, VAbstractValApplication::VApp()->patternUnits());

    ui->doubleSpinBoxLeftField->setValue(margins.left());
    ui->doubleSpinBoxTopField->setValue(margins.top());
    ui->doubleSpinBoxRightField->setValue(margins.right());
    ui->doubleSpinBoxBottomField->setValue(margins.bottom());
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSaveLayout::GetTiledMargins() const -> QMarginsF
{
    QMarginsF margins = QMarginsF(
        ui->doubleSpinBoxLeftField->value(),
        ui->doubleSpinBoxTopField->value(),
        ui->doubleSpinBoxRightField->value(),
        ui->doubleSpinBoxBottomField->value()
    );

    return UnitConvertor(margins, VAbstractValApplication::VApp()->patternUnits(), Unit::Mm);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveLayout::SetTiledPageFormat(PaperSizeTemplate format)
{
    int index = ui->comboBoxTemplates->findData(static_cast<int>(format));
    if (index != -1)
    {
        ui->comboBoxTemplates->setCurrentIndex(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSaveLayout::GetTiledPageFormat() const -> VAbstractLayoutDialog::PaperSizeTemplate
{
    if (ui->comboBoxTemplates->currentIndex() != -1)
    {
        return static_cast<PaperSizeTemplate>(ui->comboBoxTemplates->currentData().toInt());
    }
    return PaperSizeTemplate::A0;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveLayout::SetTiledPageOrientation(PageOrientation orientation)
{
    if(orientation == PageOrientation::Portrait)
    {
        ui->toolButtonPortrait->setChecked(true);
    }
    else
    {
        ui->toolButtonLandscape->setChecked(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSaveLayout::GetTiledPageOrientation() const -> PageOrientation
{
    if(ui->toolButtonPortrait->isChecked())
    {
        return PageOrientation::Portrait;
    }

    return PageOrientation::Landscape;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveLayout::SetXScale(qreal scale)
{
    ui->doubleSpinBoxHorizontalScale->setValue(scale * 100.);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSaveLayout::GetXScale() const -> qreal
{
    return ui->doubleSpinBoxHorizontalScale->value() / 100.;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveLayout::SetYScale(qreal scale)
{
    ui->doubleSpinBoxVerticalScale->setValue(scale * 100.);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSaveLayout::GetYScale() const -> qreal
{
    return ui->doubleSpinBoxVerticalScale->value() / 100.;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveLayout::showEvent(QShowEvent *event)
{
    QDialog::showEvent( event );
    if ( event->spontaneous() )
    {
        return;
    }

    if (m_isInitialized)
    {
        return;
    }
    // do your init stuff here

    setFixedHeight(size().height());

    m_isInitialized = true;//first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSaveLayout::SupportPSTest() -> bool
{
    if (!tested)
    {
        havePdf = VLayoutExporter::SupportPDFConversion();
        tested = true;
    }
    return havePdf;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSaveLayout::InitFormats() -> QVector<std::pair<QString, LayoutExportFormats> >
{
    QVector<std::pair<QString, LayoutExportFormats>> list;

    auto InitFormat = [&list](LayoutExportFormats format)
    {
        list.append(std::make_pair(VLayoutExporter::ExportFormatDescription(format), format));
    };

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
    InitFormat(LayoutExportFormats::DXF_AC1006_AAMA);
    InitFormat(LayoutExportFormats::DXF_AC1009_AAMA);
    InitFormat(LayoutExportFormats::DXF_AC1012_AAMA);
    InitFormat(LayoutExportFormats::DXF_AC1014_AAMA);
    InitFormat(LayoutExportFormats::DXF_AC1015_AAMA);
    InitFormat(LayoutExportFormats::DXF_AC1018_AAMA);
    InitFormat(LayoutExportFormats::DXF_AC1021_AAMA);
    InitFormat(LayoutExportFormats::DXF_AC1024_AAMA);
    InitFormat(LayoutExportFormats::DXF_AC1027_AAMA);
    InitFormat(LayoutExportFormats::DXF_AC1006_ASTM);
    InitFormat(LayoutExportFormats::DXF_AC1009_ASTM);
    InitFormat(LayoutExportFormats::DXF_AC1012_ASTM);
    InitFormat(LayoutExportFormats::DXF_AC1014_ASTM);
    InitFormat(LayoutExportFormats::DXF_AC1015_ASTM);
    InitFormat(LayoutExportFormats::DXF_AC1018_ASTM);
    InitFormat(LayoutExportFormats::DXF_AC1021_ASTM);
    InitFormat(LayoutExportFormats::DXF_AC1024_ASTM);
    InitFormat(LayoutExportFormats::DXF_AC1027_ASTM);
    InitFormat(LayoutExportFormats::PDFTiled); 
//    InitFormat(LayoutExportFormats::NC);
    InitFormat(LayoutExportFormats::RLD);
    InitFormat(LayoutExportFormats::TIF);

    return list;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveLayout::RemoveFormatFromList(LayoutExportFormats format)
{
    const int index = ui->comboBoxFormat->findData(static_cast<int>(format));
    if (index != -1)
    {
        ui->comboBoxFormat->removeItem(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ReadSettings reads the values of the variables needed for the save layout dialog, for instance
 * the margins, teamplte and orientation of tiled pdf. Then sets the corresponding
 * elements of the dialog to these values.
 */
void DialogSaveLayout::ReadSettings()
{
    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
    const Unit unit = VAbstractValApplication::VApp()->patternUnits();

    // read Margins top, right, bottom, left
    const QMarginsF margins = settings->GetTiledPDFMargins(unit);

    ui->doubleSpinBoxLeftField->setValue(margins.left());
    ui->doubleSpinBoxTopField->setValue(margins.top());
    ui->doubleSpinBoxRightField->setValue(margins.right());
    ui->doubleSpinBoxBottomField->setValue(margins.bottom());

    ui->doubleSpinBoxLeftField->setSuffix(UnitsToStr(unit, true));
    ui->doubleSpinBoxTopField->setSuffix(UnitsToStr(unit, true));
    ui->doubleSpinBoxRightField->setSuffix(UnitsToStr(unit, true));
    ui->doubleSpinBoxBottomField->setSuffix(UnitsToStr(unit, true));

    // read Template
    const QSizeF size = QSizeF(settings->GetTiledPDFPaperWidth(Unit::Mm), settings->GetTiledPDFPaperHeight(Unit::Mm));

    const int max = static_cast<int>(PaperSizeTemplate::Custom);
    for (int i=0; i < max; ++i)
    {

        const QSizeF tmplSize = GetTemplateSize(static_cast<PaperSizeTemplate>(i), Unit::Mm);
        if (size == tmplSize)
        {
            ui->comboBoxTemplates->setCurrentIndex(i);
            break;
        }
    }

    // read Orientation
    if(settings->GetTiledPDFOrientation() == PageOrientation::Portrait)
    {
        ui->toolButtonPortrait->setChecked(true);
    }
    else
    {
        ui->toolButtonLandscape->setChecked(true);
    }

    if (m_mode != Draw::Layout)
    {
        SelectFormat(static_cast<LayoutExportFormats>(settings->GetDetailExportFormat()));
    }
    else
    {
        SelectFormat(static_cast<LayoutExportFormats>(settings->GetLayoutExportFormat()));
        SetShowGrainline(settings->GetShowGrainline());
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief WriteSettings writes the values of some variables (like the margins, template and orientation of tiled pdf)
 * of the save layout dialog into the settings.
 */
void DialogSaveLayout::WriteSettings() const
{
    if (m_tiledExportMode)
    {
        return;
    }

    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
    const Unit unit = VAbstractValApplication::VApp()->patternUnits();

    // write Margins top, right, bottom, left
    QMarginsF margins = QMarginsF(
        ui->doubleSpinBoxLeftField->value(),
        ui->doubleSpinBoxTopField->value(),
        ui->doubleSpinBoxRightField->value(),
        ui->doubleSpinBoxBottomField->value()
    );
    settings->SetTiledPDFMargins(margins,unit);

    // write Template
    const PaperSizeTemplate temp = static_cast<PaperSizeTemplate>(ui->comboBoxTemplates->currentData().toInt());
    const QSizeF size = GetTemplateSize(temp, Unit::Mm);

    settings->SetTiledPDFPaperHeight(size.height(), Unit::Mm);
    settings->SetTiledPDFPaperWidth(size.width(), Unit::Mm);

    // write Orientation
    if(ui->toolButtonPortrait->isChecked())
    {
        settings->SetTiledPDFOrientation(PageOrientation::Portrait);
    }
    else
    {
        settings->SetTiledPDFOrientation(PageOrientation::Landscape);
    }

    if (m_mode != Draw::Layout)
    {
        settings->SetDetailExportFormat(static_cast<qint8>(Format()));
    }
    else
    {
        settings->SetLayoutExportFormat(static_cast<qint8>(Format()));
        settings->SetShowGrainline(IsShowGrainline());
    }
}

