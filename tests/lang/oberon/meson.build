dir = meson.current_source_dir()

tests = {
    'values': {
        'pass': [
            'single-single',
            'single-multi',
            'multi-single',
            'multi-multi'
        ],
        'fail': [

        ]
    },
    'warns': {
        'pass': [
            'mod'
        ],
        'fail': [

        ]
    }
}

foreach case, tests : tests
    passes = tests['pass']
    fails = tests['fail']

    where = case.replace('.', '/')

    foreach name : passes
        path = dir/where/'pass'/name + '.obr'
        id = 'pass-' + case + '-' + name
        test(id, harness,
            args : [ id, path ],
            suite : 'obr'
        )
    endforeach

    foreach name : fails
        path = dir/where/'fail'/name + '.obr'
        id = 'fail-' + case + '-' + name
        test(id, harness,
            args : [ id, path ],
            suite : 'obr',
            should_fail : true
        )
    endforeach
endforeach
