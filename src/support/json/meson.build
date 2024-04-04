json_api = configure_file(
    input : api_header,
    output : 'ctu_json_api.h',
    configuration : { 'MOD': 'JSON' }
)

json_include = include_directories('.', 'include')
json_impl_include = include_directories('src', 'src/query')
json_args = [ '-DCT_JSON_BUILD=1', generated_args ]

headers = [
    'include/json/json.h',
    'include/json/query.h',
    'include/json/json.inc',
]

src = [
    'src/json.c',
    'src/ast.c',
    'src/scan.c',
    'src/actions.c',
    lex.process('src/json.l'),
    parse.process('src/json.y'),

    'src/query/query.c',
    'src/query/query_ast.c',
    'src/query/query_scan.c',
    lex.process('src/query/query.l'),
    parse.process('src/query/query.y'),
]

if has_cpp
    headers += [ 'include/json/json.hpp' ]
    src += [ 'src/json.cpp' ]
endif

libjson = library('json', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    c_args : json_args,
    cpp_args : json_args,
    dependencies : [ core, base, scan, interop, gmp, std, notify, events, util, io ],
    include_directories : [ json_include, json_impl_include ],
    extra_files : headers,
    override_options : [ 'unity=off' ]
)

json = declare_dependency(
    link_with : libjson,
    include_directories : json_include,
    dependencies : [ core, std, gmp ]
)

install_headers(json_api, subdir : 'json')
install_subdir('include', install_dir : 'include', strip_directory : true)
