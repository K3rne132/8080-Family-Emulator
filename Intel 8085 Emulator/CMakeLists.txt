cmake_minimum_required(VERSION 3.5)
project(intel8085emulator)

add_compile_definitions(E_I8085)

file(GLOB_RECURSE SOURCES_8085
    "*.h"
    "*.cc"
)

file(GLOB_RECURSE SOURCES_8080
    "./../Intel 8080 Emulator/*.h"
    "./../Intel 8080 Emulator/*.cc"
)

add_executable(${PROJECT_NAME} ${SOURCES_8085} ${SOURCES_8080})
target_include_directories(${PROJECT_NAME} PRIVATE "./../Intel 8080 Emulator")

target_link_libraries(${PROJECT_NAME} -lpthread -lncurses)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
