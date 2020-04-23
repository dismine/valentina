/************************************************************************
 **
 **  @file   vpuzzlelayoutfilewriter.h
 **  @author Ronan Le Tiec
 **  @date   18 4, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2020 Valentina project
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
 ** *************************************************************************/

#ifndef VPUZZLELAYOUTFILEWRITER_H
#define VPUZZLELAYOUTFILEWRITER_H

#include <QXmlStreamWriter>
#include "../ifc/xml/vabstractconverter.h"

class VPuzzleLayout;
class VPuzzleLayer;
class VPuzzlePiece;

class VPuzzleLayoutFileWriter : public QXmlStreamWriter
{
public:
    VPuzzleLayoutFileWriter();
    ~VPuzzleLayoutFileWriter();

    void WriteFile(VPuzzleLayout *layout, QFile *file);

    /*
     * Version rules:
     * 1. Version have three parts "major.minor.patch";
     * 2. major part only for stable releases;
     * 3. minor - 10 or more patch changes, or one big change;
     * 4. patch - little change.
     */
    /**
     * @brief LayoutFileFormatVersion holds the version
     *
     */
    static Q_DECL_CONSTEXPR const int LayoutFileFormatVersion = FORMAT_VERSION(1, 0, 0);
    const QString LayoutFileFormatVersionStr = QStringLiteral("1.0.0");

private:

    void WriteLayout(VPuzzleLayout *layout);
    void WriteProperties(VPuzzleLayout *layout);
    void WriteTiles(VPuzzleLayout *layout);
    void WriteLayers(VPuzzleLayout *layout);
    void WriteLayer(VPuzzleLayer *layer);
    void WriteLayer(VPuzzleLayer *layer, const QString &tagName);
    void WritePiece(VPuzzlePiece *piece);

    void WriteMargins(QMarginsF margins);
    void WriteSize(QSizeF size);


};

#endif // VPUZZLELAYOUTFILEWRITER_H
