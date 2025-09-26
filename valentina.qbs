import qbs.FileInfo

import "qbs/imports/conan/ConanfileProbe.qbs" as ConanfileProbe

Project {
    name: "Valentina"
    minimumQbsVersion: "2.4"

    property bool enableConan: false
    property bool conanWithXerces: false
    property bool conanWithCrashReporting: false
    property bool conanWithICU: false
    property bool conanWithICONV: false
    property string minimumMacosVersion: undefined
    property string minimumQtVersion: "5.15"
    property stringList conanProfiles: []
    property bool enableSigning: true

    // Enable the QTextCodec API and support for character encodings.
    // Required for any text codec functionality.
    property bool withTextCodec: true
    // Include minimal, common built-in codecs (UTF-8, UTF-16, Latin-1, local 8-bit encodings).
    // Lightweight, always sufficient for standard text conversions.
    property bool withBasicCodecs: true
    // Include extended or “large” built-in codecs (rare legacy encodings, extra code pages).
    // Optional; increases binary size.
    property bool withBigCodecs: false
    // Use ICU library for Unicode conversion and extended codec support.
    // Provides widest coverage; optional if you have ICU.
    property bool withICUCodecs: false
    // Use system iconv library for encoding conversions.
    // Alternative to ICU; optional depending on platform.
    property bool withICONVCodecs: false


    property string conanInstallPath: conanProbe.generatedFilesPath
    ConanfileProbe {
        id: conanProbe
        condition: enableConan && (conanWithXerces || conanWithCrashReporting || conanWithICU || conanWithICONV)
        conanfilePath: project.sourceDirectory + "/conanfile.py"
        verbose: true
        generators: []
        options: {
            console.info("Conan options:");
            console.info("* xerces: " + conanWithXerces);
            console.info("* crash reporting: " + conanWithCrashReporting);
            console.info("* ICU: " + conanWithICU);
            console.info("* ICONV: " + conanWithICONV);

            var o = {};
            if (conanWithXerces)
                o['&:with_xerces'] = "True";

            if (conanWithCrashReporting)
                o['&:with_crash_reporting'] = "True";

            if (conanWithICU)
                o['&:with_icu'] = "True";
            else if (conanWithICONV)
                o['&:with_iconv'] = "True";

            return o;
        }
        additionalArguments: {
            var args = [];

            for (var i = 0; i < conanProfiles.length; i++) {
                args.push("-pr:a=" + conanProfiles[i])
            }

            // Conan dependencies are usually built in Release mode for consistency.
            // However, ICU enforces a strict check on build type, so we must provide
            // matching Debug/RelWithDebInfo builds when needed on Windows.
            // - Debug Qbs build  -> use Conan Debug package
            // - Profiling Qbs build -> use Conan RelWithDebInfo package
            if (qbs.targetOS.contains("windows") && conanWithICU) {
                if (qbs.buildVariant === "debug") {
                    args.push("-s=build_type=Debug");
                } else if (qbs.buildVariant === "profiling") {
                    args.push("-s=build_type=RelWithDebInfo");
                }
            }

            return args;
        }
    }

    references: [
        "src/src.qbs",
        "share/translations/translations.qbs",
        "dmg.qbs",
        "winsetup.qbs"
    ]
    qbsSearchPaths: "qbs"

    AutotestRunner {
        Depends { name: "buildconfig" }

        arguments: ["-silent", "-o", "-,txt"]

        Properties {
            condition: qbs.targetOS.contains("macos")
            bundle.isBundle: false
        }

        environment: {
            var env = base;

            if (qbs.targetOS.contains("unix") && !qbs.targetOS.contains("macos")) {
                env.push("LD_LIBRARY_PATH=" + qbs.installRoot + qbs.installPrefix + "/" + buildconfig.libDirName + "/valentina");
            } else if (qbs.targetOS.contains("windows")) {
                // PATH
                var path = "";
                for (var i = 0; i < env.length; ++i) {
                    if (env[i].startsWith("PATH=")) {
                        path = env[i].substring(5);
                        break;
                    }
                }

                var fullInstallDir = FileInfo.joinPaths(qbs.installRoot, qbs.installPrefix);
                if (path.length === 0) {
                    path = fullInstallDir;
                } else {
                    path = fullInstallDir + ";" + path;
                }

                var arrayElem = "PATH=" + path;
                if (i < env.length)
                    env[i] = arrayElem;
                else
                    env.push(arrayElem);

                // QT_QPA_PLATFORM_PLUGIN_PATH
                for (var i = 0; i < env.length; ++i) {
                    if (env[i].startsWith("QT_QPA_PLATFORM_PLUGIN_PATH=")) {
                        break;
                    }
                }

                if (i >= env.length) {
                    var pluginsPath = "QT_QPA_PLATFORM_PLUGIN_PATH=" + FileInfo.joinPaths(Qt.core.pluginPath, "platforms")
                    env.push(pluginsPath);
                }

                console.info("env_after: " + env);
            }
            return env;
        }
    }
}
