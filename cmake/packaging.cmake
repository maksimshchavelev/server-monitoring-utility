# cmake/packaging.cmake
# Component-aware packaging helpers for DEB + convenience targets.
# Compatible with top-level CMakeLists that call:
#   check_packaging_dependencies()
#   initialize_packaging_system()
#   create_individual_package_targets(<list of component ids>)
#   setup_sdk_packaging()   (optional, defined below)
#   create_final_package_all()

# -----------------------
# check_packaging_dependencies()
# -----------------------
function(check_packaging_dependencies)
  if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(WARNING "DEB packaging is only supported on Linux systems")
  endif()

  find_program(DPKG_PROGRAM dpkg QUIET)
  if(NOT DPKG_PROGRAM)
    message(WARNING "dpkg not found - DEB packaging may not work correctly")
  endif()

  find_program(FAKEROOT_PROGRAM fakeroot QUIET)
  if(NOT FAKEROOT_PROGRAM)
    message(WARNING "fakeroot not found - package building may require root privileges")
  endif()
endfunction()

# -----------------------
# initialize_packaging_system()
# -----------------------
function(initialize_packaging_system)
  # Provide sensible defaults, can be overridden before calling include(cmake/packaging.cmake)
  if(NOT DEFINED PROJECT_VERSION)
    set(PROJECT_VERSION "0.0.0" CACHE STRING "Project version for packaging")
  endif()

  # Ensure generator and component install mode
  set(CPACK_GENERATOR "DEB" CACHE STRING "CPack generator")
  set(CPACK_DEB_COMPONENT_INSTALL ON CACHE BOOL "Enable DEB component install")

  # Basic metadata — change as needed in top-level CMake before calling this if required
  if(NOT DEFINED CPACK_PACKAGE_CONTACT)
    set(CPACK_PACKAGE_CONTACT "smu <noreply@example.com>" CACHE STRING "Package contact")
  endif()
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_CONTACT}" CACHE STRING "Debian maintainer")
  set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}" CACHE STRING "Package version")

  # Architecture
  if(NOT DEFINED CPACK_DEBIAN_PACKAGE_ARCHITECTURE)
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" _arch_lower)
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${_arch_lower}" CACHE STRING "Debian architecture")
  endif()

  # Make sure file name default is defined (we override it per-component later)
  if(NOT DEFINED CPACK_PACKAGE_FILE_NAME)
    string(TOLOWER "${CMAKE_PROJECT_NAME}-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}" _fname)
    set(CPACK_PACKAGE_FILE_NAME "${_fname}" CACHE STRING "Default package filename")
  endif()

  # Now include CPack module (defines cpack_add_component etc.)
  include(CPack)

  # prepare internal list for created package targets
  set(PACKAGE_TARGETS "" PARENT_SCOPE)
endfunction()

# -----------------------
# create_individual_package_targets(<component1> <component2> ...)
#   Creates:
#     - cpack components dynamically (cpack_add_component(...))
#     - add_custom_target(package-<component>) for each component that runs cpack to produce only that component
#   component ids must match COMPONENT names used in install(... COMPONENT <id>) in subprojects.
# -----------------------
function(create_individual_package_targets)
  find_program(CPACK_EXECUTABLE cpack REQUIRED)

  set(PACKAGE_TARGETS "")

  foreach(comp ${ARGN})
    # human readable
    string(REPLACE "-" " " comp_display "${comp}")

    # filename base: <component>-<version>-<sys>-<arch>
    string(TOLOWER "${comp}" comp_lower)
    string(TOLOWER "${PROJECT_VERSION}" version_lower)
    string(TOLOWER "${CMAKE_SYSTEM_NAME}" sys_lower)
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" arch_lower)
    set(filename_base "${comp_lower}-${version_lower}-${sys_lower}-${arch_lower}")

    # Build variable name for CPACK_DEBIAN_<COMP>_FILE_NAME
    # Replace non-alnum with underscore and uppercase
    string(REGEX REPLACE "[^A-Za-z0-9]" "_" comp_var "${comp}")
    string(TOUPPER "${comp_var}" comp_var_upper)

    set(target_name "package-${comp_lower}")

    add_custom_target(${target_name}
      COMMAND ${CPACK_EXECUTABLE}
              -G DEB
              -D CPACK_COMPONENTS_ALL="${comp}"
              -D CPACK_PACKAGE_FILE_NAME="${filename_base}"
              -D CPACK_DEBIAN_${comp_var_upper}_FILE_NAME="${filename_base}.deb"
              -D CPACK_PACKAGE_NAME="${comp_lower}"
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMENT "Building DEB package for component '${comp}' -> ${filename_base}.deb"
    )

    list(APPEND PACKAGE_TARGETS ${target_name})
  endforeach()

  set(PACKAGE_TARGETS ${PACKAGE_TARGETS} PARENT_SCOPE)
  message(STATUS "Packaging: created package targets for components: ${ARGN}")
