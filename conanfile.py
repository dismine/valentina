import os

from conan import ConanFile

class Recipe(ConanFile):
  settings = "os"
  requires = "xerces-c/[~3.2]", "crashpad/cci.20220219", "breakpad/cci.20210521"
  options = {
      "with_xerces": [True, False], 
      "with_crash_reporting": [True, False]
  }
  default_options = {
      "xerces-c/*:shared": True, 
      "with_xerces": False, 
      "with_crash_reporting": False
  }

  def configure(self):
    if self.settings.os == "Linux":
      self.options["xerces-c"].shared = False

    if self.settings.os == "Macos" and "MACOS_DEPLOYMENT_TARGET" in os.environ:
      self.settings.os.version = os.environ["MACOS_DEPLOYMENT_TARGET"]

  def requirements(self):
    if not self.options.with_xerces:
      del self.requires["xerces-c"]

    if not self.options.with_crash_reporting:
      del self.requires["crashpad/cci.20220219"]
      del self.requires["breakpad/cci.20210521"]
