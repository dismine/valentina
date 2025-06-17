VTestApp {
    Depends { name: "buildconfig" }
    Depends { name: "QMUParserLib" }

    name: "ParserTest"
    buildconfig.appTarget: qbs.targetOS.contains("macos") ? "ParserTest" : "parserTest"
    targetName: buildconfig.appTarget
    consoleApplication: true

    files: [
      "main.cpp"
    ]
}
