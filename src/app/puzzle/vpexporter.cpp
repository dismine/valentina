#include "vpexporter.h"

#include <QFileDialog>

#include "../vwidgets/vmaingraphicsscene.h"
#include "vpsheet.h"
#include "vpmaingraphicsview.h"


//---------------------------------------------------------------------------------------------------------------------
VPExporter::VPExporter()
{

}

//---------------------------------------------------------------------------------------------------------------------
VPExporter::~VPExporter()
{

}

//---------------------------------------------------------------------------------------------------------------------
void VPExporter::Export(VPLayout* layout, LayoutExportFormats format, VPMainGraphicsView *mainGraphicsView)
{
    QString dir = QDir::homePath();
    QString filters(ExportFormatDescription(format));

    // is it ok to have a null ptr hier as a parent?
    QString fileName = QFileDialog::getSaveFileName(nullptr, tr("Save as"),
                                                    dir + QLatin1String("/") + layout->GetFocusedSheet()->GetName() + ExportFormatSuffix(format),
                                                    filters, nullptr
#ifdef Q_OS_LINUX
                                                    , QFileDialog::DontUseNativeDialog
#endif
                                                    );

    if(not fileName.isEmpty())
    {
        mainGraphicsView->PrepareForExport();

        SetFileName(fileName);

        QSizeF size = QSizeF(layout->GetFocusedSheet()->GetSheetSize());
        if(layout->GetFocusedSheet()->GetOrientation() == PageOrientation::Landscape)
        {
            size.transpose();
        }
        const QRectF rect = QRectF(0, 0, size.width(), size.height());

        SetImageRect(rect);

        switch(format)
        {
            case LayoutExportFormats::SVG:
                ExportToSVG(mainGraphicsView->scene());
            break;

            case LayoutExportFormats::PDF:
                ExportToPDF(mainGraphicsView->scene());
            break;

            case LayoutExportFormats::TIF:
                ExportToTIF(mainGraphicsView->scene());
            break;

            default:
                // do nothing
            break;
        }

        mainGraphicsView->CleanAfterExport();
    }
}


