project('cthulhu', 'c',
    license : 'APGLv3',
    version : '0.0.2',
    meson_version : '>=0.58',
    default_options : [
        'c_std=gnu11',
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

    'ctu/ast/ops/ops.c',

    'ctu/lir/lir.c',
    'ctu/lir/sema.c',
    'ctu/lir/print.c',
    'ctu/lir/abi.c',
    
    'ctu/type/type.c',
    'ctu/type/query.c',
    'ctu/type/format.c',
    
    'ctu/gen/emit.c',
    'ctu/gen/value.c',
    'ctu/gen/operand.c',
    'ctu/gen/print.c',
    'ctu/gen/eval.c',

    'ctu/perf/perf.c',

    'ctu/util/report.c',
    'ctu/util/report-ext.c',
    'ctu/util/str.c',
    'ctu/util/util.c',
    'ctu/util/optimal.c',
    'ctu/util/panic.c',

    'ctu/driver/driver.c',
    'ctu/driver/cmd.c',
    'ctu/driver/include.c'
]

gmp = dependency('gmp')

args = []
deps = [ gmp ]

if get_option('fuzzing')
    args += [ '-DFUZZING=1' ]
endif

libgeneric = library('generic', src,
    c_args : args,
    dependencies : deps
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

options = [ generic ]

foreach gen : get_option('backends')
    subdir('ctu/backend'/gen)
endforeach

foreach lang : get_option('languages')
    subdir('ctu/frontend'/lang)
    subdir('tests'/lang)
endforeach

tune = executable('tune', [ 'tools/tune.c' ],
    dependencies : generic
)
