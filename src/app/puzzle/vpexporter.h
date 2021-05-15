#ifndef VPEXPORTER_H
#define VPEXPORTER_H

#include <QObject>

#include "vplayout.h"
#include "../vmisc/def.h"
#include "vcommonsettings.h"
#include "../vlayout/vlayoutdef.h"

//#include "../dialogs/dialogsavelayout.h"

class VPMainGraphicsView;

class VPExporter : QObject
{
    Q_OBJECT

public:
    VPExporter(VPLayout* layout, VCommonSettings *commonSettings);
    ~VPExporter();

    void Export(LayoutExportFormats format, VPMainGraphicsView *mainGraphicsView);


    // FIXME Bad copy paste from DialogSaveLayout, because I didn't know how to call this function from here
    // to be removed as soon as I know how to call the central function from valentina
    QVector<std::pair<QString, LayoutExportFormats> > InitFormats();

private:
    Q_DISABLE_COPY(VPExporter)

    VPLayout *m_layout{nullptr};
    VCommonSettings *m_commonSettings{nullptr};

    /**
     * @brief ExportSVG exports the current maingraphicViews to an svg file
     * @param fileName the file name of the export
     */
    void ExportToSVG(QString fileName, VPMainGraphicsView *mainGraphicsView);

    /**
     * @brief ExportSVG exports the current maingraphicViews to a pdf file
     * @param fileName the file name of the export
     */
    void ExportToPDF(QString filename, VPMainGraphicsView *mainGraphicsView);

    /**
     * @brief ExportSVG exports the current maingraphicViews to a png file
     * @param fileName the file name of the export
     */
    void ExportToPNG(QString filename, VPMainGraphicsView *mainGraphicsView);

    /**
     * @brief ExportSVG exports the current maingraphicViews to a TIF file
     * @param fileName the file name of the export
     */
    void ExportToTIF(QString filename, VPMainGraphicsView *mainGraphicsView);


    // FIXME Bad copy paste from DialogSaveLayout, because I didn't know how to call this function from here
    // to be removed as soon as I know how to call the central function from valentina
    static QString ExportFormatDescription(LayoutExportFormats format);
    static QString ExportFormatSuffix(LayoutExportFormats format);


};

#endif // VPEXPORTER_H
