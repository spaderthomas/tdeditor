#!/bin/sh
mkdir build > /dev/null
cd build

/usr/local/Cellar/gcc/8.3.0/bin/gcc-8 -g -Wall \
../src/*.c \
-I../include \
/Users/thspader/Programming/tdeditor/lib/libfreetype.a /Users/thspader/Programming/tdeditor/lib/libglfw3.a -lbz2 -lz \
-framework Cocoa -framework OpenGL -framework Cocoa -framework CoreVideo -framework IOKit \
-o tdeditor
