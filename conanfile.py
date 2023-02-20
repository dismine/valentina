import os

from conans import ConanFile

class Recipe(ConanFile):
  settings = "os"
  requires = "xerces-c/[>=3.2,<4.0]"
  default_options = {"xerces-c:shared": True}

  def configure(self):
    if self.settings.os == "Linux":
      self.options["xerces-c"].shared = False

    if self.settings.os == "Macos" and "MACOS_DEPLOYMENT_TARGET" in os.environ:
      self.settings.os.version = os.environ["MACOS_DEPLOYMENT_TARGET"]
