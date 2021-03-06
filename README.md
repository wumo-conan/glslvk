# glslvk

GLSL shader compiler. Features:
* Support generating ninja depfile for shader includes. Thus modified shader header files can only trigger necessary shader source files' recompilation. (Only work on `Ninja` Generator)
* Generate  seperated header and source cpp files for compiled SPIRV binaries.
* C++ 20

This project is based on [shaderc](https://github.com/google/shaderc). [conan](https://conan.io/) is used as the package manager.

# Usage

Add additional repo to conan:
```
conan remote add wumo https://api.bintray.com/conan/wumo/public
```

Add `glslvk/0.0.1@wumo/stable` to field `build_requires` in `conanfile.py`:

```python
build_requires = ("glslvk/0.0.1@wumo/stable")
```

In your `CMakeLists.txt`:

```
static_shader_ns(<target> <namespace> <shader_source_dir> <output_dir>)
```

* \<target\> is the binary target that the compiled SPIRV binaries will be linked to.
* \<namespace\> is the c++ namespace that groups all your shader spans.
* \<shader_source_dir\> and \<output_dir\> are the input and outpu of shader files.

In your c++ code:

```c++
#include "deferred/gbuffer_vert.hpp"

auto shader_spv = shader::deferred::gbuffer_vert_span;
```

# Build

## Prerequisite

install `python3`, `git`, `cmake`, `conan`, `make`, `gcc`(>= 10), `Microsoft Visual Studio`(>=2019)

## Ubuntu
```
$ sudo apt install -y git gcc cmake make
# install conan
$ pip install conan
```

## Windows
Using [scoop](https://scoop.sh/) to install dependencies:
```
# install scoop
$ Set-ExecutionPolicy RemoteSigned -scope CurrentUser
$ iex (new-object net.webclient).downloadstring('https://get.scoop.sh')
# install dependencies
$ scoop install python git gcc cmake
# install conan
$ pip install conan
```

## Additional conan remotes
```
$ conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan
$ conan remote add wumo https://api.bintray.com/conan/wumo/public
``` 

## Build 
```
mkdir build && cd build

# (win)
$ cmake .. -G "Visual Studio 16 2019" -A x64
$ cmake --build . --config Release

# (linux, mac)
$ cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
$ cmake --build .
```
