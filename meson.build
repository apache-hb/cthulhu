project('cthulhu', 'c',
    license : 'APGLv3',
    version : '0.0.3',
    meson_version : '>=0.58',
    default_options : [
        'c_std=gnu11',
        'warning_level=2', # bison generates code that warns on gcc at level 3
        'default_library=static'#,
        #'werror=true'
    ]
)

target = target_machine.system()

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
    'ctu/util/compat-platform.c'.replace('platform', target),

    'ctu/driver/driver.c',
    'ctu/driver/cmd.c',
    'ctu/driver/include.c'
]

gmp = dependency('gmp', 
    not_found_message : 'no system gmp found, falling back to mini-gmp',
    fallback : [ 'mini-gmp', 'gmp' ]
)

args = []
deps = [ gmp ]

flex = find_program('flex', 'win_flex', version : '>=2.6')
bison = find_program('bison', 'win_bison', version : '>=3.5')

lexargs = []

if host_machine.system() == 'windows'
    lexargs += [ '--wincompat' ]
    args += [ '-D_CRT_SECURE_NO_WARNINGS=1', '/Wv:18' ]
endif

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

lex = generator(flex, 
    output : [ '@BASENAME@-flex.c', '@BASENAME@-flex.h' ],
    arguments : lexargs + [
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

if get_option('tuning')
    tune = executable('tune', [ 'tools/tune.c' ],
        dependencies : generic
    )
endif
