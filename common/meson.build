libbase = library('base', [ 'src/base/base.c' ], 
    c_args : args,
    dependencies : deps,
    include_directories : 'include'
)

base = declare_dependency(
    link_with : libbase, 
    dependencies : deps,
    include_directories : 'include'
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
    'src/report/report.c',
    'src/report/report-ext.c'
]

deps += [ base, std, platform ]

libargparse = library('argparse', [ 'src/argparse/argparse.c' ], 
    c_args : args, 
    dependencies : deps,
    include_directories : 'include'
)

argparse = declare_dependency(
    link_with : libargparse, 
    dependencies : deps, 
    include_directories : 'include'
)
