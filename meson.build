project('ctu', 'c',
    default_options : [
        'c_std=gnu11', # flex under msvc requires c11
        'warning_level=2', # bison generates code that warns on gcc at level 3
        'default_library=static',
        'werror=true'
    ]
)

src = [
    'ctu/ast/scan.c',
    'ctu/ast/interop.c',
    'ctu/ast/ast.c',

    'ctu/lir/lir.c',
    'ctu/lir/sema.c',
    
    'ctu/type/type.c',

    'ctu/emit/emit.c',
    'ctu/gen/emit.c',
    'ctu/gen/print.c',

    'ctu/util/report.c',
    'ctu/util/str.c',
    'ctu/util/util.c'
]

gmp = dependency('gmp')

libgeneric = library('generic', src,
    dependencies : gmp
)

generic = declare_dependency(
    link_with : libgeneric,
    include_directories : '.',
    dependencies : gmp
)

flex = find_program('flex', version : '>=2.6')
bison = find_program('bison', version : '>=3.5')

lex = generator(flex, 
    output : [ '@BASENAME@-flex.c', '@BASENAME@-flex.h' ],
    arguments : [ 
        '--outfile=@OUTPUT0@', 
        '--header-file=@OUTPUT1@', 
        '@INPUT@' 
    ]
)

parse = generator(bison,
    output : [ '@BASENAME@-bison.c', '@BASENAME@-bison.h' ],
    arguments : [ 
        '-d', '@INPUT@', '-Wdeprecated', '-v',
        '--output=@OUTPUT0@', 
        '--defines=@OUTPUT1@' 
    ]
)

options = [ ]

foreach lang : [ 
    'c', 
    'ctu', 
    'pl0' 
    ] 
    subdir('ctu/frontend'/lang)
endforeach

driver = executable('driver', 'ctu/driver/main.c',
    dependencies : options
)
