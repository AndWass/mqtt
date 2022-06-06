from conan import ConanFile


class Recipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps", "VirtualRunEnv"
    default_options = {
    }

    def layout(self):
        self.folders.generators = "conan"

    def requirements(self):
        self.requires("boost/[>=1.78.0]")

        # Testing only dependencies below
        self.requires("gtest/1.11.0")
