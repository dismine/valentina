var FileInfo = require("qbs.FileInfo");

function isValidAttributeName(name) {
    // Check if the name starts with a letter, underscore, or dollar sign
    if (!/^[a-zA-Z_$]/.test(name.charAt(0))) {
        return false;
    }

    // Check for subsequent characters: letters, digits, underscores, or dollar signs
    for (var i = 1; i < name.length; i++) {
        if (!/^[0-9a-zA-Z_$]/.test(name.charAt(i))) {
            return false;
        }
    }

    // Check if the name is a reserved word
    const reservedWords = [
        'abstract', 'await', 'boolean', 'break', 'byte', 'case', 'catch', 'char', 'class', 'const',
        'continue', 'debugger', 'default', 'delete', 'do', 'double', 'else', 'enum', 'export', 'extends',
        'false', 'final', 'finally', 'float', 'for', 'function', 'goto', 'if', 'implements', 'import',
        'in', 'instanceof', 'int', 'interface', 'let', 'long', 'native', 'new', 'null', 'package',
        'private', 'protected', 'public', 'return', 'short', 'static', 'super', 'switch', 'synchronized',
        'this', 'throw', 'throws', 'transient', 'true', 'try', 'typeof', 'var', 'void', 'volatile', 'while', 'with', 'yield'
    ];

    if (reservedWords.includes(name)) {
        return false;
    }

    // If all checks passed, the name is valid
    return true;
}

function filterUniqueRootPaths(paths) {
    if (paths.length < 2) {
        return paths;
    }

    // Always compare canonocal paths
    const canonicalPaths = paths.map(function(folder) {
      return FileInfo.cleanPath(folder);
    });

    const isChildOf = function(child, parent) {
        if (child === parent) return false;

        var parentTokens = parent.split('/').filter(function(i) {
            return i.length;
        });

        var childTokens = child.split('/').filter(function(i) {
            return i.length;
        });

        return parentTokens.every(function(t, i) {
            return childTokens[i] === t;
        });
    }

    return canonicalPaths.filter(function(folder) {
        return !canonicalPaths.some(function(otherFolder) {
            return folder !== otherFolder && isChildOf(folder, otherFolder);
        });
    });
}
