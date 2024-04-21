cmake_minimum_required(VERSION 3.5)
project(intel8080family_tests)

file(GLOB_RECURSE SOURCES
    "*.h"
    "*.cc"
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} gtest gtest_main lib_intel8080emulator)
