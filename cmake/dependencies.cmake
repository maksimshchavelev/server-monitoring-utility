# Dependencies loader
# Centralized dependency management

set(CXXOPTS_VERSION v3.3.1)         # load_cxxopts()
set(JSONCPP_VERSION 1.9.6)          # load_jsoncpp()
set(IXWEBSOCKET_VERSION v11.4.6)    # load_ixwebsocket()
set(FTXUI_VERSION v6.1.9)           # load_ftxui()
set(DROGON_VERSION v1.9.11)         # load_drogon()

include(FetchContent)


function(load_openssl)
    message(STATUS "=== OpenSSL Configuration ===")
    message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
    message(STATUS "CMAKE_CROSSCOMPILING: ${CMAKE_CROSSCOMPILING}")
    message(STATUS "OPENSSL_ROOT_DIR: ${OPENSSL_ROOT_DIR}")
    message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
    message(STATUS "CMAKE_LIBRARY_PATH: ${CMAKE_LIBRARY_PATH}")
    message(STATUS "CMAKE_INCLUDE_PATH: ${CMAKE_INCLUDE_PATH}")

    if(OPENSSL_ROOT_DIR)
        message(STATUS "Using explicit OpenSSL root: ${OPENSSL_ROOT_DIR}")
        
        if(NOT EXISTS "${OPENSSL_ROOT_DIR}")
            message(FATAL_ERROR "OPENSSL_ROOT_DIR directory does not exist: ${OPENSSL_ROOT_DIR}")
        endif()

        # Find include dir
        set(OPENSSL_POSSIBLE_INCLUDE_DIRS
            "${OPENSSL_ROOT_DIR}/include"
            "${OPENSSL_ROOT_DIR}/include/openssl"
            "${OPENSSL_ROOT_DIR}/inc32"
            "${OPENSSL_ROOT_DIR}/inc64"
        )
        
        set(OPENSSL_INCLUDE_DIR "")
        foreach(dir ${OPENSSL_POSSIBLE_INCLUDE_DIRS})
            if(EXISTS "${dir}" AND IS_DIRECTORY "${dir}")
                set(OPENSSL_INCLUDE_DIR "${dir}")
                message(STATUS "Found OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
                break()
            endif()
        endforeach()
        
        if(NOT OPENSSL_INCLUDE_DIR)
            message(FATAL_ERROR "Could not find OpenSSL include directory in ${OPENSSL_ROOT_DIR}")
        endif()

        # Find libs
        set(OPENSSL_POSSIBLE_LIB_DIRS
            "${OPENSSL_ROOT_DIR}/lib"
            "${OPENSSL_ROOT_DIR}/lib64"
            "${OPENSSL_ROOT_DIR}/lib32"
            "${OPENSSL_ROOT_DIR}/out/lib"
            "${OPENSSL_ROOT_DIR}/lib/MinGW"
            "${OPENSSL_ROOT_DIR}/lib/mingw"
        )

        message(STATUS "Searching for OpenSSL libraries in:")
        foreach(lib_dir ${OPENSSL_POSSIBLE_LIB_DIRS})
            if(EXISTS "${lib_dir}" AND IS_DIRECTORY "${lib_dir}")
                message(STATUS "  - ${lib_dir}")
                file(GLOB lib_files "${lib_dir}/*")
                foreach(lib_file ${lib_files})
                    message(STATUS "    - ${lib_file}")
                endforeach()
            endif()
        endforeach()

        # Find SSL lib
        find_library(OPENSSL_SSL_LIBRARY
            NAMES 
                ssl libssl 
                ssl-3-x64 libssl-3-x64 ssl-3 libssl-3
                ssl-1_1-x64 libssl-1_1-x64 ssl-1_1 libssl-1_1
                ssleay32 libssleay32
            PATHS ${OPENSSL_POSSIBLE_LIB_DIRS}
            NO_DEFAULT_PATH
            NO_CMAKE_FIND_ROOT_PATH
        )

        # Find Crypto lib  
        find_library(OPENSSL_CRYPTO_LIBRARY
            NAMES 
                crypto libcrypto 
                crypto-3-x64 libcrypto-3-x64 crypto-3 libcrypto-3
                crypto-1_1-x64 libcrypto-1_1-x64 crypto-1_1 libcrypto-1_1
                libeay32 liblibeay32
            PATHS ${OPENSSL_POSSIBLE_LIB_DIRS}
            NO_DEFAULT_PATH
            NO_CMAKE_FIND_ROOT_PATH
        )

        message(STATUS "OpenSSL SSL library search result: ${OPENSSL_SSL_LIBRARY}")
        message(STATUS "OpenSSL Crypto library search result: ${OPENSSL_CRYPTO_LIBRARY}")

        if(OPENSSL_SSL_LIBRARY AND OPENSSL_CRYPTO_LIBRARY)
            message(STATUS "Successfully found OpenSSL libraries:")
            message(STATUS "  SSL: ${OPENSSL_SSL_LIBRARY}")
            message(STATUS "  Crypto: ${OPENSSL_CRYPTO_LIBRARY}")
            
            if(NOT TARGET OpenSSL::SSL)
                add_library(OpenSSL::SSL UNKNOWN IMPORTED)
                set_target_properties(OpenSSL::SSL PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}"
                    IMPORTED_LOCATION "${OPENSSL_SSL_LIBRARY}"
                )
                message(STATUS "Created imported target OpenSSL::SSL")
            endif()
            
            if(NOT TARGET OpenSSL::Crypto)
                add_library(OpenSSL::Crypto UNKNOWN IMPORTED)
                set_target_properties(OpenSSL::Crypto PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}"
                    IMPORTED_LOCATION "${OPENSSL_CRYPTO_LIBRARY}"
                )
                message(STATUS "Created imported target OpenSSL::Crypto")
            endif()
            
            set(OpenSSL_FOUND TRUE PARENT_SCOPE)
            set(OPENSSL_FOUND TRUE PARENT_SCOPE)
            message(STATUS "OpenSSL configuration completed successfully")
        else()
            message(STATUS "Falling back to system OpenSSL search...")
            
            find_package(OpenSSL QUIET)
            if(OpenSSL_FOUND)
                message(STATUS "System OpenSSL found: ${OpenSSL_VERSION}")
            else()
                message(FATAL_ERROR "Could not find OpenSSL libraries in ${OPENSSL_ROOT_DIR} and system search also failed")
            endif()
        endif()
    else()
        message(STATUS "No OPENSSL_ROOT_DIR specified, using system OpenSSL search")
        find_package(OpenSSL REQUIRED)
        message(STATUS "System OpenSSL found: ${OpenSSL_VERSION}")
    endif()
    
    message(STATUS "=== OpenSSL Configuration Complete ===")
