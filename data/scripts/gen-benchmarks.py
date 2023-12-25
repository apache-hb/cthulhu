from sys import argv
from os import mkdir

length = int(argv[1])
path = str(argv[2])

try:
    mkdir(path)
except:
    pass

languages = [
    # (ext, template,                                                         newline, begin,           end)
    ('c',   'void {name}(int x) {{ int a = x; }}',                               '\n', '',              ''),
    ('cpp', 'void {name}(int x) {{ auto a = x; }}',                              '\n', '',              ''),
    ('pl0', 'procedure {name}; begin ! 0 end;',                                  '\n', 'module bench;', '.'),
    ('ctu', 'def {name}(x: int) {{ var a = x; }}',                               '\n', 'module bench;', ''),
    ('obr', 'PROCEDURE {name}(x: INTEGER); VAR a : INTEGER; BEGIN a := x END;', '\n', 'MODULE Bench;', 'END Bench.'),
]

for ext, template, newline, begin, end in languages:
    parts = [template.format(name = f'_{idx}') for idx in range(length)]
    with open(f'{path}/benchmark.{ext}', 'w') as f:
        f.write(begin + '\n')
        f.write(newline.join(parts))
        f.write('\n' + end)
