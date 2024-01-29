runtime_api = configure_file(
    input : api_header,
    output : 'ctu_runtime_api.h',
    configuration : { 'MOD': 'RUNTIME' }
)

runtime_include = include_directories('.', 'include')

src = [
    'src/interface.c',
    'src/driver.c'
]

libruntime = library('runtime', src,
    build_by_default : false,
    install : not meson.is_subproject(),
    c_args : user_args + [ '-DCT_RUNTIME_BUILD=1' ],
    include_directories : runtime_include,
    dependencies : [
        memory, base, std,
        scan, events, tree, interop, notify,
        tree
    ]
)

runtime = declare_dependency(
    link_with : libruntime,
    include_directories : runtime_include,
    dependencies : [ base, std, notify, tree ]
)

install_headers(runtime_api)
install_subdir('include', install_dir : 'include')
