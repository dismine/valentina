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

#include <QLocale>
#include <QXmlStreamWriter>

#include "../vmisc/literals.h"

class VPuzzleLayout;
class VPuzzleLayer;
class VPuzzlePiece;
class QFile;
class QMarginsF;

class VPuzzleLayoutFileWriter : public QXmlStreamWriter
{
public:
    VPuzzleLayoutFileWriter();
    ~VPuzzleLayoutFileWriter();

    void WriteFile(VPuzzleLayout *layout, QFile *file);

private:

    void WriteLayout(VPuzzleLayout *layout);
    void WriteProperties(VPuzzleLayout *layout);
    void WriteTiles(VPuzzleLayout *layout);
    void WriteLayers(VPuzzleLayout *layout);
    void WriteLayer(VPuzzleLayer *layer);
    void WriteLayer(VPuzzleLayer *layer, const QString &tagName);
    void WritePiece(VPuzzlePiece *piece);

    void WriteMargins(const QMarginsF &margins);
    void WriteSize(QSizeF size);

    template <typename T>
    void SetAttribute(const QString &name, const T &value);

    template <size_t N>
    void SetAttribute(const QString &name, const char (&value)[N]);

};

//---------------------------------------------------------------------------------------------------------------------
template<typename T>
void VPuzzleLayoutFileWriter::SetAttribute(const QString &name, const T &value)
{
    // See specification for xs:decimal
    const QLocale locale = QLocale::c();
    writeAttribute(name, locale.toString(value).remove(locale.groupSeparator()));
}

//---------------------------------------------------------------------------------------------------------------------
template <>
inline void VPuzzleLayoutFileWriter::SetAttribute<QString>(const QString &name, const QString &value)
{
    writeAttribute(name, value);
}

//---------------------------------------------------------------------------------------------------------------------
template <>
inline void VPuzzleLayoutFileWriter::SetAttribute<QChar>(const QString &name, const QChar &value)
{
    writeAttribute(name, value);
}

//---------------------------------------------------------------------------------------------------------------------
template <>
inline void VPuzzleLayoutFileWriter::SetAttribute<bool>(const QString &name, const bool &value)
{
    writeAttribute(name, value ? trueStr : falseStr);
}

//---------------------------------------------------------------------------------------------------------------------
template <size_t N>
inline void VPuzzleLayoutFileWriter::SetAttribute(const QString &name, const char (&value)[N])
{
    writeAttribute(name, QString(value));
}

#endif // VPUZZLELAYOUTFILEWRITER_H
