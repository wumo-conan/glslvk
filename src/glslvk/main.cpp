#include "shader_compiler.hpp"
#include <fstream>
#include "cxxopts.hpp"
#include <regex>
#include <filesystem>
using namespace glslvk;

inline char separator() {
#ifdef _WIN32
  return '\\';
#else
  return '/';
#endif
}

auto escape(std::string name) -> std::string {
  std::regex escape("[^a-zA-Z0-9]");
  std::string cxxName;
  std::regex_replace(std::back_inserter(cxxName), name.begin(), name.end(), escape, "_");
  return cxxName;
}

auto main(int argc, char **argv) -> int {
  cxxopts::Options options("glslvk_exe", "helper to convert file to header");
  auto option = options.add_options();
  option("input", "input file", cxxopts::value<std::string>());
  option("output", "output directory", cxxopts::value<std::string>());
  option("name", "variable name", cxxopts::value<std::string>());
  option("namespace", "namespace name", cxxopts::value<std::string>()->default_value(""));
  option("relative", "relative path", cxxopts::value<std::string>()->default_value(""));
  option("dep", "ninja dep file");
  option("source", "relative path", cxxopts::value<std::string>()->default_value(""));
  option("bin", "relative path", cxxopts::value<std::string>()->default_value(""));

  std::string input, output, name, ns, relative, source, bin;
  bool dep;
  try {
    auto result = options.parse(argc, argv);
    input = result["input"].as<std::string>();
    output = result["output"].as<std::string>();
    name = result["name"].as<std::string>();
    ns = result["namespace"].as<std::string>();
    relative = result["relative"].as<std::string>();
    dep = result["dep"].as<bool>();
    source = result["source"].as<std::string>();
    bin = result["bin"].as<std::string>();
  } catch(std::exception &e) {
    std::cerr << "error: " << e.what() << std::endl
              << "usage:" << argv[0] << " -i file -o header" << std::endl;
    return -1;
  }
  try {
    auto cxxName = escape(name);
    std::string nsBegin, nsEnd;
    if(!ns.empty()) {
      std::stringstream ss;
      ss << "namespace " << escape(ns);
      std::filesystem::path path(relative);

      for(const auto &p: path)
        ss << "::" << escape(p.string());
      ss << "{";
      nsBegin = ss.str();
      nsEnd = "}";
    }
    auto outputPath = std::filesystem::canonical(output);
    auto inputPath = std::filesystem::canonical(input);
    auto headerPath = outputPath / (cxxName + ".hpp");
    auto cppPath = outputPath / (cxxName + ".cpp");
    auto depPath = outputPath / (cxxName + ".d");
    std::ofstream fheader(headerPath.string());
    std::ofstream fcpp(cppPath.string());
    ShaderCompiler compiler;
    std::unordered_set<std::string> depFiles;
    auto buffer = compiler.compile(inputPath.string(), depFiles);

    if(dep) {
      auto sourcePath = std::filesystem::canonical(source);
      auto binPath = std::filesystem::canonical(bin);

      //Current version of ninja may generate relative target, so we need to align to it to
      //make depfile work.
      //if bot binPath and outputPath are within sourcePath, always use relative
      //else use relative only when binPath is the parent of outputPath
      if(
        (binPath.string().starts_with(sourcePath.string()) &&
         headerPath.string().starts_with(sourcePath.string())) ||
        headerPath.string().starts_with(binPath.string()))
        headerPath = std::filesystem::relative(headerPath, binPath);

      std::stringstream depStream;
      depStream << headerPath.string() << ": " << inputPath.string();
      // dump the dependent file names.
      for(const auto &dependent_file_name: depFiles)
        depStream << " " << dependent_file_name;
      //      depStream << inputPath << ": ";
      depStream << std::endl;

      std::ofstream fout(depPath);
      fout << depStream.str();
    }

    fheader << "#pragma once" << std::endl
            << R"(
#if !defined(GLSLVK_SHADER_EXPORTED)
  #if defined(_WIN32) && defined(GLSLVK_SHADER_BUILD_SHARED_LIBRARY)
    #define GLSLVK_SHADER_EXPORTED __declspec(dllimport)
  #else
    #define GLSLVK_SHADER_EXPORTED
  #endif
#endif
)" << std::endl
            << "#include <cstdint>" << std::endl
            << "#if defined(__cpp_lib_span) && __cpp_lib_span >= 201902L && "
               "__has_include(<span>)"
            << std::endl
            << "#include <span>" << std::endl
            << std::endl
            << "#endif" << std::endl
            << nsBegin << std::endl
            << "const uint32_t " << cxxName << "_size=" << buffer.size() << ";"
            << std::endl
            << "GLSLVK_SHADER_EXPORTED extern const uint32_t " << cxxName << "["
            << cxxName + "_size];" << std::endl
            << "#if defined(__cpp_lib_span) && __cpp_lib_span >= 201902L && "
               "__has_include(<span>)"
            << std::endl
            << "const std::span<const uint32_t> " << cxxName << "_span{" << cxxName
            << ", " << cxxName << "_size};" << std::endl
            << "#endif" << std::endl
            << nsEnd << std::endl;

    auto i = 0, line = 10;
    fcpp << "#include \"" << cxxName << ".hpp"
         << "\"" << std::endl
         << nsBegin << std::endl
         << "const uint32_t " << cxxName << "[]={" << std::endl;
    for(auto &data: buffer) {
      fcpp << "0x" << std::hex << data << ",";
      if(i++ > line) {
        fcpp << std::endl;
        i = 0;
      }
    }
    fcpp << "};" << std::endl << nsEnd << std::endl;
  } catch(std::exception &e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }
  return 0;
}