from os import path

from .codegen import CodeGen
from .filewriter import FileWriter, doc_comment
from .util import camel_case, space_name

CONFIG_ARG_NAMES = {
    'long': 'CT_ARG_LONG',
    'short': 'CT_ARG_SHORT',
    'dos': 'CT_ARG_DOS',
}

CONFIG_ROOT_NAME = 'root'
CONFIG_OPTIONS_NAME = 'options'
CONFIG_GROUP_NAME = 'group'

def _get_info_name(text: str) -> str:
    return f'k{camel_case(text)}Info'

def _get_args_name(text: str) -> str:
    return f'k{camel_case(text)}Args'

def _get_config_name(text: str) -> str:
    return f'k{camel_case(text)}Config'

def _get_choice_name(text: str) -> str:
    return f'k{camel_case(text)}Choices'

def _get_table_name(text: str) -> str:
    return f'k{camel_case(text)}Table'

def _get_case_name(enum: str, text: str) -> str:
    return f'e{enum}{camel_case(text)}'

def _emit_int_config(f: FileWriter, name: str, config: dict) -> tuple[str, str]:
    minval = config.get('min', 'INT_MIN')
    maxval = config.get('max', 'INT_MAX')
    default = config.get('default', 0)

    cfgname = _get_config_name(name)

    f.writeln(f'static const cfg_int_t {cfgname} = {{')
    f.indent()
    f.writeln(f'.initial = {default},')
    f.writeln(f'.min = {minval},')
    f.writeln(f'.max = {maxval},')
    f.dedent()
    f.writeln(f'}};')

    return ('config_int', cfgname)

def _emit_string_config(f: FileWriter, name: str, config: dict) -> tuple[str, str]:
    default = config.get('default', None)
    initial = f'"{default}"' if default is not None else 'NULL'

    cfgname = _get_config_name(name)

    f.writeln(f'static const char *const {cfgname} = {initial};')

    return ('config_string', cfgname)

def _emit_bool_config(f: FileWriter, name: str, config: dict) -> tuple[str, str]:
    cfgname = _get_config_name(name)

    default = config.get('default', False)

    f.writeln(f'static const bool {cfgname} = {"true" if default else "false"};')

    return ('config_bool', cfgname)

