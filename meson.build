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
    'ctu/ast/compile.c',

    'ctu/lir/lir.c',
    'ctu/lir/sema.c',
    'ctu/lir/print.c',
    
    'ctu/type/type.c',

    'ctu/emit/emit.c',
    'ctu/gen/emit.c',
    'ctu/gen/print.c',

    'ctu/util/report.c',
    'ctu/util/str.c',
    'ctu/util/util.c',
    'ctu/util/task/task.c'
]

gmp = dependency('gmp')
threads = dependency('threads')

libgeneric = library('generic', src,
    dependencies : [ gmp, threads ]
)

generic = declare_dependency(
    link_with : libgeneric,
    include_directories : '.',
    dependencies : [ gmp, threads ]
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

foreach lang : [ 'c', 'ctu', 'pl0' ] 
    subdir('ctu/frontend'/lang)
endforeach

main = [
    'ctu/driver/main.c',
    'ctu/driver/driver.c',
    'ctu/driver/async.c'
]

driver = executable('driver', main,
    dependencies : options
)
