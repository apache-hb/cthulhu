project('cthulhu', 'c',
    license : 'LGPLv3',
    version : '0.2.8',
    meson_version : '>=1.3.0',
    default_options : [
        # gcc is picky about POSIX stdlib extensions
        'c_std=gnu11,c11',

        'cpp_std=c++20',

        # we use setjmp/longjmp for error handling
        # on msvc c++ exceptions do not work with setjmp/longjmp
        'cpp_eh=none',

        # we dont use rtti
        'cpp_rtti=false',

        'warning_level=3',
        'werror=true',

        # do static linking by default
        # shared linking is supported but slows down the test suite
        'default_library=static'
    ]
)

buildtype = get_option('buildtype')
is_debug = buildtype.contains('debug')
is_release = buildtype.contains('release')
default_library = get_option('default_library')
cc = meson.get_compiler('c')
target = target_machine.system()

opt_paranoid = get_option('paranoid').disable_auto_if(is_release)
opt_analyze = get_option('analyze').disable_auto_if(meson.is_subproject())
win32_trickery = get_option('win32_trickery')

# disable unit tests if we are a subproject or release build
# this is because they require some different flags that slow down the compiler.
# but only on gcc
unit_tests = get_option('unit_tests').disable_auto_if(is_release or meson.is_subproject())
driver_tests = get_option('driver_tests').disable_auto_if(meson.is_subproject())
warning_level = get_option('warning_level').to_int()

trace_memory = get_option('trace_memory').disable_auto_if(is_release or meson.is_subproject())

# these are tools for internal use, so disable them in release mode
tool_notify = get_option('tool_notify').disable_auto_if(is_release or meson.is_subproject())
tool_display = get_option('tool_display').disable_auto_if(is_release or meson.is_subproject())
tool_error = get_option('tool_error').disable_auto_if(is_release or meson.is_subproject())

# all the frontend programs
frontend_gui = get_option('frontend_gui').disable_auto_if(meson.is_subproject())
frontend_cli = get_option('frontend_cli').disable_auto_if(meson.is_subproject())
frontend_example = get_option('frontend_example').disable_auto_if(meson.is_subproject())

# all the language frontends
lang_ctu = get_option('lang_ctu').disable_auto_if(meson.is_subproject())
lang_pl0 = get_option('lang_pl0').disable_auto_if(meson.is_subproject())
lang_oberon = get_option('lang_oberon').disable_auto_if(meson.is_subproject())
lang_jvm = get_option('lang_jvm').disable_auto_if(meson.is_subproject())
lang_example = get_option('lang_example').disable_auto_if(meson.is_subproject())
lang_c = get_option('lang_c').disable_auto_if(meson.is_subproject())
lang_cpp = get_option('lang_cpp').disable_auto_if(meson.is_subproject())

# all the codegen targets
target_cfamily = get_option('target_cfamily').disable_auto_if(meson.is_subproject())
target_debug = get_option('target_debug').disable_auto_if(meson.is_subproject())
default_target = get_option('default_target')

# tools like this are user facing, so dont disable them in release mode
tool_diagnostic = get_option('tool_diagnostic').disable_auto_if(meson.is_subproject())
tool_reflect = get_option('tool_reflect').disable_auto_if(meson.is_subproject())

needs_cpp = tool_reflect.enabled() or frontend_gui.enabled()
wants_cpp = tool_reflect.allowed() or frontend_gui.allowed()

if wants_cpp
    has_cpp = add_languages('cpp', native : false, required : needs_cpp)

    frontend_gui = frontend_gui.require(has_cpp)
    tool_reflect = tool_reflect.require(has_cpp)
endif

frontend_gui = frontend_gui.require(target == 'windows')

if unit_tests.enabled() and is_release and cc.get_id() == 'gcc'
    warning('enabling unit tests in release mode with gcc will result in degraded performance')
endif

version = meson.project_version()
host = host_machine.system()
parts = version.split('.')

source_dir = meson.project_source_root() / 'src'
data_dir = meson.project_source_root() / 'data'

config_cdata = configuration_data()

config_cdata.set('CTU_MAJOR', parts[0].to_int())
config_cdata.set('CTU_MINOR', parts[1].to_int())
config_cdata.set('CTU_PATCH', parts[2].to_int())

config_cdata.set10('CTU_DEBUG', is_debug)
config_cdata.set10('CTU_PARANOID', opt_paranoid.allowed())
config_cdata.set10('CTU_BUILD_SHARED', default_library == 'shared')
config_cdata.set10('CTU_WIN32_TRICKERY', win32_trickery.allowed())

# we use `__attribute__((const|pure))` heavily, but during unit tests we
# violate function preconditions to test assertions.
# msvc and clang dont mind, but gcc does. so we disable the attributes for gcc
config_cdata.set10('CTU_DISABLE_FN_PURITY', unit_tests.allowed() and cc.get_id() == 'gcc')

# gcc warns about __PRETY_FUNCTION__ not being ISO C when warning_leve=3
# which is weird seeing as we specify gnu11, maybe its a gcc bug?
config_cdata.set10('CTU_HAS_PRETTY_FUNCTION', warning_level < 3)

config_cdata.set10('CTU_TRACE_MEMORY', trace_memory.allowed())

# args for all code
args = cc.get_supported_arguments(
    # we use flexible array members
    '/wd4200',

    # we dont need these parts of C, might as well get some small perf gains
    '-fno-threadsafe-statics',
    '-fno-keep-static-consts',

    # more free perf
    '-fstrict-aliasing',
    '-fmerge-all-constants'
)

