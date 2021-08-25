#include "vpexporter.h"

#include <QFileDialog>
#include <QMessageBox>

#include "../vwidgets/vmaingraphicsscene.h"
#include "layout/vpsheet.h"
#include "scene/vpmaingraphicsview.h"
#include "../vobj/vobjpaintdevice.h"
#include "../vdxf/vdxfpaintdevice.h"


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

        QSizeF size = QSizeF(layout->GetFocusedSheet()->GetLayout()->LayoutSettings().GetSheetSize());
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

            case LayoutExportFormats::OBJ:
                ExportToOBJ(mainGraphicsView->scene());
            break;

            case LayoutExportFormats::PS:
                ExportToPS(mainGraphicsView->scene());
            break;

            case LayoutExportFormats::EPS:
                ExportToEPS(mainGraphicsView->scene());
            break;

            default:
                // just for test purpuses, to be removed:
                QMessageBox msgBox;
                msgBox.setText(QString("TODO VPExporter, Handling for format %1 is still missing.").arg(ExportFormatDescription(format)));
                int ret = msgBox.exec();
                Q_UNUSED(ret);
            break;
        }

        mainGraphicsView->CleanAfterExport();
    }
}

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
    InitFormat(LayoutExportFormats::TIF);

    InitFormat(LayoutExportFormats::OBJ);

    InitFormat(LayoutExportFormats::PS);
    InitFormat(LayoutExportFormats::EPS);

    InitFormat(LayoutExportFormats::DXF_AC1006_Flat);
/*    InitFormat(LayoutExportFormats::DXF_AC1009_Flat);
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
    InitFormat(LayoutExportFormats::DXF_AC1027_ASTM);*/

//    InitFormat(LayoutExportFormats::PDFTiled);
//    InitFormat(LayoutExportFormats::NC);
//    InitFormat(LayoutExportFormats::RLD);

    return list;
}
