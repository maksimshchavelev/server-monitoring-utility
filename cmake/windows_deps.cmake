# cmake/windows_deps.cmake
# Detects MinGW toolchain usage and, if building client for Windows,
# configures ExternalProject targets to fetch and build OpenSSL and jsoncpp
# statically for the Windows target.  It installs into a local DESTDIR
# inside build dir so that the client build can link against those libs.

include(ExternalProject)

# Guard: only run this when using mingw toolchain and client is enabled
if(NOT (DEFINED USING_MINGW AND USING_MINGW))
  return()
endif()

if(NOT (DEFINED BUILD_CLIENT AND BUILD_CLIENT))
  message(STATUS "USING_MINGW is ON but BUILD_CLIENT is OFF — skipping windows deps auto-build")
  return()
endif()

message(STATUS "USING_MINGW=ON and BUILD_CLIENT=ON -> configuring ExternalProject for jsoncpp and OpenSSL")

# Configurable variables (override via -D)
set(WIN_OPENSSL_TAG "OpenSSL_3_1_6" CACHE STRING "OpenSSL git tag to use for MinGW build")
set(WIN_JSONCPP_TAG "1.10.0" CACHE STRING "jsoncpp tag to build")
set(WIN_OPENSSL_REPO "https://github.com/openssl/openssl.git" CACHE STRING "OpenSSL git repo")
set(WIN_JSONCPP_REPO "https://github.com/open-source-parsers/jsoncpp.git" CACHE STRING "jsoncpp git repo")

# Where to put the target sysroot inside build dir (will be used as DESTDIR)
set(WIN_SYSROOT "${CMAKE_BINARY_DIR}/mingw-sysroot" CACHE PATH "Local sysroot/DESTDIR for MinGW-built deps")
file(MAKE_DIRECTORY "${WIN_SYSROOT}")

# Helper values from toolchain file
if(NOT DEFINED CROSS_PREFIX)
  set(CROSS_PREFIX "x86_64-w64-mingw32-")
endif()
set(MINGW_CC "${MINGW_CC}" CACHE FILEPATH "")
set(MINGW_CXX "${MINGW_CXX}" CACHE FILEPATH "")
set(MINGW_AR "${MINGW_AR}" CACHE STRING "")
set(MINGW_RANLIB "${MINGW_RANLIB}" CACHE STRING "")

# Define jsoncpp ExternalProject
ExternalProject_Add(jsoncpp_external
  GIT_REPOSITORY ${WIN_JSONCPP_REPO}
  GIT_TAG ${WIN_JSONCPP_TAG}
  PREFIX ${CMAKE_BINARY_DIR}/external/jsoncpp
  SOURCE_DIR ${CMAKE_BINARY_DIR}/external/jsoncpp/src
  BINARY_DIR ${CMAKE_BINARY_DIR}/external/jsoncpp/build
  # Use Unix Makefiles so DESTDIR install via env DESTDIR=... works reliably
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DBUILD_SHARED_LIBS=OFF
    -DJSONCPP_WITH_TESTS=OFF
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=/usr
    -G "Unix Makefiles"
  BUILD_BYPRODUCTS ${CMAKE_BINARY_DIR}/external/jsoncpp/build/libjsoncpp.a
  BUILD_COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/external/jsoncpp/build -- -j${CMAKE_BUILD_PARALLEL_LEVEL}
  INSTALL_COMMAND ${CMAKE_COMMAND} -E env "DESTDIR=${WIN_SYSROOT}" ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/external/jsoncpp/build --target install
)

# Define OpenSSL ExternalProject (uses Configure + make)
ExternalProject_Add(openssl_external
  GIT_REPOSITORY ${WIN_OPENSSL_REPO}
  GIT_TAG ${WIN_OPENSSL_TAG}
  PREFIX ${CMAKE_BINARY_DIR}/external/openssl
  SOURCE_DIR ${CMAKE_BINARY_DIR}/external/openssl/src
  BINARY_DIR ${CMAKE_BINARY_DIR}/external/openssl/build
  # Configure: use Configure script; provide cross tools via env
  CONFIGURE_COMMAND
    ${CMAKE_COMMAND} -E env
      CC=${MINGW_CC}
      AR=${CROSS_PREFIX}ar
      RANLIB=${CROSS_PREFIX}ranlib
      ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/external/openssl/src ./Configure mingw64 no-shared --prefix=/usr --openssldir=/usr
  BUILD_COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/external/openssl/build -- -j${CMAKE_BUILD_PARALLEL_LEVEL}
  INSTALL_COMMAND ${CMAKE_COMMAND} -E env "DESTDIR=${WIN_SYSROOT}" ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/external/openssl/build --target install
)

# Ensure client packaging depends on those builds:
# package-smu is the per-component package target created by packaging.cmake for component 'smu'
if(TARGET package-smu)
  add_dependencies(package-smu jsoncpp_external openssl_external)
endif()
# package-all should also wait for them
if(TARGET package-all)
  add_dependencies(package-all jsoncpp_external openssl_external)
endif()

# Also make the main client build depend on libs being installed into WIN_SYSROOT
# We provide a convenient imported CMake config for downstream find_package calls by adding the
# WIN_SYSROOT to CMAKE_PREFIX_PATH at configure time so find_package searches there.
list(APPEND CMAKE_PREFIX_PATH "${WIN_SYSROOT}/usr")
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" CACHE STRING "Prefix path including windows-sysroot" FORCE)

# For pkg-config based finders, ensure PKG_CONFIG_LIBDIR is set to find .pc files installed into the sysroot
if(NOT DEFINED ENV{PKG_CONFIG_LIBDIR})
  set(ENV{PKG_CONFIG_LIBDIR} "${WIN_SYSROOT}/usr/lib/pkgconfig:${WIN_SYSROOT}/usr/share/pkgconfig")
endif()

message(STATUS "Configured Windows deps ExternalProject: jsoncpp_external, openssl_external")
message(STATUS "WIN_SYSROOT = ${WIN_SYSROOT}")
message(STATUS "CMAKE_PREFIX_PATH (appended) = ${CMAKE_PREFIX_PATH}")

