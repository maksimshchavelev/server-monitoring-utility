cmake_minimum_required(VERSION 3.28 FATAL_ERROR)
project(smu VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_INSTALL_PREFIX /usr)
set(DEFAULT_PORT 5050)

if (WIN32)
  set(CONFIG_ROOT_DIR "config/") # For Windows
else()
  set(CONFIG_ROOT_DIR "/var/lib/smu/") # For Unix-like systems
endif()


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/version.hpp.in
               ${CMAKE_CURRENT_SOURCE_DIR}/include/version.hpp @ONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/compile-time_config.hpp.in
               ${CMAKE_CURRENT_SOURCE_DIR}/include/compile-time_config.hpp @ONLY)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/include/*.hpp*
    ${CMAKE_CURRENT_LIST_DIR}/include/*.tpp*)

list(LENGTH SOURCES SOURCES_SIZE)
message(STATUS "${PROJECT_NAME}: found ${SOURCES_SIZE} sources")

set(BUILD_SHARED_LIBS OFF)

# --- Dependencies ---
message(STATUS "${PROJECT_NAME}: Configuring dependencies...")
load_openssl()
load_cxxopts()
load_jsoncpp()
load_ixwebsocket()
load_ftxui()

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE include)

# --- Link libraries ---
target_link_libraries(${PROJECT_NAME} PRIVATE
    jsoncpp_static
    cxxopts
    ixwebsocket
    ftxui::component
    ftxui::dom
    ftxui::screen
    OpenSSL::SSL
    OpenSSL::Crypto
)

target_setup_compiler_options(${PROJECT_NAME})

message(STATUS ${CMAKE_EXE_LINKER_FLAGS_INIT})
message(STATUS ${CMAKE_EXE_LINKER_FLAGS})

# --- Installation ---
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${SMU_INSTALL_BIN}
    COMPONENT smu
)

if(NOT WIN32)
    install(DIRECTORY config/
        DESTINATION ${CONFIG_ROOT_DIR}
        COMPONENT smu
        FILES_MATCHING PATTERN "*"
    )
endif()


if(WIN32)
    setup_windows_dlls_on_linux(${PROJECT_NAME})
endif()


message("${PROJECT_NAME}: CONFIGURATION IS COMPLETE")

