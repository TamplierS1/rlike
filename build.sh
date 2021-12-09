#!/usr/bin/env bash

git submodule init
git submodule update

cd third_party/sds
cp sds.c ../../src
cp sds.h ../../include
cp sdsalloc.h ../../include
cd ../..

mkdir build
cd build
cmake ..
make -j8
