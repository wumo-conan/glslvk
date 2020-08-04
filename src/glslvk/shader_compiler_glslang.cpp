#include "shader_compiler.hpp"
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include "DirStackFileIncluder.h"
#include <iostream>

namespace glslvk {
std::string GetFilePath(const std::string &str) {
  size_t found = str.find_last_of("/\\");
  return str.substr(0, found);
  //size_t FileName = str.substr(found+1);
}

std::string GetSuffix(const std::string &name) {
  const size_t pos = name.rfind('.');
  return (pos == std::string::npos) ? "" : name.substr(name.rfind('.') + 1);
}

EShLanguage GetShaderStage(const std::string &stage) {
  if(stage == "vert") {
    return EShLangVertex;
  } else if(stage == "tesc") {
    return EShLangTessControl;
  } else if(stage == "tese") {
    return EShLangTessEvaluation;
  } else if(stage == "geom") {
    return EShLangGeometry;
  } else if(stage == "frag") {
    return EShLangFragment;
  } else if(stage == "comp") {
    return EShLangCompute;
  } else {
    assert(0 && "Unknown shader stage");
    return EShLangCount;
  }
}

const TBuiltInResource DefaultTBuiltInResource = {
  /* .MaxLights = */ 32,
  /* .MaxClipPlanes = */ 6,
  /* .MaxTextureUnits = */ 32,
  /* .MaxTextureCoords = */ 32,
  /* .MaxVertexAttribs = */ 64,
  /* .MaxVertexUniformComponents = */ 4096,
  /* .MaxVaryingFloats = */ 64,
  /* .MaxVertexTextureImageUnits = */ 32,
  /* .MaxCombinedTextureImageUnits = */ 80,
  /* .MaxTextureImageUnits = */ 32,
  /* .MaxFragmentUniformComponents = */ 4096,
  /* .MaxDrawBuffers = */ 32,
  /* .MaxVertexUniformVectors = */ 128,
  /* .MaxVaryingVectors = */ 8,
  /* .MaxFragmentUniformVectors = */ 16,
  /* .MaxVertexOutputVectors = */ 16,
  /* .MaxFragmentInputVectors = */ 15,
  /* .MinProgramTexelOffset = */ -8,
  /* .MaxProgramTexelOffset = */ 7,
  /* .MaxClipDistances = */ 8,
  /* .MaxComputeWorkGroupCountX = */ 65535,
  /* .MaxComputeWorkGroupCountY = */ 65535,
  /* .MaxComputeWorkGroupCountZ = */ 65535,
  /* .MaxComputeWorkGroupSizeX = */ 1024,
  /* .MaxComputeWorkGroupSizeY = */ 1024,
  /* .MaxComputeWorkGroupSizeZ = */ 64,
  /* .MaxComputeUniformComponents = */ 1024,
  /* .MaxComputeTextureImageUnits = */ 16,
  /* .MaxComputeImageUniforms = */ 8,
  /* .MaxComputeAtomicCounters = */ 8,
  /* .MaxComputeAtomicCounterBuffers = */ 1,
  /* .MaxVaryingComponents = */ 60,
  /* .MaxVertexOutputComponents = */ 64,
  /* .MaxGeometryInputComponents = */ 64,
  /* .MaxGeometryOutputComponents = */ 128,
  /* .MaxFragmentInputComponents = */ 128,
  /* .MaxImageUnits = */ 8,
  /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
  /* .MaxCombinedShaderOutputResources = */ 8,
  /* .MaxImageSamples = */ 0,
  /* .MaxVertexImageUniforms = */ 0,
  /* .MaxTessControlImageUniforms = */ 0,
  /* .MaxTessEvaluationImageUniforms = */ 0,
  /* .MaxGeometryImageUniforms = */ 0,
  /* .MaxFragmentImageUniforms = */ 8,
  /* .MaxCombinedImageUniforms = */ 8,
  /* .MaxGeometryTextureImageUnits = */ 16,
  /* .MaxGeometryOutputVertices = */ 256,
  /* .MaxGeometryTotalOutputComponents = */ 1024,
  /* .MaxGeometryUniformComponents = */ 1024,
  /* .MaxGeometryVaryingComponents = */ 64,
  /* .MaxTessControlInputComponents = */ 128,
  /* .MaxTessControlOutputComponents = */ 128,
  /* .MaxTessControlTextureImageUnits = */ 16,
  /* .MaxTessControlUniformComponents = */ 1024,
  /* .MaxTessControlTotalOutputComponents = */ 4096,
  /* .MaxTessEvaluationInputComponents = */ 128,
  /* .MaxTessEvaluationOutputComponents = */ 128,
  /* .MaxTessEvaluationTextureImageUnits = */ 16,
  /* .MaxTessEvaluationUniformComponents = */ 1024,
  /* .MaxTessPatchComponents = */ 120,
  /* .MaxPatchVertices = */ 32,
  /* .MaxTessGenLevel = */ 64,
  /* .MaxViewports = */ 16,
  /* .MaxVertexAtomicCounters = */ 0,
  /* .MaxTessControlAtomicCounters = */ 0,
  /* .MaxTessEvaluationAtomicCounters = */ 0,
  /* .MaxGeometryAtomicCounters = */ 0,
  /* .MaxFragmentAtomicCounters = */ 8,
  /* .MaxCombinedAtomicCounters = */ 8,
  /* .MaxAtomicCounterBindings = */ 1,
  /* .MaxVertexAtomicCounterBuffers = */ 0,
  /* .MaxTessControlAtomicCounterBuffers = */ 0,
  /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
  /* .MaxGeometryAtomicCounterBuffers = */ 0,
  /* .MaxFragmentAtomicCounterBuffers = */ 1,
  /* .MaxCombinedAtomicCounterBuffers = */ 1,
  /* .MaxAtomicCounterBufferSize = */ 16384,
  /* .MaxTransformFeedbackBuffers = */ 4,
  /* .MaxTransformFeedbackInterleavedComponents = */ 64,
  /* .MaxCullDistances = */ 8,
  /* .MaxCombinedClipAndCullDistances = */ 8,
  /* .MaxSamples = */ 4,
  /* .maxMeshOutputVerticesNV = */ 256,
  /* .maxMeshOutputPrimitivesNV = */ 512,
  /* .maxMeshWorkGroupSizeX_NV = */ 32,
  /* .maxMeshWorkGroupSizeY_NV = */ 1,
  /* .maxMeshWorkGroupSizeZ_NV = */ 1,
  /* .maxTaskWorkGroupSizeX_NV = */ 32,
  /* .maxTaskWorkGroupSizeY_NV = */ 1,
  /* .maxTaskWorkGroupSizeZ_NV = */ 1,
  /* .maxMeshViewCountNV = */ 4,
  /* .maxDualSourceDrawBuffersEXT = */ 1,

  /* .limits = */
  {
    /* .nonInductiveForLoops = */ true,
    /* .whileLoops = */ true,
    /* .doWhileLoops = */ true,
    /* .generalUniformIndexing = */ true,
    /* .generalAttributeMatrixVectorIndexing = */ true,
    /* .generalVaryingIndexing = */ true,
    /* .generalSamplerIndexing = */ true,
    /* .generalVariableIndexing = */ true,
    /* .generalConstantMatrixVectorIndexing = */ true,
  }};

auto ShaderCompiler::compileGlslang(const std::string &filename)
  -> std::vector<uint32_t> {
  glslang::InitializeProcess();
  std::ifstream file(filename);

  if(!file.is_open()) {
    std::cout << "Failed to load shader: " << filename << std::endl;
    throw std::runtime_error("failed to open file: " + filename);
  }

  std::string InputGLSL(
    (std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

  const char *InputCString = InputGLSL.c_str();

  EShLanguage ShaderType = GetShaderStage(GetSuffix(filename));
  glslang::TShader Shader(ShaderType);
  Shader.setStrings(&InputCString, 1);

  int ClientInputSemanticsVersion = 110; // maps to, say, #define VULKAN 100
  glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_1;
  glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_1;

  Shader.setEnvInput(
    glslang::EShSourceGlsl, ShaderType, glslang::EShClientVulkan,
    ClientInputSemanticsVersion);
  Shader.setEnvClient(glslang::EShClientVulkan, VulkanClientVersion);
  Shader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);

  TBuiltInResource Resources = DefaultTBuiltInResource;
  EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

  const int DefaultVersion = 110;

  DirStackFileIncluder Includer;
  //Get Path of File
  std::string Path = GetFilePath(filename);
  Includer.pushExternalLocalDirectory(Path);

  std::string PreprocessedGLSL;

  if(!Shader.preprocess(
       &Resources, DefaultVersion, ENoProfile, false, false, messages, &PreprocessedGLSL,
       Includer)) {
    std::cout << "GLSL Preprocessing Failed for: " << filename << std::endl;
    std::cout << Shader.getInfoLog() << std::endl;
    std::cout << Shader.getInfoDebugLog() << std::endl;
  }

  const char *PreprocessedCStr = PreprocessedGLSL.c_str();
  Shader.setStrings(&PreprocessedCStr, 1);

  if(!Shader.parse(&Resources, 110, false, messages)) {
    std::cout << "GLSL Parsing Failed for: " << filename << std::endl;
    std::cout << Shader.getInfoLog() << std::endl;
    std::cout << Shader.getInfoDebugLog() << std::endl;
  }

  glslang::TProgram Program;
  Program.addShader(&Shader);

  if(!Program.link(messages)) {
    std::cout << "GLSL Linking Failed for: " << filename << std::endl;
    std::cout << Shader.getInfoLog() << std::endl;
    std::cout << Shader.getInfoDebugLog() << std::endl;
  }

  std::vector<uint32_t> SpirV;
  spv::SpvBuildLogger logger;
  glslang::SpvOptions spvOptions;
  spvOptions.disableOptimizer = false;
  glslang::GlslangToSpv(
    *Program.getIntermediate(ShaderType), SpirV, &logger, &spvOptions);
  return SpirV;
}
}