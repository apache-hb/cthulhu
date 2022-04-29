#!/bin/bash
for directory in src include plugins driver;
do 
    find $directory/ -name *.h -o -iname *.c | xargs clang-format -i --style=Microsoft
done
