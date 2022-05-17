#!/bin/bash
for directory in src include interface driver;
do 
    find $directory/ -iname '*.h' -o -iname '*.c' -o -iname '*.cpp' -o -iname '*.hpp' | xargs clang-format -i
done
