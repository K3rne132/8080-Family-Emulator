cmake_minimum_required(VERSION 3.5)
project(intel8080_family_emulators)
set(CMAKE_CXX_STANDARD 14)

option(BUILD_TEST "Builds unit tests" ON)

add_subdirectory("Intel 8080 Emulator")
add_subdirectory("Intel 8085 Emulator")

if (BUILD_TEST)
    add_subdirectory("8080 Family Tests")
endif()
