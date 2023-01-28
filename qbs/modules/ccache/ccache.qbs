import qbs.Process
import qbs.Utilities

// Don't forget to edit ccache config:
// ccache --set-config=sloppiness=pch_defines,time_macros,include_file_mtime,include_file_ctime
Module {
    readonly property bool ccachePresent: ccacheProbe.present
    readonly property bool pchSupport: ccachePCHProbe.pchSupport

    // change to shell tool which supports key "-c" to excecute command line
    property string shellTool: "bash"

    // ccache doesn't provide key to get only version number
    readonly property string command: "ccache --version | head -n 1 | grep -oE '[0-9]((\.)[0-9]){0,2}'"

    Probe {
        id: ccacheProbe

        property bool present
        property string tool: shellTool
        property string toolCommand: command

        configure: {
            var detector = new Process();
            try {
                if (detector.exec(tool, ["-c", toolCommand]) === 0) {
                    var version = detector.readStdOut().trim(); // we can read only one time
                    present = Utilities.versionCompare(version, "0.0.0") >= 0;
                    console.info("Found ccache version " + version + ".");
                }
            } finally {
                detector.close();
            }
        }
    }

    Probe {
        id: ccachePCHProbe
        condition: ccachePresent === true
        property string tool: shellTool
        property string toolCommand: command

        property bool pchSupport

        configure: {
            var detector = new Process();
            try {
                if (detector.exec(tool, ["-c", toolCommand]) === 0) {
                    var version = detector.readStdOut().trim(); // we can read only one time

                    pchSupport = Utilities.versionCompare(version, "3.1.0") >= 0;
                    if (!pchSupport && Utilities.versionCompare(version, "3.1.0") < 0){
                        console.info("ccache is tool old, version >= 3.1.0 required to work with precompiled headers.");
                        pchSupport = false;
                    }

                    if (pchSupport)
                        console.info("ccache supports compilation with precompiled headers.")
                }
            } finally {
                detector.close();
            }
        }
    }
}
