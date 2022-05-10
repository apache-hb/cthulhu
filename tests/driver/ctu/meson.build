dir = meson.current_source_dir()
ctc = harness['ctc']

tests = {
    'types.alias': {
        'fail': [ 
            'discard-alias-name', 
            'infinite-size-alias',
            'looped-pointer-alias',
            'multiple-pointer-loops',
            'duplicate-names',
            'pointer-loop'
        ],
        'pass': [
            'chained-aliases',
            'index-alias',
            'multiple-aliases',
            'single-alias'
        ]
    },
    'types.struct': {
        'fail': [
            'alias-to-self',
            'infinite-size',
            'no-elements',
            'discard-struct',
            'duplicate-element'
        ],
        'pass': [
            'multiple-elements',
            'mutural-recursion',
            'recursive',
            'single-element',
            'discard-element'
        ]
    },
    'types.union': {
        'fail': [
            'discard-union',
            'no-elements',
            'duplicate-element',
            'infinite-size'
        ],
        'pass': [
            'multiple-elements',
            'single-element'
        ]
    },
    'types.closure': {
        'fail': [
            'void-param'
        ],
        'pass': [
            'no-params',
            'one-param',
            'many-params',
            'return-closure'
        ]
    },
    'types.variant': {
        'fail': [
            'duplicate-name',
            'discard-name'
        ],
        'pass': [
            'many-elements',
            'one-element',
            'no-elements'
        ]
    }
}

foreach case, tests : tests
    passes = tests['pass']
    fails = tests['fail']

    where = case.replace('.', '/')

    foreach name : passes
        path = dir/where/'pass'/name + '.ct'
        test('pass-' + case + '-' + name, ctc,
            args : [ path ],
            suite : 'ctu'
        )
    endforeach

    foreach name : fails
        path = dir/where/'fail'/name + '.ct'
        test('pass-' + case + '-' + name, ctc,
            args : [ path ],
            suite : 'ctu',
            should_fail : true
        )
    endforeach
endforeach
