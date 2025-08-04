cmake_minimum_required(VERSION 3.20.0)
project(HelloWorldPlugin)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

message("LLVM STATUS:
  Definitions ${LLVM_DEFINITIONS}
  Includes    ${LLVM_INCLUDE_DIRS}
  Libraries   ${LLVM_LIBRARY_DIRS}
  Targets     ${LLVM_TARGETS_TO_BUILD}"
)

add_library(HelloWorldPlugin MODULE 
  HelloWorldPlugin.cpp
  HelloWorldPlugin.h)

set_target_properties(HelloWorldPlugin PROPERTIES PREFIX "")

separate_arguments(LLVM_DEFINITIONS)
target_compile_definitions(HelloWorldPlugin PRIVATE ${LLVM_DEFINITIONS})

target_include_directories(HelloWorldPlugin PRIVATE ${LLVM_INCLUDE_DIRS})
