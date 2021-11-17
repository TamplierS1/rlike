cmake_minimum_required(VERSION 3.17.0)
project(rlike)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

set(LIBTCOD_GLAD vendored)
set(LIBTCOD_LODEPNG vendored)
set(LIBTCOD_STB vendored)

add_executable(rlike "")
add_subdirectory(src)
add_subdirectory(third_party/libtcod)
add_subdirectory(third_party/sdl2)

set(C_COMPILE_FLAGS)
set(C_LINK_FLAGS)
list(APPEND C_COMPILE_FLAGS -Wall -Wextra -pedantic)

if (CMAKE_BUILD_TYPE MATCHES Release)
    list(APPEND C_COMPILE_FLAGS -O3)
endif ()

set(C_INCLUDE_DIRS)
set(C_LINK_LIBS)
list(APPEND C_INCLUDE_DIRS
        "include"
        "third_party/libtcod/src"
        "third_party/sdl2/include")

list(APPEND C_LINK_OPTIONS PRIVATE
        -L "third_party/sdl2/build")

list(APPEND C_LINK_LIBS PRIVATE
        libtcod::libtcod SDL2-2.0)

target_compile_options(rlike PRIVATE ${C_COMPILE_FLAGS})
target_include_directories(rlike PRIVATE SYSTEM
        ${C_INCLUDE_DIRS})
target_link_options(rlike ${C_LINK_OPTIONS})
target_link_libraries(rlike ${C_LINK_LIBS})
