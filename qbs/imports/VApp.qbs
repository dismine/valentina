import qbs.FileInfo
import qbs.Utilities

CppApplication {
    Depends { name: "buildconfig" }
    Depends { name: "bundle"; condition: qbs.targetOS.contains("macos") }

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

    Properties {
        condition: qbs.targetOS.contains("unix") && buildconfig.buildWithCcache
        cpp.compilerWrapper: "ccache"
    }

    Properties {
        condition: Qt.core.versionMajor >= 5 && Qt.core.versionMinor < 12
        cpp.cxxLanguageVersion: "c++11"
    }

    // Since Qt 5.12 available support for C++17
    Properties {
        condition: Qt.core.versionMajor >= 5 && Qt.core.versionMinor >= 12
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
        prefix: project.sourceDirectory + "/dist/win/"
        files: {
            var files = [];

            if (qbs.toolchainType.contains("mingw"))
                files.push("msvcr120.dll");

            // Minimal supported OpenSSL version since Qt 5.12.4 is 1.1.1.
            if (Utilities.versionCompare(Qt.core.version, "5.12.4") >= 0) {
                if (qbs.architecture.contains("x86_64")) {
                    files.push(
                        "openssl/win64/libcrypto-1_1-x64.dll",
                        "openssl/win64/libssl-1_1-x64.dll"
                    );
                } else {
                    files.push(
                        "openssl/win32/libcrypto-1_1.dll",
                        "openssl/win32/libssl-1_1.dll"
                    );
                }
            } else {
                if (qbs.architecture.contains("x86_64")) {
                    files.push(
                        "openssl/win64/libeay32.dll",
                        "openssl/win64/ssleay32.dll"
                    );
                } else {
                    files.push(
                        "openssl/win32/libeay32.dll",
                        "openssl/win32/ssleay32.dll"
                    );
                }
            }
            return files;
        }
        fileTags: ["openssl_dist"]
        qbs.install: true
        qbs.installDir: buildconfig.installBinaryPath
    }
}
