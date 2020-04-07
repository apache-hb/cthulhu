class Typedef:
    def __init__(self, name, typeof):
        self.name = name
        self.typeof = typeof

class Import:
    def __init__(self, path, alias = None):
        self.path = path
        self.alias = alias

class Program:
    def __init__(self, body, imports = []):
        self.body = body
        self.imports = imports
