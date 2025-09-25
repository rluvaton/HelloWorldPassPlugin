cmake_minimum_required(VERSION 3.20.0)
project(HelloWorldPlugin)

set(LLVM_DIR "/opt/homebrew/opt/llvm/lib/cmake/llvm")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


find_package(LLVM REQUIRED CONFIG)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

message("LLVM STATUS:
  Definitions ${LLVM_DEFINITIONS}
  Includes    ${LLVM_INCLUDE_DIRS}
  Libraries   ${LLVM_LIBRARY_DIRS}
  Targets     ${LLVM_TARGETS_TO_BUILD}"
)

# Add the required LLVM components
llvm_map_components_to_libnames(llvm_libs support core irreader)

add_library(HelloWorldPlugin MODULE
  HelloWorldPlugin.cpp
  HelloWorldPlugin.h
  MySCCPSolver.cpp
  MySCCPSolver.h
  MyLoopUtils.cpp
  MyLoopUtils.h
  )

set_target_properties(HelloWorldPlugin PROPERTIES PREFIX "")

separate_arguments(LLVM_DEFINITIONS)
target_compile_definitions(HelloWorldPlugin PRIVATE ${LLVM_DEFINITIONS})

target_include_directories(HelloWorldPlugin PRIVATE ${LLVM_INCLUDE_DIRS})


# Link against LLVM libraries
target_link_libraries(HelloWorldPlugin PRIVATE ${llvm_libs})

# On macOS, we need to set some additional properties for the plugin
if(APPLE)
    set_target_properties(HelloWorldPlugin PROPERTIES
        LINK_FLAGS "-undefined dynamic_lookup"
    )
endif()