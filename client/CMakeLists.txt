cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

project(smu VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_INSTALL_PREFIX /usr)

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


file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/include/*.hpp*
    ${CMAKE_CURRENT_LIST_DIR}/include/*.tpp*)


list(LENGTH SOURCES SOURCES_SIZE)
message(STATUS "${PROJECT_NAME}: found ${SOURCES_SIZE} sources")


# To prevent building shared libs
set(BUILD_SHARED_LIBS OFF)


# Load deps

message(STATUS "${PROJECT_NAME}: Configuring dependencies (cxxopts, jsoncpp, ixwebsocket, ftxui)")

load_cxxopts()
load_jsoncpp()
load_ixwebsocket()
load_ftxui()


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

target_setup_compiler_options(${PROJECT_NAME})


# Copy OpenSSL .dll-s after building
if(WIN32)
    find_package(OpenSSL REQUIRED)

    # Get OpenSSL root directory from the imported target location
    get_target_property(OPENSSL_CRYPTO_LIB OpenSSL::Crypto IMPORTED_LOCATION)
    get_filename_component(OPENSSL_ROOT "${OPENSSL_CRYPTO_LIB}" DIRECTORY)
    get_filename_component(OPENSSL_ROOT "${OPENSSL_ROOT}" DIRECTORY)

    set(OPENSSL_BIN_DIR "${OPENSSL_ROOT}/bin")

    set(SSL_DLL "${OPENSSL_BIN_DIR}/libssl-3-x64.dll")
    set(CRYPTO_DLL "${OPENSSL_BIN_DIR}/libcrypto-3-x64.dll")

    # List of DLLs to copy
    set(DLLS_TO_COPY "${SSL_DLL}" "${CRYPTO_DLL}")

    foreach(DLL IN LISTS DLLS_TO_COPY)
        if(EXISTS "${DLL}")
            # Add a post-build command to copy DLL to the actual target output directory (with config)
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DLL}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
                COMMENT "Copying ${DLL} to output directory"
            )
        else()
            message(WARNING "DLL not found: ${DLL}. You may need to copy it manually next to your executable.")
        endif()
    endforeach()
endif()


# Installation for client package
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${SMU_INSTALL_BIN}
    COMPONENT smu
)

# Install config files for client
if(NOT SMU_WINDOWS)
    install(DIRECTORY config/
        DESTINATION ${CONFIG_ROOT_DIR}
        COMPONENT smu
        FILES_MATCHING PATTERN "*"
    )
endif()


message("${PROJECT_NAME}: CONFIGURATION IS COMPLETE")
