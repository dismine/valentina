import qbs.FileInfo
import qbs.Utilities

CppApplication {
  Depends { name: "buildconfig" }
  Depends { name: "Qt"; submodules: ["core"] }
  Depends { name: "VMiscLib"; }

  name: "EchoHelper"
  condition: buildconfig.enableUnitTests &&
             Utilities.versionCompare(Qt.core.version, "6") >= 0 &&
             project.withTextCodec &&
             qbs.targetOS.contains("unix")

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

  cpp.cxxLanguageVersion: "c++20"

  files: ["main.cpp"]


  Export {
      Depends { name: "cpp" }
      cpp.defines: {
          var defines = [];
          defines.push('ECHO_HELPER_BUILDDIR="' + FileInfo.joinPaths(exportingProduct.buildDirectory, exportingProduct.targetName) +'"');
          return defines;
      }
  }
}
