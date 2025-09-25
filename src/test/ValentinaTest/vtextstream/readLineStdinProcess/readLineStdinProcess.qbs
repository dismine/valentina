import qbs.FileInfo
import qbs.Utilities

CppApplication {
  Depends { name: "buildconfig" }
  Depends { name: "Qt"; submodules: ["core"] }
  Depends { name: "VMiscLib"; }

  name: "readLineStdinProcess"
  condition: buildconfig.enableUnitTests &&
             Utilities.versionCompare(Qt.core.version, "6") >= 0

  qbsModuleProviders: {
      var providers = ["Qt"];

      if (buildconfig.useConanPackages)
          providers.push("conan");

      providers.push("qbspkgconfig");
      return providers;
  }

  Properties {
      condition: buildconfig.useConanPackages
      moduleProviders.conan.installDirectory: project.conanInstallPath
  }

  Properties {
      condition: qbs.targetOS.contains("macos")
      cpp.rpaths: [
          FileInfo.joinPaths(cpp.rpathOrigin, "..", "install-root", product.qbs.installPrefix,  product.buildconfig.installAppPath + "/Valentina.app/Contents/Frameworks"),
          Qt.core.libPath
      ]
  }

  Properties {
      condition: qbs.targetOS.contains("unix") && buildconfig.buildWithCcache
      cpp.compilerWrapper: "ccache"
  }

  Properties {
      condition: qbs.targetOS.contains("macos")
      bundle.isBundle: false
  }

  cpp.cxxLanguageVersion: {
      if (Qt.core.versionMajor >= 6) // Start building with C++20 since Qt 6.0
          return "c++20";
      return "c++17";
  }

  files: ["main.cpp"]


  Export {
      Depends { name: "cpp" }
      cpp.defines: {
          var defines = [];
          defines.push('READ_LINE_STDIN_PROCESS_BUILDDIR="' + FileInfo.joinPaths(exportingProduct.buildDirectory, exportingProduct.targetName) +'"');
          return defines;
      }
  }
}
