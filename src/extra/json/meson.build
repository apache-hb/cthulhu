json_api = configure_file(
    input : api_header,
    output : 'ctu_json_api.h',
    configuration : { 'MOD': 'JSON' }
)

json_include = include_directories('.', 'include')
json_impl_include = include_directories('src')

src = [
    'src/json.c',
    'src/ast.c',
    'src/scan.c',
    lex.process('src/json.l'),
    parse.process('src/json.y')
]

if has_cpp
    src += [ 'src/json.cpp' ]
endif

libjson = library('json', src,
    build_by_default : not meson.is_subproject(),
    c_args : [ '-DCT_JSON_BUILD=1', generated_args ],
    dependencies : [ core, base, scan, interop, gmp, std, notify, events, util ],
    include_directories : [ json_include, json_impl_include ],
    override_options : [ 'unity=off' ]
)

json = declare_dependency(
    link_with : libjson,
    include_directories : json_include,
    dependencies : [ core, gmp ]
)

install_headers(json_api, subdir : 'json')
install_subdir('include', install_dir : 'include', strip_directory : true)
