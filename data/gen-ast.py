from xml.dom.minidom import parse
from dataclasses import dataclass
import os
import argparse

parser = argparse.ArgumentParser(
    prog='gen-ast',
    description='Generate AST source files from XML description.'
)

parser.add_argument('input', help='XML file describing the AST')
parser.add_argument('header', help='Output directory for generated header files')
parser.add_argument('source', help='Output directory for generated source files')
parser.add_argument('--include', help='Include directory that source files will use')
parser.add_argument('--name', help='Name of the AST (default: ast)', default='ast')

def get_text(node):
    return ''.join([n.data for n in node.childNodes if n.nodeType == n.TEXT_NODE])

@dataclass
class Source:
    name: str
    ast_type_defs: str = ''

    ast_func_decls: str = ''
    ast_func_defs: str = ''

class Field:
    name: str
    desc: str
    parent: 'Node'
    typeof: str

    def __init__(self, doc, parent, prefix):
        self.name = get_text(doc)
        self.desc = doc.getAttribute('desc') or self.name
        self.parent = parent

        kind = doc.getAttribute('type').replace('$$', prefix)
        self.typeof = 'char *' if kind == '$string' else kind + ' '

    def make_field(self):
        return f'{self.typeof}{self.name}'
    
    def make_arg(self):
        return f'{self.typeof}{self.name}'

class Node:
    parent: 'Node'
    fields: list[Field]

def gen_enum(enum) -> Source:
    desc = enum.getAttribute('desc')
    name = enum.getAttribute('name')
    prefix = enum.getAttribute('prefix')

    src = Source(name)

    src.ast_type_defs = f'// {desc}\n' + f'typedef enum {name}_t {{\n'
    
    src.ast_func_decls += f'const char *{name}_to_string({name}_t value);'
    src.ast_func_defs += f'const char *{name}_to_string({name}_t value) {{\n' + f'\tswitch (value) {{\n'    

    for field in enum.getElementsByTagName('value'):
        field_desc = field.getAttribute('desc')
        field_name = get_text(field)

        src.ast_type_defs += f'\t{prefix}{field_desc}, // {field_desc}\n'

        src.ast_func_defs += f'\tcase {prefix}{field_desc}: return "{field_name}";\n'

    src.ast_type_defs += f'\t{prefix}Total\n'

    src.ast_type_defs += '} ' + name + '_t;\n'
    src.ast_func_defs += '\tdefault: return "Unknown";\n\t}\n' + '}\n'

    return src

def gen_flags(flags) -> Source:
    desc = flags.getAttribute('desc')
    name = flags.getAttribute('name')
    prefix = flags.getAttribute('prefix')

    src = Source(name)

    fields = flags.getElementsByTagName('bitflag')

    src.ast_type_defs = f'// {desc}\n' + f'typedef enum {name}_t {{\n'
    src.ast_func_decls += f'const char *{name}_to_string({name}_t value);'
    src.ast_func_defs += f'const char *{name}_to_string({name}_t value) {{\n' + f'\tvector_t *vec = vector_new({len(fields)});\n'

    for field in fields:
        field_desc = field.getAttribute('desc')
        field_name = get_text(field)
        bit = field.getAttribute('bit')
        
        src.ast_type_defs += f'    {prefix}{field_desc} = (1 << {bit}), // {field_desc}\n'
        src.ast_func_defs += f'\tif (value & {prefix}{field_desc}) {{ vector_push(&vec, (char*)"{field_name}"); }}\n'

    src.ast_func_defs += f'\treturn str_join(" | ", vec);\n' + '}\n'
    src.ast_type_defs += '} ' + name + '_t;\n'

    return src

def snake_case(name):
    result = ''
    for i, c in enumerate(name):
        if i > 0 and c.isupper():
            result += '_'
        elif c.isspace():
            result += '_'
        else:
            result += c.lower()
    return result

def camel_case(name):
    result = ''
    space = False
    for c in name.strip('_'):
        if c.isspace() or c == '_':
            space = True
            continue
        
        if space:
            result += c.upper()
            space = False
        else:
            result += c.lower()

    return result

