# generate an obscene amount of test cases
import random
import sys
import argparse
import pathlib
import string

parser = argparse.ArgumentParser(description = 'generate tests')
parser.add_argument('--total', type=int, help='total number of tests to generate')
parser.add_argument('--out', type=str, default='tests', help='output directory for tests')
parser.add_argument('--fail', default=False, help='generate tests that should fail')
parser.add_argument('--seed', type=int, default=None, help='rng seed for reproducable output')

result = parser.parse_args(sys.argv[1:])
seed = result.seed if result.seed is not None else random.randint(0, 1000)
path = result.out
total = result.total
print(f'generating `{total}` tests with seed `{seed}` into directory `{path}`')

random.seed(seed)

pathlib.Path(path).mkdir(parents=True, exist_ok=True)

class Scope:
    def __init__(self, name = None, parent = None):
        self.items = []
        self.parent = parent
        self.data = f'// {name}\n'
        self.depth = 0

    def get_var(self):
        try:
            if random.getrandbits(1) == 1:
                return str(random.randrange(999999))
            elif self.parent is not None:
                return self.parent.get_var()
            return random.choice(self.items)

        except (IndexError, RecursionError):
            return random.choice([ 'true', 'false'])

    def root(self):
        if self.parent is not None:
            return self.parent.root()
        
        return self

    def indent(self):
        self.root().depth += 1

    def dedent(self):
        self.root().depth -= 1

    def write(self, text):
        self.root().data += ("  " * self.root().depth) + text + '\n'

    def text(self):
        return self.root().data

    def gen_if(self, depth):
        self.write(f'if {self.gen_expr()} {{')
        self.indent()
        Scope(None, self).gen_stmt(depth)
        self.dedent()
        self.write('}')

    def binop(self):
        return random.choice([ 
            '+', '-', '/', '*', '%', 
            '>', '>=', '<', '<=' 
        ])

    def unary(self):
        return random.choice([
            '-', '+', '&', '*'
        ])

    def gen_expr(self, depth = 3):
        choices = [ 'symbol' ]
        if depth > 0:
            choices += [ 'binary', 'unary' ]
        it = random.choice(choices)

        if it == 'binary':
            return f'{self.gen_expr(depth - 1)} {self.binop()} {self.gen_expr(depth - 1)}'
        elif it == 'unary':
            return f'{self.unary()}{self.gen_expr(depth - 1)}'
        else:
            return self.get_var()

    def get_export_def(self):
        return random.choice([ 'export def', 'def' ])

    def make_ident(self, num = 10):
        ident = ''.join(random.choice(string.ascii_letters) for _ in range(10))
        self.items += ident
        return ident

    def get_type(self):
        it = random.choice([ 
            'void',
            'char', 'int', 'short', 'long', 'isize', 'intptr', 'intmax',
            'uchar', 'uint', 'ushort', 'ulong', 'usize', 'uintptr', 'uintmax'
        ])

        while random.getrandbits(1):
            it = '*' + it

        return it

    def gen_return(self):
        if random.getrandbits(1):
            self.write(f'return {self.gen_expr()};')
        else:
            self.write('return;')

    def gen_stmts(self, depth):
        self.write('{')
        self.indent()
        body = Scope(None, self)
        for _ in range(random.randint(0, 20)):
            body.gen_stmt(depth)
        self.dedent()
        self.write('}')

    def gen_final(self):
        self.write(f'final {self.make_ident()} = {self.gen_expr()};')

    def gen_stmt(self, depth = 5):
        choices = [ 'return', 'final' ]
        if depth > 0:
            choices += [ 'if', 'compound' ]

        it = random.choice(choices)
        if it == 'if':
            self.gen_if(depth - 1)
        elif it == 'return':
            self.gen_return()
        elif it == 'final':
            self.gen_final()
        elif it == 'compound':
            self.gen_stmts(depth - 1)

    def build(self):
        for _ in range(random.randint(0, 50)):
            name = self.make_ident()
            self.write(f'{self.get_export_def()} {name}(): {self.get_type()} {{')
            self.indent()
            Scope(None, self).gen_stmt()
            self.dedent()
            self.write('}')

for i in range(total):
    name = f'test-{i}.ct'
    it = Scope(name)
    it.build()
    with open(f'{path}/{name}', 'w') as f:
        f.write(it.text())
