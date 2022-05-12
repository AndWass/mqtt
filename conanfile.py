from conans import ConanFile, CMake

class PurpleConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    requires = "boost/1.78.0"
    generators = "cmake",
    options = {"build_tests": [True, False]}
    default_options = {"boost:header_only": True, "build_tests": True}

    def requirements(self):
        if self.options.build_tests:
            self.requires("gtest/1.11.0")