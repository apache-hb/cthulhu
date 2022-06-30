from sys import argv
from glob import iglob
from subprocess import run, DEVNULL

cc = argv[1]
lang = argv[2]
tests = argv[3]

for test in iglob(tests, recursive = True):
    print(f'compiling {test}')
    try:
        run([lang, test], stderr = DEVNULL, check = True)
    except:
        continue

    try:
        run([cc, 'out.c'], stdout = DEVNULL, check = True)
    except:
        print(f'{test} generated invalid C')
