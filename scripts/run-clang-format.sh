#!/bin/bash
for directory in src include interface driver;
do 
    find $directory/ -iname '*.h' -o -iname '*.c' | xargs clang-format -i
done
