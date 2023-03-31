#!/usr/bin/bash
SOURCES="src io app"
find $SOURCES \( -name "*.cpp" -or -name "*.h" \) -exec clang-format --style=Google -i {} \;
clang-format --style=Google -i *.h
