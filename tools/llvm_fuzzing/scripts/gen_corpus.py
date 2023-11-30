#!/usr/bin/env python3

import sys
import os
from subprocess import run

# gen_corpus.py <fuzzer> <output_dir> <corpus_dir> <dummy_output>

fuzzer = sys.argv[1]
output_dir = sys.argv[2]
corpus_dir = sys.argv[3]
dummy_output = sys.argv[4]

try:
    os.makedirs(output_dir)
except OSError:
    pass

run([ fuzzer, '-merge=1', output_dir, corpus_dir ])

# touch dummy output file
open(dummy_output, 'a').close()
