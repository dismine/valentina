/************************************************************************
 **
 **  @file   vabstractconverter.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 12, 2014
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

#ifndef VABSTRACTCONVERTER_H
#define VABSTRACTCONVERTER_H

#include <QCoreApplication>
#include <QString>
#include <QTemporaryFile>
#include <QtGlobal>

#include "vdomdocument.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")
QT_WARNING_DISABLE_GCC("-Wsuggest-final-methods")

class VAbstractConverter : public VDomDocument
{
    Q_OBJECT // NOLINT

public:
    explicit VAbstractConverter(const QString &fileName);
    ~VAbstractConverter() override = default;

    auto Convert() -> QString;

    auto GetCurrentFormatVersion() const -> unsigned;

protected:
    unsigned m_ver;
    QString m_originalFileName;
    QString m_convertedFileName;

    void ValidateInputFile(const QString &currentSchema) const;
    Q_NORETURN void InvalidVersion(unsigned ver) const;
    virtual void Save();
    virtual void SetVersion(const QString &version);

    virtual auto MinVer() const -> unsigned = 0;
    virtual auto MaxVer() const -> unsigned = 0;

    virtual auto MinVerStr() const -> QString = 0;
    virtual auto MaxVerStr() const -> QString = 0;

    virtual auto XSDSchema(unsigned ver) const -> QString;
    virtual void ApplyPatches() = 0;
    virtual void DowngradeToCurrentMaxVersion() = 0;

    virtual auto IsReadOnly() const -> bool = 0;

    virtual auto Schemas() const -> QHash<unsigned, QString> = 0;

    void Replace(QString &formula, const QString &newName, vsizetype position, const QString &token,
                 vsizetype &bias) const;
    void CorrectionsPositions(vsizetype position, vsizetype bias, QMap<vsizetype, QString> &tokens) const;
    static void BiasTokens(vsizetype position, vsizetype bias, QMap<vsizetype, QString> &tokens);

    void ValidateXML(const QString &schema) const;

private:
    Q_DISABLE_COPY_MOVE(VAbstractConverter) // NOLINT

    QTemporaryFile m_tmpFile{};

    void ReserveFile() const;
};

QT_WARNING_POP

#endif // VABSTRACTCONVERTER_H
