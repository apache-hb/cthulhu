project('cthulhu', [ 'c', 'cpp' ],
    license : 'APGLv3',
    version : '0.1.0',
    meson_version : '>=0.58',
    default_options : [
        'c_std=gnu99',
        'warning_level=2', # bison generates code that warns on gcc at level 3
        'default_library=static',
        'werror=true' # werror has to be disabled on windows for mini-gmp to build
    ]
)

version = meson.project_version()
target = target_machine.system()
host = host_machine.system()
cc = meson.get_compiler('c')

parts = version.split('.')

config = {
    'major': parts[0].to_int(),
    'minor': parts[1].to_int(),
    'patch': parts[2].to_int(),
}

configure_file(
    input : 'data/version.h.in',
    output : 'version.h',
    configuration : config
)

src = [
    # command line
    'src/driver/driver.c',
    
    # high level intermediate representation
    'src/hlir/sema.c',
    'src/hlir/attribs.c',
    
    'src/hlir/type.c',
    'src/hlir/hlir.c',
    'src/hlir/decl.c',
    'src/hlir/query.c',

    'src/hlir/common.c',

    # tree output
    'src/emit/json.c',
    'src/emit/c89.c',
    'src/emit/wasm.c',

    # serialization
    'src/loader/load.c',
    'src/loader/save.c',
    'src/loader/common.c',
    'src/loader/hlir.c',

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
    'src/util/set.c',
    'src/util/optimal.c',

    # platform glue
    'src/util/compat-platform.c'.replace('platform', target)
]

json = subproject('cjson').get_variable('json')

gmp = dependency('gmp', 
    not_found_message : 'no system gmp found, falling back to mini-gmp',
    fallback : [ 'mini-gmp', 'gmp' ]
)

args = []
deps = [ gmp, json ]
lexargs = []

if host != 'windows'
    deps += [ cc.find_library('m') ]
endif

tuning = get_option('tuning')
if tuning
    args += [ '-DENABLE_TUNING=1' ]
endif

if host == 'windows'
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
    dependencies : deps,
    compile_args : args
)

options = { '': generic }

awsopt = subproject('aws', required : get_option('aws'))
if awsopt.found()
    miniz = subproject('miniz').get_variable('miniz')
    executable('create-lambda', [ 'tools/create-lambda.c' ],
        dependencies : [ generic, miniz ]
    )

    aws = awsopt.get_variable('aws')

    libaws = library('aws', src,
        include_directories : 'include',
        c_args : args + [ '-DHAVE_AWS=1' ],
        dependencies : deps + [ aws ]
    )

    awsdep = declare_dependency(
        link_with : libaws,
        include_directories : 'include',
        dependencies : deps + [ aws ]
    )

    options += { '-aws': awsdep }

    subdir('tests/aws')
endif

flex = find_program('flex', 'win_flex', version : '>=2.6')
bison = find_program('bison', 'win_bison', version : '>=3.5')

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

if tuning
    tune = executable('tune-map', [ 'tools/tune-map.c' ],
        dependencies : generic
    )
endif

foreach lang : get_option('languages')
    subdir('driver'/lang)
    subdir('tests'/lang)
endforeach
