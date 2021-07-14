# unit tests
subdir('unit')

pass = [
    'args', 'binary', 'call', 'func',
    'hello', 'hello2', 'implicit-bool-cast',
    'long', 'loop', 'mut', 'pointers', 'pointers2',
    'regalloc', 'var', 'suffix'
]

foreach it : pass 
    path = meson.current_source_dir()/'pass'/it + '.ct'
    test('pass-' + it, driver, args : [ 
        '--verbose', '--emit', 
        '--speed', path 
    ])
endforeach

test('pass-help', driver, args : [ '--help' ])

subdir('multiple')

# TODO: find a way to test init code
#test('init', driver, args : [ '--init' ])

warn = [
    'int-overflow'
]

foreach it : warn 
    path = meson.current_source_dir()/'warn'/it + '.ct'
    test('warn-' + it, driver, args : [ 
        '--verbose', '--emit', 
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
    'invalid-char'
]

foreach it : fail 
    path = meson.current_source_dir()/'fail'/it + '.ct'
    test('fail-' + it, driver, 
        args : [ '--verbose', '--emit', '--speed', path ],
        should_fail : true
    )
endforeach
