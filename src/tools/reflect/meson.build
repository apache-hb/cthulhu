src = [
    'src/scan.c',
    'src/ast.c',
    'src/sema.cpp',
    lex.process('src/ref.l'),
    parse.process('src/ref.y')
]

deps = [ interop, gmp, scan, notify, events, std, memory, io, tree ]

library_deps = [ util ]

cli_deps = [ setup, config, io, argparse, format ]

args = cpp.get_supported_arguments('/wd4201', '-Wno-format-security')

ref = library('ref', src,
    dependencies : deps + library_deps,
    c_args : generated_args,
    cpp_args : user_args + args,
    include_directories : [ 'src', 'include' ]
)

src = [ 'src/main.cpp' ]

reflect_cli = executable('reflect_cli', src,
    dependencies : deps + cli_deps,
    link_with : ref,
    include_directories : [ 'include', ref.private_dir_include() ],
    cpp_args : user_args + args
)

reflect = generator(reflect_cli,
    output : [ '@BASENAME@.reflect.h', '@BASENAME@.reflect.cpp' ],
    arguments : [ '/header:@OUTPUT0@', '/source:@OUTPUT1@', '@INPUT@' ]
)

reflect_include = include_directories('data/include')
