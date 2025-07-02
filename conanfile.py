import os

from conan import ConanFile

class Recipe(ConanFile):
  settings = "os"
  requires = "xerces-c/[~3.3]", "sentry-crashpad/0.6.5"
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

  def requirements(self):
    if not self.options.with_xerces:
      del self.requires["xerces-c"]

    if not self.options.with_crash_reporting:
      del self.requires["sentry-crashpad"]
