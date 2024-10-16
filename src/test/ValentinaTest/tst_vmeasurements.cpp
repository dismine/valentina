/************************************************************************
 **
 **  @file   tst_vmeasurements.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 10, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#include "tst_vmeasurements.h"
#include "../ifc/xml/vvitconverter.h"
#include "../ifc/xml/vvstconverter.h"
#include "../vformat/vmeasurements.h"
#include "../vpatterndb/vcontainer.h"

#include <QtTest>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
TST_VMeasurements::TST_VMeasurements(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief CreateEmptyMultisizeFile check if empty multisize measurement file is valid.
 */
void TST_VMeasurements::CreateEmptyMultisizeFile()
{
    Unit const mUnit = Unit::Cm;

    auto const data = QSharedPointer<VContainer>(new VContainer(nullptr, &mUnit, VContainer::UniqueNamespace()));
    auto m_xDimension = QSharedPointer<VXMeasurementDimension>::create(mUnit, 50, 200, 6);
    m_xDimension->SetBaseValue(176);

    auto m_yDimension = QSharedPointer<VYMeasurementDimension>::create(mUnit, 22, 72, 2);
    m_yDimension->SetBaseValue(50);
    m_yDimension->SetBodyMeasurement(true);

    QVector<MeasurementDimension_p> const dimensions{m_xDimension, m_yDimension};

    auto const m = QSharedPointer<VMeasurements>(new VMeasurements(mUnit, dimensions, data.data()));

    QTemporaryFile file;
    QString fileName;
    // In Windows we have problems when we try to open QSaveFile when QTemporaryFile with the same name is already open.
    if (file.open())
    {
        // So, before we try to open file in m->SaveDocument function we need to close it and remove.
        // Just closing - is not enough, if we just close QTemporaryFile we get "access denied" in Windows.
        fileName = file.fileName();
        file.close();
        file.remove();
        QString error;
        const bool result = m->SaveDocument(fileName, error);

        QVERIFY2(result, error.toUtf8().constData());
    }
    else
    {
        QFAIL("Can't open temporary file.");
    }

    try
    {
        VVSTConverter const converter(fileName);
    }
    catch (VException &e)
    {
        QFAIL(e.ErrorMessage().toUtf8().constData());
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief CreateEmptyIndividualFile check if empty individual measurement file is valid.
 */
void TST_VMeasurements::CreateEmptyIndividualFile()
{
    Unit const mUnit = Unit::Cm;

    auto const data = QSharedPointer<VContainer>(new VContainer(nullptr, &mUnit, VContainer::UniqueNamespace()));

    auto const m = QSharedPointer<VMeasurements>(new VMeasurements(mUnit, data.data()));

    QTemporaryFile file;
    QString fileName;
    if (file.open())
    {
        fileName = file.fileName();
        file.close();
        file.remove();
        QString error;
        const bool result = m->SaveDocument(fileName, error);

        QVERIFY2(result, error.toUtf8().constData());
    }
    else
    {
        QFAIL("Can't open temporary file.");
    }

    try
    {
        VVITConverter const converter(fileName);
    }
    catch (VException &e)
    {
        QFAIL(e.ErrorMessage().toUtf8().constData());
    }
}
