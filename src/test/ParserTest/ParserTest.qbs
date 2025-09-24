import qbs.FileInfo

VTestApp {
    Depends { name: "buildconfig" }
    Depends { name: "QMUParserLib" }

    name: "ParserTest"
    buildconfig.appTarget: qbs.targetOS.contains("macos") ? "ParserTest" : "parserTest"
    targetName: buildconfig.appTarget
    consoleApplication: true

    Properties {
        condition: qbs.targetOS.contains("macos")
        cpp.rpaths: [
            FileInfo.joinPaths(cpp.rpathOrigin, "..", "install-root", product.qbs.installPrefix,  product.buildconfig.installAppPath + "/Valentina.app/Contents/Frameworks"),
            Qt.core.libPath
        ]
    }

    files: [
      "main.cpp"
    ]
}
