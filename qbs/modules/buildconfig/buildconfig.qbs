import qbs.FileInfo
import qbs.File
import qbs.Utilities

Module {
    property bool staticBuild: true
    property bool frameworksBuild: qbs.targetOS.contains("macos") && !staticBuild

    property bool enableAddressSanitizer: false
    property bool enableMemorySanitizer: false
    property bool enableUbSanitizer: false
    property bool enableThreadSanitizer: false

    // Use this property to disable building unit tests.
    property bool enableUnitTests: true

    // Use this property to disable the use of rpath. This can be used when packaging Valentina for distributions which
    // do not permit the use of rpath, such as Fedora.
    property bool enableRPath: true

    // Use this property to disable the use of precompiled headers. This can be used when packaging Valentina for
    // distributions which provides not enough space on build servers.
    property bool enablePCH: true

    property bool enableAppImage: false

    property bool enableMultiBundle: false

    property string valentina_copyright_year: { return new Date().getFullYear().toString(); }
    property string valentina_copyright_string: "(C) 2013-" + valentina_copyright_year + ", Valentina project"

    Depends { name: "ccache"; }

    // Use this property to disable the use of ccache.
    property bool enableCcache: true
    readonly property bool ccachePresent: ccache.ccachePresent
    readonly property bool ccachePCHSupport: ccache.pchSupport
    readonly property bool buildWithCcache: enableCcache && (enablePCH && ccachePresent && ccachePCHSupport || (!enablePCH && ccachePresent))

    property bool treatWarningsAsErrors: true

    readonly property bool useConanPackages : {
        if (Utilities.versionCompare(Qt.core.version, "6") < 0)
            return false;

        return project.enableConan;
    }

    readonly property bool enableCodeSigning: project.enableSigning

    property string signingIdentity: "-"

    property string libDirName: "lib"

    property string appTarget
    property string projectVersion: "0.7.52"

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
        else if (qbs.targetOS.contains("windows"))
            return installAppPath
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

            "QBS_BUILD",
            "QT_NO_FOREACH"
        ];

        if (qbs.targetOS.contains("unix")) {
            defines.push('BINDIR="' + FileInfo.joinPaths(qbs.installPrefix, "bin") + '"');
            const dataDir = FileInfo.joinPaths(qbs.installPrefix, "share");
            defines.push('DATADIR="' + dataDir + '"');
            defines.push('PKGDATADIR="' + FileInfo.joinPaths(dataDir, "valentina") + '"');
        }

        if (qbs.buildVariant !== "debug")
            defines.push('V_NO_ASSERT');

        if (enableAppImage && qbs.targetOS.contains("unix") && !qbs.targetOS.contains("macos"))
            defines.push('APPIMAGE');

        if (enableMultiBundle)
            defines.push('MULTI_BUNDLE');

        return defines;
    }

    cpp.visibility: "minimal"

    readonly property string minimumMacosVersion: {
        if (project.minimumMacosVersion !== undefined)
            return project.minimumMacosVersion;

        // Check which minimal OSX version supports current Qt version
        if (Qt.core.versionMajor >= 6) {
            // For Qt 6.5 https://doc-snapshots.qt.io/qt6-6.5/supported-platforms.html
            if (Qt.core.versionMinor >= 5) // Qt 6.5
                return "11.0";

            // See page https://doc.qt.io/qt-6.4/supported-platforms.html
            // According to the documentation minimal version must be 10.14. But for some reason it requires 10.15 to
            // build.
            return "10.15"; // Qt 6.4 and above
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

            return "10.7";
        }

        return undefined;
    }

    cpp.separateDebugInformation: true
    cpp.treatWarningsAsErrors: treatWarningsAsErrors

    property stringList debugFlags: {
        var debugFlags = [];
        if (qbs.toolchain.contains("gcc")) {
            if (qbs.toolchain.contains("clang")) {
                debugFlags.push(
                    "-gdwarf-4",
                    "-fparse-all-comments",
                    "-Wabi",
                    "-Wabstract-final-class",
                    "-Wabstract-vbase-init",
                    "-Waddress",
                    "-Waddress-of-temporary",
                    "-Waggregate-return",
                    "-Wall",
                    "-Wambiguous-macro",
                    "-Wambiguous-member-template",
                    "-Wanalyzer-incompatible-plugin",
                    "-Wanonymous-pack-parens",
                    "-Warc",
                    "-Warc-bridge-casts-disallowed-in-nonarc",
                    "-Warc-maybe-repeated-use-of-weak",
                    "-Warc-non-pod-memaccess",
                    "-Warc-performSelector-leaks",
                    "-Warc-repeated-use-of-weak",
                    "-Warc-retain-cycles",
                    "-Warc-unsafe-retained-assign",
                    "-Warray-bounds",
                    "-Warray-bounds-pointer-arithmetic",
                    "-Wasm",
                    "-Wasm-operand-widths",
                    "-Wassign-enum",
                    "-Watomic-properties",
                    "-Watomic-property-with-user-defined-accessor",
                    "-Wattributes",
                    "-Wauto-import",
                    "-Wauto-storage-class",
                    "-Wauto-var-id",
                    "-Wavailability",
                    "-Wbackslash-newline-escape",
//                    "-Wbad-array-new-length",
                    "-Wbad-function-cast",
                    "-Wbind-to-temporary-copy",
                        "-Wno-c++98-compat-bind-to-temporary-copy",
                    "-Wbitfield-constant-conversion",
                    "-Wbitwise-op-parentheses",
                    "-Wbool-conversion",
                    "-Wbool-conversions",
                    "-Wbuiltin-macro-redefined",
                    "-Wbuiltin-requires-header",
                    "-Wc++-compat",
                    "-Wc++0x-compat",
                    "-Wc++0x-extensions",
                    "-Wc++0x-narrowing",
                    "-Wc++11-compat",
                    "-Wc++11-compat-pedantic",
                    "-Wc++11-compat-reserved-user-defined-literal",
                    "-Wc++11-extensions",
                    "-Wc++11-extra-semi",
                    "-Wc++11-long-long",
                    "-Wc++11-narrowing",
                    "-Wc11-extensions",
                    "-Wcast-align",
                    "-Wcast-of-sel-type",
                    "-Wcast-qual",
                    "-WCFString-literal",
                    "-Wchar-align",
                    "-Wchar-subscripts",
                    "-Wcomment",
                    "-Wcomments",
                    "-Wcompare-distinct-pointer-types",
                    "-Wcomplex-component-init",
                    "-Wconditional-type-mismatch",
                    "-Wconditional-uninitialized",
                    "-Wconfig-macros",
                    "-Wconstant-conversion",
                    "-Wconstant-logical-operand",
                    "-Wconstexpr-not-const",
                    "-Wconversion",
                    "-Wconversion-null",
                    "-Wno-covered-switch-default",
                    "-Wctor-dtor-privacy",
                    "-Wcustom-atomic-properties",
                    "-Wdangling-else",
                    "-Wdangling-field",
                    "-Wdangling-initializer-list",
                    "-Wdeclaration-after-statement",
                    "-Wdelegating-ctor-cycles",
                    "-Wdelete-incomplete",
                    "-Wdelete-non-virtual-dtor",
                    "-Wdeprecated",
                    "-Wdeprecated-declarations",
                    "-Wdeprecated-implementations",
                    "-Wdeprecated-objc-isa-usage",
                    "-Wdeprecated-objc-pointer-introspection",
                    "-Wdeprecated-objc-pointer-introspection-performSelector",
                    "-Wdeprecated-register",
                    "-Wdeprecated-writable-strings",
                    "-Wdirect-ivar-access",
//                   "-Wdisabled-macro-expansion", Disabled
                    "-Wdisabled-optimization",
                    "-Wdiscard-qual",
                    "-Wdistributed-object-modifiers",
                    "-Wdiv-by-zero",
                    "-Wdivision-by-zero",
                    "-Wdocumentation",
                    "-Wdocumentation-deprecated-sync",
                    "-Wdocumentation-html",
                    "-Wdollar-in-identifier-extension",
                    "-Wduplicate-decl-specifier",
                    "-Wduplicate-enum",
                    "-Wduplicate-method-arg",
                    "-Wduplicate-method-match",
                    "-Wdynamic-class-memaccess",
                    "-Weffc++",
                    "-Wembedded-directive",
                    "-Wempty-body",
                    "-Wempty-translation-unit",
                    "-Wendif-labels",
                    "-Wenum-compare",
                    "-Wenum-conversion",
                    "-Wexplicit-ownership-type",
                    "-Wextern-initializer",
                    "-Wextra",
                    "-Wextra-semi",
                    "-Wextra-tokens",
                    "-Wflexible-array-extensions",
                    "-Wfloat-equal",
                    "-Wformat",
                    "-Wformat-extra-args",
                    "-Wformat-invalid-specifier",
                    "-Wformat-non-iso",
                    "-Wformat-nonliteral",
                    "-Wformat-security",
                    "-Wformat-y2k",
                    "-Wformat-zero-length",
                    "-Wformat=2",
                    "-Wfour-char-constants",
                    "-Wgcc-compat",
                    "-Wgnu",
                    "-Wgnu-array-member-paren-init",
                    "-Wgnu-designator",
                    "-Wgnu-static-float-init",
                    "-Wheader-guard",
                    "-Wheader-hygiene",
                    "-Widiomatic-parentheses",
                    "-Wignored-attributes",
                    "-Wignored-qualifiers",
                    "-Wimplicit",
                    "-Wimplicit-atomic-properties",
                    "-Wimplicit-conversion-floating-point-to-bool",
                    "-Wimplicit-exception-spec-mismatch",
                    "-Wimplicit-fallthrough",
                    "-Wimplicit-fallthrough-per-function",
                    "-Wimplicit-function-declaration",
                    "-Wimplicit-int",
                    "-Wimplicit-retain-self",
                    "-Wimport",
                    "-Wimport-preprocessor-directive-pedantic",
                    "-Wincompatible-library-redeclaration",
                    "-Wincompatible-pointer-types",
                    "-Wincompatible-pointer-types-discards-qualifiers",
                    "-Wincomplete-implementation",
                    "-Wincomplete-module",
                    "-Wincomplete-umbrella",
                    "-Winit-self",
                    "-Winitializer-overrides",
//                    "-Winline",
                    "-Wint-conversion",
                    "-Wint-conversions",
                    "-Wint-to-pointer-cast",
                    "-Wint-to-void-pointer-cast",
                    "-Winteger-overflow",
                    "-Winvalid-constexpr",
                    "-Winvalid-iboutlet",
                    "-Winvalid-noreturn",
                    "-Winvalid-offsetof",
                    "-Winvalid-pch",
                    "-Winvalid-pp-token",
                    "-Winvalid-source-encoding",
                    "-Winvalid-token-paste",
                    "-Wknr-promoted-parameter",
                    "-Wlanguage-extension-token",
                    "-Wlarge-by-value-copy",
                    "-Wliteral-conversion",
                    "-Wliteral-range",
                    "-Wlocal-type-template-args",
                        "-Wno-c++98-compat-local-type-template-args",
                    "-Wlogical-not-parentheses",
                    "-Wlogical-op-parentheses",
//                    "-Wlong-long", // We have been using C++11
                    "-Wloop-analysis",
                    "-Wmain",
                    "-Wmain-return-type",
                    "-Wmalformed-warning-check",
                    "-Wmethod-signatures",
                    "-Wmicrosoft",
                    "-Wmicrosoft-exists",
                    "-Wmismatched-parameter-types",
                    "-Wmismatched-return-types",
                    "-Wmismatched-tags",
                    "-Wmissing-braces",
                    "-Wmissing-declarations",
                    "-Wmissing-field-initializers",
                    "-Wmissing-format-attribute",
                    "-Wmissing-include-dirs",
                    "-Wmissing-method-return-type",
                    "-Wmissing-noreturn",
                    "-Wmissing-prototypes",
                    "-Wmissing-selector-name",
                    "-Wmissing-sysroot",
                    "-Wmissing-variable-declarations",
                    "-Wmodule-conflict",
                    "-Wmost",
                    "-Wmultichar",
                    "-Wnarrowing",
                    "-Wnested-externs",
                    "-Wnewline-eof",
                    "-Wnon-gcc",
                    "-Wnon-literal-null-conversion",
                    "-Wnon-pod-varargs",
                    "-Wnon-virtual-dtor",
                    "-Wnonnull",
                    "-Wnonportable-cfstrings",
                    "-Wno-c++98-compat",
                    "-WNSObject-attribute",
                    "-Wnull-arithmetic",
                    "-Wnull-character",
                    "-Wnull-conversion",
                    "-Wnull-dereference",
                    "-Wodr",
                    "-Wold-style-cast",
                    "-Wold-style-definition",
                    "-Wout-of-line-declaration",
                    "-Wover-aligned",
                    "-Woverflow",
                    "-Woverlength-strings",
                    "-Woverloaded-shift-op-parentheses",
                    "-Woverloaded-virtual",
                    "-Woverriding-method-mismatch",
                    "-Wpacked",
                    "-Wparentheses",
                    "-Wparentheses-equality",
                    "-Wpedantic",
                    "-Wpointer-arith",
                    "-Wpointer-sign",
                    "-Wpointer-to-int-cast",
                    "-Wpointer-type-mismatch",
                    "-Wpredefined-identifier-outside-function",
                    "-Wprivate-extern",
                    "-Wprotocol",
                    "-Wprotocol-property-synthesis-ambiguity",
                    "-Wreadonly-iboutlet-property",
                    "-Wreceiver-expr",
                    "-Wreceiver-forward-class",
                    "-Wredundant-decls",
                    "-Wreinterpret-base-class",
                    "-Wreorder",
                    "-Wrequires-super-attribute",
                    "-Wreserved-user-defined-literal",
                    "-Wreturn-stack-address",
                    "-Wreturn-type",
                    "-Wreturn-type-c-linkage",
                    "-Wsection",
                    "-Wselector",
                    "-Wselector-type-mismatch",
                    "-Wself-assign",
                    "-Wself-assign-field",
                    "-Wsemicolon-before-method-body",
                    "-Wsentinel",
                    "-Wsequence-point",
                    "-Wserialized-diagnostics",
                    "-Wshadow-ivar",
                    "-Wshift-count-negative",
                    "-Wshift-count-overflow",
                    "-Wshift-op-parentheses",
                    "-Wshift-overflow",
                    "-Wshift-sign-overflow",
                    "-Wshorten-64-to-32",
                    "-Wsign-compare",
                    "-Wsign-conversion",
                    "-Wsign-promo",
                    "-Wsizeof-array-argument",
                    "-Wsizeof-array-decay",
                    "-Wsizeof-pointer-memaccess",
                    "-Wsometimes-uninitialized",
                    "-Wsource-uses-openmp",
//                    "-Wstack-protector",
                    "-Wstatic-float-init",
                    "-Wstatic-in-inline",
                    "-Wstatic-local-in-inline",
                    "-Wstatic-self-init",
                    "-Wstrict-aliasing",
                    "-Wstrict-aliasing=0",
                    "-Wstrict-aliasing=1",
                    "-Wstrict-aliasing=2",
                    "-Wstrict-overflow",
                    "-Wstrict-overflow=0",
                    "-Wstrict-overflow=1",
                    "-Wstrict-overflow=2",
                    "-Wstrict-overflow=3",
                    "-Wstrict-overflow=4",
                    "-Wstrict-overflow=5",
                    "-Wno-error=strict-overflow",
                    "-Wstrict-prototypes",
                    "-Wstrict-selector-match",
                    "-Wstring-compare",
                    "-Wstring-conversion",
                    "-Wstring-plus-int",
                    "-Wstrlcpy-strlcat-size",
                    "-Wstrncat-size",
                    "-Wsuper-class-method-mismatch",
                    "-Wswitch",
                    "-Wswitch-default",
                    "-Wsynth",
                    "-Wtautological-compare",
                    "-Wtautological-constant-out-of-range-compare",
                    "-Wtentative-definition-incomplete-type",
                    "-Wthread-safety",
                    "-Wthread-safety-analysis",
                    "-Wthread-safety-attributes",
                    "-Wthread-safety-beta",
                    "-Wthread-safety-precise",
                    "-Wtrigraphs",
                    "-Wtype-limits",
                    "-Wtype-safety",
                    "-Wtypedef-redefinition",
                    "-Wtypename-missing",
                    "-Wundeclared-selector",
                    "-Wundef",
                    "-Wundefined-inline",
                    "-Wundefined-internal",
                    "-Wundefined-reinterpret-cast",
                    "-Wunicode",
                    "-Wunicode-whitespace",
                    "-Wuninitialized",
                    "-Wunnamed-type-template-args",
                    "-Wunneeded-internal-declaration",
                    "-Wunneeded-member-function",
                    "-Wunsequenced",
                    "-Wunsupported-visibility",
                    "-Wunused",
                    "-Wunused-argument",
                    "-Wunused-command-line-argument",
                    "-Wunused-comparison",
                    "-Wunused-exception-parameter",
                    "-Wunused-function",
                    "-Wunused-label",
                    "-Wunused-member-function",
                    "-Wunused-parameter",
                    "-Wunused-private-field",
                    "-Wunused-result",
                    "-Wunused-value",
                    "-Wunused-variable",
                    "-Wunused-volatile-lvalue",
                    "-Wuser-defined-literals",
                    "-Wvarargs",
                    "-Wvariadic-macros",
                    "-Wvector-conversion",
                    "-Wvector-conversions",
                    "-Wvexing-parse",
                    "-Wvisibility",
                    "-Wvla",
                    "-Wvla-extension",
                    "-Wvolatile-register-var",
//                    "-Wweak-vtables",
                    "-Wwrite-strings",
                    "-Wzero-length-array",
                    "-Qunused-arguments",
                    "-fcolor-diagnostics",
                    "-Wno-gnu-zero-variadic-macro-arguments",
                    "-fms-extensions" // Need for pragma message
                )

                if (Utilities.versionCompare(cpp.compilerVersion, "14") < 0) {
                    debugFlags.push("-Wweak-template-vtables")
                }

                if (Utilities.versionCompare(cpp.compilerVersion, "13") >= 0) {
                    debugFlags.push(
                        "-Wreserved-identifier",
                        "-Wno-error=reserved-identifier",
                        "-Wunused-but-set-parameter",
                        "-Wunused-but-set-variable"
                    )
                }

                if (Utilities.versionCompare(cpp.compilerVersion, "15") >= 0) {
                    debugFlags.push(
                        "-Warray-parameter"
                    )
                }

                if (qbs.targetOS.contains("macos")) {
                    // Cannot suppress warnings from Qt headers
                    debugFlags.push(
                        "-Wno-sign-conversion",
                        "-Wno-duplicate-enum",
                        "-Wno-shorten-64-to-32",
                        "-Wno-deprecated"
                    )
                }

                if (qbs.buildVariant === "release") {
                    debugFlags.push("-Wno-unknown-warning-option")
                }

                if (qbs.targetOS.contains("windows")) {
                    // Don't use additional keys on Windows system.
                    // Can't find a way to ignore Qt header on there.
                    debugFlags.push(
                        "-Wall",
                        "-Wextra",
                        "-pedantic",
                        "-Wno-gnu-zero-variadic-macro-arguments",
                        "-fms-extensions # Need for pragma message"
                    )
                }
            } else if (!qbs.toolchain.contains("clang")) {
                debugFlags.push(
                    "-Wall",
                    "-Wextra",
                    "-pedantic",
                    "-Weffc++",
                    "-Woverloaded-virtual",
                    "-Wctor-dtor-privacy",
                    "-Wnon-virtual-dtor",
                    "-Wold-style-cast",
                    "-Wconversion",
                    "-Winit-self",
//                    "-Wstack-protector",
                    "-Wunreachable-code",
                    "-Wcast-align",
                    "-Wcast-qual",
                    "-Wdisabled-optimization",
                    "-Wfloat-equal",
                    "-Wformat=2",
                    "-Wimport",
//                    "-Wmissing-include-dirs",
                    "-Wpacked",
                    "-Wredundant-decls",
//                    "-Winline",
                    "-Winvalid-pch",
//                    "-Wunsafe-loop-optimizations",
//                    "-Wlong-long", // We have been using C++11
                    "-Wmissing-format-attribute",
                    "-Wswitch-default",
                    "-Wuninitialized",
                    "-Wvariadic-macros",
                    "-Wlogical-op",
                    "-Wnoexcept",
                    "-Wmissing-noreturn",
                    "-Wpointer-arith",
                    "-Wstrict-null-sentinel",
                    "-Wstrict-overflow=5",
                    "-Wno-error=strict-overflow",
                    "-Wundef",
                    "-Wno-unused",
                    "-Wno-pragmas"
                )

                if (Utilities.versionCompare(cpp.compilerVersion, "5") >= 0) {
                    debugFlags.push(
                        "-Wswitch-bool",
                        "-Wlogical-not-parentheses",
                        "-Wsizeof-array-argument",
//                        "-Wsuggest-final-types",
//                        "-Wsuggest-final-methods",
                        "-Wbool-compare"
                    )
                }

                if (Utilities.versionCompare(cpp.compilerVersion, "6") >= 0) {
                    debugFlags.push(
                        "-Wshift-negative-value",
                        "-Wshift-overflow",
                        "-Wshift-overflow=2",
                        "-Wtautological-compare",
//                        "-Wnull-dereference",
                        "-Wduplicated-cond",
                        "-Wmisleading-indentation"
                    )
                }

                if (Utilities.versionCompare(cpp.compilerVersion, "7") >= 0) {
                    debugFlags.push(
                        "-Wduplicated-branches",
                        "-Wrestrict",
                        "-Walloc-zero",
                        "-Wnonnull",
                        "-Wno-stringop-overflow" // cannot suppress warning in Qt headers
                    )
                }

                if (Utilities.versionCompare(cpp.compilerVersion, "8") >= 0) {
                    debugFlags.push(
                        "-Wmultistatement-macros",
                        "-Warray-bounds"
                    )
                }

                if (Utilities.versionCompare(cpp.compilerVersion, "10") >= 0) {
                    debugFlags.push(
                        "-Wredundant-tags"
                    )
                }

                if (Utilities.versionCompare(cpp.compilerVersion, "11") >= 0) {
                    debugFlags.push(
                        "-Wctad-maybe-unsupported"
                    )
                }

                if (qbs.targetOS.contains("unix") && !qbs.targetOS.contains("macos") &&
                        Utilities.versionCompare(cpp.compilerVersion, "10") >= 0) {
                    debugFlags.push(
                        "-Wno-mismatched-tags" // bugged in gcc
                    )
                }

                if (qbs.targetOS.contains("windows")) {
                    // Don't use additional keys on Windows system.
                    // Can't find a way to ignore Qt header on there.
                    debugFlags.push(
                        "-Wall",
                        "-Wno-pragmas",
                        "-Wno-error=strict-overflow",
                        "-Wno-strict-overflow",
                        "-Wextra"
                    )

                    if (Qt.core.versionMajor === 5  &&  Qt.core.versionMinor === 6) {
                        debugFlags.push(
                            "-Wno-array-bounds" // annoying warning
                        )
                    }
                }
            }
        }
        else if (qbs.toolchain.contains("msvc")) {
            debugFlags.push(
                "-Wall",
                "-wd4061", // enum value is not *explicitly* handled in switch
                "-wd4099", // first seen using 'struct' now seen using 'class'
                "-wd4127", // conditional expression is constant
                "-wd4217", // member template isn't copy constructor
                "-wd4250", // inherits (implements) some member via dominance
                "-wd4251", // needs to have dll-interface to be used by clients
                "-wd4275", // exported class derived from non-exported class
                "-wd4347", // "behavior change", function called instead of template
                "-wd4355", // "'this': used in member initializer list
                "-wd4505", // unreferenced function has been removed
                "-wd4510", // default constructor could not be generated
                "-wd4511", // copy constructor could not be generated
                "-wd4512", // assignment operator could not be generated
                "-wd4513", // destructor could not be generated
                "-wd4514", // 'function' : unreferenced inline function has been removed
                "-wd4610", // can never be instantiated user defined constructor required
                "-wd4623", // default constructor could not be generated
                "-wd4624", // destructor could not be generated
                "-wd4625", // copy constructor could not be generated
                "-wd4626", // assignment operator could not be generated
                "-wd4640", // a local static object is not thread-safe
                "-wd4661", // a member of the template class is not defined.
                "-wd4670", // a base class of an exception class is inaccessible for catch
                "-wd4672", // a base class of an exception class is ambiguous for catch
                "-wd4673", // a base class of an exception class is inaccessible for catch
                "-wd4675", // resolved overload was found by argument-dependent lookup
                "-wd4702", // unreachable code, e.g. in <list> header.
                "-wd4710", // call was not inlined
                "-wd4711", // call was inlined
                "-wd4820", // some padding was added
                "-wd4917", // a GUID can only be associated with a class, interface or namespace
                "-wd4351", // elements of array 'array' will be default initialized
                           // The following are real warnings but are generated by almost all MS headers, including
                           // standard library headers, so it's impractical to leave them on.
                "-wd4619", // there is no warning number 'XXXX'
                "-wd4668", // XXX is not defined as a preprocessor macro
                "-wd5045", // Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
                           // Because Microsoft doesn't provide a way to suppress warnings in headers we will suppress
                           // all warnings we meet in headers globally
                "-wd4548",
                "-wd4350",
                "-wd4242",
                "-wd4265",
                "-wd4599",
                "-wd4371",
                "-wd4718",
                "-wd4946",
                "-wd4868", // compiler may not enforce left-to-right evaluation order in braced initializer list
                "-wd4866", // compiler may not enforce left-to-right evaluation order for call to 'C++17 operator'
                "-wd4571", // catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
                "-wd5026", // move constructor was implicitly defined as deleted
                "-wd5027", // move assignment operator was implicitly defined as deleted
                "-wd5039", // pointer or reference to potentially throwing function passed to extern C function under -EHc.
                "-wd4774", // format string expected in argument 3 is not a string literal
                "-wd5204", // class has virtual functions, but its trivial destructor is not virtual; instances of objects derived
                           // from this class may not be destructed correctly
                "-wd5219", // implicit conversion from '<type>' to '<type>', possible loss of data
                "-wd5243", // 'type-name': using incomplete class 'class-name' can cause potential one definition rule violation due
                           // to ABI limitation
                "-wd4464", // relative include path contains '..'
                "-wd4458", // declaration of 'X' hides class member
                "-wd4456", // declaration of 'X' hides previous local declaration
                "-wd4459", // declaration of 'X' hides global declaration
                "-wd4457", // declaration of 'X' hides function parameter
                "-wd4125"  // decimal digit terminates octal escape sequence
            )
        }
        return debugFlags;
    }

    cpp.cxxFlags: {
        var flags = debugFlags;
        if ((qbs.toolchain.contains("gcc") || qbs.toolchain.contains("clang")) &&
                (enableAddressSanitizer || enableMemorySanitizer))
            flags.push("-fno-omit-frame-pointer");

        if ((qbs.toolchain.contains("gcc") || qbs.toolchain.contains("clang")) && enableMemorySanitizer)
            flags.push("-fPIE", "-pie", "-fsanitize-memory-track-origins");

        if (qbs.toolchain.contains("msvc"))
            flags.push("/utf-8");
        return flags;
    }

    Properties {
        cpp.systemIncludePaths: {
            var paths = [FileInfo.joinPaths(product.buildDirectory, "qt.headers")];

            if (Utilities.versionCompare(Qt.core.version, "6") >= 0 && !useConanPackages)
            {
                var includePaths = product.cpp.includePaths;

                for(var i = 0; i < includePaths.length; i++)
                {
                    if (includePaths[i].indexOf("xerces-c") !== -1)
                    {
                        paths.push(includePaths[i]);
                    }
                }

                if (qbs.targetOS.contains("unix"))
                {
                    var xercescHeaders = "/usr/local/include/xercesc";
                    if (File.exists(xercescHeaders) && !paths.contains(xercescHeaders))
                    {
                        paths.push(xercescHeaders);
                    }
                }
            }

            if (Utilities.versionCompare(qbs.version, "1.22") < 0)
            {
                var qtLibs = [
                    "QtCore",
                    "QtSvg",
                    "QtXml",
                    "QtPrintSupport",
                    "QtXmlPatterns",
                    "QtWidgets",
                    "QtGui",
                    "QtNetwork",
                    "QtTest",
                    "QtConcurrent"
                ];

                if (!qbs.targetOS.contains("macos"))
                {
                    paths.push(Qt.core.incPath);

                    for (var i = 0; i < qtLibs.length; i++) {
                        paths.push(FileInfo.joinPaths(Qt.core.incPath, qtLibs[i]));
                    }

                } else {
                    for (var i = 0; i < qtLibs.length; i++) {
                        paths.push(FileInfo.joinPaths(Qt.core.incPath,
                                                      qtLibs[i] + ".framework/Versions/" + Qt.core.versionMajor +
                                                      "/Headers"));
                        paths.push(FileInfo.joinPaths(Qt.core.incPath, qtLibs[i] + ".framework/Headers"));
                    }
                }
            }

            return paths;
        }
    }

    Properties {
        condition: qbs.toolchain.contains("gcc") || qbs.toolchain.contains("clang")
        cpp.driverFlags: {
            var flags = [];
            if (enableAddressSanitizer)
                flags.push("-fsanitize=address");
            if (enableMemorySanitizer)
                flags.push("-fsanitize=memory");
            if (enableUbSanitizer)
                flags.push("-fsanitize=undefined");
            if (enableThreadSanitizer)
                flags.push("-fsanitize=thread");
            return flags;
        }
    }

    vcs2.headerFileName: "vcsRepoState.h"
}
