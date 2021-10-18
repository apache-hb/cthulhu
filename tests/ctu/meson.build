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
        args : [ '-V', path, '-gen', 'c99' ], 
        suite : 'ctu',
        should_fail : true
    )
endforeach

fails = [
    'array-2', 'array-oob', 'array-ref', 'array',
    'bad-return', 'case-names', 'direct-index', 'div-0',
    'fuzz-1', 'implicit-bool-cast', 'incorrect-return1',
    'incorrect-return2', 'infinite-struct', 'invalid-char',
    'invalid-init', 'loop', 'nested-refs', 'no-init', 'recurse',
    'return-local-ref', 'return-struct-ref', 'scope',
    'type-in-expr', 'uninit-global', 'void-var', 'weird', 'large'
]

foreach case : fails
    where = case.replace('.', '/')
    path = dir/'fails'/where + '.ct'
    test('fail-' + case, ctc, 
        args : [ '-V', path, '-gen', 'c99' ], 
        suite : 'ctu',
        should_fail : true
    )
endforeach

pass = [
    'args', 'array-1', 'array-3', 'array-init', 'array-of',
    'array-struct', 'binary', 'call', 'chained', 'comments',
    'direct-index', 'fizzbuzz', 'forward', 'func', 'fuzz-8',
    'global-const', 'global-init', 'global-uninit', 'global-var',
    'global', 'hello', 'hello2', 'horror', 'implicit-var-cast',
    'local', 'long-while', 'long', 'loop', 'multiline', 'mut',
    'nest', 'order', 'pointers', 'pointers2', 'posix-interop', 'regalloc',
    'string', 'struct', 'suffix', 'union', 'unused-var', 'var', 'win32-interop',
    'win32-wsa', 'simple', 'extern', 'fuzz-10',
    'funcs.fwd', 'funcs.single', 'funcs.stmts'
]

foreach case : pass
    where = case.replace('.', '/')
    path = dir/'pass'/where + '.ct'
    test('pass-' + case, ctc, 
        args : [ '-V', path, '-gen', 'c99' ],
        suite : 'ctu'
    )
endforeach
