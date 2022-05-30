src = [
    'src/hlir/sema.c',
    'src/hlir/attribs.c',
    
    'src/hlir/type.c',
    'src/hlir/hlir.c',
    'src/hlir/decl.c',
    'src/hlir/ops.c',
    'src/hlir/query.c',
    'src/hlir/common.c',
    'src/hlir/validate.c',

    # common frontend components
    'src/interface/runtime.c',
    'src/interface/cthulhu.c',

    # tree output
    'src/emit/c89.c',

    # ast data and flex/bison interop
    'src/ast/scan.c',
    'src/ast/interop.c',
    'src/ast/ast.c',
    'src/ast/compile.c',

    # error reporting
    'src/report/report.c',
    'src/report/report-ext.c',

    # standard library
    'src/util/stream.c'
]

libgeneric = library('generic', src,
    include_directories : 'include',
    c_args : args,
    dependencies : deps
)

generic = declare_dependency(
    link_with : libgeneric,
    include_directories : 'include',
    dependencies : deps,
    compile_args : args
)
