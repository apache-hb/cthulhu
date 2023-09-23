project('cthulhu', [ 'c' ],
    license : 'LGPLv3',
    version : '0.2.4',
    meson_version : '>=0.59',
    default_options : [
        'c_std=gnu11', # gcc is picky about POSIX stdlib extensions
        'cpp_std=c++20',
        'warning_level=2', # we use unnamed structs
        'default_library=static',
        'werror=true'
    ]
)

# we use dearimgui for the gui interface (TODO: maybe qt in the future?)
if get_option('interfaces').contains('gui')
    add_languages('cpp', native : false)
endif

# we use java to generate test cases for the jvm interface
if get_option('languages').contains('jvm')
    add_languages('java', native : false)
endif

analyze = get_option('analyze')
tuning = get_option('tuning')

version = meson.project_version()
target = target_machine.system()
host = host_machine.system()
cc = meson.get_compiler('c')
buildtype = get_option('buildtype')

message('building for', target, 'on', host)

parts = version.split('.')
cwd = meson.current_source_dir()
bld = meson.current_build_dir()

config = {
    'major': parts[0].to_int(),
    'minor': parts[1].to_int(),
    'patch': parts[2].to_int(),
    'source': cwd/'interface' + ' ' + cwd/'common' + ' ' + cwd/'cthulhu' + ' ' + cwd/'driver'
}

configure_file(
    input : 'data/version.h.in',
    output : 'version.h',
    configuration : config
)

versiondir = include_directories('.')

gmp = dependency('gmp',
    not_found_message : 'no system gmp found, falling back to mini-gmp',
    fallback : [ 'mini-gmp', 'gmp' ]
)

args = []
lexargs = []
parseargs = []

fs = import('fs')

if tuning.allowed()
    args += [ '-DENABLE_TUNING=1' ]
endif

if buildtype.contains('debug')
    message('enabling `ENABLE_DEBUG` for debug build')
    args += [ '-DENABLE_DEBUG=1' ]
else
    message('disabling `ENABLE_DEBUG` for release build')
endif

# bison generates a few unused variables
if cc.has_argument('-Wno-unused-but-set-variable')
    args += [ '-Wno-unused-but-set-variable' ]
endif

if host == 'windows'
    lexargs += [ '--wincompat' ]

    if cc.get_id() != 'msvc'
        args += [ '-D__USE_MINGW_ANSI_STDIO=1' ]
    endif
else
    parseargs += [ '-Wdeprecated' ]
endif

if host == 'darwin'
    # flex generates iffy code
    args += [ '-Wno-sign-compare' ]
endif

# we dont use these features so might as well
foreach arg : [ '-fstrict-aliasing', '-fno-threadsafe-statics', '-fno-keep-static-consts', '-fmerge-all-constants' ]
    if cc.has_argument(arg)
        args += [ arg ]
    endif
endforeach

analyze_args = []

if cc.get_id() == 'msvc'
    args += [ '/Wv:18' ]

    analyze_args += [
        '/analyze',
        '/analyze:max_paths', '4096'
    ]
else
    analyze_args += [
        '-Wanalyzer-double-fclose',
        '-Wanalyzer-double-free',
        '-Wanalyzer-free-of-non-heap',
        '-Wanalyzer-mismatching-deallocation',
        '-Wanalyzer-null-dereference',
        '-Wanalyzer-use-after-free'
    ]
endif

if analyze.require(cc.has_multi_arguments(analyze_args), error_message : 'analyze requires a compiler that supports specified arguments').enabled()
    args += analyze_args
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

# common helper libraries used by compiler, tools, interfaces, and drivers
subdir('common')

# common compiler libraries used by interfaces and drivers
subdir('cthulhu')

message('enabled interfaces:', get_option('interfaces'))
message('enabled languages:', get_option('languages'))

langs = {}
interfaces = {}
plugins = {}

foreach lang : get_option('languages')
    subdir('driver'/lang)
endforeach

subdir('interface/support')

foreach interface : get_option('interfaces')
    subdir('interface'/interface)
endforeach

if not meson.is_subproject()
    subdir('tests/unit-tests')
    subdir('interface/harness')
endif

foreach lang : get_option('languages')
    subdir('tests/lang'/lang)
endforeach

if tuning.enabled()
    tune = executable('tune-map', [ 'tools/tune-map.c' ],
        dependencies : generic
    )
endif

afl = find_program('afl-fuzz', required : get_option('fuzzing'))

if afl.found()
    subdir('tests/fuzzing')

    here = meson.current_source_dir()
    foreach lang, interface : fuzzers
        input = here/'tests/lang'/lang/'fuzzing-input'
        cmd = [ afl, '-i', input, '-o', 'fuzz-out-' + lang, '-m', 'none', '-d', '--', interface.full_path(), '@@']
        run_target('fuzz-' + lang,
            command : cmd,
            depends : interface,
            env : { 'AFL_CRASH_EXITCODE': '99' }
        )
    endforeach
endif

doxygen = find_program('doxygen', required : get_option('doxygen'))

if doxygen.found()
    configure_file(
        input : 'data/Doxyfile.in',
        output : 'Doxyfile',
        configuration : config
    )

    run_target('docs', command : [ doxygen, meson.current_build_dir() / 'Doxyfile' ])
endif
