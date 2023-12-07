tmux \
    new-session "scripts/run-afl-instance.sh tests/corpus/pl0 pl0 $1 pl0 ; bash" \; \
    split-window -h "scripts/run-afl-instance.sh tests/corpus/ctu ctu $1 ctu ; bash" \;
