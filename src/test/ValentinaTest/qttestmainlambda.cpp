/************************************************************************
 **
 **  @file   qttestmainlambda.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   31 3, 2015
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

#include <QScopeGuard>
#include <QtTest>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <xercesc/util/PlatformUtils.hpp>
#endif

#include "../vmisc/testvapplication.h"
#include "tst_dxf.h"
#include "tst_findpoint.h"
#include "tst_misc.h"
#include "tst_nameregexp.h"
#include "tst_qmutokenparser.h"
#include "tst_readval.h"
#include "tst_svgfontwritingsystem.h"
#include "tst_vabstractcurve.h"
#include "tst_vabstractpiece.h"
#include "tst_varc.h"
#include "tst_vboundary.h"
#include "tst_vcommandline.h"
#include "tst_vcubicbezierpath.h"
#include "tst_vellipticalarc.h"
#include "tst_vgobject.h"
#include "tst_vlayoutdetail.h"
#include "tst_vlockguard.h"
#include "tst_vmeasurements.h"
#include "tst_vpiece.h"
#include "tst_vpointf.h"
#include "tst_vposter.h"
#include "tst_vspline.h"
#include "tst_vsplinepath.h"
#include "tst_vsvgpathtokenizer.h"
#include "tst_vtheme.h"
#include "tst_vtooluniondetails.h"
#include "tst_vtranslatevars.h"
#include "tst_xsdschema.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#ifdef WITH_TEXTCODEC
#include "tst_qstringiterator.h"
#include "tst_qtextcodec.h"
#include "tst_utf8.h"
#endif

#include "tst_vtextstream.h"

#include <QDir>
#if defined(SHARED_ICU_DATA)
#include <unicode/putil.h>
#endif
#endif // QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

//---------------------------------------------------------------------------------------------------------------------
auto main(int argc, char **argv) -> int
{
    Q_INIT_RESOURCE(schema); // NOLINT

#if defined(Q_OS_MACX)
    Q_INIT_RESOURCE(mac_light_theme); // NOLINT
    Q_INIT_RESOURCE(mac_dark_theme);  // NOLINT
#else
    Q_INIT_RESOURCE(win_light_theme); // NOLINT
    Q_INIT_RESOURCE(win_dark_theme);  // NOLINT
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    XERCES_CPP_NAMESPACE::XMLPlatformUtils::Initialize();

    auto Terminate = qScopeGuard([]() { XERCES_CPP_NAMESPACE::XMLPlatformUtils::Terminate(); });
#endif

    TestVApplication const app(argc, argv); // For QPrinter

    QResource::registerResource(QCoreApplication::applicationDirPath() + QStringLiteral("/test_data.rcc"));

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#if defined(SHARED_ICU_DATA)
#if defined(Q_OS_MACOS)
    u_setDataDirectory(ICU_DATA_PATH);
#endif
#endif // defined(SHARED_ICU_DATA)
#endif // QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

    int status = 0;
    auto ASSERT_TEST = [&status, argc, argv](QObject *obj)
    {
        status |= QTest::qExec(obj, argc, argv); // NOLINT(hicpp-signed-bitwise)
        delete obj;
    };

    ASSERT_TEST(new TST_FindPoint());
    ASSERT_TEST(new TST_VPiece());
    ASSERT_TEST(new TST_VPoster());
    ASSERT_TEST(new TST_VAbstractPiece());
    ASSERT_TEST(new TST_VSpline());
    ASSERT_TEST(new TST_VSplinePath());
    ASSERT_TEST(new TST_NameRegExp());
    ASSERT_TEST(new TST_VLayoutDetail());
    ASSERT_TEST(new TST_VArc());
    ASSERT_TEST(new TST_VEllipticalArc());
    ASSERT_TEST(new TST_QmuTokenParser());
    ASSERT_TEST(new TST_VMeasurements());
    ASSERT_TEST(new TST_VLockGuard());
    ASSERT_TEST(new TST_Misc());
    ASSERT_TEST(new TST_VCommandLine());
    ASSERT_TEST(new TST_VAbstractCurve());
    ASSERT_TEST(new TST_VCubicBezierPath());
    ASSERT_TEST(new TST_VGObject());
    ASSERT_TEST(new TST_VPointF());
    ASSERT_TEST(new TST_ReadVal());
    ASSERT_TEST(new TST_VTranslateVars());
    ASSERT_TEST(new TST_VToolUnionDetails());
    ASSERT_TEST(new TST_XSDShema());
    ASSERT_TEST(new TST_VSVGPathTokenizer());
    ASSERT_TEST(new TST_VBoundary());
    ASSERT_TEST(new TST_DXF());
    ASSERT_TEST(new TST_SVGFontWritingSystem());
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#ifdef WITH_TEXTCODEC
    ASSERT_TEST(new TST_Utf8());
    ASSERT_TEST(new TST_QTextCodec());
    ASSERT_TEST(new TST_QStringIterator());
#endif
    ASSERT_TEST(new TST_VTextStream());
#endif // QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    ASSERT_TEST(new TST_VTheme());

    return status;
}
