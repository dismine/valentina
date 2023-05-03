/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   30 9, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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

#ifndef TESTVAPPLICATION_H
#define TESTVAPPLICATION_H

#include "vabstractvalapplication.h"
#include "projectversion.h"
#include "../vmisc/vcommonsettings.h"
#include "../vmisc/compatibility.h"

class VTestSettings : public VCommonSettings
{
    Q_OBJECT // NOLINT
public:
    VTestSettings(Format format, Scope scope, const QString &organization, const QString &application = QString(),
                  QObject *parent = nullptr)
        : VCommonSettings(format, scope, organization, application, parent)
    {
        REGISTER_META_TYPE_STREAM_OPERATORS(QMarginsF);
    }
};

class TestVApplication final : public VAbstractValApplication
{
    Q_OBJECT // NOLINT
public:
    TestVApplication(int &argc, char ** argv)
        : VAbstractValApplication(argc, argv),
          m_trVars(nullptr)
    {
        setApplicationName("ValentinaTest");
        setOrganizationName(VER_COMPANYNAME_STR);

        TestVApplication::OpenSettings();
    }

    virtual ~TestVApplication() = default;

    virtual auto TrVars() -> const VTranslateVars * override { return m_trVars; }

    virtual void OpenSettings() override
    {
        settings = new VTestSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(),
                                     QCoreApplication::applicationName(), this);
    }

    virtual auto IsAppInGUIMode() const -> bool override { return false; }

    virtual void InitTrVars() override
    {}

    void SetTrVars(VTranslateVars *trVars)
    {
        m_trVars = trVars;
    }

    static auto VApp() -> TestVApplication * { return static_cast<TestVApplication *>(QCoreApplication::instance()); }

protected slots:
    virtual void AboutToQuit() override
    {}

private:
    Q_DISABLE_COPY_MOVE(TestVApplication) // NOLINT
    VTranslateVars *m_trVars;
};

#endif // TESTVAPPLICATION_H
