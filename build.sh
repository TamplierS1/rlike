#!/usr/bin/env bash

git submodule init
git submodule update

# Build dependencies
cd third_party/libtcod
git submodule init
git submodule update
mkdir build
cd build
cmake ..
make -j8
cd ../../..

cd third_party/sdl2
mkdir build
cd build
cmake ..
make -j8
cd ../../..

cd third_party/json-c
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DDISABLE_WERROR=ON -DBUILD_STATIC_LIBS=OFF ..
make -j8
cd ../../..

mkdir build
cd build
cmake ..
make -j8
