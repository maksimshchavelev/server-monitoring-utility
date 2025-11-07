cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

set(PROJECT_NAME smu)
set(CMAKE_CXX_STANDARD 23)
project(${PROJECT_NAME} VERSION 1.0.0 LANGUAGES CXX)

set(SDK_VERSION 1.0.0)

# Build options
option(BUILD_SERVER "Need to build server part?" ON)
option(BUILD_CLIENT "Need to build client part?" ON)
option(BUILD_CLI "Need to build CLI part?" ON)

# Include packaging helpers and checks (do NOT initialize CPack yet)
include(cmake/packaging.cmake)
check_packaging_dependencies()

# Include other helpers
include(cmake/compiler_options.cmake)
include(cmake/dependencies.cmake)

# Add subprojects
if(BUILD_SERVER)
  add_subdirectory(server)
endif()

if(BUILD_CLI)
  add_subdirectory(cli)
endif()

if(BUILD_CLIENT)
  add_subdirectory(client)
endif()


# Configure components for packaging — use component ids that match install(... COMPONENT ...)
set(ALL_PACKAGE_COMPONENTS "")

# Init packaging system
initialize_packaging_system()

if(BUILD_SERVER)
  list(APPEND ALL_PACKAGE_COMPONENTS "server")
  if(BUILD_EXTERNAL_MODULES)
    list(APPEND ALL_PACKAGE_COMPONENTS "external_modules")
  endif()
endif()

if(BUILD_CLI)
  list(APPEND ALL_PACKAGE_COMPONENTS "smu-cli")
endif()

if(BUILD_CLIENT)
  list(APPEND ALL_PACKAGE_COMPONENTS "smu")
endif()


# Create individual package targets with proper versions
create_individual_package_targets(${ALL_PACKAGE_COMPONENTS})

# Setup SDK packaging
setup_sdk_packaging(${CMAKE_SOURCE_DIR}/server/scripts/build-sdk.sh ${SDK_VERSION})

# Create final package-all target
create_final_package_all()

message(STATUS "Project configuration complete. Package targets: ${ALL_PACKAGE_COMPONENTS}")
