dir = meson.current_source_dir()
plugins = {
    'init-plugin': true,
    'no-info': false
}

foreach name, should_pass : plugins
    src = dir / name + '.c'
    item = shared_module(name, src,
        dependencies : generic
    )

    test('test-' + name, pl0c,
        args : [ item.full_path(), '-Werror' ],
        suite : 'plugin',
        should_fail : not should_pass
    )
endforeach
