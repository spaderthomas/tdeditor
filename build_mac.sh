#!/bin/sh
mkdir build > /dev/null 2>&1
cd build

echo "...building tdeditor"
/usr/local/Cellar/gcc/9.1.0/bin/gcc-9 -g \
-Wall -Wno-char-subscripts -Wno-unused-variable -Wno-missing-braces -Wno-unused-result \
../src/*.c \
-I../include \
/Users/thspader/Programming/tdeditor/lib/libfreetype.a /Users/thspader/Programming/tdeditor/lib/libglfw3.a -lbz2 -lz \
-framework Cocoa -framework OpenGL -framework CoreVideo -framework IOKit \
-o tdeditor
