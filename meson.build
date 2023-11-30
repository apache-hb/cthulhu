project('cthulhu', 'c',
    license : 'LGPLv3',
    version : '0.2.5',
    meson_version : '>=1.3.0',
    default_options : [
        # gcc is picky about POSIX stdlib extensions
        'c_std=gnu11,c11',

        # msvc doesnt support c++11 so specify c++14 first
        # we only need c++11
        'cpp_std=c++14,c++11',

        # we use unnamed structs
        'warning_level=2',
        'werror=true',

        # currently meson doesnt forward `declare_dependency:sources` between dependencies
        # which means we cant pass the ctu_xxx_api.h headers between dependencies
        # so we have to use static linking until thats fixed
        'default_library=static'
    ]
)

unit_tests = get_option('unit_tests').disable_auto_if(meson.is_subproject())
driver_tests = get_option('driver_tests').disable_auto_if(meson.is_subproject())

# not sure if theres a better way to do this
# https://github.com/mesonbuild/meson/issues/12570
unit_tests.require(add_languages('cpp', native : false, required : unit_tests.enabled()))

cc = meson.get_compiler('c')
cpp = meson.get_compiler('cpp')

afl_fuzzing = get_option('afl_fuzzing').disable_auto_if(meson.is_subproject())
llvm_fuzzing = get_option('llvm_fuzzing').disable_auto_if(meson.is_subproject())

if afl_fuzzing.enabled() and llvm_fuzzing.enabled()
    error('afl_fuzzing and llvm_fuzzing are mutually exclusive')
endif

# only enable fuzzing if its explicitly enabled
# as it disables everything else
if llvm_fuzzing.enabled()
    # do these checks conservatively
    llvm_fuzzing = llvm_fuzzing.require(cpp.has_argument('-fsanitize=fuzzer-no-link'))

    if llvm_fuzzing.enabled()
        add_project_arguments('-fsanitize=fuzzer', language : [ 'c', 'cpp' ])
        add_project_link_arguments('-fsanitize=fuzzer', language : [ 'c', 'cpp' ])
    endif
endif

if afl_fuzzing.enabled()
    afl_fuzzing = afl_fuzzing.require(cc.get_id().contains('afl'))
endif

analyze = get_option('analyze')

version = meson.project_version()
target = target_machine.system()
host = host_machine.system()
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

gmp = dependency('gmp', fallback : [ 'mini-gmp', 'gmp' ])

args = []
lexargs = []
parseargs = []

# bison generates a few unused variables
args += cc.get_supported_arguments(
    '-Wno-unused-but-set-variable',
    '-Wno-unused-function',
    '-Wno-unused-parameter',

    # may need to update bison soon
    '-Wno-deprecated-non-prototype'
)

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

driver_summary = {}
langs = {}
interfaces = {}
plugins = {}

foreach lang : get_option('languages')
    subdir('driver'/lang)
endforeach

subdir('tools')

# enabling fuzzing needs to disable everything else
# that has a main function
if llvm_fuzzing.enabled()
    subdir('tools/llvm_fuzzing')
else
    subdir('interface')

    if unit_tests.allowed()
        subdir('tests/unit')
    endif

    if driver_tests.allowed()
        subdir('interface/harness')
        subdir('tests/lang')
    endif
endif

doxygen = find_program('doxygen', required : get_option('doxygen'))

if doxygen.found()
    cwd = meson.project_source_root()
    doxy_srcdirs = [ cwd/'interface', cwd/'common', cwd/'cthulhu', cwd/'driver' ]

    doxy_cdata = configuration_data()
    doxy_cdata.merge_from(config_cdata)
    doxy_cdata.set('CTU_SOURCES', ';'.join(doxy_srcdirs))

    doxy_cfg = configure_file(
        input : 'Doxyfile.in',
        output : 'Doxyfile',
        configuration : doxy_cdata
    )

    run_target('docs', command : [ doxygen, doxy_cfg ])
endif

summary = {
    'Build': {
        'Compiler': cc.get_id(),
        'Debug': is_debug,
        'Interfaces': ', '.join(get_option('interfaces')),
        'Languages': ', '.join(get_option('languages')),
    },
    'Testing': {
        'Unit tests': unit_tests,
        'Driver tests': driver_tests,
        'AFL fuzzing': afl_fuzzing,
        'LLVM fuzzing': llvm_fuzzing,
    },
    'Documentation': {
        'Doxygen': doxygen,
    }
}

foreach lang_name, lang_config : langs
    if lang_config.has_key('fuzz_corpus')
        driver_summary += { lang_name + ' fuzz corpus': lang_config.get('fuzz_corpus') }
    endif
endforeach

summary += { 'Drivers': driver_summary }

foreach section_title, section : summary
    summary(section, bool_yn : true, section : section_title)
endforeach
