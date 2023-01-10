import qbs.FileInfo
import qbs.File

CppApplication {
    Depends { name: "buildconfig" }
    Depends { name: "bundle" }

    Properties {
        condition: qbs.targetOS.contains("macos")
        cpp.minimumMacosVersion: buildconfig.minimumMacosVersion
    }

    Properties {
        condition: buildconfig.enableRPath
        cpp.rpaths: FileInfo.joinPaths(cpp.rpathOrigin,
                                      "..",
                                      qbs.targetOS.contains("macos")
                                      ? "Frameworks"
                                      : buildconfig.installLibraryPath)
    }

    install: true
    installDir: buildconfig.installAppPath
    installDebugInformation: true
    type: base.concat("testSuit")

    Properties {
        condition: qbs.targetOS.contains("unix") && buildconfig.buildWithCcache
        cpp.compilerWrapper: "ccache"
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

    Group {
        // See question on StackOwerflow "QSslSocket error when SSL is NOT used" (http://stackoverflow.com/a/31277055/3045403)
        // Copy of answer:
        // We occasionally had customers getting very similar warning messages but the software was also crashing.
        // We determined it was because, although we weren't using SSL either, the program found a copy of OpenSSL on the
        // customer's computer and tried interfacing with it. The version it found was too old though (from Qt 5.2 onwards v1.0.0
        // or later is required).
        //
        // Our solution was to distribute the OpenSSL DLLs along with our application (~1.65 MB). The alternative is to compile
        // Qt from scratch without OpenSSL support.
        condition: qbs.targetOS.contains("windows") && (qbs.architecture.contains("x86_64") || qbs.architecture.contains("x86"))
        name: "openssl"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "dist", "win", "openssl", FileInfo.pathSeparator())
        files: {
            var files = [];
            // Minimal supported OpenSSL version since Qt 5.12.4 is 1.1.1.
            if (Qt.core.versionMajor >= 5  &&  Qt.core.versionMinor >= 12 && Qt.core.versionPatch >= 4) {
                if (qbs.architecture.contains("x86_64")) {
                    files.push(
                        FileInfo.joinPaths("win64", "libcrypto-1_1-x64.dll"),
                        FileInfo.joinPaths("win64", "libssl-1_1-x64.dll")
                    );
                } else {
                    files.push(
                        FileInfo.joinPaths("win32", "libcrypto-1_1.dll"),
                        FileInfo.joinPaths("win32", "libssl-1_1.dll")
                    );
                }
            } else {
                files.push("msvcr120.dll");
                if (qbs.architecture.contains("x86_64")) {
                    files.push(
                        FileInfo.joinPaths("win64", "libeay32.dll"),
                        FileInfo.joinPaths("win64", "ssleay32.dll")
                    );
                } else {
                    files.push(
                        FileInfo.joinPaths("win32", "libeay32.dll"),
                        FileInfo.joinPaths("win32", "ssleay32.dll")
                    );
                }
            }
            return files;
        }
        fileTags: ["openssl_dist"]
        qbs.install: true
        qbs.installDir: buildconfig.installBinaryPath
    }

    Rule {
        multiplex: true
        alwaysRun: true
        condition: qbs.targetOS.contains("windows") && (qbs.architecture.contains("x86_64") || qbs.architecture.contains("x86"))
        inputs: ["openssl_dist"]
        outputFileTags: ["testSuit"]
        outputArtifacts: {
            var artifactNames = inputs["openssl_dist"].map(function(file){
                return FileInfo.joinPaths(product.buildDirectory, file.fileName);
            });

            var artifacts = artifactNames.map(function(art){
                var a = {
                    filePath: art,
                    fileTags: ["testSuit"]
                }
                return a;
            });
            return artifacts;
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "Preparing test suit";

            var sources = inputs["openssl_dist"].map(function(artifact) {
                return artifact.filePath;
            });

            cmd.sources = sources;
            cmd.destination = outputs["testSuit"].map(function(artifact) {
                return artifact.filePath;
            });
            cmd.sourceCode = function() {
                for (var i in sources) {
                    File.copy(sources[i], destination[i]);
                }
            };
            return [cmd];
        }
    }
}
