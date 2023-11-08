import qbs.FileInfo

VTestApp {
    Depends { name: "buildconfig" }
    Depends { name: "VTestLib" }
    Depends { name: "Qt"; submodules: ["testlib", "xml", "gui", "printsupport"] }
    Depends { name: "conan.XercesC"; condition: buildconfig.useConanPackages }
    Depends { name: "autotest" }

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
