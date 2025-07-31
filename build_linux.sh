#!/bin/sh

CXX=g++-14
JOBS=8

if [ ! -d build ]; then
    mkdir build
fi
(cd build && cmake .. -DCMAKE_CXX_COMPILER=${CXX} -DCMAKE_BUILD_TYPE=Debug && cmake --build . -j${JOBS})
