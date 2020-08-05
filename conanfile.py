import os
from conans import ConanFile, CMake, tools

class glslvkConan(ConanFile):
    name = "glslvk"
    version = "0.0.1"
    settings = "os", "compiler", "build_type", "arch"
    requires = ("shaderc/2020.2@wumo/stable")
    generators = "cmake"
    scm = {
        "type": "git",
        "subfolder": name,
        "url": "auto",
        "revision": "auto"
    }
    options = {
        "shared": [True, False],
    }
    default_options = {
        "shared": False,
    }

    def configure_cmake(self):
        cmake = CMake(self)
        cmake.definitions["BUILD_TEST"] = False
        cmake.definitions["BUILD_SHARED"] = self.options.shared
        cmake.configure(source_folder=self.name)
        return cmake

    def build(self):
        cmake = self.configure_cmake()
        cmake.build()

    def imports(self):
        self.copy("*.dll", dst="bin", src="bin")
        self.copy("*.dll", dst="bin", src="lib")
        self.copy("*.dylib", dst="bin", src="lib")
        self.copy("*.pdb", dst="bin", src="bin")

    def package(self):
        cmake = self.configure_cmake()
        cmake.install()
        self.copy("*.h", dst="include", src=f"{self.name}/src")
        self.copy("*.hpp", dst="include", src=f"{self.name}/src")
        self.copy("compile_shaders.cmake", dst="cmake", src=f"{self.name}/cmake")

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
        self.cpp_info.build_modules.append("cmake/compile_shaders.cmake")
        bin_path = os.path.join(self.package_folder, "bin")
        self.output.info(f"Appending PATH environment variable: {bin_path}")
        self.env_info.PATH.append(bin_path)
