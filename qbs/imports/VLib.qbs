import qbs.FileInfo

Library {
    Depends { name: "buildconfig" }
    Depends { name: "bundle"; condition: qbs.targetOS.contains("macos") }
    Depends { name: "cpp" }

    type: buildconfig.staticBuild ? "staticlibrary" : "dynamiclibrary"

    buildconfig.appTarget: qbs.targetOS.contains("macos") ? "Valentina" : "valentina"
    cpp.includePaths: [".."]

    // Allow MAC OS X to find library inside a bundle
    cpp.sonamePrefix: (!buildconfig.staticBuild && qbs.targetOS.contains("macos")) ? "@rpath" : undefined

    Properties {
        condition: (!buildconfig.staticBuild && buildconfig.enableRPath)
        cpp.rpaths: cpp.rpathOrigin
    }

    Properties {
        condition: qbs.targetOS.contains("unix") && buildconfig.buildWithCcache
        cpp.compilerWrapper: "ccache"
    }

    install: false
    installDebugInformation: false

    Properties {
        condition: qbs.targetOS.contains("macos")
        cpp.minimumMacosVersion: buildconfig.minimumMacosVersion
        bundle.isBundle: buildconfig.frameworksBuild
    }

    Group {
        name: "Precompiled headers"
        condition: buildconfig.enablePCH
        prefix: product.sourceDirectory + "/"
        files: {
            var files = ["stable.h"];
            if (qbs.toolchain.contains("msvc"))
                files.push("stable.cpp")
            return files;
        }
        fileTags: ["cpp_pch_src"]
    }
}
