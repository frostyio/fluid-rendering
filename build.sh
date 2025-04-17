#!/bin/bash

BUILD_DIR="build"
if ! [ -d "$BUILD_DIR" ]; then
  mkdir "$BUILD_DIR"
fi

cd "$BUILD_DIR" || exit

cmake.exe .. -DUSE_CUSTOM_LIB_GLEW=ON
cmake.exe --build .
