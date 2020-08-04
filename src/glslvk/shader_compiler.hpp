#pragma once
#include <string>
#include <vector>
#include <shaderc/shaderc.hpp>
#include <libshaderc_util/file_finder.h>
#include <libshaderc_util/string_piece.h>
#include <span>
namespace glslvk {
class ShaderCompiler {

public:
  auto compile(const std::string &filename) -> std::span<uint32_t>;

private:
  static auto compileGlslang(const std::string &filename) -> std::vector<uint32_t>;

  shaderc::Compiler compiler_;

  // Reflects the command-line arguments and goes into
  // compiler_.CompileGlslToSpv().
  shaderc::CompileOptions options_;

  // A FileFinder used to substitute #include directives in the source code.
  shaderc_util::FileFinder include_file_finder_;

  shaderc::SpvCompilationResult lastResult;
};
}
