/************************************************************************
 **
 **  @file   vhpglpaintdevice.cpp
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

#include "vhpglpaintdevice.h"
#include "vhpglengine.h"

//---------------------------------------------------------------------------------------------------------------------
VHPGLPaintDevice::VHPGLPaintDevice()
  : m_engine(new VHPGLEngine())
{
}

//---------------------------------------------------------------------------------------------------------------------
VHPGLPaintDevice::~VHPGLPaintDevice()
{
    delete m_engine;
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLPaintDevice::SetFileName(const QString &filename)
{
    if (m_engine->isActive())
    {
        qWarning("VHPGLPaintDevice::SetFileName(), cannot set file name while HPGL is being generated");
        return;
    }

    m_engine->SetFileName(filename);
}

//---------------------------------------------------------------------------------------------------------------------
auto VHPGLPaintDevice::GetSingleLineFont() const -> bool
{
    return m_engine->GetSingleLineFont();
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLPaintDevice::SetSingleLineFont(bool singleLineFont)
{
    if (m_engine->isActive())
    {
        qWarning(
            "VHPGLPaintDevice::SetSingleLineFont(), cannot set single line font mode while HPGL is being generated");
        return;
    }
    m_engine->SetSingleLineFont(singleLineFont);
}

//---------------------------------------------------------------------------------------------------------------------
auto VHPGLPaintDevice::GetSingleStrokeOutlineFont() const -> bool
{
    return m_engine->GetSingleStrokeOutlineFont();
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLPaintDevice::SetSingleStrokeOutlineFont(bool singleStrokeOutlineFont)
{
    if (m_engine->isActive())
    {
        qWarning("VHPGLPaintDevice::SetSingleLineFont(), cannot set single stroke outline font mode while HPGL is "
                 "being generated");
        return;
    }
    m_engine->SetSingleStrokeOutlineFont(singleStrokeOutlineFont);
}

//---------------------------------------------------------------------------------------------------------------------
auto VHPGLPaintDevice::GetPenWidth() const -> int
{
    return m_engine->GetPenWidth();
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLPaintDevice::SetPenWidth(int newPenWidth)
{
    if (m_engine->isActive())
    {
        qWarning("VHPGLPaintDevice::SetSingleLineFont(), cannot set pen width while HPGL is "
                 "being generated");
        return;
    }
    m_engine->SetPenWidth(newPenWidth);
}

//---------------------------------------------------------------------------------------------------------------------
auto VHPGLPaintDevice::ExportToHPGL(const QVector<VLayoutPiece> &details) const -> bool
{
    m_engine->setActive(true);
    const bool res = m_engine->GenerateHPGL(details);
    m_engine->setActive(false);
    return res;
}

//---------------------------------------------------------------------------------------------------------------------
auto VHPGLPaintDevice::ExportToHPGL2(const QVector<VLayoutPiece> &details) const -> bool
{
    m_engine->setActive(true);
    const bool res = m_engine->GenerateHPGL2(details);
    m_engine->setActive(false);
    return res;
}

//---------------------------------------------------------------------------------------------------------------------
auto VHPGLPaintDevice::GetFileName() const -> QString
{
    return m_engine->GetFileName();
}

//---------------------------------------------------------------------------------------------------------------------
auto VHPGLPaintDevice::GetSize() const -> QSize
{
    return m_engine->GetSize();
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLPaintDevice::SetSize(QSize size)
{
    if (m_engine->isActive())
    {
        qWarning("VHPGLPaintDevice::setSize(), cannot set size while HPGL is being generated");
        return;
    }
    m_engine->SetSize(size);
}

//---------------------------------------------------------------------------------------------------------------------
auto VHPGLPaintDevice::GetXScale() const -> qreal
{
    return m_engine->GetYScale();
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLPaintDevice::SetXScale(const qreal &xscale)
{
    if (m_engine->isActive())
    {
        qWarning("VHPGLPaintDevice::SetXScale(), cannot set x scale while HPGL is being generated");
        return;
    }
    m_engine->SetXScale(xscale);
}

//---------------------------------------------------------------------------------------------------------------------
auto VHPGLPaintDevice::GetYScale() const -> qreal
{
    return m_engine->GetXScale();
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLPaintDevice::SetYScale(const qreal &yscale)
{
    if (m_engine->isActive())
    {
        qWarning("VHPGLPaintDevice::SetYScale(), cannot set y scale while HPGL is being generated");
        return;
    }
    m_engine->SetYScale(yscale);
}

//---------------------------------------------------------------------------------------------------------------------
auto VHPGLPaintDevice::GetShowGrainline() const -> bool
{
    return m_engine->GetShowGrainline();
}

//---------------------------------------------------------------------------------------------------------------------
void VHPGLPaintDevice::SetShowGrainline(bool newShowGrainline)
{
    if (m_engine->isActive())
    {
        qWarning(
            "VHPGLPaintDevice::SetShowGrainline(), cannot control grainline visibility while HPGL is being generated");
        return;
    }
    m_engine->SetShowGrainline(newShowGrainline);
}
