cmake_minimum_required(VERSION 3.17.0)
project(rlike)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

set(LIBTCOD_GLAD vendored)
set(LIBTCOD_LODEPNG vendored)
set(LIBTCOD_STB vendored)

set(SDL2_SHARED OFF)
set(DISABLE_WERROR ON)
set(DISABLE_EXTRA_LIBST ON)

add_executable(rlike "")
add_subdirectory(src)
add_subdirectory(third_party/libtcod)
add_subdirectory(third_party/SDL)
add_subdirectory(third_party/json-c)

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
        "third_party/SDL/include"
        "third_party/json-c/build")

list(APPEND C_LINK_OPTIONS PRIVATE
        -L"third_party/SDL/build"
        -L"third_party/libtcod/build/lib"
        -L"third_party/json-c/build")

list(APPEND C_LINK_LIBS PRIVATE
        libtcod::libtcod SDL2-2.0 json-c)

target_compile_options(rlike PRIVATE ${C_COMPILE_FLAGS})
target_include_directories(rlike PRIVATE SYSTEM
        ${C_INCLUDE_DIRS})
target_link_options(rlike ${C_LINK_OPTIONS})
target_link_libraries(rlike ${C_LINK_LIBS})
