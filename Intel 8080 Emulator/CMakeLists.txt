cmake_minimum_required(VERSION 3.5)
project(intel8080emulator)

file(GLOB_RECURSE SOURCES_8080
    "*.h"
    "*.cc"
)

set(CMAKE_STATIC_LIBRARY_PREFIX "")
add_executable(${PROJECT_NAME} ${SOURCES_8080})
add_library(lib_${PROJECT_NAME} STATIC ${SOURCES_8080})
target_compile_definitions(${PROJECT_NAME} PUBLIC E_I8080)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(lib_${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} -lpthread -lncurses)
target_link_libraries(lib_${PROJECT_NAME} -lpthread -lncurses)

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(lib_${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)



project(am9080emulator)

add_executable(${PROJECT_NAME} ${SOURCES_8080})
target_compile_definitions(${PROJECT_NAME} PUBLIC E_AM9080)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} -lpthread -lncurses)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)



project(nec8080emulator)

add_executable(${PROJECT_NAME} ${SOURCES_8080})
target_compile_definitions(${PROJECT_NAME} PUBLIC E_NEC8080)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} -lpthread -lncurses)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
