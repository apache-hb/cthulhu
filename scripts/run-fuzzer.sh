export AFL_CRASH_EXITCODE=99
afl-fuzz -i tests/pl0/multi/0/ -o afl-out -m none -d -- ./build-fuzz/driver/pl0/pl0c @@
