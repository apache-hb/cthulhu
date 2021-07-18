def gen_ct(name, num):
    with open(name, 'w') as f:
        for i in range(0, num):
            f.write(f'def func{i}(): void {{ }}\n')

def gen_c(name, num):
    with open(name, 'w') as f:
        for i in range(0, num):
            f.write(f'void func{i}() {{ }}\n')

gen_ct('big-1m.ct', 1000000)
gen_ct('big-10k.ct', 10000)

gen_c('big-1m.c', 1000000)
gen_c('big-10k.c', 10000)
