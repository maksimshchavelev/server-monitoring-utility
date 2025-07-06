cmake_minimum_required(VERSION 3.28 FATAL_ERROR)


set(PROJECT_NAME smu)
set(CMAKE_CXX_STANDARD 23)
project(${PROJECT_NAME} LANGUAGES CXX)



option(BUILD_SERVER "Need to build the server?" ON)
option(BUILD_CLI    "Need to build the CLI?"    ON)
option(BUILD_CLIENT "Need to build the client?" ON)



# If no build type is set, default to Release
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' by default")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()



message(STATUS " * BUILD SERVER: ${BUILD_SERVER}")
message(STATUS " * BUILD CLIENT: ${BUILD_CLIENT}")
message(STATUS " * BUILD CLI:    ${BUILD_CLI}")



if(${BUILD_SERVER})
  message(STATUS "Configuring server...")
  add_subdirectory(server)
endif()



if(${BUILD_CLIENT})
  message(STATUS "Configuring client...")
  add_subdirectory(client)
endif()



if(${BUILD_CLI})
  message(STATUS "Configuring CLI...")
  add_subdirectory(cli)
endif()



