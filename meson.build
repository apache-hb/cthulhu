project('cthulhu', 'c',
    license : 'LGPLv3',
    version : '0.2.5',
    meson_version : '>=1.3.0',
    default_options : [
        'c_std=gnu11,c11', # gcc is picky about POSIX stdlib extensions
        'cpp_std=c++11,c++14',
        'warning_level=2', # we use unnamed structs
        'default_library=static',
        'werror=true'
    ]
)

unit_tests = get_option('unit_tests').disable_auto_if(meson.is_subproject())
driver_tests = get_option('driver_tests').disable_auto_if(meson.is_subproject())

# not sure if theres a better way to do this
# https://github.com/mesonbuild/meson/issues/12570
unit_tests.enable_auto_if(add_languages('cpp', native : false, required : unit_tests.enabled()))

analyze = get_option('analyze')

version = meson.project_version()
target = target_machine.system()
host = host_machine.system()
cc = meson.get_compiler('c')
buildtype = get_option('buildtype')

is_debug = buildtype.contains('debug')
is_release = buildtype.contains('release')

parts = version.split('.')

config_cdata = configuration_data({
    'CTU_MAJOR': parts[0].to_int(),
    'CTU_MINOR': parts[1].to_int(),
    'CTU_PATCH': parts[2].to_int()
})

config_cdata.set10('CTU_DEBUG', is_debug)

config_header = configure_file(
    output : 'ctu-config.h',
    configuration : config_cdata
)

config_include = include_directories('.')

gmp = dependency('gmp',fallback : [ 'mini-gmp', 'gmp' ])

args = []
lexargs = []
parseargs = []

# bison generates a few unused variables
args += cc.get_supported_arguments('-Wno-unused-but-set-variable')

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
args += cc.get_supported_arguments(
    '-fstrict-aliasing',
    '-fno-threadsafe-statics',
    '-fno-keep-static-consts',
    '-fmerge-all-constants'
)

analyze_args = []

if cc.get_id() == 'msvc'
    args += cc.get_supported_arguments('/Wv:18')

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

if analyze.require(cc.has_multi_arguments(analyze_args), error_message : 'analyze requires a compiler that supports alanyzer arguments').enabled()
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

if unit_tests.allowed()
    subdir('tests/unit')
endif

if driver_tests.allowed()
    subdir('interface/harness')

    foreach lang : get_option('languages')
        subdir('tests/lang'/lang)
    endforeach
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

subdir('tools/docs')

summary = {
    'Build': {
        'Compiler': cc.get_id(),
        'Debug': is_debug,
        'Interfaces': ', '.join(get_option('interfaces')),
        'Languages': ', '.join(get_option('languages')),
    },
    'Testing': {
        'Unit tests': unit_tests.allowed(),
        'Driver tests': driver_tests.allowed(),
    },
    'Documentation': {
        'Doxygen': doxygen.found(),
    }
}

foreach section_title, section : summary
    summary(section, bool_yn : true, section : section_title)
endforeach

foreach lang_name, lang_config : langs
    summary(lang_config.get('summary', {}), bool_yn : true, section : lang_name)
endforeach
