dir = meson.current_source_dir()
ctc = harness['ctu']

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
    },
    'types.array': {
        'fail': [
            'non-int-size'
        ],
        'pass': [
            'simple',
            '2d',
            'expr-size'
        ]
    },
    'funcs': {
        'fail': [
            'ret-invalid'
        ],
        'pass': [
            'decl-arg',
            'empty-body',
            'empty',
            'int-return',
            'many-args',
            'return-int',
            'type-return',
            'use-arg'
        ]
    }
}

cases = {
    'imports': {
        'fails': {
            'missing-import': [ 'main' ],
            'duplicate-name': [ 'main', 'lib', 'subdir/lib' ],
            'duplicate-import': [ 'main', 'lib' ]
        },
        'passes': { 
            'no-imports': [ 'main' ],
            'single-import': [ 'main', 'lib' ],
            'circular-import': [ 'main', 'left', 'right' ]
        }
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
        test('fail-' + case + '-' + name, ctc,
            args : [ path ],
            suite : 'ctu',
            should_fail : true
        )
    endforeach
endforeach

foreach name, data : cases
    fails = data['fails']
    passes = data['passes']

    foreach id, src : fails
        root = dir/name/'fail'/id
        sources = []

        foreach file : src
            sources += [ root/file + '.ct' ]
        endforeach

        test('fail-' + name + '-' + id, ctc,
            args : sources,
            suite : 'ctu',
            should_fail : true
        )
    endforeach

    foreach id, src : passes
        root = dir/name/'pass'/id
        sources = []

        foreach file : src
            sources += [ root/file + '.ct' ]
        endforeach

        test('pass-' + name + '-' + id, ctc,
            args : sources,
            suite : 'ctu'
        )
    endforeach
endforeach