cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

project(smu-cli VERSION 0.1.3 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_INSTALL_PREFIX /usr/local)

# For IPC using an abstract Unix Domain Socket
set(ABSTRACT_SOCKET_NAME "smu.ipc.socket")


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



message(STATUS "${PROJECT_NAME}: finding source files...")



file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/include/*.hpp*
    ${CMAKE_CURRENT_LIST_DIR}/include/*.tpp*)



list(LENGTH SOURCES SOURCES_SIZE)
message(STATUS "${PROJECT_NAME}: found ${SOURCES_SIZE} sources")




# To prevent building shared libs
set(BUILD_SHARED_LIBS OFF)


include(FetchContent)


message(STATUS "${PROJECT_NAME}: Configuring cxxopts...")

FetchContent_Declare(
  cxxopts
  GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
  GIT_TAG v3.3.1
)
FetchContent_Populate(cxxopts)
add_subdirectory(${cxxopts_SOURCE_DIR} ${cxxopts_BINARY_DIR} EXCLUDE_FROM_ALL)

message(STATUS "${PROJECT_NAME}: Configuration of cxxopts is complete")




add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE cxxopts)



target_compile_options(${PROJECT_NAME} PRIVATE
  -pedantic
  -Wall -Wextra
  -Wnon-virtual-dtor
  -Wold-style-cast
  -Wcast-align
  -Wunused
  -Woverloaded-virtual
  -Wpedantic
  -Wconversion
  -Wsign-conversion
  -Wlogical-op
  -Wuseless-cast
)



if(CMAKE_BUILD_TYPE EQUAL "Release")
  message(STATUS "Setting O3 optimization")
  target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-O3>)
endif()





# Installation rules
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

string(TOLOWER "${CMAKE_SYSTEM_NAME}" OS_NAME_LOWER)

set(CPACK_GENERATOR "DEB")

set(CPACK_PACKAGE_VENDOR "maksimshchavelev")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Client part (1 of 3 components) of server monitoring utility")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_CONTACT "Maksim Shchavelev <maksimshchavelev@gmail.com>")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_CONTACT}")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/maksimshchavelev/server-monitoring-utility")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${OS_NAME_LOWER}-${CPACK_DEBIAN_ARCHITECTURE}")

include(CPack)



message(STATUS "${PROJECT_NAME}: CONFIGURATION IS COMPLETE")

