def gen_ct(name, num):
    with open(name, 'w') as f:       
        for i in range(0, num):       
            f.write(f'def func{i}(): void {{ }}\n')

gen_ct('big-1m.ct', 1000000)
gen_ct('big-10k.ct', 10000)
