/************************************************************************
 **
 **  @file   vknownmeasurementsdocument.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   27 10, 2023
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
#ifndef VKNOWNMEASUREMENTSDOCUMENT_H
#define VKNOWNMEASUREMENTSDOCUMENT_H

#include "../ifc/xml/vdomdocument.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../vmisc/defglobal.h"
#endif

class VKnownMeasurements;
class VPatternImage;

class VKnownMeasurementsDocument : public VDomDocument
{
    Q_OBJECT // NOLINT

public:
    explicit VKnownMeasurementsDocument(QObject *parent = nullptr);
    ~VKnownMeasurementsDocument() override = default;

    auto SaveDocument(const QString &fileName, QString &error) -> bool override;

    void CreateEmptyFile();

    void AddEmptyMeasurement(const QString &name);
    void AddEmptyMeasurementAfter(const QString &after, const QString &name);
    void AddImage(const VPatternImage &image);
    void RemoveMeasurement(const QString &name);
    void RemoveImage(const QUuid &id);
    void MoveTop(const QString &name);
    void MoveUp(const QString &name);
    void MoveDown(const QString &name);
    void MoveBottom(const QString &name);

    auto GetUId() const -> QUuid;
    void SetUId(const QUuid &id);

    auto Name() const -> QString;
    void SetName(const QString &name);

    auto Description() const -> QString;
    void SetDescription(const QString &desc);

    auto IsReadOnly() const -> bool;
    void SetReadOnly(bool ro);

    auto KnownMeasurements() const -> VKnownMeasurements;

    void SetMName(const QString &name, const QString &text);
    void SetMFormula(const QString &name, const QString &text);
    void SetMSpecialUnits(const QString &name, bool special);
    void SetMDescription(const QString &name, const QString &text);
    void SetMFullName(const QString &name, const QString &text);
    void SetMImage(const QString &name, const QUuid &imageId);

    void SetImageContent(const QUuid &id, const VPatternImage &image);
    void SetImageTitle(const QUuid &id, const QString &text);

private:
    Q_DISABLE_COPY_MOVE(VKnownMeasurementsDocument) // NOLINT

    auto MakeEmptyMeasurement(const QString &name) -> QDomElement;
    auto FindM(const QString &name) const -> QDomElement;
    auto MakeEmptyImage(const VPatternImage &image) -> QDomElement;
    auto FindImage(const QUuid &id) const -> QDomElement;

    void ReadImages(VKnownMeasurements &known) const;
    void ReadMeasurements(VKnownMeasurements &known) const;
};

#endif // VKNOWNMEASUREMENTSDOCUMENT_H
