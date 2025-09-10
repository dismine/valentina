import os

from conan import ConanFile

class Recipe(ConanFile):
  settings = "os"
  generators = "QbsDeps"
  options = {
      "with_xerces": [True, False], 
      "with_crash_reporting": [True, False],
      "with_icu": [True, False],
      "with_iconv": [True, False],
  }
  default_options = {
      "xerces-c/*:shared": True, 
      "with_xerces": False, 
      "with_crash_reporting": False,
      "with_icu" : False,
      "with_iconv" : False,
  }

  def configure(self):
    if self.options.get_safe("with_xerces") and self.settings.os == "Linux":
      self.options["xerces-c"].shared = False

    if self.options.get_safe("with_icu") and self.settings.os == "Linux":
      self.options["icu"].data_packaging = "static"

  def requirements(self):
    if self.options.with_xerces:
        self.requires("xerces-c/3.3.0")

    if self.options.with_crash_reporting:
        self.requires("sentry-crashpad/0.6.5")

    if self.options.with_icu:
        self.requires("icu/77.1")
    elif self.options.with_iconv:
        self.requires("libiconv/1.18")
