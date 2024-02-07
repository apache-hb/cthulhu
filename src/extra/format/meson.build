# common format components
src = [
    'src/common.c',
    'src/colour.c'
]

deps = [
    std, core, base, arena
]

format_common_include = include_directories('include', 'src')
format_args = user_args + [ '-DCT_FORMAT_BUILD=1' ]

libformat_common = shared_library('format_common', src,
    build_by_default : not meson.is_subproject(),
    c_args : format_args,
    dependencies : deps,
    include_directories : format_common_include
)

format_objects = libformat_common.extract_all_objects(recursive : false)

format_api = configure_file(
    input : api_header,
    output : 'ctu_format_api.h',
    configuration : { 'MOD': 'FORMAT' }
)

format_include = include_directories('.', 'include')


# simple format api, backtraces and version info
src = [
    'src/backtrace.c',
    'src/version.c'
]

libformat_core = library('format_core', src,
    objects : format_objects,
    build_by_default : not meson.is_subproject(),
    c_args : format_args,
    include_directories : format_include,
    dependencies : [ std, core, base, io, arena, backtrace ]
)

format_core = declare_dependency(
    include_directories : format_include,
    link_with : libformat_core,
    dependencies : [ core ]
)

# more complete format api, error messages and configuration objects
src = [
    'src/common_extra.c',

    'src/text_rich.c',
    'src/text_simple.c',
    'src/notify.c',
    'src/config.c'
]

libformat = library('format', src,
    objects : format_objects,
    build_by_default : not meson.is_subproject(),
    c_args : format_args,
    dependencies : [ std, base, io, scan, notify, config, backtrace, memory, arena, format_core ],
    include_directories : format_include
)

format = declare_dependency(
    link_with : libformat,
    dependencies : [ std, core, notify, format_core ],
    include_directories : format_include
)
