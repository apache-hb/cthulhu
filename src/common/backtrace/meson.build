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
    src += [ 'src/null.c' ]
    backtrace_deps = [ ]
endif

libbacktrace = library('backtrace', src,
    c_args : user_args,
    include_directories : 'include',
    dependencies : [ backtrace_deps, core ]
)

backtrace = declare_dependency(
    link_with : libbacktrace,
    include_directories : 'include',
    dependencies : [ core ]
)
