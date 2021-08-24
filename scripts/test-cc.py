import sys
from glob import glob
from subprocess import check_call

ctc = sys.argv[1]
cc = sys.argv[2]
testdir = sys.argv[3]

for f in glob(f'{testdir}/*.ct'):
    if 'win32' in f:
        continue
    
    print(f'compiling {f}')
    check_call([ ctc, f, '--speed', '--c99' ])
    check_call([ cc, 'out.c', '-c' ])