from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain
import os

class SomeIpProject(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"

    def requirements(self):
        self.requires("boost/1.74.0")
        self.requires("protobuf/3.21.12")
        self.requires("zlib/1.3.1")

    def layout(self):
        # Explicitly setting folders to 'build' instead of using cmake_layout
        self.folders.build = "build"
        self.folders.generators = "build/conan"
        self.cpp.build.bindirs = ["."]

    def generate(self):
        tc = CMakeToolchain(self)
        # 1. STOP the creation of CMakeUserPresets.json
        tc.user_presets_path = False
        
        # 2. Force the path to your external vsomeip build
        tc.variables["EXTERNAL_VSOMEIP_DIR"] = "/home/jay/vsomeip_build/vsomeip/install"
        
        # 3. Ensure CMake knows we want a flat structure
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
