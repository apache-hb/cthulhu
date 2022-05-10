tmux \
    new-session 'scripts/run-afl-instance.sh tests/pl0/multi/0 pl0 pl0/pl0c ; bash' \; \
    split-window -h 'scripts/run-afl-instance.sh tests/ctu/types/alias/pass ctu ctu/ctc ; bash' \;
