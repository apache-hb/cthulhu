src = [ 'src/stacktrace/common.c' ]

if stacktrace.allowed()
    src += [ 'src/stacktrace/stacktrace-target.c'.replace('target', target) ]
    if target == 'windows'
        stacktrace_links = [ 'dbghelp.lib' ]
    else
        stacktrace_links = []
    endif
else
    src += [ 'src/stacktrace/stacktrace-null.c' ]
    stacktrace_links = []
endif

libstacktrace = library('stacktrace', src,
    c_args : args,
    include_directories : 'include',
    link_args : stacktrace_links
)

stacktrace = declare_dependency(
    link_with : libstacktrace,
    include_directories : 'include',
    link_args : stacktrace_links
)

src = [
    'src/base/base.c',
    'src/base/panic.c',
    'src/base/memory.c'
]

libbase = library('base', src,
    c_args : args,
    dependencies : [ gmp, stacktrace ],
    include_directories : 'include'
)

base = declare_dependency(
    link_with : libbase,
    include_directories : [ 'include', versiondir ]
)

src = [
    'src/std/map.c',
    'src/std/optimal.c',
    'src/std/set.c',
    'src/std/str.c',
    'src/std/vector.c'
]

libstd = library('std', src,
    c_args : args,
    dependencies : base,
    include_directories : 'include'
)

std = declare_dependency(
    link_with : libstd,
    dependencies : base,
    include_directories : 'include'
)

src = [
    'src/platform/error.c',
    'src/platform/file.c',
    'src/platform/library.c',
    'src/platform/platform/target.c'.replace('target', target)
]

libplatform = library('platform', src,
    c_args : args,
    dependencies : std,
    include_directories : 'include'
)

platform = declare_dependency(
    link_with : libplatform,
    dependencies : std,
    include_directories : 'include'
)

src = [ 
    'src/io/io.c',
    'src/io/fs.c'
]

libio = library('io', src,
    c_args : args,
    dependencies : [ platform, std ],
    include_directories : 'include'
)

io = declare_dependency(
    link_with : libio,
    dependencies : [ platform, std ],
    include_directories : 'include'
)

src = [
    'src/report/report.c',
    'src/report/report-ext.c',
    'src/scan/node.c',
    'src/scan/scan.c',
    'src/scan/compile.c'
]

# this is both libreport and libscan as they depend on each other
liblocation = library('location', src,
    c_args : args,
    dependencies : [ std, io ],
    include_directories : 'include'
)

location = declare_dependency(
    link_with : liblocation,
    dependencies : std,
    include_directories : 'include'
)

libinterop = library('interop', [ 'src/interop/flex.c' ],
    c_args : args,
    dependencies : std,
    include_directories : 'include'
)

interop = declare_dependency(
    link_with : libinterop,
    dependencies : std,
    include_directories : 'include'
)

src = [
    'src/argparse/argparse.c',
    'src/argparse/common.c',
    'src/argparse/commands.c',
    lex.process('src/argparse/ap.l'),
    parse.process('src/argparse/ap.y')
]

libargparse = library('argparse', src,
    c_args : args,
    dependencies : [ gmp, std, location, interop ],
    include_directories : [ 'src', 'include', versiondir ]
)

argparse = declare_dependency(
    link_with : libargparse,
    include_directories : 'include',
    dependencies : [ gmp ]
)

src = [
    'src/report2/report.c'
]

libreport2 = library('report2', src,
    c_args : args,
    dependencies : [ std, location, interop ],
    include_directories : 'include'
)

report = declare_dependency(
    link_with : libreport2,
    dependencies : std,
    include_directories : 'include'
)
