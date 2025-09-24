import qbs.Utilities
import qbs.FileInfo

VTestApp {
    Depends { name: "buildconfig" }
    Depends { name: "VTestLib" }
    Depends { name: "Qt"; submodules: ["core", "testlib"] }
    Depends { name: "VToolsLib" }
    Depends { name: "VGeometryLib" }
    Depends { name: "IFCLib" }
    Depends { name: "VDXFLib" }
    Depends { name: "VFormatLib" }
    Depends { name: "ebr" }
    Depends { name: "autotest" }
    Depends { name: "QMUParserLib" }

    Depends {
        name: "Qt.xmlpatterns"
        condition: Utilities.versionCompare(Qt.core.version, "6") < 0
    }

    Depends {
        name: "EchoHelper"
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 &&
                   project.withTextCodec &&
                   qbs.targetOS.contains("unix")
    }

    Depends {
        name: "icu";
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 &&
                   project.withTextCodec && project.withICUCodecs &&
                   buildconfig.useConanPackages && buildconfig.conanWithICUEnabled
    }

    Depends {
        name: "stdinProcess"
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0
    }

    Depends {
        name: "readLineStdinProcess"
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0
    }

    Depends {
        name: "readAllStdinProcess"
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0
    }

    name: "ValentinaTest"
    buildconfig.appTarget: qbs.targetOS.contains("macos") ? "ValentinaTest" : "valentinaTest"
    targetName: buildconfig.appTarget

    autotest.workingDir: product.buildDirectory

    Properties {
        condition: qbs.targetOS.contains("macos")
        cpp.rpaths: [
            FileInfo.joinPaths(cpp.rpathOrigin, "..", "install-root", product.qbs.installPrefix,  product.buildconfig.installAppPath + "/Valentina.app/Contents/Frameworks"),
            Qt.core.libPath
        ]
    }

    files: [
        "qttestmainlambda.cpp",
        "tst_dxf.cpp",
        "tst_dxf.h",
        "tst_svgfontwritingsystem.cpp",
        "tst_svgfontwritingsystem.h",
        "tst_vboundary.cpp",
        "tst_vboundary.h",
        "tst_vposter.cpp",
        "tst_vspline.cpp",
        "tst_nameregexp.cpp",
        "tst_vlayoutdetail.cpp",
        "tst_varc.cpp",
        "tst_qmutokenparser.cpp",
        "tst_vmeasurements.cpp",
        "tst_vlockguard.cpp",
        "tst_misc.cpp",
        "tst_vcommandline.cpp",
        "tst_vpiece.cpp",
        "tst_vabstractcurve.cpp",
        "tst_findpoint.cpp",
        "tst_vellipticalarc.cpp",
        "tst_vcubicbezierpath.cpp",
        "tst_vgobject.cpp",
        "tst_vsplinepath.cpp",
        "tst_vpointf.cpp",
        "tst_readval.cpp",
        "tst_vsvgpathtokenizer.cpp",
        "tst_vsvgpathtokenizer.h",
        "tst_vtranslatevars.cpp",
        "tst_vabstractpiece.cpp",
        "tst_vtooluniondetails.cpp",
        "tst_vposter.h",
        "tst_vspline.h",
        "tst_nameregexp.h",
        "tst_vlayoutdetail.h",
        "tst_varc.h",
        "tst_qmutokenparser.h",
        "tst_vmeasurements.h",
        "tst_vlockguard.h",
        "tst_misc.h",
        "tst_vcommandline.h",
        "tst_vpiece.h",
        "tst_vabstractcurve.h",
        "tst_findpoint.h",
        "tst_vellipticalarc.h",
        "tst_vcubicbezierpath.h",
        "tst_vgobject.h",
        "tst_vsplinepath.h",
        "tst_vpointf.h",
        "tst_readval.h",
        "tst_vtranslatevars.h",
        "tst_vabstractpiece.h",
        "tst_vtooluniondetails.h",
        "tst_xsdschema.cpp",
        "tst_xsdschema.h",
    ]

    Group {
        name: "Test data"
        files: "share/test_data.qrc"
        fileTags: "ebr.external_qrc"
    }

    Group {
        name: "VTextStream"
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0
        files: [
            "tst_vtextstream.cpp",
            "tst_vtextstream.h"
        ]
    }

    Group {
        name: "VTextStream test data"
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0
        prefix: "vtextstream/"
        files: ["*.txt"]
        fileTags: []
    }

    Group {
        Depends { name: "Qt.core" }
        name: "Text codec tests"
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 && project.withTextCodec
        files: [
            "tst_qstringiterator.cpp",
            "tst_qstringiterator.h",
            "tst_utf8.cpp",
            "tst_utf8.h",
            "tst_qtextcodec.cpp",
            "tst_qtextcodec.h",
        ]
    }

    Group {
        name: "Codecs test data"
        condition: project.withTextCodec
        prefix: "codecs/"
        files: ["*.txt"]
        fileTags: []
    }
}
