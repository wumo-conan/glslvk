#include "shader_compiler.hpp"
#include <glslc/shader_stage.h>
#include <glslc/file_compiler.h>
#include <libshaderc_util/io.h>
#include <glslc/file_includer.h>

#include <memory>
#include <sstream>
#include <fstream>

namespace glslvk {
auto ShaderCompiler::compile(
  const std::string &filename, std::unordered_set<std::string> &depFiles)
  -> std::span<uint32_t> {
  glslc::InputFileSpec inputFile{
    filename, glslc::DeduceDefaultShaderKindFromFileName(filename),
    shaderc_source_language_glsl, "main"};

  std::string error_file_name = inputFile.name;

  std::vector<char> input_data;
  std::string path = inputFile.name;
  if(!shaderc_util::ReadFile(path, &input_data))
    throw std::runtime_error("failed to read shader file " + filename);

  shaderc_util::string_piece source_string = "";
  if(!input_data.empty())
    source_string = {&input_data.front(), &input_data.front() + input_data.size()};

  std::unique_ptr<glslc::FileIncluder> includer(
    new glslc::FileIncluder(&include_file_finder_));
  // Get a reference to the dependency trace before we pass the ownership to
  // shaderc::CompileOptions.
  const auto &used_source_files = includer->file_path_trace();
  options_.SetIncluder(std::move(includer));

  // Set the language.  Since we only use the options object in this
  // method, then it's ok to always set it without resetting it after
  // compilation.  A subsequent compilation will set it again anyway.
  options_.SetSourceLanguage(inputFile.language);

  lastResult = compiler_.CompileGlslToSpv(
    source_string.data(), source_string.size(), inputFile.stage, error_file_name.c_str(),
    inputFile.entry_point_name.c_str(), options_);
  if(lastResult.GetCompilationStatus() != shaderc_compilation_status_success)
    throw std::runtime_error(lastResult.GetErrorMessage());
  std::span<uint32_t> span{
    (uint32_t *)lastResult.begin(),
    static_cast<size_t>(lastResult.end() - lastResult.begin())};
  depFiles = used_source_files;
  return span;
}
}