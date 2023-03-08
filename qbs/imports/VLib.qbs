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

    cpp.cxxLanguageVersion: {
        if (Qt.core.versionMajor >= 6) // Start building with C++20 since Qt 6.0
            return "c++20";
        else if(Qt.core.versionMajor >= 5 && Qt.core.versionMinor >= 12) // Since Qt 5.12 available support for C++17
            return "c++17";
        return "c++11";
    }

    Properties {
        condition: qbs.targetOS.contains("windows") && qbs.toolchain.contains("gcc") && !qbs.toolchain.contains("clang")
        cpp.minimumWindowsVersion: {
            if (Qt.core.versionMajor >= 6)
                return "6.02"; // should be 10.0

            if (Qt.core.versionMajor >= 5 && Qt.core.versionMinor >= 7)
                return "6.00";

            return "5.01";
        }
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
//            if (qbs.toolchain.contains("msvc"))
//                files.push("stable.cpp")
            return files;
        }
        fileTags: ["cpp_pch_src"]
    }
}
