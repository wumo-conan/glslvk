#pragma once
#include <string>
#include <vector>
#include <shaderc/shaderc.hpp>
#include <libshaderc_util/file_finder.h>
#include <libshaderc_util/string_piece.h>
#include <glslc/dependency_info.h>
#include <span>
#include <unordered_set>
namespace glslvk {
class ShaderCompiler {

public:
  auto compile(const std::string &filename, std::unordered_set<std::string> &depFiles)
    -> std::span<uint32_t>;

private:
  
  shaderc::Compiler compiler_;

  // Reflects the command-line arguments and goes into
  // compiler_.CompileGlslToSpv().
  shaderc::CompileOptions options_;

  // A FileFinder used to substitute #include directives in the source code.
  shaderc_util::FileFinder include_file_finder_;

  shaderc::SpvCompilationResult lastResult;
};
}
