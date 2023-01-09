import qbs.FileInfo

Library {
    Depends { name: "buildconfig" }
    Depends { name: "bundle" }
    Depends { name: "cpp" }

    type: buildconfig.staticBuild ? "staticlibrary" : "dynamiclibrary"

    buildconfig.appTarget: "valentina"
    bundle.isBundle: buildconfig.frameworksBuild
    cpp.includePaths: [".."]

    // Allow MAC OS X to find library inside a bundle
    cpp.sonamePrefix: (!buildconfig.staticBuild && qbs.targetOS.contains("macos")) ? "@rpath" : undefined

    Properties {
        condition: (!buildconfig.staticBuild && buildconfig.enableRPath)
        cpp.rpaths: cpp.rpathOrigin
    }

    install: !buildconfig.staticBuild
    installDir: buildconfig.installLibraryPath
    installDebugInformation: !buildconfig.staticBuild

    Properties {
        condition: qbs.targetOS.contains("macos")
        cpp.minimumMacosVersion: buildconfig.minimumMacosVersion
    }
}
