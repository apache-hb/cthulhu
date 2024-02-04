format_api = configure_file(
    input : api_header,
    output : 'ctu_format_api.h',
    configuration : { 'MOD': 'FORMAT' }
)

format_include = include_directories('.', 'include')

src = [
    'src/colour.c',
    'src/common_simple.c',

    'src/backtrace.c',
    'src/version.c'
]

format_args = user_args + [ '-DCT_FORMAT_BUILD=1' ]

libformat_core = library('format_core', src,
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

src = [
    'src/common.c',

    'src/text_rich.c',
    'src/text_simple.c',
    'src/notify.c',
    'src/config.c'
]

libformat = library('format', src,
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
