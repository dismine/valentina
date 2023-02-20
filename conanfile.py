from conans import ConanFile

class Recipe(ConanFile):
  settings = "os"
  requires = "xerces-c/[>=3.2,<4.0]"
  default_options = {"xerces-c:shared": True}

  def configure(self):
    if self.settings.os == "Linux":
         self.options["xerces-c"].shared = False
