import qbs.File
import qbs.FileInfo
import qbs.Environment
import qbs.Utilities
import qbs.ModUtils

/**
  This module uses _windeployqt_ program to collect Qt runtime for Windows deployment.
  */
Module {
    additionalProductTypes: ["windeployqt"]

    property bool artifacts: true

    property bool json: false

    property string windeployqtArtifact: artifacts ? (json ? "windeployqt.json" : "windeployqt.txt")
                                                   : undefined

    property string windeployqtProgram: "windeployqt.exe"

    property string qmake: undefined

    /**
      Defines where collected files are being copied. This can be useful to distinguish Qt runtime from project artifacts. Uses
      location of the binary if undefined.
      */
    property string dir: undefined

    property string libdir: undefined

    property string plugindir: product.windeployqt.dir !== undefined ? product.windeployqt.dir + "/plugins"
                                                                     : undefined

    property bool debug: false

    property bool release: false

    property bool pdb: false

    property bool force: false

    property bool dryRun: false

    property bool noPatchqt: false

    property bool ignoreLibraryErrors: false

    property bool noPlugins: false

    property bool noLibraries: false

    property string languages: undefined

    property bool noTranslations: false

    property bool noSystemD3dCompiler: false

    property bool compilerRuntime: false

    property bool noVirtualkeyboard: false

    property bool noCompilerRuntime: false

    property bool noOpenglSw: false

    property string list: undefined

    property int verbose: undefined

    Depends { name: "Qt.core" }

    Rule {
//        alwaysRun: true
        condition: product.qbs.targetOS.contains("windows")
        inputs: product.type.contains("dynamiclibrary") ? ["dynamiclibrary"] : ["application"]

        prepare: {
            var windeployqtProgram = product.windeployqt.windeployqtProgram;

            // Checking if directory exists as a dirty workaround to check if `--no-install` options has been set from command line.

            if (!File.exists(product.qbs.installRoot)) {
                var cmd = new JavaScriptCommand();
                cmd.windeployqtProgram = windeployqtProgram;
                cmd.description = "can not invoke '" + windeployqtProgram + "' when '--no-install` options has been set from command line";
                cmd.sourceCode = function () {
                    console.warn("Can not invoke '" + windeployqtProgram + "' when '--no-install` options has been set from command line (tip: remove your build directory entirely after unchecking '--no-install' option)");
                }
            } else {
                var cmdArgs = []

                if (Utilities.versionCompare(product.Qt.core.version, "6") >= 0) {
                    if (product.windeployqt.qmake !== undefined)
                        cmdArgs.push("--qmake", product.windeployqt.qmake)

                    if (product.windeployqt.ignoreLibraryErrors)
                        cmdArgs.push("--ignore-library-errors")

                    if (!product.windeployqt.noTranslations && product.windeployqt.languages !== undefined)
                        cmdArgs.push("--translations", product.windeployqt.languages)
                }

                if (product.windeployqt.dir !== undefined)
                    cmdArgs.push("--dir", product.windeployqt.dir)

                if (product.windeployqt.noLibraries)
                    cmdArgs.push("--no-libraries")
                else if (product.windeployqt.libdir !== undefined)
                    cmdArgs.push("--libdir", product.windeployqt.libdir)

                if (product.windeployqt.noPlugins)
                    cmdArgs.push("--no-plugins")
                else if (product.windeployqt.plugindir !== undefined)
                    cmdArgs.push("--plugindir", product.windeployqt.plugindir)

                if (product.windeployqt.debug)
                    cmdArgs.push("--debug")

                if (product.windeployqt.release)
                    cmdArgs.push("--release")

                if (product.windeployqt.pdb && product.qbs.toolchain.contains("msvc"))
                    cmdArgs.push("--pdb")

                if (product.windeployqt.force)
                    cmdArgs.push("--force")

                if (product.windeployqt.dryRun)
                    cmdArgs.push("--dry-run")

                if (product.windeployqt.noPatchqt)
                    cmdArgs.push("--no-patchqt")

                if (product.windeployqt.noTranslations)
                    cmdArgs.push("--no-translations")

                if (product.windeployqt.noSystemD3dCompiler)
                    cmdArgs.push("--no-system-d3d-compiler")

                if (product.windeployqt.compilerRuntime)
                    cmdArgs.push("--compiler-runtime")
                else if (product.windeployqt.noCompilerRuntime)
                    cmdArgs.push("--no-compiler-runtime")

                if (product.windeployqt.noVirtualkeyboard)
                    cmdArgs.push("--no-virtualkeyboard")

                if (product.windeployqt.json)
                    cmdArgs.push("--json")

                if (product.windeployqt.noOpenglSw)
                    cmdArgs.push("--no-opengl-sw")

                if (product.windeployqt.list !== undefined)
                    cmdArgs.push("--list", product.windeployqt.list)

                if (product.windeployqt.verbose !== undefined)
                    cmdArgs.push("--verbose", product.windeployqt.verbose)

                cmdArgs.push(product.installDir + "/" + input.fileName);

                var cmd = new Command(product.Qt.core.binPath + "/" + windeployqtProgram, cmdArgs);
                cmd.jobPool = "windeployqt";
                cmd.description = "invoking '" + windeployqtProgram;
                cmd.stdoutFilePath = product.buildDirectory + "/" + product.windeployqt.windeployqtArtifact;
            }
            cmd.workingDirectory = product.qbs.installRoot;
            cmd.highlight = "filegen";
            return [cmd]
        }

        Artifact {
            filePath: product.buildDirectory + "/" + product.windeployqt.windeployqtArtifact
            fileTags: ["windeployqt", "windeployqt.json", "json"]
        }
    }

    JobLimit {
        jobPool: "windeployqt"
        jobCount: 1
    }
}
