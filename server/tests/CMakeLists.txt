# TESTS CMAKELISTS.TXT

include(FetchContent)

message(STATUS "Configuring googletest...")

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)

set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

message(STATUS "googletest configured")



# Finding test sources

message(STATUS "${PROJECT_NAME}: finding tests files...")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/unit/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/unit/*.hpp*)


list(LENGTH SOURCES SOURCES_SIZE)
message(STATUS "${PROJECT_NAME}: found ${SOURCES_SIZE} tests")



add_executable(tests ${SOURCES})
target_link_libraries(tests PRIVATE
    GTest::gtest_main
    jsoncpp_static
    smu-server-libary)
target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR}/include)



gtest_discover_tests(tests)
