import qbs.File
import qbs.FileInfo
import qbs.TextFile

import "utils.js" as Utils

ModuleProvider {
    relativeSearchPaths: {
        var conanPackageDir = FileInfo.cleanPath(FileInfo.joinPaths(outputBaseDir, "../../..", "genconan"));
        var dirs = File.directoryEntries(conanPackageDir, File.AllDirs | File.NoDotAndDotDot);
        var packageVersions = {};

        /*
         * The generated conanbuildinfo.json files are inside of the 'genconan' folder in the build directory.
         * Since there might be more than one, they're placed inside of folders with a hashed name to avoid conflicts.
         * Here we have to iterate over those folders and process the json files within.
         */
        for(d in dirs) {
            var conanbuildinfo = FileInfo.joinPaths(conanPackageDir, dirs[d], "conanbuildinfo.json");

            if(!File.exists(conanbuildinfo)) {
                continue;
            }

            var file = new TextFile(conanbuildinfo, TextFile.ReadOnly);
            var fileContent = JSON.parse(file.readAll());

            file.close();

            var settings = fileContent.settings;
            var deps = fileContent.dependencies;

            for(i in deps){
                if(packageVersions[deps[i].name]) {
                    if(deps[i].version != packageVersions[deps[i].name]) {
                        console.error("Conan package '" + deps[i].name + "' found in multiple conanfile.txt's with different versions: " +
                                      packageVersions[deps[i].name] + " and " + deps[i].version);

                        return [];
                    }

                    console.info("Already generated module for conan package '" + deps[i].name + "', skipping...")

                    continue;
                }

                console.info("Generating module for conan package '" + deps[i].name + "'");
                packageVersions[deps[i].name] = deps[i].version;

                // module name can be invalid for Javascrip. Search for alternative names for cmake.
                var moduleName = deps[i].name;
                if (!Utils.isValidAttributeName(moduleName) && deps[i].hasOwnProperty("names"))
                {
                    if (deps[i].names.hasOwnProperty("cmake_find_package"))
                        moduleName = deps[i].names.cmake_find_package;
                    else if (deps[i].names.hasOwnProperty("cmake_find_package_multi"))
                        moduleName = deps[i].names.cmake_find_package_multi;
                }

                var moduleDir = FileInfo.joinPaths(outputBaseDir, "modules", name, moduleName);

                File.makePath(moduleDir);

                var moduleFile = new TextFile(FileInfo.joinPaths(moduleDir, moduleName + ".qbs"), TextFile.WriteOnly);

                var shared = false;
                if (fileContent.options.hasOwnProperty(deps[i].name) && fileContent.options[deps[i].name].hasOwnProperty("shared"))
                {
                    shared = (fileContent.options[deps[i].name].shared === 'True');
                }

                var cppLibraries = shared ? "\tcpp.dynamicLibraries: " : "\tcpp.staticLibraries: ";

                var cppLibrariesTag = "staticlibrary";
                if (shared)
                {
                    if (settings["os"] == "Linux" || settings["os"] == "Macos")
                    {
                        cppLibrariesTag = "dynamiclibrary";
                    }
                    else
                    {
                        cppLibrariesTag = "dynamiclibrary_import";
                    }
                }

                var cppLibrarySuffix = "cpp.staticLibrarySuffix";
                if (shared)
                {
                    if (settings["os"] == "Linux" || settings["os"] == "Macos")
                    {
                        cppLibrarySuffix = "cpp.dynamicLibrarySuffix";
                    }
                    else
                    {
                        cppLibrarySuffix = "cpp.staticLibrarySuffix";
                    }
                }

                moduleFile.write("import qbs\n" +
                                 "Module {\n" +
                                     "\tDepends { name: \"cpp\" }\n\n" +
                                     "\tproperty bool installBin: false\n" +
                                     "\tproperty bool installLib: false\n" +
                                     "\tproperty bool installRes: false\n" +
                                     "\tproperty bool installInclude: false\n" +
                                     "\tproperty string binInstallDir: \"bin\"\n" +
                                     "\tproperty string libInstallDir: \"lib\"\n" +
                                     "\tproperty string resInstallDir: \"res\"\n" +
                                     "\tproperty string includeInstallDir: \"include\"\n" +
                                     "\tproperty stringList binFilePatterns: [\"**/*\"]\n" +
                                     "\tproperty stringList libFilePatterns: [\"**/*\" + " + cppLibrarySuffix + "]\n" +
                                     "\tproperty stringList resFilePatterns: [\"**/*\"]\n" +
                                     "\tproperty stringList includeFilePatterns: [\"**/*\"]\n\n" +
                                     "\tcpp.includePaths: " + JSON.stringify(deps[i].include_paths) + "\n" +
                                     "\tcpp.systemIncludePaths: " + JSON.stringify(deps[i].include_paths) + "\n" +
                                     "\tcpp.libraryPaths: " + JSON.stringify(deps[i].lib_paths) + "\n" +
                                     cppLibraries + JSON.stringify(deps[i].libs) + "\n" +
                                     "\tcpp.defines: " + JSON.stringify(deps[i].defines) + "\n\n");

                function writeGroups(file, moduleName, prefix, pathList, install) {
                    for(j in pathList) {
                        file.write("\tGroup {\n" +
                                       "\t\tname: \"" + prefix + (j > 0 ? j : "") + "\"\n" +
                                       "\t\tprefix: \"" + FileInfo.fromNativeSeparators(pathList[j]) + "/\"\n" +
                                       "\t\tfilesAreTargets: true\n");

                        if (install)
                            file.write("\t\tqbs.install: product.conan." + moduleName + ".install" + (prefix.charAt(0).toUpperCase() + prefix.substring(1)) + "\n" +
                                       "\t\tqbs.installPrefix: \"\"\n" +
                                       "\t\tqbs.installDir: product.conan." + moduleName + "." + prefix + "InstallDir\n" +
                                       "\t\tqbs.installSourceBase: \"" + FileInfo.fromNativeSeparators(pathList[j]) + "\"\n");

                        file.write("\t\tfiles: product.conan." + moduleName + "." + prefix + "FilePatterns\n");

                        if (prefix === "lib")
                            file.write("\t\tfileTags: [\"" + cppLibrariesTag + "\"]\n");

                        file.write("\t}\n");
                    }
                }

                writeGroups(moduleFile, moduleName, "bin", deps[i].bin_paths, true);
                writeGroups(moduleFile, moduleName, "lib", deps[i].lib_paths, shared);
                writeGroups(moduleFile, moduleName, "res", deps[i].res_paths, true);
                writeGroups(moduleFile, moduleName, "include", Utils.filterUniqueRootPaths(deps[i].include_paths), shared);

                moduleFile.writeLine("}");
                moduleFile.close();
            }
        }

        return [""];
    }
}
