dir = meson.current_source_dir()

tests = {
    'consts': {
        'pass': [
            'chain-init',
            'case-insensitive-lookup',
            'many-elements',
            'single-element'
        ],
        'fail': [
            'case-insensitive',
            'complex-loop',
            'duplicate-names',
            'loop-init',
            'simple-loop-init'
        ]
    },
    'values': {
        'pass': [
            'multiple-names',
            'single-name'
        ],
        'fail': [
            'case-insensitive-duplicate',
            'duplicate-name',
            'shadow-const'
        ]
    },
    'procedures': {
        'pass': [
            'call',
            'mutual-recurse',
            'recurse'
        ],
        'fail': [
            'case-insensitive',
            'proc-shadow',
            'shadow-const',
            'shadow-local',
            'shadow-var'
        ]
    },
    'programs': {
        'pass': [
            'prime',
            'square',
            'empty'
        ],
        'fail': [
            'crash'
        ]
    }
}

crashes = {
    '0': false, '1': false, '2': false,
    '3': false, '4': false, '5': false, '6': false,
    '7': false, '8': false, '9': false, '10': false,
    '11': false, '12': false, '13': false, '14': false,
    '15': false, '16': false,
    '17': false, '18': false,
    '19': false, '20': true, '21': false, '22': false,
    '23': false, '24': false, '25': false, '26': false,
    '27': true, '28': true, '29': true, '30': false,
    '31': false
}

multi = {
    'multi.0': {
        'pass': true,
        'files': [ 'lib', 'main' ],
    },
    'multi.1': {
        'pass': true,
        'files': [ 'lhs', 'rhs', 'main' ]
    },
    'multi.multiple-entries': {
        'pass': false,
        'files': [ 'main1', 'main2' ]
    }
}

foreach case, tests : tests
    passes = tests['pass']
    fails = tests['fail']

    where = case.replace('.', '/')

    foreach name : passes
        path = dir/where/'pass'/name + '.pl0'
        id = 'pass-' + case + '-' + name
        test(id, harness,
            args : [ id, path ],
            suite : 'pl0'
        )
    endforeach

    foreach name : fails
        path = dir/where/'fail'/name + '.pl0'
        id = 'fail-' + case + '-' + name
        test(id, harness,
            args : [ id, path ],
            suite : 'pl0',
            should_fail : true
        )
    endforeach
endforeach

foreach case, cfg : multi
    paths = []
    foreach file : cfg['files']
        paths += [ dir/case.replace('.', '/')/file + '.pl0' ]
    endforeach

    id = 'pass-multi-' + case
    test(id, harness,
        args : [ id ] + paths,
        suite : 'pl0',
        should_fail : not cfg['pass']
    )
endforeach

foreach case, success : crashes
    path = dir/'crashes'/case + '.pl0'
    id = 'crash-' + case
    test(id, harness,
        args : [ id, path ],
        suite : 'pl0',
        should_fail : not success
    )
endforeach
