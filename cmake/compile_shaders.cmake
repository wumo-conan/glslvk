function(glslvkcompile name isStatic namespace srcDir outputDir)
  set(glslvk_exe ${CONAN_BIN_DIRS_GLSLVK}/glslvk_exe)
  file(MAKE_DIRECTORY "${outputDir}")
  file(GLOB_RECURSE shaders
    "${srcDir}/*.vert"
    "${srcDir}/*.frag"
    "${srcDir}/*.geom"
    "${srcDir}/*.tese"
    "${srcDir}/*.tesc"
    "${srcDir}/*.comp"
    "${srcDir}/*.rgen"
    "${srcDir}/*.rahit"
    "${srcDir}/*.rmiss"
    "${srcDir}/*.rchit"
    "${srcDir}/*.rint"
    "${srcDir}/*.rcall"
    "${srcDir}/*.task"
    "${srcDir}/*.mesh"
    )
  set(outputFiles)
  foreach(file ${shaders})
    get_filename_component(fileName ${file} NAME)
    get_filename_component(dir ${file} DIRECTORY)
    file(RELATIVE_PATH relativeFile "${srcDir}" "${file}")
    file(RELATIVE_PATH relativeDir "${srcDir}" "${dir}")
    
    string(REGEX REPLACE "[^a-zA-Z0-9]" "_" cxxName ${fileName})
    
    set(genDir "${outputDir}/${relativeDir}")
    set(genHeader "${genDir}/${cxxName}.hpp")
    set(genSource "${genDir}/${cxxName}.cpp")
    
    set(outputFiles ${outputFiles} ${genSource})
    
    add_custom_command(OUTPUT ${genHeader} ${genSource}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${genDir}
      COMMAND ${CMAKE_COMMAND} -E rm -f ${genHeader} ${genSource}
      COMMAND ${glslvk_exe} -i ${file} -o ${genDir} -n ${cxxName} -s ${namespace} -r ${relativeDir}
      DEPENDS ${file})
  endforeach()
  if(${isStatic})
    add_library(${name} STATIC ${outputFiles})
  else()
    add_library(${name} SHARED ${outputFiles})
    target_compile_definitions(${name} PUBLIC "GLSLVK_SHADER_BUILD_SHARED_LIBRARY")
  endif()
  
  target_include_directories(${name} INTERFACE ${outputDir})
endfunction()

function(static_shader target srcDir outputDir)
  glslvkcompile(${target}_shaders True "" ${srcDir} ${outputDir})
  target_link_libraries(${target} PUBLIC ${target}_shaders)
endfunction()

function(shared_shader target srcDir outputDir pattern)
  glslvkcompile(${target}_shaders False "" ${srcDir} ${outputDir})
  target_link_libraries(${target} PUBLIC ${target}_shaders)
endfunction()

function(static_shader_ns target namespace srcDir outputDir)
  glslvkcompile(${target}_shaders True ${namespace} ${srcDir} ${outputDir})
  target_link_libraries(${target} PUBLIC ${target}_shaders)
endfunction()

function(shared_shader_ns target namespace srcDir outputDir)
  glslvkcompile(${target}_shaders False ${namespace} ${srcDir} ${outputDir})
  target_link_libraries(${target} PUBLIC ${target}_shaders)
endfunction()