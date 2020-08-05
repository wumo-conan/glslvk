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
  option("i,input", "input file", cxxopts::value<std::string>());
  option("o,output", "output directory", cxxopts::value<std::string>());
  option("n,name", "variable name", cxxopts::value<std::string>());
  option(
    "s,namespace", "namespace name", cxxopts::value<std::string>()->default_value(""));
  option("r,relative", "relative path", cxxopts::value<std::string>()->default_value(""));

  std::string input, output, name, ns, relative;
  try {
    auto result = options.parse(argc, argv);
    input = result["input"].as<std::string>();
    output = result["output"].as<std::string>();
    name = result["name"].as<std::string>();
    ns = result["namespace"].as<std::string>();
    relative = result["relative"].as<std::string>();
  } catch(std::exception &e) {
    std::cerr << "error: " << e.what() << std::endl
              << "usage:" << argv[0] << " -i file -o header" << std::endl;
    return -1;
  }
  try {
    ShaderCompiler compiler;
    auto buffer = compiler.compile(input);
    std::cout << buffer.size() << std::endl;
    auto cxxName = escape(name);
    std::string nsBegin{""}, nsEnd{""};
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
    output = (output.ends_with("/") || output.ends_with("\\")) ? output :
                                                                 output + separator();
    std::ofstream fheader(output + cxxName + ".hpp");
    std::ofstream fcpp(output + cxxName + ".cpp");

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