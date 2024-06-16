project('cthulhu', 'c',
    license : 'LGPL-3.0-only AND GPL-3.0-only',
    version : '0.2.9',
    license_files : 'LICENSE',
    meson_version : '>=1.3.0',
    default_options : [
        # gcc is picky about POSIX stdlib extensions
        'c_std=gnu2x,c2x,gnu11,c11',

        'cpp_std=c++latest,c++23',

        # we use setjmp/longjmp for error handling
        # on msvc c++ exceptions do not work with setjmp/longjmp
        'cpp_eh=none',

        # we dont use rtti
        'cpp_rtti=false',

        'warning_level=3',
        'werror=true',

        # do static linking by default
        # shared linking is supported but slows down the test suite
        'default_library=static',

        # llvm c++ windows build issues
        'b_vscrt=mt'
    ]
)

buildtype = get_option('buildtype')
is_debug = buildtype.contains('debug')
is_release = buildtype.contains('release')
default_library = get_option('default_library')
cc = meson.get_compiler('c')
target = target_machine.system()

if default_library == 'both'
    error('cthulhu does not support building both static and shared libraries in a single build')
endif

opt_asserts = get_option('asserts').disable_auto_if(is_release)
opt_paranoid = get_option('paranoid').disable_auto_if(is_release)
opt_analyze = get_option('analyze').disable_auto_if(meson.is_subproject())
trace_time = get_option('trace_time').disable_auto_if(meson.is_subproject())

# disable stb_sprintf by default if we're a subproject, people may not
# want us to bring in new dependencies that we dont require to function
opt_stb_sprintf = get_option('stb_sprintf').disable_auto_if(meson.is_subproject())

# disable unit tests if we are a subproject or release build
# this is because they require some different flags that slow down the final binary.
# but only on gcc
unit_tests = get_option('unit_tests').disable_auto_if(is_release or meson.is_subproject())
driver_tests = get_option('driver_tests').disable_auto_if(meson.is_subproject())
warning_level = get_option('warning_level').to_int()

trace_memory = get_option('trace_memory').disable_auto_if(is_release or meson.is_subproject())

# these are tools for internal use, so disable them in release mode
tool_notify = get_option('tool_notify').disable_auto_if(is_release or meson.is_subproject())
tool_display = get_option('tool_display').disable_auto_if(is_release or meson.is_subproject())
tool_error = get_option('tool_error').disable_auto_if(is_release or meson.is_subproject())
tool_tar = get_option('tool_tar').disable_auto_if(is_release or meson.is_subproject())

# all the frontend programs
frontend_gui = get_option('frontend_gui').disable_auto_if(meson.is_subproject())
frontend_cli = get_option('frontend_cli').disable_auto_if(meson.is_subproject())
frontend_example = get_option('frontend_example').disable_auto_if(meson.is_subproject())

# all the language frontends
lang_ctu = get_option('lang_ctu').disable_auto_if(meson.is_subproject())
lang_pl0 = get_option('lang_pl0').disable_auto_if(meson.is_subproject())
lang_oberon = get_option('lang_oberon').disable_auto_if(meson.is_subproject())
lang_example = get_option('lang_example').disable_auto_if(meson.is_subproject())
lang_cc = get_option('lang_cc').disable_auto_if(meson.is_subproject())

# all the codegen targets
target_cfamily = get_option('target_cfamily').disable_auto_if(meson.is_subproject())
target_debug = get_option('target_debug').disable_auto_if(meson.is_subproject())

# all compiler plugins
plugin_timer = get_option('plugin_timer').disable_auto_if(meson.is_subproject())

# tools like this are user facing, so dont disable them in release mode
tool_diagnostic = get_option('tool_diagnostic').disable_auto_if(meson.is_subproject())

needs_cpp = frontend_gui.allowed()

has_cpp = add_languages('cpp', native : false, required : needs_cpp)
frontend_gui = frontend_gui.require(has_cpp)

frontend_gui = frontend_gui.require(target == 'windows')

if unit_tests.enabled() and is_release and cc.get_id() == 'gcc'
    warning('enabling unit tests in release mode with gcc will result in degraded performance')
endif

# args for all code
all_args = cc.get_supported_arguments(
    # we use flexible array members
    '/wd4200',

    # we dont need these parts of C, might as well get some small perf gains
    '-fno-threadsafe-statics',
    '-fno-keep-static-consts',

    # more free perf
    '-fstrict-aliasing',
    '-fmerge-all-constants',

    '-D_ITERATOR_DEBUG_LEVEL=0'
)

has_time_trace = cc.has_argument('-ftime-trace')
ninjatracing = find_program('data/scripts/ninjatracing/ninjatracing', required : trace_time.enabled())

if trace_time.enabled() and has_time_trace
    all_args += [ '-ftime-trace' ]
endif

if ninjatracing.found()
    trace_args = has_time_trace ? [ '--embed-time-trace' ] : [ ]
    custom_target('gather_time_trace',
        output : 'trace.json',
        capture : true,
        command : [ ninjatracing, meson.global_build_root() / '.ninja_log' ] + trace_args
    )
