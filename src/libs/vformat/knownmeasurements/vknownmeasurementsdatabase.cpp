/************************************************************************
 **
 **  @file   vknownmeasurementsdatabase.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 10, 2023
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
#include "vknownmeasurementsdatabase.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/xml/vknownmeasurementsconverter.h"
#include "../vformat/knownmeasurements/vknownmeasurements.h"
#include "../vformat/knownmeasurements/vknownmeasurementsdocument.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QMutex>

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

Q_GLOBAL_STATIC(QMutex, knownMeasurementsDatabaseMutex) // NOLINT

QT_WARNING_POP
} // namespace

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDatabase::PopulateMeasurementsDatabase()
{
    QMutexLocker locker(knownMeasurementsDatabaseMutex());

    m_measurementsCache.clear();

    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();

    m_measurementsDB.clear();
    m_populated = false;

    ParseDirectory(settings->GetPathKnownMeasurements());

    UpdateIndexes();
    m_populated = true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurementsDatabase::IsPopulated() const -> bool
{
    QMutexLocker locker(knownMeasurementsDatabaseMutex());
    return m_populated;
}

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurementsDatabase::AllKnownMeasurements() const -> QHash<QUuid, VKnownMeasurementsHeader>
{
    return m_measurementsDB;
}

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurementsDatabase::KnownMeasurements(const QUuid &id) const -> VKnownMeasurements
{
    if (id.isNull())
    {
        return {};
    }

    if (m_measurementsCache.contains(id))
    {
        return {*m_measurementsCache.object(id)};
    }

    QString measurementsFilePath = m_indexMeasurementsPath.value(id);
    if (measurementsFilePath.isEmpty())
    {
        return {};
    }

    try
    {
        VKnownMeasurementsConverter converter(measurementsFilePath);
        VKnownMeasurementsDocument measurements;
        measurements.setXMLContent(converter.Convert());

        VKnownMeasurements known = measurements.KnownMeasurements();
        m_measurementsCache.insert(id, new VKnownMeasurements(known));
        return known;
    }
    catch (VException &e)
    {
        qDebug("%s\n\n%s\n\n%s", qUtf8Printable("File error."), qUtf8Printable(e.ErrorMessage()),
               qUtf8Printable(e.DetailedInformation()));
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDatabase::UpdateIndexes()
{
    m_indexMeasurementsPath.clear();

    auto i = m_measurementsDB.constBegin();
    while (i != m_measurementsDB.constEnd())
    {
        m_indexMeasurementsPath.insert(i.key(), i.value().path);
        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDatabase::ParseDirectory(const QString &path)
{
    QDirIterator it(path, {"*.vkm"}, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        QString measurementsFilePath = it.next();

        try
        {
            VKnownMeasurementsConverter converter(measurementsFilePath);
            VKnownMeasurementsDocument measurements;
            measurements.setXMLContent(converter.Convert());

            VKnownMeasurementsHeader header;
            header.uid = measurements.GetUId();
            header.name = measurements.Name();
            header.description = measurements.Description();
            header.path = measurementsFilePath;

            m_measurementsDB.insert(header.uid, header);
        }
        catch (VException &e)
        {
            qDebug("%s\n\n%s\n\n%s", qUtf8Printable("File error."), qUtf8Printable(e.ErrorMessage()),
                   qUtf8Printable(e.DetailedInformation()));
        }
    }
}
