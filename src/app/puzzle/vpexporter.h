#ifndef VPEXPORTER_H
#define VPEXPORTER_H

#include <QObject>

#include "vplayout.h"
#include "../vmisc/def.h"
#include "vcommonsettings.h"
#include "../vlayout/vlayoutdef.h"
#include "../vlayout/vlayoutexporter.h"

//#include "../dialogs/dialogsavelayout.h"

class VPMainGraphicsView;

class VPExporter : VLayoutExporter
{
public:
    VPExporter();
    ~VPExporter();

    void Export(VPLayout* layout, LayoutExportFormats format, VPMainGraphicsView *mainGraphicsView);


    // FIXME Bad copy paste from DialogSaveLayout, because I didn't know how to call this function from here
    // to be removed as soon as I know how to call the central function from valentina
    QVector<std::pair<QString, LayoutExportFormats> > InitFormats();

private:



    // FIXME Bad copy paste from DialogSaveLayout, because I didn't know how to call this function from here
    // to be removed as soon as I know how to call the central function from valentina
    static QString ExportFormatDescription(LayoutExportFormats format);
    static QString ExportFormatSuffix(LayoutExportFormats format);


};

#endif // VPEXPORTER_H
