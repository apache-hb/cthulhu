project('cthulhu', 'c',
    license : 'GPLv3',
    version : '0.2.0',
    meson_version : '>=0.58',
    default_options : [
        'c_std=gnu11',
        'warning_level=2', # we use unnamed structs
        'default_library=static',
        'werror=true'
    ]
)

analyze = get_option('analyze')
tuning = get_option('tuning')

version = meson.project_version()
target = target_machine.system()
host = host_machine.system()
cc = meson.get_compiler('c')
buildtype = get_option('buildtype')

parts = version.split('.')
cwd = meson.current_source_dir()
bld = meson.current_build_dir()

config = {
    'major': parts[0].to_int(),
    'minor': parts[1].to_int(),
    'patch': parts[2].to_int(),
    'source': cwd/'src' + ' ' + cwd/'include' + ' ' + cwd/'driver'
}

configure_file(
    input : 'data/version.h.in',
    output : 'version.h',
    configuration : config
)

gmp = dependency('gmp', 
    not_found_message : 'no system gmp found, falling back to mini-gmp',
    fallback : [ 'mini-gmp', 'gmp' ]
)

links = []
args = []
deps = [ gmp ]
lexargs = []
parseargs = []

if tuning
    args += [ '-DENABLE_TUNING=1' ]
endif

if buildtype.startswith('debug')
    args += [ '-DENABLE_DEBUG=1' ]
endif

if host == 'windows'
    links += [ 'setargv.obj' ]
    lexargs += [ '--wincompat' ]
    args += [ '/Wv:18' ]

    if analyze
        args += [
            '/external:W0', '/external:I' + bld, 
            '/analyze', '/analyze:external-'
        ]
    endif
else
    parseargs += [ '-Wdeprecated' ]
    deps += [ cc.find_library('m'), cc.find_library('dl') ]

    analyze_args = [ 
        '-Wanalyzer-double-fclose',
        '-Wanalyzer-double-free',
        '-Wanalyzer-free-of-non-heap',
        '-Wanalyzer-mismatching-deallocation',
        '-Wanalyzer-null-dereference',
        '-Wanalyzer-use-after-free'
    ]

    if analyze and cc.has_multi_arguments(analyze_args)
        args += analyze_args
    endif
endif

flex = find_program('flex', 'win_flex', version : '>=2.6')
bison = find_program('bison', 'win_bison', version : host == 'windows' ? '>=2.6' : '>=3.5')

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
    arguments : parseargs + [ 
        '-d', '@INPUT@', '-v',
        '--output=@OUTPUT0@', 
        '--defines=@OUTPUT1@' 
    ]
)

src = [
    # command line
    'src/driver/driver.c',
    'src/driver/cmd.c',
    'src/driver/plugins.c',
    lex.process('src/driver/cmd.l'),
    parse.process('src/driver/cmd.y'),
    
    # high level intermediate representation
    'src/hlir/sema.c',
    'src/hlir/attribs.c',
    
    'src/hlir/type.c',
    'src/hlir/hlir.c',
    'src/hlir/decl.c',
    'src/hlir/query.c',

    'src/hlir/common.c',

    # tree output
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
    'src/util/map.c',
    'src/util/vec.c',
    'src/util/optimal.c',

    'src/util/defs.c',
    'src/util/file.c',
    'src/util/library.c',

    # platform specific code
    'src/platform/target.c'.replace('target', target),
]

libgeneric = library('generic', src,
    include_directories : 'include',
    c_args : args,
    dependencies : deps
)

generic = declare_dependency(
    link_with : libgeneric,
    include_directories : 'include',
    dependencies : deps,
    compile_args : args,
    link_args : links
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

if tuning
    tune = executable('tune-map', [ 'tools/tune-map.c' ],
        dependencies : generic
    )
endif

foreach lang : get_option('languages')
    subdir('driver'/lang)
    subdir('tests'/lang)
endforeach

foreach plugin : get_option('plugins')
    subdir('plugins'/plugin)
endforeach

subdir('tests/unit-tests')

doxygen = find_program('doxygen', required : get_option('doxygen'))

if doxygen.found()
    configure_file(
        input : 'data/Doxyfile.in',
        output : 'Doxyfile',
        configuration : config
    )

    run_target('docs', command : [doxygen, meson.current_build_dir() / 'Doxyfile'])
endif