// FIXME Bad copy paste from DialogSaveLayout, because I didn't know how to call this function from here
// to be removed as soon as I know how to call the central function from valentina
//---------------------------------------------------------------------------------------------------------------------
QString VPExporter::ExportFormatDescription(LayoutExportFormats format)
{
    const QString dxfSuffix = QStringLiteral("(*.dxf)");
    const QString dxfFlatFilesStr = tr("(flat) files");
    const QString filesStr = tr("files");

    switch(format)
    {
        case LayoutExportFormats::SVG:
            return QStringLiteral("Svg %1 (*.svg)").arg(filesStr);
        case LayoutExportFormats::PDF:
            return QStringLiteral("PDF %1 (*.pdf)").arg(filesStr);
        case LayoutExportFormats::PNG:
            return tr("Image files") + QStringLiteral(" (*.png)");
        case LayoutExportFormats::OBJ:
            return QStringLiteral("Wavefront OBJ (*.obj)");
        case LayoutExportFormats::PS:
            return QStringLiteral("PS %1 (*.ps)").arg(filesStr);
        case LayoutExportFormats::EPS:
            return QStringLiteral("EPS %1 (*.eps)").arg(filesStr);
        case LayoutExportFormats::DXF_AC1006_Flat:
            return QStringLiteral("AutoCAD DXF R10 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1009_Flat:
            return QStringLiteral("AutoCAD DXF R11/12 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1012_Flat:
            return QStringLiteral("AutoCAD DXF R13 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1014_Flat:
            return QStringLiteral("AutoCAD DXF R14 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1015_Flat:
            return QStringLiteral("AutoCAD DXF 2000 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1018_Flat:
            return QStringLiteral("AutoCAD DXF 2004 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1021_Flat:
            return QStringLiteral("AutoCAD DXF 2007 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1024_Flat:
            return QStringLiteral("AutoCAD DXF 2010 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1027_Flat:
            return QStringLiteral("AutoCAD DXF 2013 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1006_AAMA:
            return QStringLiteral("AutoCAD DXF R10 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1009_AAMA:
            return QStringLiteral("AutoCAD DXF R11/12 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1012_AAMA:
            return QStringLiteral("AutoCAD DXF R13 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1014_AAMA:
            return QStringLiteral("AutoCAD DXF R14 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1015_AAMA:
            return QStringLiteral("AutoCAD DXF 2000 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1018_AAMA:
            return QStringLiteral("AutoCAD DXF 2004 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1021_AAMA:
            return QStringLiteral("AutoCAD DXF 2007 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1024_AAMA:
            return QStringLiteral("AutoCAD DXF 2010 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1027_AAMA:
            return QStringLiteral("AutoCAD DXF 2013 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1006_ASTM:
            return QStringLiteral("AutoCAD DXF R10 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1009_ASTM:
            return QStringLiteral("AutoCAD DXF R11/12 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1012_ASTM:
            return QStringLiteral("AutoCAD DXF R13 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1014_ASTM:
            return QStringLiteral("AutoCAD DXF R14 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1015_ASTM:
            return QStringLiteral("AutoCAD DXF 2000 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1018_ASTM:
            return QStringLiteral("AutoCAD DXF 2004 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1021_ASTM:
            return QStringLiteral("AutoCAD DXF 2007 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1024_ASTM:
            return QStringLiteral("AutoCAD DXF 2010 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1027_ASTM:
            return QStringLiteral("AutoCAD DXF 2013 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::PDFTiled:
            return QStringLiteral("PDF %1 %2 (*.pdf)").arg(tr("tiled"), filesStr);
        case LayoutExportFormats::NC:
            return QStringLiteral("%1 %2 (*.nc)").arg(tr("Numerical control"), filesStr);
        case LayoutExportFormats::RLD:
            return QStringLiteral("%1 %2 (*.rld)").arg(tr("Raw Layout Data"), filesStr);
        case LayoutExportFormats::TIF:
            return QStringLiteral("TIFF %1 (*.tif)").arg(filesStr);
        default:
            return QString();
    }
}

// FIXME Bad copy paste from DialogSaveLayout, because I didn't know how to call this function from here
// to be removed as soon as I know how to call the central function from valentina
//---------------------------------------------------------------------------------------------------------------------
QString VPExporter::ExportFormatSuffix(LayoutExportFormats format)
{
    switch(format)
    {
        case LayoutExportFormats::SVG:
            return QStringLiteral(".svg");
        case LayoutExportFormats::PDF:
        case LayoutExportFormats::PDFTiled:
            return QStringLiteral(".pdf");
        case LayoutExportFormats::PNG:
            return QStringLiteral(".png");
        case LayoutExportFormats::OBJ:
            return QStringLiteral(".obj");
        case LayoutExportFormats::PS:
            return QStringLiteral(".ps");
        case LayoutExportFormats::EPS:
            return QStringLiteral(".eps");
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
            return QStringLiteral(".dxf");
        case LayoutExportFormats::NC:
            return QStringLiteral(".nc");
        case LayoutExportFormats::RLD:
            return QStringLiteral(".rld");
        case LayoutExportFormats::TIF:
            return QStringLiteral(".tif");
        default:
            return QString();
    }
}


// FIXME Bad copy paste from DialogSaveLayout, because I didn't know how to call this function from here
// to be removed as soon as I know how to call the central function from valentina
//---------------------------------------------------------------------------------------------------------------------
QVector<std::pair<QString, LayoutExportFormats> > VPExporter::InitFormats()
{
    QVector<std::pair<QString, LayoutExportFormats>> list;

    auto InitFormat = [&list](LayoutExportFormats format)
    {
        list.append(std::make_pair(ExportFormatDescription(format), format));
    };

    InitFormat(LayoutExportFormats::SVG);
    InitFormat(LayoutExportFormats::PDF);
//    InitFormat(LayoutExportFormats::PNG);

//    InitFormat(LayoutExportFormats::OBJ);

//    InitFormat(LayoutExportFormats::PS);
//    InitFormat(LayoutExportFormats::EPS);

//    InitFormat(LayoutExportFormats::DXF_AC1006_Flat);
//    InitFormat(LayoutExportFormats::DXF_AC1009_Flat);
//    InitFormat(LayoutExportFormats::DXF_AC1012_Flat);
//    InitFormat(LayoutExportFormats::DXF_AC1014_Flat);
//    InitFormat(LayoutExportFormats::DXF_AC1015_Flat);
//    InitFormat(LayoutExportFormats::DXF_AC1018_Flat);
//    InitFormat(LayoutExportFormats::DXF_AC1021_Flat);
//    InitFormat(LayoutExportFormats::DXF_AC1024_Flat);
//    InitFormat(LayoutExportFormats::DXF_AC1027_Flat);
//    InitFormat(LayoutExportFormats::DXF_AC1006_AAMA);
//    InitFormat(LayoutExportFormats::DXF_AC1009_AAMA);
//    InitFormat(LayoutExportFormats::DXF_AC1012_AAMA);
//    InitFormat(LayoutExportFormats::DXF_AC1014_AAMA);
//    InitFormat(LayoutExportFormats::DXF_AC1015_AAMA);
//    InitFormat(LayoutExportFormats::DXF_AC1018_AAMA);
//    InitFormat(LayoutExportFormats::DXF_AC1021_AAMA);
//    InitFormat(LayoutExportFormats::DXF_AC1024_AAMA);
//    InitFormat(LayoutExportFormats::DXF_AC1027_AAMA);
//    InitFormat(LayoutExportFormats::DXF_AC1006_ASTM);
//    InitFormat(LayoutExportFormats::DXF_AC1009_ASTM);
//    InitFormat(LayoutExportFormats::DXF_AC1012_ASTM);
//    InitFormat(LayoutExportFormats::DXF_AC1014_ASTM);
//    InitFormat(LayoutExportFormats::DXF_AC1015_ASTM);
//    InitFormat(LayoutExportFormats::DXF_AC1018_ASTM);
//    InitFormat(LayoutExportFormats::DXF_AC1021_ASTM);
//    InitFormat(LayoutExportFormats::DXF_AC1024_ASTM);
//    InitFormat(LayoutExportFormats::DXF_AC1027_ASTM);
//    InitFormat(LayoutExportFormats::PDFTiled);
//    InitFormat(LayoutExportFormats::NC);
//    InitFormat(LayoutExportFormats::RLD);
    InitFormat(LayoutExportFormats::TIF);

    return list;
}