endfunction()


# -----------------------
# setup_sdk_packaging()
#   Optional: if top-level/server sets SDK_SCRIPT and SDK_VERSION, this creates sdk-package target.
# -----------------------
function(setup_sdk_packaging PACKAGING_SCRIPT SDK_VERSION)
  # If SDK_SCRIPT is defined in subproject (server), use it
  if(PACKAGING_SCRIPT)
    add_custom_target(sdk-package
      COMMAND ${PACKAGING_SCRIPT} ${SDK_VERSION}
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMENT "Building SDK packages with script ${PACKAGING_SCRIPT}"
    )

    add_custom_command(TARGET sdk-package POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/packages"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_BINARY_DIR}/sdk-build/*.deb" "${CMAKE_BINARY_DIR}/packages/"
      COMMENT "Copying SDK packages to packages directory"
    )

    message(STATUS "Packaging: sdk-package target created using script ${PACKAGING_SCRIPT}")
  else()
    # Create a no-op target so package-all can always depend on sdk-package
    if(NOT TARGET sdk-package)
      add_custom_target(sdk-package
        COMMENT "No SDK script configured (noop sdk-package target)"
      )
      message(STATUS "Packaging: created noop sdk-package (PACKAGING_SCRIPT is empty)")
    endif()
  endif()
endfunction()


# --- Backward-compatible server packaging helper ---
# Call from server/CMakeLists.txt: setup_server_component_packaging()
# This will ensure component-mode is enabled and will export a sensible
# CPACK_COMPONENTS_ALL value into parent scope so top-level packaging sees it.
function(setup_server_component_packaging)
  # Ensure DEB component install is enabled (cache so it is visible globally)
  set(CPACK_DEB_COMPONENT_INSTALL ON CACHE BOOL "Enable DEB component install")

  # Declare default component ids used by the server CMakeLists
  # These names should match COMPONENT names used in install(...) calls in server CMake
  set(SERVER_COMPONENT_ID "server" PARENT_SCOPE)
  set(EXTERNAL_MODULES_COMPONENT_ID "external_modules" PARENT_SCOPE)

  # Configure CPACK_COMPONENTS_ALL so packaging tools know which components exist by default
  # Start with server; add external_modules if BUILD_EXTERNAL_MODULES is ON.
  set(_components_list server)
  if(DEFINED BUILD_EXTERNAL_MODULES AND BUILD_EXTERNAL_MODULES)
    list(APPEND _components_list external_modules)
  endif()
  # export to parent scope so top-level create_individual_package_targets sees it if necessary
  set(CPACK_COMPONENTS_ALL "${_components_list}" PARENT_SCOPE)

  # You can still set more CPACK_DEBIAN_* variables in server/CMakeLists after calling this function,
  # but do NOT call include(CPack) in server/CMakeLists if packaging.cmake already included CPack
  # via initialize_packaging_system(). That would be redundant and may cause ordering confusion.
  message(STATUS "Packaging: server component packaging initialized (components: ${_components_list})")
endfunction()


# -----------------------
# create_final_package_all()
#   Aggregates all per-component package targets + sdk-package into package-all.
# -----------------------
function(create_final_package_all)
  if(NOT DEFINED PACKAGE_TARGETS)
    set(PACKAGE_TARGETS "" )
  endif()

  add_custom_target(package-all
    COMMENT "Building all component .deb packages and collecting them into ${CMAKE_BINARY_DIR}/packages"
    DEPENDS ${PACKAGE_TARGETS} sdk-package
  )

  add_custom_command(TARGET package-all POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/packages"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_BINARY_DIR}/*.deb" "${CMAKE_BINARY_DIR}/packages/"
    COMMENT "Collected packages into ${CMAKE_BINARY_DIR}/packages"
  )
endfunction()
