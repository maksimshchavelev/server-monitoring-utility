cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

project(smu-server VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_INSTALL_PREFIX /usr)

# For IPC using an abstract Unix Domain Socket
set(ABSTRACT_SOCKET_NAME "smu.ipc.socket")

option(BUILD_TESTS "Need to build tests?" ON)
option(BUILD_EXTERNAL_MODULES "Need to build external modules?" ON)

# This is where the program configuration will be
set(CONFIGS_DIR "/var/lib/${PROJECT_NAME}")
set(SERVER_CONFIG_PATH "${CONFIGS_DIR}/config.json")
set(MODULES_CONFIGS_DIR "${CONFIGS_DIR}/modules.d")
set(INSTALLED_BINARY_PATH "${CMAKE_INSTALL_PREFIX}/bin/${PROJECT_NAME}")
set(SMU_UPLOAD_PATH ${CONFIGS_DIR}/uploads/tmp)

# ============================== EXTERNAL MODULES ==============================

# Where external modules live
set(EXTERNAL_MODULES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external_modules")

# Where their builds will be placed
set(EXTERNAL_MODULES_BUILD_DIR "${CMAKE_BINARY_DIR}/external_modules_build")

# Collect module targets
set(SMU_MODULE_TARGETS "")

# ==============================================================================

# =============================== SDK PACKAGING ================================

set(SDK_SCRIPT "${CMAKE_SOURCE_DIR}/scripts/build-sdk.sh")

# ==============================================================================

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

include(FetchContent)

message(STATUS "${PROJECT_NAME}: finding source files...")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/include/*.hpp*
    ${CMAKE_CURRENT_LIST_DIR}/include/*.tpp*)

list(LENGTH SOURCES SOURCES_SIZE)
message(STATUS "${PROJECT_NAME}: found ${SOURCES_SIZE} sources")

find_package(jsoncpp REQUIRED)

message(STATUS "${PROJECT_NAME}: configuring Drogon...")

# Disable uneccessary things for Drogon
set(BUILD_CTL OFF)
set(BUILD_EXAMPLES OFF)
set(BUILD_SHARED_LIBS OFF)
set(BUILD_ORM FALSE)
set(DROGON_BUILD_STATIC ON)
set(USE_STATIC_LIBS_ONLY TRUE)
set(BUILD_POSTGRESQL FALSE)
set(LIBPQ_BATCH_MODE FALSE)
set(BUILD_MYSQL FALSE)
set(BUILD_SQLITE FALSE)
set(BUILD_REDIS FALSE)
set(USE_SPDLOG FALSE)

FetchContent_Declare(drogon
    GIT_REPOSITORY https://github.com/drogonframework/drogon.git
    GIT_TAG v1.9.11
    EXCLUDE_FROM_ALL
)
FetchContent_Populate(drogon)
add_subdirectory(${drogon_SOURCE_DIR} ${drogon_BINARY_DIR} EXCLUDE_FROM_ALL)

message(STATUS "${PROJECT_NAME}: Configuration of Drogon is complete")
message(STATUS "${PROJECT_NAME}: Configuring cxxopts...")

FetchContent_Declare(
  cxxopts
  GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
  GIT_TAG v3.3.1
)
FetchContent_Populate(cxxopts)
add_subdirectory(${cxxopts_SOURCE_DIR} ${cxxopts_BINARY_DIR} EXCLUDE_FROM_ALL)

message(STATUS "${PROJECT_NAME}: Configuration of cxxopts is complete")
message(STATUS "${PROJECT_NAME}: Configuring target and includes...")

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE
    drogon
    jsoncpp_static
    cxxopts
)

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

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  message(STATUS "Setting O3 optimization")
  target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-O3>)
endif()

# Enable sanitizers if Debug
if((CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU") AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    message(STATUS "${PROJECT_NAME}: Using GCC/Clang sanitizer configuration")

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wpedantic -g")
    add_compile_definitions(_GLIBCXX_DEBUG)

    set(SANITIZER_FLAGS "-fsanitize=address,undefined,leak -fno-omit-frame-pointer")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SANITIZER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${SANITIZER_FLAGS}")
endif()

# ========================== CONFIGURING EXTERNAL MODULES ==========================

if(BUILD_EXTERNAL_MODULES)
  # Find directories under external_modules
  file(GLOB MODULE_ENTRIES RELATIVE "${EXTERNAL_MODULES_DIR}" "${EXTERNAL_MODULES_DIR}/*")

  foreach(MOD ${MODULE_ENTRIES})
    set(MOD_DIR "${EXTERNAL_MODULES_DIR}/${MOD}")
    if(IS_DIRECTORY "${MOD_DIR}")

      # If module provides a CMakeLists.txt, integrate it as a subproject.
      if(EXISTS "${MOD_DIR}/CMakeLists.txt")
        # Build each module in a dedicated binary directory to avoid collisions.
        add_subdirectory("${MOD_DIR}" "${EXTERNAL_MODULES_BUILD_DIR}/${MOD}")
        # Convention: module's CMake should create a target with the same name as the directory.
        # If module uses a different target name, it should set SMU_MODULE_TARGET variable.
        if(TARGET "${MOD}")
          list(APPEND SMU_MODULE_TARGETS "${MOD}")
          message(STATUS "${PROJECT_NAME}: configured external module with name '${MOD}'")
        else()
          # Allow the module to declare a target name explicitly as SMU_MODULE_TARGET
          if(DEFINED ${MOD}_SMU_MODULE_TARGET)
            list(APPEND SMU_MODULE_TARGETS "${${MOD}_SMU_MODULE_TARGET}")
            message(STATUS "${PROJECT_NAME}: configured external module with name '${MOD}'")
          else()
            message(WARNING "Module '${MOD}' added by add_subdirectory but target '${MOD}'
              was not found; module must create target named '${MOD}' or set variable '${MOD}_SMU_MODULE_TARGET'")
          endif()
        endif()

      elseif(EXISTS "${MOD_DIR}/build.sh")
        # Use build.sh -> expected contract: build.sh <out-dir> produces <out-dir>/<MOD>.so
        set(MOD_OUT_DIR "${EXTERNAL_MODULES_BUILD_DIR}/${MOD}")
        set(MOD_OUT_SO "${MOD_OUT_DIR}/${MOD}.so")

        add_custom_command(
          OUTPUT "${MOD_OUT_SO}"
          COMMAND ${CMAKE_COMMAND} -E make_directory "${MOD_OUT_DIR}"
          COMMAND "${MOD_DIR}/build.sh" "${MOD_OUT_DIR}"
          WORKING_DIRECTORY "${MOD_DIR}"
          COMMENT "Building external module ${MOD}"
          VERBATIM
        )

        add_custom_target(build_${MOD} DEPENDS "${MOD_OUT_SO}")

        # Create an IMPORTED target so we can refer to it uniformly
        add_library(${MOD} SHARED IMPORTED GLOBAL)
        set_target_properties(${MOD} PROPERTIES
          IMPORTED_LOCATION "${MOD_OUT_SO}"
          # ensure packaging installer understands it's a library
          IMPORTED_CONFIGURATIONS "Release"
        )
        add_dependencies(${MOD} build_${MOD})

        list(APPEND SMU_MODULE_TARGETS "${MOD}")
        message(STATUS "${PROJECT_NAME}: configured external module with name '${MOD}'")

      else()
        message(WARNING "External module '${MOD}' has neither CMakeLists.txt nor build.sh — skipping")
      endif()

    endif()
  endforeach()

  # Install each module .so into package layout: /var/lib/smu-server/modules.d/<MOD>/<MOD>.so
  foreach(MOD_TARGET ${SMU_MODULE_TARGETS})
    # This installs both add_subdirectory-built shared libs and IMPORTED ones
    install(
      FILES "$<TARGET_FILE:${MOD_TARGET}>"
      DESTINATION "${CONFIGS_DIR}/modules.d/${MOD_TARGET}"
      PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
      COMPONENT external_modules
    )
  endforeach()
endif()
# ==================================================================================


# =============================== SDK PACKAGING ================================

# Type cmake --build . --target sdk to pack SDKs
add_custom_target(sdk
  COMMAND ${SDK_SCRIPT} ${PROJECT_VERSION}
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Building combined SDK packages"
)

# ==============================================================================


# Configuring tests
if(${BUILD_TESTS})
  message(STATUS "${PROJECT_NAME}: Configuring tests...")

  add_library(smu-server-libary STATIC ${SOURCES}) # For linking tests with smu-server
  target_include_directories(smu-server-libary PUBLIC ${CMAKE_SOURCE_DIR}/include)
  target_link_libraries(smu-server-libary PRIVATE
    drogon
    jsoncpp_static
    cxxopts)

  include(CTest)
  enable_testing()
  add_subdirectory(tests)

  # Add custom target to run tests
  add_custom_target(run-tests
    COMMAND ${CMAKE_CTEST_COMMAND} "--output-on-failure"
    DEPENDS tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )

  message(STATUS "${PROJECT_NAME}: Tests configured")
endif()

# Configuring systemd
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/systemd/smu-server.service.in
  ${CMAKE_CURRENT_SOURCE_DIR}/systemd/smu-server.service
  @ONLY
)

# Installation rules
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin COMPONENT server)

# Install config files with proper component handling
install(
  DIRECTORY config/
  DESTINATION ${CONFIGS_DIR}
  COMPONENT server
  PATTERN "*"
)

install(FILES systemd/smu-server.service DESTINATION "/etc/systemd/system" COMPONENT server)

# For drogon tmp files
install(
  DIRECTORY
  DESTINATION ${SMU_UPLOAD_PATH}
  COMPONENT server
  DIRECTORY_PERMISSIONS
    OWNER_WRITE OWNER_READ OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
)

# Configuring CPack
include(InstallRequiredSystemLibraries)
set(CPACK_GENERATOR "DEB")

set(CPACK_PACKAGE_VENDOR "maksimshchavelev")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Server part (1 of 3 components) of server monitoring utility")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_CONTACT "Maksim Shchavelev <maksimshchavelev@gmail.com>")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_CONTACT}")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/maksimshchavelev/server-monitoring-utility")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
    "${CMAKE_CURRENT_SOURCE_DIR}/packaging/postinst;${CMAKE_CURRENT_SOURCE_DIR}/packaging/prerm"
)
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-linux-${CPACK_DEBIAN_ARCHITECTURE}")

# Component-based packaging setup
set(CPACK_COMPONENTS_ALL server)
if(BUILD_EXTERNAL_MODULES)
  list(APPEND CPACK_COMPONENTS_ALL external_modules)
endif()

set(CPACK_DEB_COMPONENT_INSTALL ON)

# Server package settings
set(CPACK_DEBIAN_SERVER_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_DEBIAN_SERVER_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")

# External modules package settings
set(CPACK_DEBIAN_EXTERNAL_MODULES_PACKAGE_NAME "${PROJECT_NAME}-external-modules")
set(CPACK_DEBIAN_EXTERNAL_MODULES_PACKAGE_DESCRIPTION "External modules for ${PROJECT_NAME}")
set(CPACK_DEBIAN_EXTERNAL_MODULES_PACKAGE_DEPENDS "${PROJECT_NAME} (>= ${PROJECT_VERSION})")

# Set component display names
set(CPACK_COMPONENT_SERVER_DISPLAY_NAME "SMU Server")
set(CPACK_COMPONENT_EXTERNAL_MODULES_DISPLAY_NAME "External Modules")
set(CPACK_COMPONENT_EXTERNAL_MODULES_DESCRIPTION "External modules for SMU Server")

# Set file names for component packages
set(CPACK_DEBIAN_SERVER_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-linux-${CPACK_DEBIAN_ARCHITECTURE}.deb")
set(CPACK_DEBIAN_EXTERNAL_MODULES_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-linux-${CPACK_DEBIAN_ARCHITECTURE}-external-modules.deb")

# Add custom targets for package building
add_custom_target(package_server
  COMMAND ${CMAKE_CPACK_COMMAND} -D CPACK_COMPONENTS_ALL=server
  COMMENT "Building server package"
  DEPENDS ${PROJECT_NAME}
)

if(BUILD_EXTERNAL_MODULES)
  add_custom_target(package_external_modules
    COMMAND ${CMAKE_CPACK_COMMAND} -D CPACK_COMPONENTS_ALL=external_modules
    COMMENT "Building external modules package"
    DEPENDS ${SMU_MODULE_TARGETS}
  )

  add_custom_target(package_all
    COMMAND ${CMAKE_CPACK_COMMAND}
    COMMENT "Building all packages"
    DEPENDS ${PROJECT_NAME} ${SMU_MODULE_TARGETS}
  )
endif()

include(CPack)

message(STATUS "${PROJECT_NAME}: CONFIGURATION IS COMPLETE")
