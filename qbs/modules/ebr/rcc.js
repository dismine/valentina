var Utilities = require("qbs.Utilities");
var FileInfo = require("qbs.FileInfo");
var Process = require("qbs.Process");

function fullPath(product)
{
    if (Utilities.versionCompare(product.Qt.core.version, "6.1") < 0)
        return product.Qt.core.binPath + '/' + product.Qt.core.rccName;
    return product.Qt.core.libExecPath + '/' + product.Qt.core.rccName;
}

function bound(min, val, max)
{
    return Math.max(min, Math.min(max, val));
}

function scanQrc(product, qrcFilePath) {
    var absInputDir = FileInfo.path(qrcFilePath);
    var result = [];
    var process = new Process();
    try {
        var rcc = FileInfo.joinPaths(fullPath(product) + FileInfo.executableSuffix());
        var exitCode = process.exec(rcc, ["--list", qrcFilePath], true);
        for (;;) {
            var line = process.readLine();
            if (!line)
                break;
            line = line.trim();
            line = FileInfo.relativePath(absInputDir, line);
            result.push(line);
        }
    } finally {
        process.close();
    }
    return result;
}
