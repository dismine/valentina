/************************************************************************
 **
 **  @file   vobjpaintdevice.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   6 12, 2014
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

#include "vobjpaintdevice.h"

#include <QFile>
#include <QIODevice>
#include <QMessageLogger>
#include <QtDebug>

#include "vobjengine.h"

//---------------------------------------------------------------------------------------------------------------------
VObjPaintDevice::VObjPaintDevice()
  : QPaintDevice(),
    engine(new VObjEngine()),
    fileName(),
    owns_iodevice(1)
{
    owns_iodevice = static_cast<int>(false);
}

//---------------------------------------------------------------------------------------------------------------------
VObjPaintDevice::~VObjPaintDevice()
{
    if (owns_iodevice)
    {
        delete engine->getOutputDevice();
    }
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
auto VObjPaintDevice::paintEngine() const -> QPaintEngine *
{
    return engine.data();
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
auto VObjPaintDevice::getFileName() const -> QString
{
    return fileName;
}

//---------------------------------------------------------------------------------------------------------------------
void VObjPaintDevice::setFileName(const QString &value)
{
    if (engine->isActive())
    {
        qWarning("VObjPaintDevice::setFileName(), cannot set file name while OBJ is being generated");
        return;
    }

    if (owns_iodevice)
    {
        delete engine->getOutputDevice();
    }

    owns_iodevice = static_cast<int>(true);

    fileName = value;
    auto *file = new QFile(fileName);
    engine->setOutputDevice(file);
}

//---------------------------------------------------------------------------------------------------------------------
auto VObjPaintDevice::getSize() -> QSize
{
    return engine->getSize();
}

//---------------------------------------------------------------------------------------------------------------------
void VObjPaintDevice::setSize(const QSize &size)
{
    if (engine->isActive())
    {
        qWarning("VObjPaintDevice::setSize(), cannot set size while OBJ is being generated");
        return;
    }
    engine->setSize(size);
}

//---------------------------------------------------------------------------------------------------------------------
auto VObjPaintDevice::getOutputDevice() -> QIODevice *
{
    return engine->getOutputDevice();
}

//---------------------------------------------------------------------------------------------------------------------
void VObjPaintDevice::setOutputDevice(QIODevice *outputDevice)
{
    if (engine->isActive())
    {
        qWarning("VObjPaintDevice::setOutputDevice(), cannot set output device while OBJ is being generated");
        return;
    }
    owns_iodevice = static_cast<int>(false);
    engine->setOutputDevice(outputDevice);
    fileName = QString();
}

//---------------------------------------------------------------------------------------------------------------------
auto VObjPaintDevice::getResolution() const -> int
{
    return engine->getResolution();
}

//---------------------------------------------------------------------------------------------------------------------
void VObjPaintDevice::setResolution(int dpi)
{
    engine->setResolution(dpi);
}

//---------------------------------------------------------------------------------------------------------------------
auto VObjPaintDevice::metric(QPaintDevice::PaintDeviceMetric metric) const -> int
{
    switch (metric)
    {
        case QPaintDevice::PdmDepth:
            return 32;
        case QPaintDevice::PdmWidth:
            return engine->getSize().width();
        case QPaintDevice::PdmHeight:
            return engine->getSize().height();
        case QPaintDevice::PdmHeightMM:
            return qRound(engine->getSize().height() * 25.4 / engine->getResolution());
        case QPaintDevice::PdmWidthMM:
            return qRound(engine->getSize().width() * 25.4 / engine->getResolution());
        case QPaintDevice::PdmNumColors:
            return static_cast<int>(0xffffffff);
        case QPaintDevice::PdmDpiX:
        case QPaintDevice::PdmDpiY:
        case QPaintDevice::PdmPhysicalDpiX:
        case QPaintDevice::PdmPhysicalDpiY:
            return engine->getResolution();
        case QPaintDevice::PdmDevicePixelRatio:
        case QPaintDevice::PdmDevicePixelRatioScaled:
            return 1;
        default:
            qWarning("VObjPaintDevice::metric(), unhandled metric %d\n", metric);
            break;
    }
    return 0;
}
