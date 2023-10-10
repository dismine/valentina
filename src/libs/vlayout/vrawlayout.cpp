/************************************************************************
 **
 **  @file   vrawlayout.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 4, 2020
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
 *************************************************************************/
#include "vrawlayout.h"

#include <QDataStream>
#include <QDebug>
#include <QFile>
#include <QIODevice>

#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
#include "../vmisc/backport/qscopeguard.h"
#else
#include <QScopeGuard>
#endif

#include "../ifc/exception/vexception.h"
#include "../vmisc/def.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

const QByteArray VRawLayout::fileHeaderByteArray = "RLD!..."_ba;
const quint16 VRawLayout::fileVersion = 1;

const quint32 VRawLayoutData::streamHeader = 0x8B0E8A27; // CRC-32Q string "VRawLayoutData"
const quint16 VRawLayoutData::classVersion = 1;

//---------------------------------------------------------------------------------------------------------------------
auto operator<<(QDataStream &dataStream, const VRawLayoutData &data) -> QDataStream &
{
    dataStream << VRawLayoutData::streamHeader << VRawLayoutData::classVersion;

    // Added in classVersion = 1
    dataStream << data.pieces;

    // Added in classVersion = 2

    return dataStream;
}

//---------------------------------------------------------------------------------------------------------------------
auto operator>>(QDataStream &dataStream, VRawLayoutData &data) -> QDataStream &
{
    quint32 actualStreamHeader = 0;
    dataStream >> actualStreamHeader;

    if (actualStreamHeader != VRawLayoutData::streamHeader)
    {
        QString message = QCoreApplication::tr("VRawLayoutData prefix mismatch error: actualStreamHeader = 0x%1 and "
                                               "streamHeader = 0x%2")
                              .arg(actualStreamHeader, 8, 0x10, '0'_L1)
                              .arg(VRawLayoutData::streamHeader, 8, 0x10, '0'_L1);
        throw VException(message);
    }

    quint16 actualClassVersion = 0;
    dataStream >> actualClassVersion;

    if (actualClassVersion > VRawLayoutData::classVersion)
    {
        QString message = QCoreApplication::tr("VRawLayoutData compatibility error: actualClassVersion = %1 and "
                                               "classVersion = %2")
                              .arg(actualClassVersion)
                              .arg(VRawLayoutData::classVersion);
        throw VException(message);
    }

    dataStream >> data.pieces;

    //    if (actualClassVersion >= 2)
    //    {
    //        // read value in version 2
    //    }

    return dataStream;
}

//---------------------------------------------------------------------------------------------------------------------
auto VRawLayout::WriteFile(QIODevice *ioDevice, const VRawLayoutData &data) -> bool
{
    SCASSERT(ioDevice != nullptr)
    m_errorString.clear();

    const bool wasOpen = ioDevice->isOpen();

    if (wasOpen || ioDevice->open(QIODevice::WriteOnly))
    {
        QDataStream dataStream(ioDevice);
        dataStream.setVersion(QDataStream::Qt_5_6);

        // Don't use the << operator for QByteArray. See the note in ReadFile() below.
        dataStream.writeRawData(fileHeaderByteArray.constData(), static_cast<int>(fileHeaderByteArray.size()));
        dataStream << fileVersion;
        dataStream << data;

        if (not wasOpen)
        {
            ioDevice->close(); // Only close this if it was opened by this function.
        }
        return true;
    }

    m_errorString = ioDevice->errorString();
    return false;

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VRawLayout::ReadFile(QIODevice *ioDevice, VRawLayoutData &data) -> bool
{
    SCASSERT(ioDevice != nullptr)
    m_errorString.clear();

    const bool wasOpen = ioDevice->isOpen();

    if (wasOpen || ioDevice->open(QIODevice::ReadOnly))
    {
        auto CloseFile = qScopeGuard(
            [wasOpen, ioDevice]()
            {
                if (not wasOpen) // Only close this if it was opened by this function.
                {
                    ioDevice->close();
                }
            });

        QDataStream dataStream(ioDevice);
        dataStream.setVersion(QDataStream::Qt_5_6);

        // Note: we could have used the QDataStream << and >> operators on QByteArray but since the first
        // bytes of the stream will be the size of the array, we might end up attempting to allocate
        // a large amount of memory if the wrong file type was read. Instead, we'll just read the
        // same number of bytes that are in the array we are comparing it to. No size was written.
        const int len = static_cast<int>(fileHeaderByteArray.size());
        QByteArray actualFileHeaderByteArray(len, '\0');
        dataStream.readRawData(actualFileHeaderByteArray.data(), len);

        if (actualFileHeaderByteArray != fileHeaderByteArray)
        {
            // prefixes don't match
            m_errorString = tr("VRawLayout::ReadFile() failed. Raw layout format prefix mismatch error.");
            return false;
        }

        quint16 actualFileVersion = 0;
        dataStream >> actualFileVersion;

        if (actualFileVersion > fileVersion)
        {
            // file is from a future version that we don't know how to load
            m_errorString = tr("VRawLayout::ReadFile() failed.\n"
                               "Raw layout format compatibility error: actualFileVersion = %1 and fileVersion = %2")
                                .arg(actualFileVersion)
                                .arg(fileVersion);
            return false;
        }

        try
        {
            // This may throw an exception if one of the VRawLayoutData objects is corrupt or unsupported.
            // For example, if this file is from a future version of this code.
            dataStream >> data;
        }
        catch (const VException &e)
        {
            qCritical() << e.ErrorMessage();

            m_errorString = e.ErrorMessage();
            return false;
        }

        return true;
    }

    m_errorString = ioDevice->errorString();
    return false;

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VRawLayout::WriteFile(const QString &filePath, const VRawLayoutData &data) -> bool
{
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        auto CloseFile = qScopeGuard(
            [&file]()
            {
                file.flush();
                file.close();
            });
        return WriteFile(&file, data);
    }

    m_errorString = file.errorString();
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto VRawLayout::ReadFile(const QString &filePath, VRawLayoutData &data) -> bool
{
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly))
    {
        auto CloseFile = qScopeGuard(
            [&file]()
            {
                file.flush();
                file.close();
            });
        return ReadFile(&file, data);
    }

    m_errorString = file.errorString();
    return false;
}
