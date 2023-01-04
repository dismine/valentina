import qbs.FileInfo

Module {
    property bool staticBuild: true
    property bool frameworksBuild: qbs.targetOS.contains("macos") && !staticBuild

    property bool enableAddressSanitizer: false
    property bool enableUbSanitizer: false
    property bool enableThreadSanitizer: false

    property string libDirName: "lib"

    property string appTarget

    readonly property string installAppPath: {
        if (qbs.targetOS.contains("macos"))
            return "Applications";
        else if (qbs.targetOS.contains("windows"))
            return ".";
        else
            return "bin";
    }

    readonly property string installBinaryPath: {
        if (qbs.targetOS.contains("macos"))
            return installAppPath + "/" + appTarget + ".app/Contents/MacOS"
        else
            return installAppPath
    }

    readonly property string installLibraryPath: {
        if (qbs.targetOS.contains("macos"))
            return installAppPath + "/" + appTarget + ".app/Contents/Frameworks"
        else if (qbs.targetOS.contains("windows"))
            return installAppPath
        else
            return libDirName + "/" + appTarget
    }

    readonly property string installPluginPath: {
        if (qbs.targetOS.contains("macos"))
            return installAppPath + "/" + appTarget + ".app/Contents/Plugins"
        else
            return installLibraryPath + "/plugins"
    }

    readonly property string installDataPath: {
        if (qbs.targetOS.contains("macos"))
            return installAppPath + "/" + appTarget + ".app/Contents/Resources"
        else
            return "share/" + appTarget
    }

    Depends { name: "cpp" }
    Depends { name: "Qt.core"; versionAtLeast: project.minimumQtVersion }
    Depends { name: "vcs2"; }

    cpp.defines: {
        var defines = [
            // The following define makes your compiler emit warnings if you use
            // any feature of Qt which has been marked as deprecated (the exact warnings
            // depend on your compiler). Please consult the documentation of the
            // deprecated API in order to know how to port your code away from it.
            "QT_DEPRECATED_WARNINGS",

            // You can make your code fail to compile if it uses deprecated APIs.
            // In order to do so, uncomment the following line.
            "QT_DISABLE_DEPRECATED_BEFORE=0x060000", // disables all the APIs deprecated before Qt 6.0.0

            // Since Qt 5.4.0 the source code location is recorded only in debug builds.
            // We need this information also in release builds. For this need define QT_MESSAGELOGCONTEXT.
            "QT_MESSAGELOGCONTEXT",

            "QBS_BUILD"
        ];

        if (qbs.targetOS.contains("unix")) {
            defines.push('BINDIR="' + FileInfo.joinPaths(qbs.installPrefix, "bin") + '"');
            const dataDir = FileInfo.joinPaths(qbs.installPrefix, "share");
            defines.push('DATADIR="' + dataDir + '"');
            defines.push('PKGDATADIR="' + FileInfo.joinPaths(dataDir, "valentina") + '"');
        }

        return defines;
    }

    Properties {
        condition: Qt.core.versionMajor >= 5 &&  Qt.core.versionMinor < 12
        cpp.cxxLanguageVersion: "c++11"
    }
    // Since Qt 5.12 available support for C++17
    Properties {
        condition: Qt.core.versionMajor >= 5 &&  Qt.core.versionMinor >= 12
        cpp.cxxLanguageVersion: "c++17"
    }

    readonly property string minimumMacosVersion: {
        // Check which minimal OSX version supports current Qt version
        if (Qt.core.versionMajor >= 6) {
            // For Qt 6.5 https://doc-snapshots.qt.io/qt6-6.5/supported-platforms.html
            if (Qt.core.versionMinor >= 5) // Qt 6.5
                return "11.0";

            // See page https://doc.qt.io/qt-6.4/supported-platforms.html
            return "10.14"; // Qt 6.4 and above
        }

        if (Qt.core.versionMajor >= 5) {
            // See page https://doc.qt.io/qt-5.15/supported-platforms.html
            // For qt 5.14 https://doc.qt.io/archives/qt-5.14/supported-platforms.html
            if (Qt.core.versionMinor >= 14) // Qt 5.14
                return "10.13";

            // For Qt 5.13 https://doc.qt.io/archives/qt-5.13/supported-platforms.html
            // For Qt 5.12 https://doc.qt.io/archives/qt-5.12/supported-platforms.html
            if (Qt.core.versionMinor >= 12) // Qt 5.12
                return "10.12";

            // For older versions https://doc.qt.io/archives/qt-5.11/supported-platforms-and-configurations.html
            if (Qt.core.versionMinor >= 10) // Qt 5.11 and Qt 5.10
                return "10.11";

            if (Qt.core.versionMinor >= 9) // Qt 5.9
                return "10.10";

            if (Qt.core.versionMinor >= 8) // Qt 5.8
                return "10.9";

            if (Qt.core.versionMinor >= 7) // Qt 5.7
                return "10.8";

            if (Qt.core.versionMinor >= 4) // Qt 5.4
                return "10.7";

            return "10.6";
        }

        return undefined;
    }

    cpp.separateDebugInformation: true

    Properties {
        condition: qbs.buildVariant === "debug"
        cpp.warningLevel: "all"
        cpp.treatWarningsAsErrors: true
    }

    Properties {
        condition: qbs.toolchain.contains("gcc")
        cpp.cxxFlags: {
            var flags = [];
            if (enableAddressSanitizer)
                flags.push("-fno-omit-frame-pointer");
            return flags;
        }
        cpp.driverFlags: {
            var flags = [];
            if (enableAddressSanitizer)
                flags.push("-fsanitize=address");
            if (enableUbSanitizer)
                flags.push("-fsanitize=undefined");
            if (enableThreadSanitizer)
                flags.push("-fsanitize=thread");
            return flags;
        }
    }

    vcs2.headerFileName: "vcsRepoState.h"
}
