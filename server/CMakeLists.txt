cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

project(smu-server VERSION 0.2.2 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_INSTALL_PREFIX /usr)

# For IPC using an abstract Unix Domain Socket
set(ABSTRACT_SOCKET_NAME "smu.ipc.socket")


# This is where the program configuration will be
set(CONFIGS_DIR "/var/lib/${PROJECT_NAME}")
set(SERVER_CONFIG_PATH "${CONFIGS_DIR}/config.json")
set(MODULES_CONFIGS_DIR "${CONFIGS_DIR}/modules.d")
set(INSTALLED_BINARY_PATH "${CMAKE_INSTALL_PREFIX}/bin/${PROJECT_NAME}")
set(SMU_UPLOAD_PATH ${CONFIGS_DIR}/uploads/tmp)


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


if(CMAKE_BUILD_TYPE EQUAL "Release")
  message(STATUS "Setting O3 optimization")
  target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-O3>)
endif()




# Configuring systemd
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/systemd/smu-server.service.in
  ${CMAKE_CURRENT_SOURCE_DIR}/systemd/smu-server.service
  @ONLY
)


# Installation rules
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(DIRECTORY config/ DESTINATION ${CONFIGS_DIR} FILES_MATCHING PATTERN "*")
install(FILES systemd/smu-server.service DESTINATION "/etc/systemd/system")

# For drogon tmp files
install(DIRECTORY DESTINATION ${SMU_UPLOAD_PATH}
         DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE
                               GROUP_READ GROUP_EXECUTE
                               WORLD_READ WORLD_EXECUTE)



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

include(CPack)



message(STATUS "${PROJECT_NAME}: CONFIGURATION IS COMPLETE")


