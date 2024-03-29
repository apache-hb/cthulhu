src = [
    'src/scan.c',
    'src/ast.c',
    'src/eval.c',
    'src/events.c',
    'src/emit.c',
    'src/sema.c',
    'src/sema.cpp',
    lex.process('src/ref.l'),
    parse.process('src/ref.y')
]

deps = [ interop, gmp, scan, notify, events, std, memory, io, tree ]

library_deps = [ util ]

cli_deps = [ setup, config, argparse, format ]

cpp = meson.get_compiler('cpp')

args = cpp.get_supported_arguments('/wd4201', '-Wno-format-security')

reflect_cpp_args = args + generated_args

ref = library('ref', src,
    dependencies : deps + library_deps,
    c_args : generated_args,
    cpp_args : reflect_cpp_args,
    include_directories : [ 'src', 'include' ],
    override_options : [ 'unity=off' ]
)

reflect_cli = executable('reflect_cli', 'src/main.cpp',
    dependencies : deps + cli_deps,
    link_with : ref,
    include_directories : [ 'include', ref.private_dir_include() ],
    cpp_args : reflect_cpp_args
)

reflect = generator(reflect_cli,
    output : [ '@BASENAME@.reflect.h' ],
    arguments : [ '/header:@OUTPUT0@', '@INPUT@' ]
)

reflect_include = include_directories('data/include')
