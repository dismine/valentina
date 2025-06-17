/************************************************************************
 **
 **  @file   vhpglpaintdevice.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   7 7, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#ifndef VHPGLPAINTDEVICE_H
#define VHPGLPAINTDEVICE_H

#include <QtGlobal>

class VHPGLEngine;
class QSize;
class VLayoutPiece;

class VHPGLPaintDevice
{
public:
    VHPGLPaintDevice();
    ~VHPGLPaintDevice();

    auto GetSize() const -> QSize;
    void SetSize(QSize size);

    auto GetFileName() const -> QString;
    void SetFileName(const QString &filename);

    auto GetSingleLineFont() const -> bool;
    void SetSingleLineFont(bool singleLineFont);

    auto GetSingleStrokeOutlineFont() const -> bool;
    void SetSingleStrokeOutlineFont(bool singleStrokeOutlineFont);

    auto GetPenWidth() const -> int;
    void SetPenWidth(int newPenWidth);

    auto GetXScale() const -> qreal;
    void SetXScale(const qreal &xscale);

    auto GetYScale() const -> qreal;
    void SetYScale(const qreal &yscale);

    auto GetShowGrainline() const -> bool;
    void SetShowGrainline(bool newShowGrainline);

    void SetBoundaryTogetherWithNotches(bool value);
    auto IsBoundaryTogetherWithNotches() const -> bool;

    auto ExportToHPGL(const QVector<VLayoutPiece> &details) const -> bool;
    auto ExportToHPGL2(const QVector<VLayoutPiece> &details) const -> bool;

private:
    Q_DISABLE_COPY_MOVE(VHPGLPaintDevice) // NOLINT
    VHPGLEngine *m_engine;
};

#endif // VHPGLPAINTDEVICE_H
