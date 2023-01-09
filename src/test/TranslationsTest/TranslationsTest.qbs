import qbs.FileInfo

VApp {
    Depends { name: "buildconfig" }
    Depends { name: "VTestLib" }
    Depends { name: "Qt"; submodules: ["testlib", "xml", "gui", "printsupport"] }

    name: "TranslationsTest"
    buildconfig.appTarget: qbs.targetOS.contains("macos") ? "TranslationsTest" : "translationsTest"
    targetName: buildconfig.appTarget
    type: base.concat("autotest")
    bundle.isBundle: false
    install: false
    condition: buildconfig.enableUnitTests

    files: [
      "qttestmainlambda.cpp",
      "tst_measurementregexp.cpp",
      "tst_qmuparsererrormsg.cpp",
      "tst_tstranslation.cpp",
      "tst_buitinregexp.cpp",
      "tst_abstractregexp.cpp",
      "tst_tslocaletranslation.cpp",
      "tst_abstracttranslation.cpp",
      "tst_measurementregexp.h",
      "tst_qmuparsererrormsg.h",
      "tst_tstranslation.h",
      "tst_buitinregexp.h",
      "tst_abstractregexp.h",
      "tst_tslocaletranslation.h",
      "tst_abstracttranslation.h",  
    ]

    Group {
        name: "Precompiled headers"
        files: {
            var files = ["stable.h"];
            if (qbs.toolchain.contains("msvc"))
                files.push("stable.cpp")
            return files;
        }
        fileTags: ["cpp_pch_src"]
    }

    cpp.defines: 'TS_DIR="' + FileInfo.joinPaths(project.sourceDirectory, "share", "translations")  + '"'
}
