cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

project(smu-cli VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_INSTALL_PREFIX /usr)

# For IPC using an abstract Unix Domain Socket
set(ABSTRACT_SOCKET_NAME "smu.ipc.socket")
# The directory where certificates should be saved when executing the keygen command
set(SERVER_CTYPTO_CERTS_DIR "/var/lib/smu-server")


configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/version.hpp.in
  ${CMAKE_CURRENT_SOURCE_DIR}/include/version.hpp
  @ONLY
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/compile-time_config.hpp.in
  ${CMAKE_CURRENT_SOURCE_DIR}/include/compile-time_config.hpp
  @ONLY
)


find_package(OpenSSL REQUIRED)


file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/include/*.hpp*
    ${CMAKE_CURRENT_LIST_DIR}/include/*.tpp*)

list(LENGTH SOURCES SOURCES_SIZE)
message(STATUS "${PROJECT_NAME}: found ${SOURCES_SIZE} sources")



# To prevent building shared libs
set(BUILD_SHARED_LIBS OFF)


# Load deps
load_cxxopts()


add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE cxxopts OpenSSL::SSL)


target_setup_compiler_options(${PROJECT_NAME})

# Installation for CLI package
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    COMPONENT smu-cli
)

message(STATUS "${PROJECT_NAME}: CONFIGURATION IS COMPLETE")

