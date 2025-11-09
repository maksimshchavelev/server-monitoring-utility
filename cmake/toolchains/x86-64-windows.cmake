# cmake/toolchains/x86-64-windows.cmake
# Minimal Windows toolchain

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Compilers
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)

# Flags
set(CMAKE_CXX_FLAGS_INIT "-std=c++23")
set(CMAKE_C_FLAGS_INIT "")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-static-libgcc")
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc")

if(NOT TARGET Crypt32) 
    find_library(CRYPT32_LIBRARY crypt32 PATHS /usr/x86_64-w64-mingw32/lib NO_DEFAULT_PATH)
    if(CRYPT32_LIBRARY)
    	add_library(Crypt32 INTERFACE IMPORTED)
        set_target_properties(Crypt32 PROPERTIES
            INTERFACE_LINK_LIBRARIES "${CRYPT32_LIBRARY}"
        )
        message(STATUS "Created imported target Crypt32")
    endif()
endif()

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

message(STATUS "Using minimal Windows toolchain")
message(STATUS "CXX compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "CXX flags: ${CMAKE_CXX_FLAGS_INIT}")
message(STATUS "Linker flags: ${CMAKE_EXE_LINKER_FLAGS_INIT}")
