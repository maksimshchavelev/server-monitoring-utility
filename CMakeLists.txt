cmake_minimum_required(VERSION 3.28 FATAL_ERROR)


set(PROJECT_NAME smu)
set(CMAKE_CXX_STANDARD 23)
project(${PROJECT_NAME} NONE)


option(BUILD_SERVER "Need to build server part?" ON)
option(BUILD_CLIENT "Need to build client part?" ON)
option(BUILD_CLI "Need to build CLI part?" ON)


include(ExternalProject)


if(BUILD_SERVER)
  ExternalProject_Add(server
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/server
    BINARY_DIR ${CMAKE_BINARY_DIR}/server-build
    INSTALL_COMMAND ""
  )
endif()


if(BUILD_CLI)
ExternalProject_Add(cli
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/cli
  BINARY_DIR ${CMAKE_BINARY_DIR}/cli-build
  INSTALL_COMMAND ""
)
endif()


if(BUILD_CLIENT)
ExternalProject_Add(client
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/client
  BINARY_DIR ${CMAKE_BINARY_DIR}/client-build
  INSTALL_COMMAND ""
)
endif()


add_custom_target(build-all)


if(BUILD_SERVER)
    add_dependencies(build-all server)
endif()

if(BUILD_CLI)
    add_dependencies(build-all cli)
endif()

if(BUILD_CLIENT)
    add_dependencies(build-all client)
endif()



# Packaging settings
set(PACKAGE_OUTPUT_DIR ${CMAKE_BINARY_DIR}/packages)
file(MAKE_DIRECTORY ${PACKAGE_OUTPUT_DIR})

add_custom_target(package-all
  COMMENT "Building and collecting packages for server, cli and client"
)


function(add_subproject_package_target name)
  set(build_dir ${CMAKE_BINARY_DIR}/${name}-build)

  add_custom_target(${name}-package
    COMMAND ${CMAKE_COMMAND} --build ${build_dir} --target package
    COMMENT "Packaging ${name}"
    WORKING_DIRECTORY ${build_dir}
  )

  add_custom_command(
    TARGET ${name}-package POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${build_dir}/*.deb ${PACKAGE_OUTPUT_DIR}
    COMMENT "Copying ${name} package to ${PACKAGE_OUTPUT_DIR}"
  )

  add_custom_target(${name}-all
    DEPENDS ${name}-package
  )

  add_dependencies(package-all ${name}-all)
endfunction()




if(BUILD_SERVER)
  add_subproject_package_target(server)
endif()


if(BUILD_CLIENT)
  add_subproject_package_target(client)
endif()


if(BUILD_CLI)
  add_subproject_package_target(cli)
endif()