endif

# args for handwritten code only
user_args = cc.get_supported_arguments(
    # this isnt on by default in gcc
    '-Wnonnull'
)

# args for code generated by flex/bison only
generated_args = cc.get_supported_arguments(
    '-Wno-unused-but-set-variable',
    '-Wno-unused-function',
    '-Wno-unused-parameter',
    '-Wno-unused-function',

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
    # we only want to enable analysis on our own code
    # the stuff generated by flex and bison is a bit too much for the analyzers
    user_args += cc.get_supported_arguments(analyze_user_args)
endif

if has_cpp
    cpp = meson.get_compiler('cpp')
    extra_cpp_args = cpp.get_supported_arguments(
        # introduced in clang-19, warns on missing designated initializer fields
        # i've never had a bug from not initializing a field, since they are always
        # default initialized to 0 in c. so this warning is just noise
        '-Wno-missing-field-initializers'
    )

    add_project_arguments(extra_cpp_args + all_args, language : 'cpp')
endif

add_project_arguments(all_args, language : [ 'c', 'cpp' ])

lexargs = []
parseargs = []

host = host_machine.system()
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
plugins = {}
targets = {}
frontends = {}

setargv = cc.get_id() == 'msvc' ? [ 'setargv.obj' ] : []

subdir('src')

if unit_tests.allowed()
    subdir('tests/unit')
endif

if driver_tests.allowed()
    subdir('tests/lang')
endif

doxygen = find_program('doxygen', required : get_option('doxygen'))

if doxygen.found()
    source_dir = meson.project_source_root() / 'src'
    data_dir = meson.project_source_root() / 'data'

    doxy_sources = [
        source_dir / 'common',
        source_dir / 'cthulhu',
        source_dir / 'frontend',
        source_dir / 'language',
        source_dir / 'loader',
        source_dir / 'plugins',
        source_dir / 'support',
        source_dir / 'target',
        source_dir / 'tools',

        data_dir / 'docs'
    ]

    stylesheets = [
        data_dir / 'doxygen-awesome-css/doxygen-awesome.css',
        data_dir / 'doxygen-awesome-css/doxygen-awesome-sidebar-only.css'
    ]

    doxy_includes = [
        source_dir / 'common',
        source_dir / 'cthulhu/events/include',
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

pkgconfig = import('pkgconfig')

common_deps = [
    core, backtrace, base, arena, std, os,
    io, scan, fs,
    interop, config, notify, endian
]

runtime_deps = [
    memory, events, tree, ssa,
    util, broker, check
]

support_deps = [
    driver, argparse, format, setup, json
]

pkg_common = pkgconfig.generate(
    description : 'Cthulhu common libraries',
    libraries : common_deps,
    version : meson.project_version(),
    name : 'cthulhu-common',
    filebase : 'cthulhu-common'
)

pkg_runtime = pkgconfig.generate(
    description : 'Cthulhu runtime library components',
    libraries : runtime_deps,
    requires : 'cthulhu-base',
    version : meson.project_version(),
    name : 'cthulhu-runtime',
    filebase : 'cthulhu-runtime'
)

pkg_support = pkgconfig.generate(
    description : 'Cthulhu support library',
    libraries : support_deps,
    requires : 'cthulhu-runtime',
    version : meson.project_version(),
    name : 'cthulhu-support',
    filebase : 'cthulhu-support'
)

build_summary = {
    'Build': {
        'Debug': is_debug,
        'Memory tracing': trace_memory.allowed(),
        'Analyze': opt_analyze.enabled(),
        'Paranoid asserts': opt_paranoid.allowed(),
        'Flex': flex,
        'Bison': bison,
        'Compile time tracing': trace_time.enabled(),
        'Module loader': default_library,
        'Unit tests': unit_tests,
        'Driver tests': driver_tests,
        'Doxygen': doxygen,
        'STB sprintf': opt_stb_sprintf,
    },
    'Targets': {
        'C Family': target_cfamily.allowed(),
        'Debug': target_debug.allowed(),
    },
    'Frontends': {
        'GUI': frontend_gui.allowed(),
        'CLI': frontend_cli.allowed()
    },
    'Languages': {
        'Cthulhu': lang_ctu.allowed(),
        'PL/0': lang_pl0.allowed(),
        'Oberon-2': lang_oberon.allowed(),
        'Example': lang_example.allowed(),
        'C': lang_cc.allowed()
    },
    'Tools': {
        'Notify': tool_notify.allowed(),
        'Diagnostics': tool_diagnostic.allowed(),
        'Display': tool_display.allowed(),
        'Error': tool_error.allowed(),
        'Tar': tool_tar.allowed()
    }
}

foreach lang_name, lang_config : langs
    if lang_config.has_key('fuzz_corpus')
        driver_summary += { lang_name + ' fuzz corpus': lang_config.get('fuzz_corpus') }
    endif
endforeach

build_summary += { 'Drivers': driver_summary }

foreach section_title, content : build_summary
    summary(content, bool_yn : true, section : section_title)
endforeach
