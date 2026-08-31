import qbs.FileInfo

Library {
    Depends { name: "buildconfig" }
    Depends { name: "bundle"; condition: qbs.targetOS.contains("macos") }
    Depends { name: "cpp" }
    Depends { name: "coverage"; required: false }
    // Header-only immer backs VContainer's persistent storage. vcontainer.h and calculator.h are
    // included across effectively every library, so the include path is wired centrally here
    // instead of per product. Default: found as a system package (immersystem probes for it).
    // project.conanWithImmer:true switches to a pinned version fetched through Conan instead.
    Depends {
        name: "immersystem"
        condition: !(buildconfig.useConanPackages && buildconfig.conanImmerEnabled)
    }
    Depends {
        name: "immer"
        condition: buildconfig.useConanPackages && buildconfig.conanImmerEnabled
    }

    qbsModuleProviders: {
        var providers = ["Qt"];

        if (buildconfig.useConanPackages)
            providers.push("conan");

        providers.push("qbspkgconfig");
        return providers;
    }

    type: buildconfig.staticBuild ? "staticlibrary" : "dynamiclibrary"

    buildconfig.appTarget: qbs.targetOS.contains("macos") ? "Valentina" : "valentina"
    cpp.includePaths: [".."]

    // Allow MAC OS X to find library inside a bundle
    cpp.sonamePrefix: (!buildconfig.staticBuild && qbs.targetOS.contains("macos")) ? "@rpath" : undefined

    Properties {
        condition: buildconfig.useConanPackages
        moduleProviders.conan.installDirectory: project.conanInstallPath
    }

    Properties {
        condition: (!buildconfig.staticBuild && buildconfig.enableRPath)
        cpp.rpaths: cpp.rpathOrigin
    }

    Properties {
        condition: qbs.targetOS.contains("unix") && buildconfig.buildWithCcache
        cpp.compilerWrapper: "ccache"
    }

    cpp.cxxLanguageVersion: "c++20"

    Properties {
        condition: qbs.targetOS.contains("windows") && qbs.toolchain.contains("gcc") && !qbs.toolchain.contains("clang")
        cpp.minimumWindowsVersion: {
            if (Qt.core.versionMajor >= 6)
                return "6.02"; // should be 10.0

            return "6.00";
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

    Export {
        Depends { name: "buildconfig" }
        Depends { name: "coverage"; required: false }
    }
}