class NodeGen:
    prefix = ''
    nodes = {}
    root = ''

    def __init__(self, prefix, root):
        self.prefix = prefix
        self.root = self.make_name(root)

    def make_name(self, name):
        return self.prefix + '_' + name

    def add_node(self, node):
        desc = node.getAttribute('desc')
        name = self.make_name(node.getAttribute('name'))
        fields = [Field(field, node, self.prefix) for field in node.getElementsByTagName('field')]

        self.nodes[name] = { 
            'desc': desc, 
            'fields': fields, 
            'parent': node.getAttribute('extends'),
            'abstract': node.getAttribute('abstract') == 'true',
        }

    def calc_depth(self, node):
        if not node['parent']:
            return 0

        name = self.make_name(node['parent'])
        return 1 + self.calc_depth(self.nodes[name])

    def gen_node_tag(self):
        src = Source(self.prefix + '_tag_t')

        src.ast_func_decls += f'const char *{self.prefix}_tag_to_string({self.prefix}_tag_t value);'
        src.ast_func_defs += f'const char *{self.prefix}_tag_to_string({self.prefix}_tag_t value) {{\n' + f'\tswitch (value) {{\n'

        src.ast_type_defs = f'typedef enum {self.prefix}_tag_t {{\n'
        for name, node in self.nodes.items():
            if node['abstract']:
                continue

            src.ast_type_defs += f'\teTag{node["desc"]},\n'
            src.ast_func_defs += f'\tcase eTag{node["desc"]}: return "{name.replace("_", "-")}";\n'

        src.ast_func_defs += '\tdefault: return "Unknown";\n\t}\n' + '}\n'
        src.ast_type_defs += f'\teTagTotal\n}} {self.prefix}_tag_t;\n'

        return src

    def collect_fields(self, node):
        fields = []
        if node['parent']:
            name = self.make_name(node['parent'])
            fields += self.collect_fields(self.nodes[name])
        
        result = node['fields'] 
        return result + fields

    def gen_init(self, node, args, tag, depth = 2):
        has_field = lambda name: any(map(lambda x: x.name == name and x.parent == node, args))
        result = ' {\n'
        if node['parent']:
            parent = self.nodes[self.make_name(node['parent'])]
            result += '\t' * depth + '.base ='
            result += self.gen_init(parent, parent['fields'], tag, depth + 1)
        else:
            result += ('\t' * depth) + f'.tag = {tag},\n'

        for arg in args:
            if not has_field(arg.name):
                continue

            result += ('\t' * depth) + f'.{arg["name"]} = {arg["name"]},\n'
        
        return result + ('\t' *( depth - 1)) + '}' + (',' if depth > 2 else ';') + '\n'

    def gen_data(self, structs):
        header = Source(self.prefix)

        header.ast_func_decls += f'#define {self.prefix.upper()}_IS(node, id) (({self.root}_t*)(node)->tag == (id))'

        result = [ header ]

        for it in structs:
            name = self.prefix + '_' + snake_case(it.getAttribute('name'))
            header.ast_type_defs += f'typedef struct {name}_t {name}_t;\n'

            fields = [Field(field, it, self.prefix) for field in it.getElementsByTagName('field')]

            src = Source(name)

            src.ast_type_defs += f'typedef struct {name}_t {{\n'
            for field in fields:
                src.ast_type_defs += f'\t{field.make_field()};\n'
            src.ast_type_defs += f'}} {name}_t;\n'

            result.append(src)

        header.ast_type_defs += '\n'

        for name, node in self.nodes.items():
            depth = self.calc_depth(node)
            id = snake_case(name)
            header.ast_type_defs += ('\t' * depth) + f'typedef struct {id}_t {id}_t;\n'

        for name, node in self.nodes.items():
            src = Source(name)

            args = [field for field in self.collect_fields(node) if field.name != 'tag']
            argstr = ', '.join([field.make_arg() for field in args]) if args else 'void'

            id = snake_case(name)

            if name != self.root:
                if not node['abstract']:
                    src.ast_func_decls += f'{id}_t *{id}_new({argstr});'

                    src.ast_func_defs += f'{id}_t *{id}_new({argstr}) {{\n'
                    src.ast_func_defs += f'\t{id}_t self = '
                    src.ast_func_defs += self.gen_init(node, args, f'eTag{node["desc"]}')

                    src.ast_func_defs += f'\treturn BOX(self);\n}}\n'

            src.ast_type_defs += f'// {node["desc"]}\n' + f'typedef struct {id}_t {{\n'
            if node['parent']:
                src.ast_type_defs += f'\t{self.prefix}_{node["parent"]}_t base;\n'

            if name == self.root:
                src.ast_type_defs += f'\t{self.prefix}_tag_t tag;\n'

            for field in node['fields']:
                src.ast_type_defs += f'\t{field.make_field()}; // {field.desc}\n'

            src.ast_type_defs += '} ' + id + '_t;\n'

            result.append(src)

        union = Source(self.prefix + '_t')
        union.ast_type_defs += f'typedef union {self.prefix}_t {{\n'
        
        for name, node in self.nodes.items():
            id = snake_case(name)
            field = camel_case(name.strip(self.prefix))
            union.ast_type_defs += f'\t{id}_t {field};\n'
        
        union.ast_type_defs += '} ' + self.prefix + '_t;\n'

        result.append(union)

        return result

