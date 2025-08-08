cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

project(smu VERSION 0.2.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_INSTALL_PREFIX /usr/local)

set(DEFAULT_PORT 5050)

if (WIN32)
  set(CONFIG_ROOT_DIR "config/") # For windows
else()
  set(CONFIG_ROOT_DIR "/var/lib/smu/") # For Unix like systems
endif()


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



# Enable sanitizers if Debug
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU" AND ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message(STATUS "${PROJECT_NAME}: Using GCC/Clang sanitizer configuration")

    # Base Debug flags
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wpedantic -g")

    # _GLIBCXX_DEBUG
    message(STATUS "${PROJECT_NAME}: Enabling _GLIBCXX_DEBUG")
    add_compile_definitions(_GLIBCXX_DEBUG)

    # Sanitizers
    message(STATUS "${PROJECT_NAME}: Enabling ASan and UBSan")
    set(SANITIZER_FLAGS "-fsanitize=address,undefined,leak -fno-omit-frame-pointer")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SANITIZER_FLAGS}")
    set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} ${SANITIZER_FLAGS}")
endif()




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




message(STATUS "${PROJECT_NAME}: Configuring jsoncpp")
FetchContent_Declare(
  jsoncpp
  GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
  GIT_TAG 1.9.6
)
FetchContent_Populate(jsoncpp)
add_subdirectory(${jsoncpp_SOURCE_DIR} ${jsoncpp_BINARY_DIR} EXCLUDE_FROM_ALL)
message(STATUS "${PROJECT_NAME}: Configuration of jsoncpp is complete")




message(STATUS "${PROJECT_NAME}: Configuring IXWebSocket")

set(USE_TLS ON)
set(USE_OPEN_SSL ON)
set(USE_MBED_TLS OFF)
set(USE_ZLIB OFF)

FetchContent_Declare(
  ixwebsocket
  GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
  GIT_TAG v11.4.6
)
FetchContent_Populate(ixwebsocket)
add_subdirectory(${ixwebsocket_SOURCE_DIR} ${ixwebsocket_BINARY_DIR} EXCLUDE_FROM_ALL)
message(STATUS "${PROJECT_NAME}: Configuration of IXWebSocket is complete")




message(STATUS "${PROJECT_NAME}: Configuring FTXUI")

set(FTXUI_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v6.1.9
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(ftxui)

message(STATUS "${PROJECT_NAME}: Configuration of FTXUI is complete")




message(STATUS "${PROJECT_NAME}: Configuring target and includes...")



add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE
    jsoncpp_static
    cxxopts
    ixwebsocket
    ftxui::component
    ftxui::dom
    ftxui::screen
)



if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE
    /W4
    /permissive-
    /sdl
    /w14254
    /w14265
    /w14287
    /we4265
    /w14296
  )
else()
  # G++ / CLANG
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
endif()



target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>:/O2>
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GNU>>:-O3>
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:Clang>>:-O3>
)

if(WIN32)
  target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()



# Installation rules
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(DIRECTORY config/ DESTINATION ${CONFIG_ROOT_DIR} FILES_MATCHING PATTERN "*")


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
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-linux-${CPACK_DEBIAN_ARCHITECTURE}")
# Dependencies
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libssl3")

include(CPack)



message("${PROJECT_NAME}: CONFIGURATION IS COMPLETE")


