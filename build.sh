#!/bin/bash

set -e

build_validate=false
build_benchmark=false

for arg in "$@"; do
  case $arg in
    --validate|-v)
      build_validate=true
      ;;
    --benchmark|-b)
      build_benchmark=true
      ;;
    --help|-h)
      echo "usage: $0 [--validate/-v] [--benchmark/-b]"
      echo "  --validate/-v    validateLab"
      echo "  --benchmark/-b   benchmarkLab"
      exit 0
      ;;
  esac
done

BUILD_DIR="build"

cmake -E rm -rf "$BUILD_DIR"
cmake -E make_directory "$BUILD_DIR"

# 使用子 shell (...) 来执行编译，这样就不会改变你当前终端的路径
(
  cd "$BUILD_DIR"

  cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS="-g" -DCMAKE_CXX_FLAGS="-g" ..
  cmake --build . --config Debug --parallel 8

  if [ "$build_validate" = true ]; then
    cmake --build . --target validateLab
  fi

  if [ "$build_benchmark" = true ]; then
    cmake --build . --target benchmarkLab
  fi
)
