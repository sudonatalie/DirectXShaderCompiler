# Builds dxil2spv.exe

set( LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  dxcsupport
  Support    # just for assert and raw streams
  dxil
  dxilcontainer
  )

add_clang_executable(dxil2spv
  main.cpp
  )

target_link_libraries(dxil2spv
  dxcompiler
  )

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs support core irreader dxil)

# Link against LLVM libraries
target_link_libraries(dxil2spv ${llvm_libs} clangSPIRV)

set_target_properties(dxil2spv PROPERTIES VERSION ${CLANG_EXECUTABLE_VERSION})

add_dependencies(dxil2spv dxcompiler TablegenHLSLOptions)

if(UNIX)
  set(CLANGXX_LINK_OR_COPY create_symlink)
# Create a relative symlink
  set(dxil2spv_binary "dxil2spv${CMAKE_EXECUTABLE_SUFFIX}")
else()
  set(CLANGXX_LINK_OR_COPY copy)
  set(dxil2spv_binary "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/dxil2spv${CMAKE_EXECUTABLE_SUFFIX}")
endif()

install(TARGETS dxil2spv
  RUNTIME DESTINATION bin)
