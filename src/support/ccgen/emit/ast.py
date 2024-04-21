from .codegen import CodeGen
from .filewriter import FileWriter, doc_comment
from .util import camel_case

from os import path

class AstCodeGen(CodeGen):
    def __init__(self, cg):
        super().__init__(cg.info, cg.defs, cg.source, cg.header, cg.inl, cg.sourceroot)

        # enum values for each node type
        self.enum = FileWriter()

        # struct definitions for each node type
        self.structs = FileWriter()

        # factory functions for each type of node
        self.ctors = FileWriter()

        # type information for each node, used for serialization
        # and stringification
        self.typeinfo = FileWriter()

        self.add_public_headers([ 'core/where.h' ])

    # CodeGen overrides

    def gen_public_prelude(self):
        super().gen_public_prelude()
        self.h.writeln('typedef struct scan_t scan_t;')
        self.h.newline()

        prefix = self.get_prefix()
        self.h.writeln(f'typedef enum {prefix}_kind_t {{')
        self.h.writeln_define(f'define {prefix.upper()}_NODE(id, key) id,')
        self.h.writeln_define(f'include "{path.basename(self.inl)}"')
        self.h.writeln(f'}} {prefix}_kind_t;')
        self.h.newline()
        self._gen_enum_prelude()

    def gen_public_epilogue(self):
        self._gen_enum_epilogue()
        self.i.append(self.enum)

        self.h.append(self.structs)
        self.h.append(self.ctors)
        return super().gen_public_epilogue()

    # private methods

    def _get_case_name(self, name: str) -> str:
        return f'eAst{camel_case(name)}'

    def _gen_enum_prelude(self):
        prefix = self.get_prefix().upper()
        self.enum.writeln_define(f'ifndef {prefix}_NODE')
        self.enum.indent_define()
        self.enum.writeln_define(f'define {prefix}_NODE(id, key)')
        self.enum.dedent_define()
        self.enum.writeln_define(f'endif')
        self.enum.newline()

    def _gen_enum_epilogue(self):
        prefix = self.get_prefix().upper()
        self.enum.newline()
        self.enum.writeln_define(f'undef {prefix}_NODE')

    # AstCodeGen methods

    def _gen_enum_case(self, node):
        prefix = self.get_prefix()
        key = node.get('key', node['name'])
        self.enum.writeln(f'{prefix.upper()}_NODE(eAst{self._get_case_name(node["name"])}, "{key}")')

    def _gen_field(self, field):
        ty = field['type']
        if ty == 'mpz':
            self.structs.writeln(f'mpz_t {field["name"]};')
        elif ty == 'string':
            self.structs.writeln(f'text_view_t {field["name"]};')

    def _gen_struct(self, node):
        prefix = self.get_prefix()
        name = f'{prefix}_{node["name"]}_t'

        self.h.writeln(f'typedef struct {name} {name};')

        self.structs.writeln(f'/* {self._get_case_name(node["name"])} */')
        self.structs.writeln(f'typedef struct {name} {{')
        self.structs.indent()
        for field in node['fields']:
            self._gen_field(field)
        self.structs.dedent()
        self.structs.writeln(f'}} {name};')

    def _gen_ctor(self, node):
        prefix = self.get_prefix()
        name = f'{prefix}_{node["name"]}_t'

        params = [ 'scan_t *scan', 'where_t where' ]
        for field in node['fields']:
            params.append(f'{field["type"]} {field["name"]}')

        self.ctors.writeln(f'{name} *{prefix}_{node["name"]}({", ".join(params)});')

    def gen_node(self, node):
        self._gen_enum_case(node)
        self._gen_struct(node)
        self._gen_ctor(node)

def gen_ast(data, cg) -> AstCodeGen:
    nodes = data['nodes']
    gen = AstCodeGen(cg)

    gen.gen_public_prelude()
    gen.gen_private_prelude()

    for node in nodes:
        gen.gen_node(node)

    return gen
