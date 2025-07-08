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
        name: "XercesC"
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 && buildconfig.useConanPackages &&
                   buildconfig.conanXercesEnabled
    }

    Depends {
        name: "crashpad";
        condition: buildconfig.useConanPackages && buildconfig.conanCrashReportingEnabled
    }

    Properties {
        condition: buildconfig.useConanPackages && buildconfig.conanCrashReportingEnabled && qbs.targetOS.contains("windows") && qbs.toolchain.contains("msvc")
        cpp.dynamicLibraries: ["Advapi32"]
    }

    Properties {
        condition: buildconfig.useConanPackages && buildconfig.conanCrashReportingEnabled && qbs.targetOS.contains("macos")
        cpp.libraryPaths: ["/usr/lib"]
        cpp.dynamicLibraries: ["bsm"]
        cpp.frameworks: ["AppKit", "Security"]
    }

    Properties {
        condition: buildconfig.useConanPackages && buildconfig.conanCrashReportingEnabled && qbs.targetOS.contains("unix") && !qbs.targetOS.contains("macos")
        cpp.dynamicLibraries: ["dl"]
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
