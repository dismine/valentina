import qbs.FileInfo

Library {
    Depends { name: "buildconfig" }
    Depends { name: "bundle" }
    Depends { name: "cpp" }

    type: buildconfig.staticBuild ? "staticlibrary" : "dynamiclibrary"

    buildconfig.appTarget: "valentina"
    bundle.isBundle: buildconfig.frameworksBuild
    cpp.includePaths: [".."]
    cpp.sonamePrefix: qbs.targetOS.contains("macos") ? "@rpath" : undefined
    cpp.rpaths: cpp.rpathOrigin

    install: !buildconfig.staticBuild
    installDir: buildconfig.installLibraryPath
    installDebugInformation: !buildconfig.staticBuild

    Properties {
        condition: qbs.targetOS.contains("macos")
        cpp.minimumMacosVersion: buildconfig.minimumMacosVersion
    }
}
