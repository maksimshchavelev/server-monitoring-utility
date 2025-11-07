# Dependencies loader
# Centralized dependency management

set(CXXOPTS_VERSION v3.3.1)         # load_cxxopts()
set(JSONCPP_VERSION 1.9.6)          # load_jsoncpp()
set(IXWEBSOCKET_VERSION v11.4.6)    # load_ixwebsocket()
set(FTXUI_VERSION v6.1.9)           # load_ftxui()
set(DROGON_VERSION v1.9.11)         # load_drogon()

include(FetchContent)

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

    message(STATUS "Loading IXWebSocket ${IXWEBSOCKET_VERSION}")

    set(USE_TLS ON)
    set(USE_OPEN_SSL ON)
    set(USE_MBED_TLS OFF)
    set(USE_ZLIB OFF)

    FetchContent_Declare(ixwebsocket
        GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
        GIT_TAG ${IXWEBSOCKET_VERSION}
        EXCLUDE_FROM_ALL
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
