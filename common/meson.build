###
### stacktrace
###

src = [ 'src/stacktrace/common.c' ]

dbghlp = cc.find_library('dbghlp', required : false)

if dbghlp.found()
    src += [ 'src/stacktrace/stacktrace-dbghlp.c' ]
    stacktrace_deps = [ dbghlp ]
else
    src += [ 'src/stacktrace/stacktrace-null.c' ]
    stacktrace_deps = [ ]
endif

libstacktrace = library('stacktrace', src,
    c_args : user_args,
    include_directories : 'include',
    dependencies : stacktrace_deps
)

stacktrace = declare_dependency(
    link_with : libstacktrace,
    include_directories : 'include',
    dependencies : stacktrace_deps
)

###
### base
###

config_header = configure_file(
    output : 'ctu-config.h',
    configuration : config_cdata
)

config_include = include_directories('.')

src = [
    config_header,

    'src/base/base.c',
    'src/base/panic.c',
    'src/base/endian.c',
    'src/base/memory.c'
]

libbase = library('base', src,
    c_args : user_args,
    dependencies : [ gmp, stacktrace ],
    include_directories : [ 'include', config_include ]
)

base = declare_dependency(
    sources : config_header,
    link_with : libbase,
    include_directories : [ 'include', config_include ]
)

###
### std
###

src = [
    'src/std/map.c',
    'src/std/optimal.c',
    'src/std/set.c',
    'src/std/str.c',
    'src/std/vector.c',

    'src/std/typed/vector.c'
]

libstd = library('std', src,
    c_args : user_args,
    dependencies : base,
    include_directories : 'include'
)

std = declare_dependency(
    link_with : libstd,
    dependencies : base,
    include_directories : 'include'
)

# platform glue code

src = [ 'src/os/common.c' ]

libcommon = library('common', src,
    c_args : user_args,
    dependencies : [ std ],
    include_directories : 'include'
)

common = declare_dependency(
    link_with : libcommon,
    dependencies : [ std ],
    include_directories : [ 'include', 'src' ]
)

# get the backend

subdir('src/os'/target)

# create the native library

os = declare_dependency(
    include_directories : 'include',
    dependencies : [ std, native ]
)

src = [
    'src/io/io.c',
    'src/io/io/common.c',
    'src/io/io/view.c',
    'src/io/io/buffer.c',
    'src/io/io/file.c',

    'src/io/fs.c',
    'src/io/fs/common.c',
    'src/io/fs/physical.c',
    'src/io/fs/virtual.c',
]

libio = library('io', src,
    c_args : user_args,
    dependencies : [ std, os ],
    include_directories : [ 'include', 'src/io' ]
)

io = declare_dependency(
    link_with : libio,
    dependencies : [ std, os ],
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
    c_args : user_args,
    dependencies : [ std, io ],
    include_directories : 'include'
)

location = declare_dependency(
    link_with : liblocation,
    dependencies : std,
    include_directories : 'include'
)

libinterop = library('interop', [ 'src/interop/flex.c' ],
    c_args : user_args,
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
]

libargparse_base = library('argparse_base', src,
    c_args : user_args,
    dependencies : [ gmp, std, location, interop ],
    include_directories : [ 'src', 'include' ]
)

src = [
    'src/argparse/parse.c',
    lex.process('src/argparse/ap.l'),
    parse.process('src/argparse/ap.y')
]

libargparse = library('argparse', src,
    c_args : generated_args,
    link_with : libargparse_base,
    dependencies : [ gmp, std, location, interop ],
    include_directories : [ 'src', 'include' ]
)

argparse = declare_dependency(
    link_with : [ libargparse_base, libargparse ],
    include_directories : 'include',
    dependencies : [ gmp ]
)
