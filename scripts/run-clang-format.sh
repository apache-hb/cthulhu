#!/bin/bash
for directory in common cthulhu interface driver tools;
do 
    find $directory/ -iname '*.h' -o -iname '*.c' -o -iname '*.cpp' -o -iname '*.hpp' | xargs clang-format -i
done