endfunction()



function(load_cxxopts)
    if(TARGET cxxopts::cxxopts)
        return()
    endif()

    message(STATUS "Loading cxxopts ${CXXOPTS_VERSION}")

    FetchContent_Declare(cxxopts
        GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
        GIT_TAG ${CXXOPTS_VERSION}
        EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(cxxopts)

    if(NOT TARGET cxxopts::cxxopts)
        add_library(cxxopts::cxxopts ALIAS cxxopts)
    endif()
endfunction()



function(load_jsoncpp)
    if(TARGET jsoncpp_static)
        return()
    endif()

    message(STATUS "Loading jsoncpp ${JSONCPP_VERSION}")

    FetchContent_Declare(jsoncpp
        GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
        GIT_TAG ${JSONCPP_VERSION}
        EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(jsoncpp)
endfunction()




function(load_ixwebsocket)
    if(TARGET ixwebsocket)
        return()
    endif()

    message(STATUS "Fetching IXWebSocket ${IXWEBSOCKET_VERSION}")
	
    set(USE_TLS ON)
    set(USE_OPEN_SSL ON)
    set(USE_MBED_TLS OFF)
    set(USE_ZLIB OFF)
	
    include(FetchContent)
    FetchContent_Declare(ixwebsocket
        GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
        GIT_TAG ${IXWEBSOCKET_VERSION}
        CMAKE_ARGS
            -DOPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR}
            -DOPENSSL_INCLUDE_DIR=${OPENSSL_INCLUDE_DIR}
            -DOPENSSL_CRYPTO_LIBRARY=${OPENSSL_CRYPTO_LIBRARY}
            -DOPENSSL_SSL_LIBRARY=${OPENSSL_SSL_LIBRARY}
    )
    FetchContent_MakeAvailable(ixwebsocket)
endfunction()






function(load_ftxui)
    if(TARGET ftxui::component)
        return()
    endif()

    message(STATUS "Loading FTXUI ${FTXUI_VERSION}")

    set(FTXUI_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
        GIT_TAG ${FTXUI_VERSION}
        EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(ftxui)
endfunction()



function(load_drogon)
    if(TARGET drogon)
        return()
    endif()

    message(STATUS "Loading Drogon ${DROGON_VERSION}")

    # Drogon configuration
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
        GIT_TAG ${DROGON_VERSION}
        EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(drogon)
endfunction()



# Function to setup Windows DLL deployment on Linux host
function(setup_windows_dlls_on_linux target_name)
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(DLL_PATH "/usr/lib/gcc/x86_64-w64-mingw32/13-win32")
        
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DLL_PATH}/libstdc++-6.dll"
                "$<TARGET_FILE_DIR:${target_name}>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DLL_PATH}/libgcc_s_seh-1.dll" 
                "$<TARGET_FILE_DIR:${target_name}>"
            COMMENT "Copying Windows DLLs"
        )
        message(STATUS "DLLs will be copied from: ${DLL_PATH}")
    endif()
endfunction()
