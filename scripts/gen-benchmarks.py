from sys import argv

length = int(argv[1])
path = str(argv[2])

languages = [
    ('c', 'void {name}(void) {{ }}', '\n', ''),
    ('cpp', 'void {name}(void) {{ }}', '\n', ''),
    ('pl0', 'procedure {name}; begin ! 0 end;', '\n', '.'),
    ('ctu', 'def {name} {{ }}', '\n', '')
]

for ext, template, newline, end in languages:
    parts = [template.format(name = f'_{idx}') for idx in range(length)]
    with open(f'{path}/benchmark.{ext}', 'w') as f:
        f.write(newline.join(parts))
        f.write(end)
