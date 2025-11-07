# cmake file describing compiler options
# Usage: target_setup_compiler_options(<your target>

# setup standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

message(STATUS "Using C/C++ 23 standard")


# setup options
function(target_setup_compiler_options target)
    # windows
    if(MSVC)
        target_compile_options(${target} PRIVATE
            /W4 /permissive- /sdl
            /w14254 /w14265 /w14287 /we4265 /w14296
        )

        target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)

        message(STATUS "Using MSVC C/C++ compiler")
    # linux
    else()
        target_compile_options(${target} PRIVATE
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
        message(STATUS "Using gcc/clang C/C++ compiler")

    endif()


    # Optimization for Release
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(${target} PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/O2>
            $<$<CXX_COMPILER_ID:GNU,Clang>:-O3>
        )
    message(STATUS "Release optimizations are enabled")

    endif()


    # Sanitizers for Debug
    if(NOT CMAKE_CROSSCOMPILING)
        if((CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU") AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
            target_compile_options(${target} PRIVATE
                -fsanitize=address,undefined,leak -fno-omit-frame-pointer
                -g
            )
            target_link_options(${target} PRIVATE
                -fsanitize=address,undefined,leak
            )

            # Enable libstdc++ debug mode
            target_compile_definitions(${target} PRIVATE _GLIBCXX_DEBUG)
        endif()
    endif()

endfunction()
