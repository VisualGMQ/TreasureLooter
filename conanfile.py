from conan import ConanFile
from conan.tools.cmake import cmake_layout


class ExampleRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("sdl/2.30.7", force=True)
        self.requires("sdl_ttf/2.22.0", force=True)
        self.requires("sdl_image/2.6.3", force=True)
        self.requires("sdl_mixer/2.8.0", force=True)

    def layout(self):
        cmake_layout(self)