# args for handwritten code
user_args = cc.get_supported_arguments(
    # this isnt on by default in gcc
    '-Wnonnull'
)

# args for code generated by flex/bison
generated_args = cc.get_supported_arguments(
    '-Wno-unused-but-set-variable',
    '-Wno-unused-function',
    '-Wno-unused-parameter',

    # may need to update bison soon
    '-Wno-deprecated-non-prototype',

    # bison generates some unreachable code
    '/wd4702',

    # flex generates iffy code on mac
    '-Wno-sign-compare',

    # mingw needs this define for flex/bison to work
    '-D__USE_MINGW_ANSI_STDIO=1'
)

analyze_user_args = [
    # msvc analyze args
    '/analyze',
    '/analyze:max_paths 4096',

    # gcc analyze args
    '-fanalyzer',

    # gcc is a bit too aggressive with these
    '-Wno-analyzer-malloc-leak',

    # this one looks like a gcc bug
    '-Wno-analyzer-double-free',

    '-D_FORTIFY_SOURCE=2'
]

if opt_analyze.enabled()
    user_args += cc.get_supported_arguments(analyze_user_args)
endif

add_project_arguments(args, language : [ 'c' ])

lexargs = []
parseargs = []

# setup required flex/bison args
if host == 'windows'
    lexargs += [ '--wincompat' ]
else
    parseargs += [ '-Wdeprecated' ]
endif

flex = find_program('flex', 'win_flex', version : '>=2.6')
bison = find_program('bison', 'win_bison', version : host == 'windows' ? '>=2.6' : '>=3.5')

lex = generator(flex,
    output : [ '@BASENAME@_flex.c', '@BASENAME@_flex.h' ],
    arguments : lexargs + [
        '--outfile=@OUTPUT0@',
        '--header-file=@OUTPUT1@',
        '@INPUT@'
    ]
)

parse = generator(bison,
    output : [ '@BASENAME@_bison.c', '@BASENAME@_bison.h' ],
    arguments : parseargs + [
        '-d', '@INPUT@', '-v',
        '--output=@OUTPUT0@',
        '--defines=@OUTPUT1@'
    ]
)

driver_summary = {}
langs = {}
interfaces = {}
plugins = {}
targets = {}

setargv = []
if cc.get_id() == 'msvc'
    setargv = [ 'setargv.obj' ]
endif

subdir('src')

if unit_tests.allowed()
    subdir('tests/unit')
endif

if driver_tests.allowed()
    subdir('tests/lang')
endif

doxygen = find_program('doxygen', required : get_option('doxygen'))

if doxygen.found()
    doxy_sources = [
        source_dir / 'frontend',
        source_dir / 'common',
        source_dir / 'support',
        source_dir / 'cthulhu',
        source_dir / 'language',
        source_dir / 'target',

        data_dir / 'docs'
    ]

    stylesheets = [
        data_dir / 'doxygen-awesome.css',
        data_dir / 'doxygen-awesome-sidebar-only.css'
    ]

    doxy_includes = [
        source_dir / 'common',
        source_dir / 'cthulhu/events/include',
        source_dir / 'cthulhu/runtime/include',
        source_dir / 'cthulhu/ssa/include',
        source_dir / 'cthulhu/tree/include'
    ]

    doxy_cdata = configuration_data()
    doxy_cdata.merge_from(config_cdata)
    doxy_cdata.set('CTU_SOURCES', ' '.join(doxy_sources))
    doxy_cdata.set('ROOT_SOURCE_DIR', source_dir)
    doxy_cdata.set('INCLUDE_PATHS', ' '.join(doxy_includes))
    doxy_cdata.set('EXTRA_STYLESHEET', ' '.join(stylesheets))
    doxy_cdata.set('CLANG_DATABASE', meson.global_build_root() / 'compile_commands.json')

    doxy_cfg = configure_file(
        input : 'data/Doxyfile.in',
        output : 'Doxyfile',
        configuration : doxy_cdata
    )

    run_target('docs', command : [ doxygen, doxy_cfg ])
endif

summary = {
    'Build': {
        'Debug': is_debug,
        'Memory tracing': trace_memory.allowed(),
        'Analyze': opt_analyze.enabled(),
        'Paranoid asserts': opt_paranoid.allowed(),
        'Flex': flex,
        'Bison': bison,
        'Win32 trickery': win32_trickery.allowed(),
    },
    'Targets': {
        'C Family': target_cfamily.allowed(),
        'Debug': target_debug.allowed(),
        'Default': default_target
    },
    'Frontends': {
        'GUI': frontend_gui.allowed(),
        'CLI': frontend_cli.allowed(),
        'Example': frontend_example.allowed()
    },
    'Languages': {
        'C': lang_c.allowed(),
        'C preprocessor': lang_cpp.allowed(),
        'Cthulhu': lang_ctu.allowed(),
        'PL/0': lang_pl0.allowed(),
        'Oberon-2': lang_oberon.allowed(),
        'JVM bytecode': lang_jvm.allowed(),
        'Example': lang_example.allowed()
    },
    'GUI Frontend': {
        'Enabled': frontend_gui.allowed(),
        'Backend': 'd3d12+win32'
    },
    'Testing': {
        'Unit tests': unit_tests,
        'Driver tests': driver_tests
    },
    'Tools': {
        'Notify': tool_notify.allowed(),
        'Diagnostics': tool_diagnostic.allowed(),
        'Display': tool_display.allowed(),
        'Error': tool_error.allowed(),
        'Reflect': tool_reflect.allowed()
    },
    'Documentation': {
        'Doxygen': doxygen
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
