/************************************************************************
 **
 **  @file   vdxfpaintdevice.h
 **  @author Valentina Zhuravska <zhuravska19(at)gmail.com>
 **  @date   12 812, 2015
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

#ifndef VDXFPAINTDEVICE_H
#define VDXFPAINTDEVICE_H

#include <QPaintDevice>
#include <QSize>
#include <QString>
#include <QtGlobal>

#include "dxfdef.h"
#include "libdxfrw/drw_base.h"

class VDxfEngine;
class VLayoutPiece;

class VDxfPaintDevice : public QPaintDevice
{
public:
    VDxfPaintDevice();
    ~VDxfPaintDevice() override;

    auto paintEngine() const -> QPaintEngine * override;

    auto GetFileName() const -> QString;
    void SetFileName(const QString &value);

    auto GetSize() -> QSize;
    void SetSize(const QSize &size);

    auto GetResolution() const -> double;
    void SetResolution(double dpi);

    auto GetVersion() const -> DRW::Version;
    void SetVersion(DRW::Version version);

    void SetBinaryFormat(bool binary);
    auto IsBinaryFromat() const -> bool;

    void SetMeasurement(const VarMeasurement &var);
    void SetInsunits(const VarInsunits &var);

    auto GetXScale() const -> qreal;
    void SetXScale(const qreal &xscale);

    auto GetYScale() const -> qreal;
    void SetYScale(const qreal &yscale);

    void SetBoundaryTogetherWithNotches(bool value);
    auto IsBoundaryTogetherWithNotches() const -> bool;

    auto DxfApparelCompatibility() const -> DXFApparelCompatibility;
    void SetDxfApparelCompatibility(DXFApparelCompatibility mode);

    auto ExportToAAMA(const QVector<VLayoutPiece> &details) const -> bool;
    auto ExportToASTM(const QVector<VLayoutPiece> &details) const -> bool;

    auto ErrorString() const -> QString;

protected:
    auto metric(PaintDeviceMetric metric) const -> int override;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VDxfPaintDevice) // NOLINT
    VDxfEngine *m_engine;
    QString m_fileName{};
};

#endif // VDXFPAINTDEVICE_H
