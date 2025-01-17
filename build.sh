#!/bin/env sh
FLAGS="-Wextra -Wno-unknown-pragmas -std=c++20 -std=gnu++20"
LIBS="-I /home/amirs/code-libs/magic_enum/include"
FILES="./src/*.cpp"

clang++ $FILES -o bin/qux $FLAGS $LIBS
