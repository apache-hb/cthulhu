dir = meson.current_source_dir()

tests = {
    'values': {
        'pass': [
            'single-single',
            'single-multi',
            'multi-single',
            'multi-multi',
            'const-value'
        ],
        'fail': [ ]
    },
    'warns': {
        'pass': [ 'mod' ],
        'fail': [ ]
    },
    'functions': {
        'pass': [
            'no-return',
            'return-val',
            'return-void'
        ],
        'fail': [
            'return-type',
            'void-return',
            'void-return2'
        ]
    },
    'types': {
        'pass': [
            'alias',
            'builtins',
            'nested',
            'records',

            'arrays',
            'nested-arrays'
        ],
        'fail': [
            'invalid-type',
            'recursive-pointer',
            'recursive',

            'array-length-bool',
            'array-length-not-const',
            'array-size-zero',
            'array-length-wrong-sign',
            'array-length-too-big'
        ]
    },
    'programs': {
        'pass': [ 'test' ],
        'fail': [ ]
    }
}

load_lang = default_library == 'shared' ? [ obr_shared.full_path() ] : [ ]

foreach case, tests : tests
    passes = tests['pass']
    fails = tests['fail']

    where = case.replace('.', '/')

    foreach name : passes
        path = dir/where/'pass'/name + '.obr'
        id = 'pass-' + case + '-' + name
        test(id, harness,
            args : [ id ] + load_lang + [ path ],
            suite : 'obr'
        )
    endforeach

    foreach name : fails
        path = dir/where/'fail'/name + '.obr'
        id = 'fail-' + case + '-' + name
        test(id, harness,
            args : [ id ] + load_lang + [ path ],
            suite : 'obr',
            should_fail : true
        )
    endforeach
endforeach
