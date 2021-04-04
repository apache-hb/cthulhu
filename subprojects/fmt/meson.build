project('fmt', 'cpp', version : '7.1.3')

incs = [ 'fmt/include' ]

args = [ '-DFMT_SUPPRESS_MSC_WARNING=1' ]

if target_machine.system() == 'windows'
    args += '/wd4702'
endif

libfmt = library('fmt', [ 'fmt/src/format.cc', 'fmt/src/os.cc' ],
    include_directories : incs,
    cpp_args : args
)

dep = declare_dependency(
    link_with : libfmt,
    include_directories : incs
)
