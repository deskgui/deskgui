from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps

class DeskguiRecipe(ConanFile):
    name = "deskgui"
    version = "1.0"
    package_type = "library"

    # Optional metadata
    license = "MIT"
    author = "Marc Ortuño <mob.bdn@gmail.com>"
    url = "https://github.com/deskgui/deskgui"
    description = "Cross-platform library for building desktop apps with integrated webview in C++."
    topics = ("multi-platform", "gui", "webview", "cplusplus", "desktop")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    build_policy = "missing"
    
    exports_sources = "**"

    def requirements(self):
        self.requires("wil/1.0.240122.1")
        self.requires("zlib/1.2.11")
        self.requires("rapidjson/cci.20230929")

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["deskgui"]

    

    

