import qbs.FileInfo
import qbs.Utilities

CppApplication {
    Depends { name: "buildconfig" }
    Depends { name: "bundle"; condition: qbs.targetOS.contains("macos") }
    Depends { name: "macdeployqt"; condition: qbs.targetOS.contains("macos") }

    Properties {
        condition: qbs.targetOS.contains("macos")
        cpp.minimumMacosVersion: buildconfig.minimumMacosVersion
        codesign.enableCodeSigning: buildconfig.enableCodeSigning
    }

    Properties {
        condition: qbs.targetOS.contains("macos") && qbs.buildVariant !== "release"
        codesign.codesignFlags: ["--deep"]
        codesign.signingType: "ad-hoc"
    }

    Properties {
        condition: qbs.targetOS.contains("macos") && qbs.buildVariant === "release"
        codesign.signingType: "apple-id"
        macdeployqt.signingIdentity: buildconfig.signingIdentity
        macdeployqt.signForNotarization: true
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
        qbs.install: true
        qbs.installDir: buildconfig.installBinaryPath
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
        Depends { name: "bundle"; condition: qbs.targetOS.contains("macos") }
        Depends { name: "macdeployqt"; condition: qbs.targetOS.contains("macos") }
    }
}
