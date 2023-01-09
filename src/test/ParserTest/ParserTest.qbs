VApp {
    Depends { name: "buildconfig" }
    Depends { name: "QMUParserLib" }

    name: "ParserTest"
    buildconfig.appTarget: qbs.targetOS.contains("macos") ? "ParserTest" : "parserTest"
    targetName: buildconfig.appTarget
    type: base.concat("autotest")
    consoleApplication: true
    bundle.isBundle: false
    install: false
    condition: buildconfig.enableUnitTests

    files: [
      "main.cpp"
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
