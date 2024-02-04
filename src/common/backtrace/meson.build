backtrace_api = configure_file(
    input : api_header,
    output : 'ctu_backtrace_api.h',
    configuration : { 'MOD': 'BACKTRACE' }
)

backtrace_include = include_directories('.', 'include')

src = [ 'src/common.c' ]

dbghlp = cc.find_library('dbghelp', required : false)
unwind = cc.find_library('unwind', required : false)

has_unwind = cc.check_header('<unwind.h>', dependencies : unwind)

if dbghlp.found()
    src += [ 'src/dbghelp.c' ]
    backtrace_deps = [ dbghlp ]
elif has_unwind
    src += [ 'src/unwind.c' ]
    backtrace_deps = [ unwind ]
else
    src += [ 'src/stub.c' ]
    backtrace_deps = [ ]
endif

libbacktrace = library('backtrace', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    c_args : user_args + [ '-DCT_BACKTRACE_BUILD=1' ],
    include_directories : backtrace_include,
    dependencies : [ backtrace_deps, core ]
)

backtrace = declare_dependency(
    link_with : libbacktrace,
    include_directories : backtrace_include,
    dependencies : [ core ]
)

install_headers(backtrace_api)
install_subdir('include', install_dir : 'include', strip_directory : true)
