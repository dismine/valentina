import qbs.File
import qbs.FileInfo
import qbs.Utilities

VAppleApplicationDiskImage {
    Depends { name: "buildconfig" }
    Depends { name: "ib" }

    condition: qbs.targetOS.contains("macos") && bundleProbe.ready
    name: "Valentina DMG"
    targetName: "valentina"
    version: buildconfig.projectVersion
    builtByDefault: false
    codesign.enableCodeSigning: buildconfig.enableCodeSigning

    Probe {
        id: bundleProbe
        property string root: absoluteSourceBase
        property bool enableMultiBundle: buildconfig.enableMultiBundle
        property bool ready
        configure: {
            if (!enableMultiBundle)
                ready = File.exists(FileInfo.joinPaths(root, "Valentina.app"));
            else
                ready = File.exists(FileInfo.joinPaths(root, "Valentina.app"))
                        && File.exists(FileInfo.joinPaths(root, "Tape.app"))
                        && File.exists(FileInfo.joinPaths(root, "Puzzle.app"));
            found = true;
        }
    }

    Properties {
        condition: qbs.buildVariant !== "release"
        codesign.codesignFlags: ["--deep"]
        codesign.signingType: "ad-hoc"
    }

    Properties {
        condition: qbs.buildVariant === "release"
        codesign.codesignFlags: ["--options", "runtime"]
        codesign.signingType: "apple-id"
    }

    files: {
        var files = ["dist/macx/dmg/dmg.iconset"];

        // Tested on 2.1.1. At least this version doesn't crash even if the feature still doesn't work
        if (Utilities.versionCompare(qbs.version, "2.1.1") >= 0)
            files.push("dist/macx/dmg/en_US.lproj/LICENSE");

        return files;
    }

    Group {
        name: "Bundles"
        prefix: absoluteSourceBase + "/"
        files: {
            var files = ["Valentina.app"];

            if (buildconfig.enableMultiBundle)
            {
                files.push("Tape.app");
                files.push("Puzzle.app");
            }

            return files;
        }
        fileTags: "dmg.input.app"
    }

    // set to false to use a solid-color background (see dmg.backgroundColor below)
    property bool useImageBackground: true
    Group {
        name: "Background image"
        condition: useImageBackground
        files: {
            if (buildconfig.enableMultiBundle)
                return ["dist/macx/dmg/background_multibundle.tiff"]

            return ["dist/macx/dmg/background.tiff"]
        }
    }

    dmg.backgroundColor: "#41cd52"
    dmg.badgeVolumeIcon: false
    dmg.iconPositions: {
        if (buildconfig.enableMultiBundle) {
            return [{"x": 140, "y": 199, "path": "Valentina.app"},
                    {"x": 140, "y": 331, "path": "Tape.app"},
                    {"x": 140, "y": 450, "path": "Puzzle.app"},
                    {"x": 455, "y": 321, "path": "Applications"},];
        } else {
            return [{"x": 162, "y": 190, "path": "Valentina.app"},
                    {"x": 162, "y": 450, "path": "Applications"},];
        }
    }
    dmg.windowX: 420
    dmg.windowY: 250
    dmg.windowWidth: {
        if (buildconfig.enableMultiBundle)
            return 742;

        return 576;
    }

    dmg.windowHeight: 606
    dmg.iconSize: {
        if (buildconfig.enableMultiBundle)
            return 64;

        return 128;
    }
    dmg.format: {
        if (Qt.core.versionMajor >= 6)
            return "ULMO"; // macOS 10.15+ only

        if (Qt.core.versionMajor >= 5 && Qt.core.versionMinor >= 10)
            return "ULFO"; // OS X 10.11+ only

        return "UDBZ"; // OS X 10.4+ only
    }
}
