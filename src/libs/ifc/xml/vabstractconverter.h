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

#include <qcompilerdetection.h>
#include <QCoreApplication>
#include <QString>
#include <QTemporaryFile>
#include <QtGlobal>

#include "vdomdocument.h"
#include "../vmisc/projectversion.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")
QT_WARNING_DISABLE_GCC("-Wsuggest-final-methods")

class VAbstractConverter :public VDomDocument
{
public:
    explicit VAbstractConverter(const QString &fileName);
    virtual ~VAbstractConverter() = default;

    QString Convert();

    unsigned GetCurrentFormatVersion() const;

protected:
    unsigned m_ver;
    QString m_originalFileName;
    QString m_convertedFileName;

    void ValidateInputFile(const QString &currentSchema) const;
    Q_NORETURN void InvalidVersion(unsigned ver) const;
    virtual void Save();
    virtual void SetVersion(const QString &version);

    virtual unsigned MinVer() const =0;
    virtual unsigned MaxVer() const =0;

    virtual QString MinVerStr() const =0;
    virtual QString MaxVerStr() const =0;

    virtual QString XSDSchema(unsigned ver) const =0;
    virtual void    ApplyPatches() =0;
    virtual void    DowngradeToCurrentMaxVersion() =0;

    virtual bool IsReadOnly() const =0;

    void Replace(QString &formula, const QString &newName, int position, const QString &token, int &bias) const;
    void CorrectionsPositions(int position, int bias, QMap<int, QString> &tokens) const;
    static void BiasTokens(int position, int bias, QMap<int, QString> &tokens);

    void ValidateXML(const QString &schema) const;

private:
    Q_DISABLE_COPY_MOVE(VAbstractConverter) // NOLINT

    QTemporaryFile m_tmpFile{};

    void ReserveFile() const;
};

QT_WARNING_POP

#endif // VABSTRACTCONVERTER_H