def gen_ast(header_dir, source_dir, doc, name, inc):
    root = doc.documentElement
    if root.tagName != 'root':
        raise Exception('Invalid XML root element: ' + root.tagName)

    prefix = root.getAttribute('prefix')

    ast_header = '#pragma once\n'
    ast_header += '#include "base/macros.h"\n'

    ast_source = f'#include "{name}-ast.h"\n'
    ast_source += f'#include "base/util.h"\n'
    ast_source += f'#include "std/vector.h"\n'
    ast_source += f'#include "std/str.h"\n'

    enums = []
    flags = []
    nodes = []

    for include in root.getElementsByTagName('includes'):
        for file in include.getElementsByTagName('system'):
            ast_header += f'\n#include <{get_text(file)}>'
        for file in include.getElementsByTagName('local'):
            ast_header += f'\n#include "{get_text(file)}"'

    ast_header += '\n\nBEGIN_API'

    for child in root.getElementsByTagName('enums'):
        for enum in child.getElementsByTagName('enum'):
            it = gen_enum(enum)

            enums.append(it)

    for child in root.getElementsByTagName('bitflags'):
        for bitflags in child.getElementsByTagName('flags'):
            it = gen_flags(bitflags)

            flags.append(it)

    structs = []

    for child in root.getElementsByTagName('structs'):
        structs += child.getElementsByTagName('struct')

    for child in root.getElementsByTagName('nodes'):
        nodegen = NodeGen(prefix, child.getAttribute('root'))
        for node in child.getElementsByTagName('node'):
            nodegen.add_node(node)

        for node in nodegen.gen_data(structs):
            nodes.append(node)

        enums.append(nodegen.gen_node_tag())

    ast_header += '\n\n// Enums\n'

    for enum in enums:
        ast_header += '\n' + enum.ast_type_defs

    for enum in enums:
        ast_header += '\n' + enum.ast_func_decls
        ast_source += '\n' + enum.ast_func_defs
    
    ast_header += '\n// Bitflags\n'

    for flag in flags:
        ast_header += '\n' + flag.ast_type_defs

    for flag in flags:
        ast_header += '\n' + flag.ast_func_decls
        ast_source += '\n' + flag.ast_func_defs

    ast_header += '\n// Nodes\n'

    for node in nodes:
        ast_header += '\n' + node.ast_type_defs

    for node in nodes:
        ast_header += '\n' + node.ast_func_decls
        ast_source += '\n' + node.ast_func_defs

    ast_header += '\n\nEND_API'

    with open(os.path.join(header_dir, name + '-ast.h'), 'w') as f:
        f.write(ast_header + '\n')

    with open(os.path.join(source_dir, name + '-ast.c'), 'w') as f:
        f.write(ast_source + '\n')

def main():
    args = parser.parse_args()

    with open(args.input, 'r') as f:
        doc = parse(f)
        gen_ast(args.header, args.source, doc, args.name, args.include)

if __name__ == '__main__':
    main()
