cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

project(smu-server VERSION 1.0.0 LANGUAGES CXX)

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


file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/include/*.hpp*
    ${CMAKE_CURRENT_LIST_DIR}/include/*.tpp*)

list(LENGTH SOURCES SOURCES_SIZE)
message(STATUS "${PROJECT_NAME}: found ${SOURCES_SIZE} sources")


# Setup target
add_executable(${PROJECT_NAME} ${SOURCES})
include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE
    drogon
    jsoncpp_static
    cxxopts
)

# Configuring dependencies
find_package(jsoncpp REQUIRED)
load_drogon()
load_cxxopts()

# Setup compiler
target_setup_compiler_options(${PROJECT_NAME})


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


# Component packaging setup
setup_server_component_packaging()

# Installation with components
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    COMPONENT server
)

# Install config files
install(
    DIRECTORY config/
    DESTINATION ${CONFIGS_DIR}
    COMPONENT server
)

# Install systemd service
install(FILES systemd/smu-server.service
    DESTINATION "/etc/systemd/system"
    COMPONENT server
)

# Install external modules with component
if(BUILD_EXTERNAL_MODULES)
    foreach(MOD_TARGET ${SMU_MODULE_TARGETS})
        install(
            FILES "$<TARGET_FILE:${MOD_TARGET}>"
            DESTINATION "${CONFIGS_DIR}/modules.d/${MOD_TARGET}"
            PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
            COMPONENT external_modules
        )
    endforeach()
endif()

message(STATUS "${PROJECT_NAME}: CONFIGURATION IS COMPLETE")
