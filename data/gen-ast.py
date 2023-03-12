from xml.dom.minidom import parse
import os
import argparse

parser = argparse.ArgumentParser(
    prog='gen-ast',
    description='Generate AST source files from XML description.'
)

parser.add_argument('input', help='XML file describing the AST')
parser.add_argument('output', help='Output directory for generated files')
parser.add_argument('--name', help='Name of the AST (default: ast)', default='ast')

def get_text(node):
    return ''.join([n.data for n in node.childNodes if n.nodeType == n.TEXT_NODE])

def gen_enum(enum):
    desc = enum.getAttribute('desc')
    name = enum.getAttribute('name')
    prefix = enum.getAttribute('prefix')

    result = f'// {desc}\n' + f'typedef enum {name} {{\n'

    for field in enum.getElementsByTagName('value'):
        field_desc = field.getAttribute('desc')
        field_name = get_text(field)
        result += f'    {prefix}{field_desc}, // {field_desc}\n'

    result += '} ' + name + ';\n'

    return { 'name': name, 'result': result }

def gen_flags(flags):
    desc = flags.getAttribute('desc')
    name = flags.getAttribute('name')
    prefix = flags.getAttribute('prefix')

    result = f'// {desc}\n' + f'typedef enum {name} {{\n'

    for field in flags.getElementsByTagName('bitflag'):
        field_desc = field.getAttribute('desc')
        field_name = get_text(field)
        bit = field.getAttribute('bit')
        result += f'    {prefix}{field_desc} = (1 << {bit}), // {field_desc}\n'
    
    result += '} ' + name + ';\n'

    return { 'name': name, 'result': result }

def gen_node(prefix, node):
    desc = node.getAttribute('desc')
    name = prefix + node.getAttribute('name')

    result = f'// {desc}\n' + f'typedef struct {name} {{\n'

    for field in node.getElementsByTagName('field'):
        field_desc = field.getAttribute('desc')
        field_name = get_text(field)
        field_type = field.getAttribute('type')
        result += f'    {field_type} {field_name}; // {field_desc}\n'

    result += '} ' + name + ';\n'

    return { 'name': name, 'result': result }

def gen_ast(dir, doc, name):
    root = doc.documentElement
    if root.tagName != 'root':
        raise Exception('Invalid XML root element: ' + root.tagName)

    header = '#pragma once\n'
    source = f'#include "{name}.h"'

    enums = []
    flags = []
    nodes = []

    for include in root.getElementsByTagName('includes'):
        for file in include.getElementsByTagName('system'):
            header += f'\n#include <{get_text(file)}>'
        for file in include.getElementsByTagName('local'):
            header += f'\n#include "{get_text(file)}"'

    for child in root.getElementsByTagName('enums'):
        for enum in child.getElementsByTagName('enum'):
            it = gen_enum(enum)

            enums.append(it)

    for child in root.getElementsByTagName('bitflags'):
        for bitflags in child.getElementsByTagName('flags'):
            it = gen_flags(bitflags)

            flags.append(it)

    for child in root.getElementsByTagName('nodes'):
        prefix = child.getAttribute('prefix')
        for node in child.getElementsByTagName('node'):
            it = gen_node(prefix, node)

            nodes.append(it)

    header += '\n\n// Enums\n'

    for enum in enums:
        header += '\n' + enum['result']
        print(enum['result'])

    header += '\n// Bitflags\n'

    for flags in flags:
        header += '\n' + flags['result']
        print(flags['result'])

    header += '\n// Nodes\n'

    for node in nodes:
        header += '\n' + node['result']
        print(node['result'])

    with open(os.path.join(dir, name + '.h'), 'w') as f:
        f.write(header)

    with open(os.path.join(dir, name + '.c'), 'w') as f:
        f.write(source)

def main():
    args = parser.parse_args()
    if not os.path.exists(args.output):
        os.makedirs(args.output)
    
    with open(args.input, 'r') as f:
        doc = parse(f)
        gen_ast(args.output, doc, args.name)

if __name__ == '__main__':
    main()
