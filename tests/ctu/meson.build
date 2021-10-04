crashes = [ 
    'fuzz-1', 'fuzz-2', 'fuzz-3', 'fuzz-4', 'fuzz-5',
    'fuzz-6', 'fuzz-7', 'fuzz-8', 'fuzz-9', 'fuzz-10',
    'overflow.1', 'overflow.2', 'overflow.3', 'overflow.4', 'overflow.5'
]

foreach case : crashes
    where = case.replace('.', '/')
    path = meson.current_source_dir()/'crashes'/where + '.ct'
    test('crash-' + case, driver, args : [ '-V', path ], should_fail : true)
endforeach
