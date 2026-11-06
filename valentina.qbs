import qbs.Environment
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
    property bool conanWithImmer: false
    property string minimumMacosVersion: undefined
    property string minimumQtVersion: "5.15"
    property stringList conanProfiles: []

    // Build-variant label, set by CI (see .github/workflows/*). Baked into the binary as
    // CRASH_BUILD_LABEL and reported by AppCrashVersion(); the same value must be passed to
    // scripts/upload_symbols.py --label, or crash reports won't resolve to uploaded symbols.
    // Empty (local builds): AppCrashVersion() falls back to computing Qt_<major>_<minor>.
    property string crashBuildLabel: ""

    property bool enableSigning: {
        if (qbs.targetOS.contains("macos"))
            return true;

        return false;
    }

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
        condition: enableConan && (conanWithXerces || conanWithCrashReporting || conanWithICU || conanWithICONV ||
                                   conanWithImmer)
        conanfilePath: project.sourceDirectory + "/conanfile.py"
        verbose: true
        generators: []
        options: {
            console.info("Conan options:");
            console.info("* xerces: " + conanWithXerces);
            console.info("* crash reporting: " + conanWithCrashReporting);
            console.info("* ICU: " + conanWithICU);
            console.info("* ICONV: " + conanWithICONV);
            console.info("* immer: " + conanWithImmer);

            var o = {};
            if (conanWithXerces)
                o['&:with_xerces'] = "True";

            if (conanWithCrashReporting)
                o['&:with_crash_reporting'] = "True";

            if (conanWithICU)
                o['&:with_icu'] = "True";
            else if (conanWithICONV)
                o['&:with_iconv'] = "True";

            if (conanWithImmer)
                o['&:with_immer'] = "True";

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

        // NOTE: on Windows QTest sends its output to OutputDebugString unless stderr is a console or
        // QT_FORCE_STDERR_LOGGING is set. This product replaces the process environment, so the variable
        // has to be pushed below. Setting it only on the workflow step never reaches valentinaTest.
        arguments: ["-silent", "-o", "-,txt"]

        Properties {
            condition: qbs.targetOS.contains("macos")
            bundle.isBundle: false
        }

        environment: {
            var env = base;

            // These entries are added to the run environment. A name that is already set, such as PATH, is replaced
            // by the entry. A variable set only on the workflow step is not already in this list: Windows QTest still
            // writes to OutputDebugString unless QT_FORCE_STDERR_LOGGING is pushed here.
            var hasForceStderr = false;
            for (var e = 0; e < env.length; ++e) {
                if (env[e].startsWith("QT_FORCE_STDERR_LOGGING=")) {
                    hasForceStderr = true;
                    break;
                }
            }
            if (!hasForceStderr) {
                env.push("QT_FORCE_STDERR_LOGGING=1");
            }

	    // QT_QPA_PLATFORM
            for (var i = 0; i < env.length; ++i) {
                if (env[i].startsWith("QT_QPA_PLATFORM=")) {
                    break;
                }
            }
            if (i >= env.length) {
                env.push("QT_QPA_PLATFORM=offscreen");
            }
 
            if (qbs.targetOS.contains("unix") && !qbs.targetOS.contains("macos")) {
                env.push("LD_LIBRARY_PATH=" + qbs.installRoot + qbs.installPrefix + "/" + buildconfig.libDirName + "/valentina");
            } else if (qbs.targetOS.contains("windows")) {
                // PATH. commonRunEnvironment does not carry the process PATH. Setting PATH to only the
                // install directory drops python, and a local nesting engine started from the tests cannot be found.
                var path = "";
                for (var i = 0; i < env.length; ++i) {
                    if (env[i].startsWith("PATH=")) {
                        path = env[i].substring(5);
                        break;
                    }
                }

                var fullInstallDir = FileInfo.joinPaths(qbs.installRoot, qbs.installPrefix);
                var machinePath = Environment.getEnv("PATH") || "";
                if (path.length === 0) {
                    path = machinePath.length === 0 ? fullInstallDir : fullInstallDir + ";" + machinePath;
                } else if (machinePath.length !== 0 && path.indexOf(machinePath) === -1) {
                    path = path + ";" + machinePath;
                }

                var arrayElem = "PATH=" + path;
                if (i < env.length)
                    env[i] = arrayElem;
                else
                    env.push(arrayElem);
            }

            // QT_QPA_PLATFORM_PLUGIN_PATH
            // On macOS and Windows the test binaries do not search the Qt SDK plugins
            // directory at run time, so the forced "offscreen" platform plugin is not
            // found beside the executable (only the default platform plugin is deployed
            // there). Point Qt at the SDK's "platforms" directory, which ships
            // libqoffscreen, so the headless plugin can be loaded.
            if (qbs.targetOS.contains("macos") || qbs.targetOS.contains("windows")) {
                for (var k = 0; k < env.length; ++k) {
                    if (env[k].startsWith("QT_QPA_PLATFORM_PLUGIN_PATH=")) {
                        break;
                    }
                }

                if (k >= env.length) {
                    env.push("QT_QPA_PLATFORM_PLUGIN_PATH="
                             + FileInfo.joinPaths(Qt.core.pluginPath, "platforms"));
                }
            }
            return env;
        }
    }
}
