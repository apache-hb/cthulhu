src = [
    'src/scan.c',
    'src/ast.c',
    lex.process('src/sql.l'),
    parse.process('src/sql.y'),
]

deps = [
    broker, interop, scan,
    notify, tree, util, events,
    config, memory, driver, std
]

sql = { }

sql_lang = static_library('sql_lang', src,
    dependencies : deps,
    c_args : generated_args,
    include_directories : [ 'src', 'include' ],
    override_options : [ 'unity=off' ],
    kwargs : libkwargs
)

if default_library == 'static'
    sql_static = static_library('sql_static', 'src/main.c',
        link_with : sql_lang,
        dependencies : deps,
        c_args : user_args,
        include_directories : [ 'src', 'include', sql_lang.private_dir_include() ],
        kwargs : libkwargs
    )

    sql += {
        'dep': declare_dependency(link_with : sql_static),
        'mod': 'kSqlModule',

        'static': sql_static,
        'module': 'kSqlModule'
    }
elif default_library == 'shared'
    sql_shared = shared_module('pl0_shared', 'src/main.c',
        link_with : sql_lang,
        dependencies : deps,
        c_args : user_args + [ '-DCTU_DRIVER_SHARED=1' ],
        include_directories : [ 'src', 'include', sql_lang.private_dir_include() ],
        kwargs : libkwargs
    )

    sql += { 'shared': sql_shared }
endif

langs += { 'sql': sql }
