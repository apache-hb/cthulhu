#!/bin/bash
for directory in src include plugins driver;
do 
    find $directory/ -iname '*.h' -o -iname '*.c' | xargs clang-format -i
done
