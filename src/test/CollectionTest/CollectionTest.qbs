VApp {
    Depends { name: "buildconfig" }
    Depends { name: "Qt"; submodules: ["testlib", "gui", "printsupport"] }
    Depends { name: "VTestLib" }

    name: "CollectionTest"
    buildconfig.appTarget: qbs.targetOS.contains("macos") ? "CollectionTest" : "collectionTest"
    targetName: buildconfig.appTarget
    type: base.concat("autotest")
    bundle.isBundle: false
    install: false

    files: [
      "qttestmainlambda.cpp",
      "tst_tapecommandline.cpp",
      "tst_valentinacommandline.cpp",
      "tst_tapecommandline.h",
      "tst_valentinacommandline.h",
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
}