class ConfigCodeGen(CodeGen):
    def __init__(self, cg, config):
        super().__init__(cg.info, cg.defs, cg.source, cg.header, cg.inl, cg.sourceroot)

        self.use_setup_options = config.get('use_setup_options', True)

        # public functions for querying the config
        self.fns = FileWriter()

        # implementation of the public functions
        self.impls = FileWriter()

        # the builder function that generates the config fields
        self.builder = FileWriter()

        # the struct that holds all the config fields
        self.struct = FileWriter()

        # current stack for nested config groups
        self.stack: list[str] = []

        self.add_public_headers([ 'core/analyze.h', 'setup/setup.h' ])
        self.add_private_headers([ 'config/config.h', 'core/version_def.h' ])

    # CodeGen overrides

    def gen_private_prelude(self):
        super().gen_private_prelude()

        version = self.info['version']
        self.s.writeln(f'static const cfg_info_t kConfigInfo = {{')
        self.s.indent()
        self.s.writeln(f'.name = "{self.info["name"]}",')
        self.s.writeln(f'.brief = "{self.info["description"]}",')
        self.s.dedent()
        self.s.writeln(f'}};')
        self.s.newline()
        self.s.writeln(f'static const version_info_t kVersionInfo = {{')
        self.s.indent()
        self.s.writeln(f'.license = "{self.info["license"]}",')
        self.s.writeln(f'.author = "{self.info["author"]}",')
        self.s.writeln(f'.desc = "{self.info["description"]}",')
        self.s.writeln(f'.version = CT_NEW_VERSION({version["major"]}, {version["minor"]}, {version["patch"]}),')
        self.s.dedent()
        self.s.writeln(f'}};')

    def gen_private_epilogue(self):
        self._gen_builder_epilogue()
        self.s.newline()
        self.s.append(self.builder)
        self.s.newline()
        self.s.append(self.impls)
        return super().gen_private_epilogue()

    def gen_public_prelude(self):
        super().gen_public_prelude()
        doc_comment(self.h, f'''
            |@defgroup {self.info['id']} {self.info['name']} Configuration
            |@brief Configuration options for {self.info['name']}
            |@{{
        ''')
        self.h.newline()

    def gen_public_epilogue(self):
        self._gen_struct_epilogue()
        prefix = self.get_prefix()
        self.h.append(self.struct)
        self.h.newline()
        doc_comment(self.h, f'''
            |@brief Get the configuration options for {self.info["name"]}
            |
            |@param arena The arena to allocate the options from
            |
            |@return The configuration options
        ''')
        self.h.writeln(f'{prefix}_options_t {prefix}_get_options(arena_t *arena);')
        self.h.newline()

        self.h.append(self.fns)

        doc_comment(self.h, '@}')
        return super().gen_public_epilogue()

    # getters and simple builders

    def _should_use_setup_options(self):
        return self.use_setup_options

    def _make_struct_name(self, name: str):
        if self.stack:
            return f'{".".join(self.stack)}.{name}'
        return f'{CONFIG_ROOT_NAME}.{name}'

    def _make_parent_struct_name(self):
        if self.stack:
            return f'{".".join(self.stack)}'
        return CONFIG_ROOT_NAME

    def _get_group_name(self):
        if self.stack:
            return f'{".".join(self.stack)}'
        return CONFIG_ROOT_NAME

    def _get_struct_group(self, name: str):
        if name == CONFIG_ROOT_NAME:
            return f'{CONFIG_OPTIONS_NAME}.{CONFIG_ROOT_NAME}'
        return f'{CONFIG_OPTIONS_NAME}.{name}.{CONFIG_GROUP_NAME}'

    # writing config entries

    def _gen_marker(self, f: FileWriter, name: str):
        if self.stack:
            f.writeln(f'/* {".".join(self.stack)}.{name} */')
        else:
            f.writeln(f'/* root.{name} */')

    def _gen_config_group(self, name: str, group):
        parent = self._make_parent_struct_name()
        infoname = _get_info_name(name)
        groupname = self._get_struct_group(name)

        self.stack.append(name)

        self._gen_marker(self.s, name)
        self.s.writeln(f'static const cfg_info_t {infoname} = {{')
        self.s.indent()
        self.s.writeln(f'.name = "{space_name(name)}",')
        self.s.writeln(f'.brief = "{group["brief"]}",')
        self.s.dedent()
        self.s.writeln('};')

        self._gen_marker(self.struct, name)
        self.struct.writeln(f'struct {{')
        self.struct.indent()
        doc_comment(self.struct, f'@brief {group["brief"]}')
        self.struct.writeln(f'cfg_group_t *{CONFIG_GROUP_NAME};')
        self.struct.newline()

        self._gen_marker(self.builder, name)
        self.builder.writeln(f'{groupname} = config_group({CONFIG_OPTIONS_NAME}.{parent}, &{infoname});')

        for child, option in group['children'].items():
            self.gen_config_entry(child, option)

        self.struct.dedent()
        self.struct.writeln(f'}} {name};')

        self.stack.pop()

    def _gen_config_option(self, name: str, option):
        infoname = _get_info_name(name)
        argsname = _get_args_name(name)

        doc_comment(self.struct, f'@brief {option["brief"]}')
        self.struct.writeln(f'cfg_field_t *{name};')

        self._gen_marker(self.s, name)

        self.s.writeln(f'static const cfg_arg_t {argsname}[] = {{')
        self.s.indent()
        for arg in option['args']:
            (ty, argname), = arg.items()
            self.s.writeln(f'{CONFIG_ARG_NAMES[ty]}("{argname}"),')
        self.s.dedent()
        self.s.writeln(f'}};')
        self.s.newline()
        self.s.writeln(f'static const cfg_info_t {infoname} = {{')
        self.s.indent()
        self.s.writeln(f'.name = "{space_name(name)}",')
        self.s.writeln(f'.brief = "{option["brief"]}",')
        self.s.writeln(f'.args = CT_ARGS({argsname}),')
        self.s.dedent()
        self.s.writeln(f'}};')

        self._gen_marker(self.builder, name)
        if option.get('int', None):
            (fn, cfg) = _emit_int_config(self.s, name, option['int'])
        elif option.get('string', None):
            (fn, cfg) = _emit_string_config(self.s, name, option['string'])
        elif option.get('flags', None):
            (fn, cfg) = self._gen_flags_config(name, option['flags'])
        elif option.get('choices', None):
            (fn, cfg) = self._gen_combo_config(name, option)
        else:
            (fn, cfg) = _emit_bool_config(self.s, name, option) # assume bool if no type is specified

        fieldname = self._make_struct_name(name)
        groupname = self._get_struct_group(fieldname)
        self.builder.writeln(f'options.{fieldname} = {fn}({groupname}, &{infoname}, {cfg});')

    def _gen_combo_config(self, name: str, config: dict) -> tuple[str, str]:
        prefix = self.get_prefix()
        capital = camel_case(name)

        macro = f'{prefix.upper()}_{name.upper()}'
        enum = f'{prefix}_{name}_t'
        # emit enum values into the inl file

        self.i.writeln_define(f'ifndef {macro}')
        self.i.indent_define()
        self.i.writeln_define(f'define {macro}(id, name)')
        self.i.dedent_define()
        self.i.writeln_define('endif')
        self.i.newline()

        for choice in config['choices']:
            casename = _get_case_name(capital, choice)
            self.i.writeln(f'{macro}({casename}, "{choice}")')

        self.i.newline()
        self.i.writeln_define(f'undef {macro}')

        # build the enum from the inl file
        self.h.writeln(f'typedef enum {enum} {{')
        self.h.writeln_define(f'define {macro}(id, name) id,')
        self.h.writeln_define(f'include "{path.basename(self.inl)}"')
        self.h.indent()
        self.h.writeln(f'e{capital}Count,')
        self.h.dedent()
        self.h.writeln(f'}} {enum};')
        self.h.newline()

        # add a string function for this enum
        doc_comment(self.fns, f'''
            |@brief Get the string representation of the {name} enum
            |@pre @p value is between @c 0 and @c e{capital}Count - 1
            |@param value The enum value
            |
            |@return The string representation of the enum value
        ''')
        self.fns.writeln(f'RET_NOTNULL const char *{prefix}_{name}_string(IN_DOMAIN(<, e{capital}Count) {enum} value);')

        self.impls.writeln(f'const char *const {_get_table_name(name)}[e{capital}Count] = {{')
        self.impls.writeln_define(f'define {macro}(id, name) [id] = (name),')
        self.impls.writeln_define(f'include "{path.basename(self.inl)}"')
        self.impls.writeln(f'}};')
        self.impls.newline()

        self.impls.writeln(f'const char *{prefix}_{name}_string({enum} value) {{')
        self.impls.indent()
        self.impls.writeln(f'CTASSERT_RANGE(value, 0, e{capital}Count - 1);')
        self.impls.writeln(f'return {_get_table_name(name)}[value];')
        self.impls.dedent()
        self.impls.writeln(f'}}')

        choices = _get_choice_name(name)
        self.s.writeln(f'static const cfg_choice_t {choices}[] = {{')
        self.s.indent()
        default = None
        for choice in config['choices']:
            casename = _get_case_name(capital, choice)
            self.s.writeln(f'{{ "{choice}", {casename} }},')
            if choice == config['default']:
                default = casename

        if default is None:
            raise ValueError(f'No default choice for {name}')

        self.s.dedent()
        self.s.writeln(f'}};')

        cfgname = _get_config_name(name)
        self.s.writeln(f'static const cfg_enum_t {cfgname} = {{')
        self.s.indent()
        self.s.writeln(f'.options = {choices},')
        self.s.writeln(f'.count = CT_ARRAY_LEN({choices}),')
        self.s.writeln(f'.initial = {default},')
        self.s.dedent()
        self.s.writeln(f'}};')

        return ('config_enum', cfgname)

    def _gen_flags_config(self, name: str, config: dict) -> tuple[str, str]:
        prefix = self.get_prefix()
        counter = 0
        capital = camel_case(name)
        self.h.writeln(f'typedef enum {prefix}_{name}_t {{')
        self.h.indent()
        names = []
        self.h.writeln(f'e{capital}None = 0,')
        for flag, _ in config.items():
            casename = _get_case_name(capital, flag)
            names.append(casename)
            self.h.writeln(f'{casename} = 1 << {counter},')
            counter += 1

        self.h.writeln(f'e{capital}All = {" | ".join(names)},')
        self.h.dedent()
        self.h.writeln(f'}} {prefix}_{name}_t;')
        self.h.newline()

        default = []
        choices = _get_choice_name(name)
        self.s.writeln(f'static const cfg_choice_t {choices}[] = {{')
        self.s.indent()
        for flag, value in config.items():
            casename = _get_case_name(capital, flag)
            self.s.writeln(f'{{ "{flag}", {casename} }},')
            if value:
                default.append(casename)

        self.s.dedent()
        self.s.writeln(f'}};')

        cfgname = _get_config_name(name)
        self.s.writeln(f'static const cfg_enum_t {cfgname} = {{')
        self.s.indent()
        self.s.writeln(f'.options = {choices},')
        self.s.writeln(f'.count = CT_ARRAY_LEN({choices}),')
        self.s.writeln(f'.initial = {" | ".join(default)},')
        self.s.dedent()
        self.s.writeln(f'}};')

        return ('config_flags', cfgname)

    def gen_config_entry(self, name: str, data: dict):
        if data.get('children', None):
            self._gen_config_group(name, data)
        else:
            self._gen_config_option(name, data)

    # struct generation

    def gen_struct_prelude(self):
        prefix = self.get_prefix()
        doc_comment(self.struct, f'@brief Configuration options for {self.info["name"]}')
        self.struct.writeln(f'typedef struct {prefix}_options_t {{')
        self.struct.indent()
        doc_comment(self.struct, f'@brief The root configuration group')
        self.struct.writeln(f'cfg_group_t *root;')

    def _gen_struct_epilogue(self):
        prefix = self.get_prefix()
        if self._should_use_setup_options():
            doc_comment(self.struct, f'@brief The common setup options')
            self.struct.writeln(f'setup_options_t options;')
        self.struct.dedent()
        self.struct.writeln(f'}} {prefix}_options_t;')

    # builder generation

    def gen_builder_prelude(self):
        prefix = self.get_prefix()
        self.builder.writeln(f'{prefix}_options_t {prefix}_get_options(arena_t *arena)')
        self.builder.writeln('{')
        self.builder.indent()
        self.builder.writeln(f'{prefix}_options_t {CONFIG_OPTIONS_NAME};')
        self.builder.writeln(f'{CONFIG_OPTIONS_NAME}.{CONFIG_ROOT_NAME} = config_root(&kConfigInfo, arena);')

        if self._should_use_setup_options():
            self.builder.writeln(f'{CONFIG_OPTIONS_NAME}.setup = setup_options(kVersionInfo, {CONFIG_OPTIONS_NAME}.{CONFIG_ROOT_NAME});')

    def _gen_builder_epilogue(self):
        self.builder.writeln(f'return {CONFIG_OPTIONS_NAME};')
        self.builder.dedent()
        self.builder.writeln('}')

def gen_config(data, cg) -> ConfigCodeGen:
    commandline = data['commandline']
    gen = ConfigCodeGen(cg, commandline)

    gen.gen_public_prelude()
    gen.gen_private_prelude()

    gen.gen_struct_prelude()
    gen.gen_builder_prelude()

    options = commandline['options']

    for name, option in options.items():
        gen.gen_config_entry(name, option)

    return gen
