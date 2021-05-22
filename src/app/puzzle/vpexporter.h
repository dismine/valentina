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

    /**
     * @brief InitFormats
     * @return returns the list of formats, that we can export in puzzle
     */
    QVector<std::pair<QString, LayoutExportFormats> > InitFormats();

};

#endif // VPEXPORTER_H
