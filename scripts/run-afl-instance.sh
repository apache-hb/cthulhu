AFL_CRASH_EXITCODE=99 afl-fuzz -i $1 -o afl-out-$2 -m none -d -- ./build-fuzz/driver/$3 @@
