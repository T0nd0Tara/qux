#!/bin/env sh
FLAGS="-Wextra -Wno-unknown-pragmas -std=c++20 -std=gnu++20"
LIBS="-I /home/amirs/code-libs/magic_enum/include"
FILES="./src/*.cpp"

NUM_ARGS=$#
ARGS=$@

for arg in $ARGS; do
  case $arg in 
    'debug') FLAGS+=" -DDEBUG";;
  esac
done

clang++ $FILES -o bin/qux $FLAGS $LIBS
