# unit tests
subdir('unit')

pass = [
    'args', 'binary', 'call', 'func',
    'hello', 'hello2', 'implicit-bool-cast',
    'long', 'loop', 'mut', 'pointers', 'pointers2',
    'regalloc', 'var', 'suffix', 'global',
    'unused-var', 'implicit-var-cast', 'string', 'forward',
    'posix-interop', 'win32-interop', 'array-1', 'array-3',
    'fuzz-8', 'array-struct', 'fizzbuzz'
]

foreach it : pass 
    path = meson.current_source_dir()/'pass'/it + '.ct'
    test('pass-' + it, driver, args : [ 
        '--verbose', '--emit', '--c99',
        '--speed', path 
    ])
endforeach

test('pass-help', driver, args : [ '--help' ])

subdir('multiple')

# TODO: find a way to test init code
#test('init', driver, args : [ '--init' ])

warn = [
    'int-overflow', 'mod-by-zero', 'fuzz-7'
]

foreach it : warn 
    path = meson.current_source_dir()/'warn'/it + '.ct'
    test('warn-' + it, driver, args : [ 
        '--verbose', '--emit', '--c99',
        '--speed', path 
    ])
endforeach

## tests that should fail
test('fail-no-files', driver, args : [ '--verbose' ], 
    should_fail : true
)

fail = [ 
    'incorrect-return1',
    'incorrect-return2',
    'invalid-char', 'scope', 'bad-return',
    'infinite-struct', 'type-in-expr', 'fuzz-1',
    'return-local-ref', 'return-struct-ref',
    'array', 'array-2'
]

foreach it : fail 
    path = meson.current_source_dir()/'fail'/it + '.ct'
    test('fail-' + it, driver, 
        args : [ '--verbose', '--emit', '--speed', '--c99', path ],
        should_fail : true
    )
endforeach

crashes = [ 
    'fuzz-1', 'fuzz-2', 'fuzz-3', 'fuzz-4',
    'fuzz-5', 'fuzz-6', 'fuzz-7', 'fuzz-8',
    'fuzz-9'
]

foreach it : crashes 
    path = meson.current_source_dir()/'crashes'/it + '.ct'
    test('fail-' + it, driver, 
        args : [ '--verbose', '--emit', '--speed', '--c99', path ],
        should_fail : true
    )
endforeach
