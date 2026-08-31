import qbs.File
import qbs.FileInfo

// Detects immer (arximboldi/immer, header-only, BSL-1.0) installed as a system package: Ubuntu
// 24.04+ ships it as libimmer-dev, Homebrew ships an "immer" formula. Neither publishes a
// pkg-config file, so there is no module provider to depend on for it -- just probe the handful
// of prefixes both installers actually use for the top-level header. This is the default path;
// project.conanWithImmer:true switches VLib/VApp to fetching a pinned version through Conan
// instead (see VLib.qbs).
Module {
    Depends { name: "cpp" }

    readonly property bool found: immerProbe.found

    Probe {
        id: immerProbe
        property string includePath
        property bool found

        configure: {
            var candidates = ["/usr/include", "/usr/local/include", "/opt/homebrew/include"];
            found = false;
            for (var i = 0; i < candidates.length; ++i) {
                if (File.exists(FileInfo.joinPaths(candidates[i], "immer", "map.hpp"))) {
                    found = true;
                    includePath = candidates[i];
                    break;
                }
            }
        }
    }

    validate: {
        if (!found) {
            throw "immer headers not found. Install libimmer-dev (Debian/Ubuntu 24.04+) or the " +
                  "Homebrew \"immer\" formula (macOS), or build with project.enableConan:true " +
                  "project.conanWithImmer:true to fetch it through Conan instead.";
        }
    }

    cpp.systemIncludePaths: found ? [immerProbe.includePath] : []
}
