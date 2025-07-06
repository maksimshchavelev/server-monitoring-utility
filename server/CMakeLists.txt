cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

project(smu-server VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)


include(FetchContent)

message(STATUS "${PROJECT_NAME}: finding source files...")



file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/include/*.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/*.tpp)



list(LENGTH SOURCES SOURCES_SIZE)
message(STATUS "${PROJECT_NAME}: found ${SOURCES_SIZE} sources")



find_package(jsoncpp REQUIRED)




message(STATUS "${PROJECT_NAME}: configuring Drogon...")

# Disable uneccessary things for Drogon
set(BUILD_CTL OFF)
set(BUILD_EXAMPLES OFF)
set(BUILD_SHARED_LIBS OFF)
set(DROGON_BUILD_STATIC ON)

FetchContent_Declare(drogon
    GIT_REPOSITORY https://github.com/drogonframework/drogon.git
    GIT_TAG v1.9.11
    EXCLUDE_FROM_ALL
)
FetchContent_Populate(drogon)
add_subdirectory(${drogon_SOURCE_DIR} ${drogon_BINARY_DIR} EXCLUDE_FROM_ALL)




message(STATUS "${PROJECT_NAME}: Configuration of Drogon is complete")
message(STATUS "${PROJECT_NAME}: Configuring target and includes...")




add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE
    drogon
    jsoncpp_static
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




message("${PROJECT_NAME}: CONFIGURATION IS COMPLETE")


