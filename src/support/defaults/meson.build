src = [
    'src/defaults.c',
    'src/memory.c'
]

libdefaults = library('defaults', src,
    include_directories : 'include',
    dependencies : [ config, format, argparse, os, io, notify, backtrace ]
)

defaults = declare_dependency(
    link_with : libdefaults,
    include_directories : 'include',
    dependencies : [ config ]
)
