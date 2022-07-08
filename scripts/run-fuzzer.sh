tmux \
    new-session 'scripts/run-afl-instance.sh tests/lang/pl0/fuzzing-input pl0 pl0 ; bash' \; \
    split-window -h 'scripts/run-afl-instance.sh tests/lang/ctu/fuzzing-input ctu ctu ; bash' \;
