src = [
    'src/driver.c',

    'src/sema/sema.c',
    'src/sema/type.c',
    'src/sema/decl.c',
    'src/sema/expr.c',

    'src/ast.c',
    'src/scan.c',
    lex.process('src/obr.l'),
    parse.process('src/obr.y')
]

deps = [
    base, broker, interop,
    scan, tree, util, notify,
    events, memory, driver
]

obr = {}

obr_lang = static_library('obr_lang', src,
    dependencies : deps,
    c_args : generated_args,
    include_directories : [ 'src', 'include' ],
    override_options : [ 'unity=off' ]
)

if default_library == 'static'
    obr_static = static_library('obr_static', 'src/main.c',
        link_with : obr_lang,
        dependencies : deps,
        c_args : user_args,
        include_directories : [ 'src', 'include', obr_lang.private_dir_include() ]
    )

    obr += {
        'dep': declare_dependency(link_with : obr_static),
        'mod': 'kOberonModule',

        'static': obr_static,
        'module': 'kOberonModule'
    }
elif default_library == 'shared'
    obr_shared = shared_module('obr_shared', 'src/main.c',
        build_by_default : not meson.is_subproject(),
        install : not meson.is_subproject(),
        link_with : obr_lang,
        dependencies : deps,
        c_args : user_args + [ '-DCTU_DRIVER_SHARED=1' ],
        include_directories : [ 'src', 'include', obr_lang.private_dir_include() ]
    )

    obr += { 'shared': obr_shared }
endif

langs += { 'obr': obr }
