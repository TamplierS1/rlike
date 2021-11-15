cmake_minimum_required(VERSION 3.17.0)
project(rlike)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

add_executable(rlike "")
add_subdirectory(src)

set(C_COMPILE_FLAGS)
set(C_LINK_FLAGS)
list(APPEND C_COMPILE_FLAGS -Wall -Wextra -pedantic)

if (CMAKE_BUILD_TYPE MATCHES Release)
    list(APPEND C_COMPILE_FLAGS -O3)
endif ()

set(C_INCLUDE_DIRS)
set(C_LINK_LIBS)
list(APPEND C_INCLUDE_DIRS
        "include")

list(APPEND C_LINK_LIBS ncurses)

target_compile_options(rlike PRIVATE ${C_COMPILE_FLAGS})

target_include_directories(rlike PRIVATE SYSTEM
        ${C_INCLUDE_DIRS})

target_link_libraries(rlike ${C_LINK_LIBS})
