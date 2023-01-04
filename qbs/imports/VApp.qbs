import qbs.FileInfo

CppApplication {
    Depends { name: "buildconfig" }
    Depends { name: "bundle" }

    Properties {
        condition: qbs.targetOS.contains("macos")
        cpp.minimumMacosVersion: buildconfig.minimumMacosVersion
    }

    cpp.rpaths: FileInfo.joinPaths(cpp.rpathOrigin,
                                  "..",
                                  qbs.targetOS.contains("macos")
                                  ? "Frameworks"
                                  : buildconfig.installLibraryPath)
    install: true
    installDir: buildconfig.installAppPath
    installDebugInformation: true

    Properties {
        condition: Qt.core.versionMajor >= 5 &&  Qt.core.versionMinor < 12
        cpp.cxxLanguageVersion: "c++11"
    }
    // Since Qt 5.12 available support for C++17
    Properties {
        condition: Qt.core.versionMajor >= 5 &&  Qt.core.versionMinor >= 12
        cpp.cxxLanguageVersion: "c++17"
    }
}
