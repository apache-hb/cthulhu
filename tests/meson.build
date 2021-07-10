pass = [
    'args', 'binary', 'call', 'func',
    'hello', 'hello2', 'implicit-bool-cast',
    'long', 'loop', 'mut', 'pointers', 'pointers2',
    'regalloc', 'var'
]

foreach it : pass 
    path = meson.current_source_dir()/'pass'/it + '.ct'
    test('pass-' + it, driver, args : [ path ])
endforeach