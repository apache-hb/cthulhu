dir = meson.current_source_dir()

crashes = [ 
    'fuzz-1', 'fuzz-2', 'fuzz-3', 'fuzz-4', 'fuzz-5',
    'fuzz-6', 'fuzz-7', 'fuzz-8', 'fuzz-9',
    'overflow.1', 'overflow.2', 'overflow.3', 'overflow.4', 'overflow.5'
]

foreach case : crashes
    where = case.replace('.', '/')
    path = dir/'crashes'/where + '.ct'
    test('crash-' + case, ctc, 
        args : [ path ], 
        suite : 'ctu',
        should_fail : true
    )
endforeach

passes = [
    'types.alias-0', 'types.alias-3',
    'types.closure-0',
    'types.mixed-1', 'types.mixed-2',
    'types.struct-2', 'types.struct-3', 'types.struct-4',
    'types.struct-5', 'types.struct-6'
]

foreach case : passes
    where = case.replace('.', '/')
    path = dir/where + '.ct'
    test('pass-' + case, ctc, 
        args : [ path ], 
        suite : 'ctu'
    )
endforeach

fails = [
    'types.alias-1', 'types.alias-2',
    'types.mixed-0'
]

foreach case : fails
    where = case.replace('.', '/')
    path = dir/where + '.ct'
    test('pass-' + case, ctc, 
        args : [ path ], 
        suite : 'ctu',
        should_fail : true
    )
endforeach

# fails = [
#     'array-2', 'array-oob', 'array-ref', 'array',
#     'bad-return', 'direct-index', 'div-0',
#     'fuzz-1', 'implicit-bool-cast', 'incorrect-return1',
#     'incorrect-return2', 'infinite-struct', 'invalid-char',
#     'invalid-init', 'loop', 'nested-refs', 'no-init', 'recurse',
#     'return-local-ref', 'return-struct-ref', 'scope',
#     'type-in-expr', 'uninit-global', 'void-var', 'weird', 'large',
#     'recurse-lambda',
#     'data.loop-complex', 'data.loop-newtype', 'data.loop-ptr',
#     'data.duplicate', 'index', 'data.newtype'
# ]

# foreach case : fails
#     where = case.replace('.', '/')
#     path = dir/'fail'/where + '.ct'
#     test('fail-' + case, ctc, 
#         args : [ '-V', path, '-gen', 'c99', '-o', case + '.c' ], 
#         suite : 'ctu',
#         should_fail : true
#     )
# endforeach

# pass = [
#     'args', 'array-1', 'array-3', 'array-init', 'array-of',
#     'array-struct', 'binary', 'call', 'chained', 'comments',
#     'direct-index', 'fizzbuzz', 'forward', 'func', 'fuzz-8',
#     'global-const', 'global-init', 'global-uninit', 'global-var',
#     'global', 'hello', 'hello2', 'horror', 'implicit-var-cast',
#     'local', 'long-while', 'long', 'loop', 'multiline', 'mut',
#     'nest', 'order', 'pointers', 'pointers2', 'posix-interop', 'regalloc',
#     'string', 'struct', 'union', 'unused-var', 'var', 'win32-interop',
#     'simple', 'extern', 'fuzz-10', 'section', 'section-2', 'immediate-invoke',
#     'funcs.fwd', 'funcs.single', 'funcs.stmts', 'lambda', 'data.string',
#     'imports.first', 'imports.a',
#     'builtins.sizeof', 'index', 'builtins.sizeof-complex', 'data.newtype'
# ]

# foreach case : pass
#     where = case.replace('.', '/')
#     path = dir/'pass'/where + '.ct'
#     test('pass-' + case, ctc, 
#         args : [ '-V', path, '-gen', 'c99', '-o', case + '.c' ],
#         suite : 'ctu'
#     )
# endforeach
