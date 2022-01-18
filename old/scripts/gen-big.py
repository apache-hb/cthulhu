def gen_any(name, num, fmt, also = None):
    with open(name, 'w') as f:
        for i in range(0, num):
            f.write(fmt.format(i))

        if also is not None:
            f.write(also())

gen_ct = lambda name, num: gen_any(name, num, 'def func{}(): void {{ }}\n')
gen_c = lambda name, num: gen_any(name, num, 'void func{}() {{ }}\n')
gen_v = lambda name, num: gen_any(name, num, 'fn func{}() {{ }}\n')
gen_rs = lambda name, num: gen_any(name, num, 'fn func{}() {{ }}\n')
gen_pl0 = lambda name, num: gen_any(name, num, 'procedure func{}; begin ! 0 end;\n', lambda: '.\n')

langs = [
    ('ct', gen_ct),
    ('v', gen_v),
    ('rs', gen_rs),
    ('c', gen_c),
    ('pl0', gen_pl0),
]

for ext, gen in langs:
    gen(f'big-10k.{ext}', 10000)
    gen(f'big-1m.{ext}', 1000000)
