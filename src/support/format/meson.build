format_api = configure_file(
    input : api_header,
    output : 'ctu_format_api.h',
    configuration : { 'MOD': 'FORMAT' }
)

format_include = include_directories('.', 'include')

src = [
    'src/colour.c',
    'src/config.c',
    'src/version.c',

    'src/common.c',
    'src/text_rich.c',
    'src/text_simple.c',
    'src/backtrace.c',
    'src/notify.c'
]

libformat = library('format', src,
    c_args : user_args + [ '-DCT_FORMAT_BUILD=1' ],
    dependencies : [ std, core, io, scan, notify, config, backtrace, memory, arena ],
    include_directories : format_include
)

format = declare_dependency(
    link_with : libformat,
    dependencies : [ std, core, notify ],
    include_directories : format_include
)
