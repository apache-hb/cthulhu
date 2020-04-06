class Typedef:
    def __str__(self):
        return f'typedef {self.t} {self.name};'

    def __init__(self, name, t):
        self.name = name
        self.t = t

class Struct(dict):
    def __str__(self):
        out = 'struct {'
        for key, val in self.items():
            out += f'{val} {key};'
        out += '}'

        return out

# dont question it alright
class Tuple(list):
    def __str__(self):
        ret = 'struct {'
        for idx, each in enumerate(self):
            ret += f'{each} field{idx};'
        ret += '}'
        return ret

class Builtin(str):
    pass

class Union(dict):
    def __str__(self):
        ret = 'union {'
        for key, val in self.items():
            ret += f'{key} {val};'
        ret += '}'
        return ret

class Ptr:
    def __str__(self):
        return f'{self.t}*'

    def __init__(self, t):
        self.t = t

class Enum(dict):
    def __init__(self, backing = Builtin('u32'), *args, **kwargs):
        super(Enum, self).__init__(*args, **kwargs)
        self.backing = backing

class Variant(dict):
    def __str__(self):
        ret = 'union {'
        for key, val in self.items():
            ret += f'{val} {key};'
        ret += '}'
        
        return ret

    def __init__(self, backing = Builtin('u32'), *args, **kwargs):
        super(Variant, self).__init__(*args, **kwargs)
        self.backing = backing

class Body:
    def __init__(self, exprs):
        self.exprs = exprs

class Function:
    def __str__(self):
        ret = f'{self.ret or "void"} {self.name}('

        if self.args:
            for key, val in self.args.items():
                ret += f'{val} {key}, '
            ret = ret[:-2]

        ret += ')'

        if self.exprs:
            if not isinstance(self.exprs, list):
                ret += '{ return ' + str(self.exprs) + '; }'
            else:
                ret += '{'
                for each in self.exprs:
                    ret += str(each) + ';'
                ret += '}'
        return ret

    def __init__(self, name, ret = None, args = None, exprs = None):
        self.name = name
        self.args = args
        self.ret = ret
        self.exprs = exprs

class Import:
    def __str__(self):
        ret = f'#include "{"/".join(self.path)}.h"'

        if self.alias:
            ret += f' // aliased to {self.alias}'
    
        return ret

    def __init__(self, path, alias = None):
        self.path = path
        self.alias = alias



class Unary:
    def __init__(self, op, expr):
        self.op = op
        self.expr = expr

class Binary:
    def __str__(self):
        return f'{self.lhs} {self.op.getstr()} {self.rhs}'

    def __init__(self, op, lhs, rhs):
        self.lhs = lhs
        self.rhs = rhs
        self.op = op

class Ternary:
    def __str__(self):
        return f'{self.cond} ? {self.true} : {self.false}'

    def __init__(self, cond, true, false):
        self.cond = cond
        self.true = true
        self.false = false

class Const:
    def __str__(self):
        return str(self.val)

    def __init__(self, val):
        self.val = val

class Cast:
    def __init__(self, expr, t):
        self.expr = expr
        self.t = t


class Program:
    def __init__(self, imports = [], body = None):
        self.imports = imports
        self.body = body