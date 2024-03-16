/************************************************************************
 **
 **  @file   vdxfpaintdevice.cpp
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

#include "vdxfpaintdevice.h"

#include <QMessageLogger>
#include <QtDebug>

#include "vdxfengine.h"

//---------------------------------------------------------------------------------------------------------------------
VDxfPaintDevice::VDxfPaintDevice()
  : m_engine(new VDxfEngine())
{
}

//---------------------------------------------------------------------------------------------------------------------
VDxfPaintDevice::~VDxfPaintDevice()
{
    delete m_engine;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
auto VDxfPaintDevice::paintEngine() const -> QPaintEngine *
{
    return m_engine;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
auto VDxfPaintDevice::GetFileName() const -> QString
{
    return m_fileName;
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfPaintDevice::SetFileName(const QString &value)
{
    if (m_engine->isActive())
    {
        qWarning("VDxfPaintDevice::setFileName(), cannot set file name while Dxf is being generated");
        return;
    }

    m_fileName = value;
    m_engine->SetFileName(m_fileName);
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfPaintDevice::GetSize() -> QSize
{
    return m_engine->GetSize();
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfPaintDevice::SetSize(const QSize &size)
{
    if (m_engine->isActive())
    {
        qWarning("VDxfPaintDevice::setSize(), cannot set size while Dxf is being generated");
        return;
    }
    m_engine->SetSize(size);
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfPaintDevice::GetResolution() const -> double
{
    return m_engine->GetResolution();
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfPaintDevice::SetResolution(double dpi)
{
    if (m_engine->isActive())
    {
        qWarning("VDxfPaintDevice::setResolution(), cannot set dpi while Dxf is being generated");
        return;
    }
    m_engine->SetResolution(dpi);
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfPaintDevice::GetVersion() const -> DRW::Version
{
    return m_engine->GetVersion();
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfPaintDevice::SetVersion(DRW::Version version)
{
    if (m_engine->isActive())
    {
        qWarning("VDxfPaintDevice::SetVersion(), cannot set version while Dxf is being generated");
        return;
    }
    m_engine->SetVersion(version);
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfPaintDevice::SetBinaryFormat(bool binary)
{
    if (m_engine->isActive())
    {
        qWarning("VDxfPaintDevice::SetBinaryFormat(), cannot set binary format while Dxf is being generated");
        return;
    }
    m_engine->SetBinaryFormat(binary);
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfPaintDevice::IsBinaryFromat() const -> bool
{
    return m_engine->IsBinaryFormat();
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfPaintDevice::SetMeasurement(const VarMeasurement &var)
{
    if (m_engine->isActive())
    {
        qWarning("VDxfPaintDevice::setMeasurement(), cannot set measurements while Dxf is being generated");
        return;
    }
    m_engine->SetMeasurement(var);
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfPaintDevice::SetInsunits(const VarInsunits &var)
{
    if (m_engine->isActive())
    {
        qWarning("VDxfPaintDevice::setInsunits(), cannot set units while Dxf is being generated");
        return;
    }
    m_engine->SetInsunits(var);
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfPaintDevice::GetXScale() const -> qreal
{
    return m_engine->GetYScale();
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfPaintDevice::SetXScale(const qreal &xscale)
{
    if (m_engine->isActive())
    {
        qWarning("VDxfPaintDevice::SetXScale(), cannot set x scale while Dxf is being generated");
        return;
    }
    m_engine->SetXScale(xscale);
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfPaintDevice::GetYScale() const -> qreal
{
    return m_engine->GetXScale();
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfPaintDevice::SetYScale(const qreal &yscale)
{
    if (m_engine->isActive())
    {
        qWarning("VDxfPaintDevice::SetYScale(), cannot set y scale while Dxf is being generated");
        return;
    }
    m_engine->SetYScale(yscale);
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfPaintDevice::SetBoundaryTogetherWithNotches(bool value)
{
    if (m_engine->isActive())
    {
        qWarning("VDxfPaintDevice::SetBoundaryTogetherWithNotches(), cannot set boundary together with notches while "
                 "Dxf is being generated");
        return;
    }
    m_engine->SetBoundaryTogetherWithNotches(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfPaintDevice::IsBoundaryTogetherWithNotches() const -> bool
{
    return m_engine->IsBoundaryTogetherWithNotches();
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfPaintDevice::DxfApparelCompatibility() const -> DXFApparelCompatibility
{
    return m_engine->DxfApparelCompatibility();
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfPaintDevice::SetDxfApparelCompatibility(DXFApparelCompatibility mode)
{
    if (m_engine->isActive())
    {
        qWarning("VDxfPaintDevice::SetDxfApparelCompatibility(), cannot set compatibility mode while Dxf is being "
                 "generated");
        return;
    }
    m_engine->SetDxfApparelCompatibility(mode);
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfPaintDevice::ExportToAAMA(const QVector<VLayoutPiece> &details) const -> bool
{
    m_engine->setActive(true);
    const bool res = m_engine->ExportToAAMA(details);
    m_engine->setActive(false);
    return res;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfPaintDevice::ExportToASTM(const QVector<VLayoutPiece> &details) const -> bool
{
    m_engine->setActive(true);
    const bool res = m_engine->ExportToASTM(details);
    m_engine->setActive(false);
    return res;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfPaintDevice::ErrorString() const -> QString
{
    return m_engine->ErrorString();
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfPaintDevice::metric(QPaintDevice::PaintDeviceMetric metric) const -> int
{
    switch (metric)
    {
        case QPaintDevice::PdmDepth:
            return 32;
        case QPaintDevice::PdmWidth:
            return m_engine->GetSize().width();
        case QPaintDevice::PdmHeight:
            return m_engine->GetSize().height();
        case QPaintDevice::PdmHeightMM:
            return qRound(m_engine->GetSize().height() * 25.4 / m_engine->GetResolution());
        case QPaintDevice::PdmWidthMM:
            return qRound(m_engine->GetSize().width() * 25.4 / m_engine->GetResolution());
        case QPaintDevice::PdmNumColors:
            return static_cast<int>(0xffffffff);
        case QPaintDevice::PdmPhysicalDpiX:
        case QPaintDevice::PdmPhysicalDpiY:
        case QPaintDevice::PdmDpiX:
        case QPaintDevice::PdmDpiY:
            return static_cast<int>(m_engine->GetResolution());
        case QPaintDevice::PdmDevicePixelRatio:
        case QPaintDevice::PdmDevicePixelRatioScaled:
            return 1;
        default:
            qWarning("VDxfPaintDevice::metric(), unhandled metric %d\n", metric);
            break;
    }
    return 0;
}
