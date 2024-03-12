import qbs.FileInfo
import qbs.Utilities

VTestApp {
    Depends { name: "buildconfig" }
    Depends { name: "VTestLib" }
    Depends { name: "Qt"; submodules: ["core", "testlib", "xml", "gui", "printsupport"] }
    Depends { name: "autotest" }

    Depends {
        name: "xerces-c"
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 &&
                   (!buildconfig.useConanPackages || (buildconfig.useConanPackages && !buildconfig.conanXercesEnabled))
    }

    Depends {
        name: "conan.XercesC"
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 && buildconfig.useConanPackages &&
                   buildconfig.conanXercesEnabled
    }

    Depends {
        name: "conan.crashpad";
        condition: buildconfig.useConanPackages && buildconfig.conanCrashReportingEnabled
    }

    name: "TranslationsTest"
    buildconfig.appTarget: qbs.targetOS.contains("macos") ? "TranslationsTest" : "translationsTest"
    targetName: buildconfig.appTarget
    autotest.workingDir: product.buildDirectory

    files: [
        "qttestmainlambda.cpp",
        "tst_qmuparsererrormsg.cpp",
        "tst_tstranslation.cpp",
        "tst_buitinregexp.cpp",
        "tst_abstractregexp.cpp",
        "tst_tslocaletranslation.cpp",
        "tst_abstracttranslation.cpp",
        "tst_qmuparsererrormsg.h",
        "tst_tstranslation.h",
        "tst_buitinregexp.h",
        "tst_abstractregexp.h",
        "tst_tslocaletranslation.h",
        "tst_abstracttranslation.h",
    ]

    cpp.defines: 'TS_DIR="' + FileInfo.joinPaths(project.sourceDirectory, "share", "translations")  + '"'
}
