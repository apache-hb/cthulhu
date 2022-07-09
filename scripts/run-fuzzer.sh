tmux \
    new-session "scripts/run-afl-instance.sh tests/lang/pl0/fuzzing-input pl0 $1 pl0 ; bash" \; \
    split-window -h "scripts/run-afl-instance.sh tests/lang/ctu/fuzzing-input ctu $1 ctu ; bash" \;
