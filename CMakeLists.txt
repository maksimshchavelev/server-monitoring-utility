cmake_minimum_required(VERSION 3.28 FATAL_ERROR)


set(PROJECT_NAME smu)
set(CMAKE_CXX_STANDARD 23)
project(${PROJECT_NAME} NONE)


include(ExternalProject)

ExternalProject_Add(server
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/server
  BINARY_DIR ${CMAKE_BINARY_DIR}/server-build
  INSTALL_COMMAND ""
)

ExternalProject_Add(cli
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/cli
  BINARY_DIR ${CMAKE_BINARY_DIR}/cli-build
  INSTALL_COMMAND ""
)

ExternalProject_Add(client
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/client
  BINARY_DIR ${CMAKE_BINARY_DIR}/client-build
  INSTALL_COMMAND ""
)

add_custom_target(build-all DEPENDS server cli client)


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


add_subproject_package_target(server)
add_subproject_package_target(cli)
add_subproject_package_target(client)


