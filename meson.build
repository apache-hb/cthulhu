project('cthulhu', 'c',
    license : 'APGLv3',
    version : '0.0.3',
    meson_version : '>=0.58',
    default_options : [
        'c_std=gnu11',
        'warning_level=2', # bison generates code that warns on gcc at level 3
        'default_library=static'#,
        #'werror=true' # werror has to be disabled on windows for mini-gmp to build
    ]
)

target = target_machine.system()

src = [
    'cthulhu/ast/scan.c',
    'cthulhu/ast/interop.c',
    'cthulhu/ast/ast.c',
    'cthulhu/ast/compile.c',
    'cthulhu/ast/ops.c',

    'cthulhu/lir/lir.c',
    'cthulhu/lir/sema.c',
    
    'cthulhu/type/type.c',
    'cthulhu/type/query.c',
    'cthulhu/type/format.c',
    
    'cthulhu/gen/emit.c',
    'cthulhu/gen/value.c',
    'cthulhu/gen/operand.c',
    'cthulhu/gen/print.c',
    'cthulhu/gen/eval.c',

    'cthulhu/perf/perf.c',

    'cthulhu/util/report.c',
    'cthulhu/util/report-ext.c',
    'cthulhu/util/str.c',
    'cthulhu/util/util.c',
    'cthulhu/util/optimal.c',
    'cthulhu/util/panic.c',
    'cthulhu/util/compat-platform.c'.replace('platform', target),

    'cthulhu/driver/driver.c',
    'cthulhu/driver/cmd.c'
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
    subdir('cthulhu/backend'/gen)
endforeach

foreach lang : get_option('languages')
    subdir('cthulhu/frontend'/lang)
    subdir('tests'/lang)
endforeach

if get_option('tuning')
    tune = executable('tune', [ 'tools/tune.c' ],
        dependencies : generic
    )
endif
