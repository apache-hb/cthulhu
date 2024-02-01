json_api = configure_file(
    input : api_header,
    output : 'ctu_json_api.h',
    configuration : { 'MOD': 'JSON' }
)

json_include = include_directories('.', 'include')

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
    build_by_default : false,
    c_args : [ '-DCT_JSON_BUILD=1' ],
    dependencies : [ core, scan, interop, gmp, std, notify ],
    include_directories : json_include
)

json = declare_dependency(
    link_with : libjson,
    include_directories : json_include,
    dependencies : [ core, gmp ]
)

install_headers(json_api, subdir : 'json')
install_subdir('include', install_dir : 'include', strip_directory : true)
