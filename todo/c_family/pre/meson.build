src = [
    'src/main.c',
    'src/scan.c',

    lex.process('src/pre.l'),
]

deps = [
    broker, interop,
    scan, notify, util,
    events, driver
]

libcpp = static_library('pre', src,
    build_by_default : not meson.is_subproject(),
    dependencies : deps,
    c_args : generated_args,
    include_directories : [ 'src', 'include' ],
    override_options : [ 'unity=off' ]
)
