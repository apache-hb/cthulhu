import sys
from glob import glob
from subprocess import check_call

testdir = sys.argv[3]
ctc = sys.argv[1]
cc = sys.argv[2]

for f in glob(f'{testdir}/*.ct'):
    check_call([ ctc, f, '--speed', '--c99' ])
    check_call([ cc, 'out.c', '-c' ])
