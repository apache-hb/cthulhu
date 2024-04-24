ccgen_script = find_program('ccgen.py')

src = [
    'src/lex.c',
    'src/parse.c',

    'src/tmlanguage.c'
]

ccgen_deps = [
    core, base, arena, io, std, json
]

inc = include_directories('include')

libccgen = library('ccgen', src,
    c_args : user_args,
    include_directories : inc,
    dependencies : ccgen_deps
)

deps = [
    ccgen_deps,

    setup, arena, memory
]

ccgen = executable('ccgen', 'src/main.c',
    include_directories : inc,
    dependencies : deps,
    link_with : libccgen
)
