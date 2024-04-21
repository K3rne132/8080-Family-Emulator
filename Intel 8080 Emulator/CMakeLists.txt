cmake_minimum_required(VERSION 3.5)
project(intel8080emulator)

file(GLOB_RECURSE SOURCES
    "*.h"
    "*.cc"
)

add_executable(${PROJECT_NAME} ${SOURCES})
add_library(lib_${PROJECT_NAME} STATIC ${SOURCES})
target_include_directories(lib_${PROJECT_NAME} INTERFACE ./)
target_link_libraries(${PROJECT_NAME})
target_link_libraries(lib_${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(lib_${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
