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
