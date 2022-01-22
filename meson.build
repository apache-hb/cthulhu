project('cthulhu', [ 'c', 'cpp' ],
    license : 'APGLv3',
    version : '0.0.4',
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
    # command line
    'src/driver/driver.c',
    
    # data and type structures
    'src/data/type.c',
    'src/data/value.c',
    
    # high level intermediate representation
    'src/hlir/hlir.c',
    'src/hlir/debug.c',
    'src/hlir/sema.c',

    # low level intermediate representation
    'src/ssa/ssa.c',
    'src/ssa/debug.c',

    # code serialization
    'src/emit/emit.c',

    # ast data and flex/bison interop
    'src/ast/scan.c',
    'src/ast/interop.c',
    'src/ast/ast.c',
    'src/ast/compile.c',
    'src/ast/ops.c',

    # error reporting
    'src/util/report.c',
    'src/util/report-ext.c',

    # standard library
    'src/util/str.c',
    'src/util/util.c',
    'src/util/optimal.c',

    # platform glue
    'src/util/compat-platform.c'.replace('platform', target),
]

json = subproject('cjson').get_variable('json')

gmp = dependency('gmp', 
    not_found_message : 'no system gmp found, falling back to mini-gmp',
    fallback : [ 'mini-gmp', 'gmp' ]
)

args = []
deps = [ gmp, json ]

flex = find_program('flex', 'win_flex', version : '>=2.6')
bison = find_program('bison', 'win_bison', version : '>=3.5')

lexargs = []

if host_machine.system() == 'windows'
    lexargs += [ '--wincompat' ]
    args += [ '-D_CRT_SECURE_NO_WARNINGS=1', '/Wv:18' ]
endif

libgeneric = library('generic', src,
    include_directories : 'include',
    c_args : args,
    dependencies : deps
)

generic = declare_dependency(
    link_with : libgeneric,
    include_directories : 'include',
    dependencies : deps
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

foreach lang : get_option('languages')
    subdir('driver'/lang)
    subdir('tests'/lang)
endforeach

if get_option('tuning')
    tune = executable('tune', [ 'tools/tune.c' ],
        dependencies : generic
    )
endif
