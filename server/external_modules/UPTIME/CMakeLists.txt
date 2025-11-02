cmake_minimum_required(VERSION 3.28)
 
# Set your module name here
set(MODULE_NAME "UPTIME")
 
# Do not change this
project(${MODULE_NAME}_module C)
 
# Create a shared library target
add_library(${MODULE_NAME} SHARED
  src/main.c
  src/utils.c
  include/utils.h)
 
# Ensure Position Independent Code for shared object
set_target_properties(${MODULE_NAME} PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  PREFIX ""        # Remove the "lib" prefix so output is MyWonderfulModule.so instead of libMyWonderfulModule.so
  OUTPUT_NAME ${MODULE_NAME}
)
 
# Compiler flags
target_compile_options(${MODULE_NAME} PRIVATE
    -fPIC
    -pedantic
    -Wall -Wextra
    -Wcast-align
    -Wunused
    -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wlogical-op
    -Wuseless-cast)
 
# Link with SDK
target_link_libraries(${MODULE_NAME} PRIVATE smu-server-sdk-c)

target_include_directories(${MODULE_NAME} PRIVATE include)
