cmake_minimum_required(VERSION 3.12)
project(glslvk LANGUAGES C CXX)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/conan.cmake)
conan_cmake_run(BASIC_SETUP CONANFILE conanfile.py BUILD missing)

option(BUILD_TEST "Build test" ON)
option(BUILD_SHARED "Build shared lib" OFF)

set(sources
  src/glslvk/shader_compiler_shaderc.cpp
  src/glslvk/shader_compiler_glslang.cpp
  )

if(BUILD_SHARED)
  add_library(glslvk SHARED ${sources})
else()
  add_library(glslvk STATIC ${sources})
endif()

target_include_directories(glslvk PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/src)
target_compile_definitions(glslvk PRIVATE
  $<$<CONFIG:DEBUG>:DEBUG>)
target_link_libraries(glslvk
  PUBLIC
  ${CONAN_LIBS}
  $<$<PLATFORM_ID:Linux>:dl>
  $<$<CXX_COMPILER_ID:GNU>:-static-libstdc++>
  )
install(TARGETS glslvk)

add_executable(glslvk_exe
  src/glslvk/main.cpp
  )
target_include_directories(glslvk_exe PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/src)
target_link_libraries(glslvk_exe PUBLIC glslvk)
install(TARGETS glslvk_exe)

if(BUILD_TEST)
  file(GLOB_RECURSE test-sources CONFIGURE_DEPENDS "test/*.cpp")
  foreach(file ${test-sources})
    get_filename_component(comp ${file} NAME_WE)
    add_executable(${comp} ${file})
    target_include_directories(${comp} PUBLIC
      ${CMAKE_CURRENT_LIST_DIR}/test)
    target_link_libraries(${comp} PUBLIC glslvk)
  endforeach()
endif()
