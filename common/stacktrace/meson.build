src = [ 'src/common.c' ]

dbghlp = cc.find_library('dbghelp', required : false)

unwind = cc.find_library('unwind', required : false)

# TODO: libunwind support, maybe needs addr2line for symbol resolution

if dbghlp.found()
    src += [ 'src/bt_dbghlp.c' ]
    stacktrace_deps = [ dbghlp ]
elif unwind.found()
    src += [ 'src/bt_unwind.c' ]
    stacktrace_deps = [ unwind ]
else
    src += [ 'src/bt_null.c' ]
    stacktrace_deps = [ ]
endif

libstacktrace = library('stacktrace', src,
    c_args : user_args,
    include_directories : 'include',
    dependencies : [ stacktrace_deps, core ]
)

stacktrace = declare_dependency(
    link_with : libstacktrace,
    include_directories : 'include',
    dependencies : [ core, stacktrace_deps ]
)
