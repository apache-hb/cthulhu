langtests = { }

foreach name, info : langs
    subdir(name)
endforeach

foreach langname, info : langtests
    simple = info.get('tests', {})
    crashes = info.get('crashes', {})
    modules = info.get('modules', {})

    langdir = meson.current_source_dir() / langname

    foreach feature, data : simple
        pass = data.get('pass', {})
        fail = data.get('fail', {})

        testdir = langdir / feature
        foreach name, path : pass
            where = testdir / 'pass' / path + '.' + info['ext']
            test(feature + ' ' + name, harness,
                args : [ path, where ],
                suite : [ langname, 'pass' ]
            )
        endforeach

        foreach name, path : fail
            where = testdir / 'fail' / path + '.' + info['ext']
            test(feature + ' ' + name, harness,
                args : [ path, where ],
                suite : [ langname, 'fail' ],
                should_fail : true
            )
        endforeach
    endforeach

    foreach name, testconfig : modules
        paths = []
        foreach file : testconfig.get('files', [])
            paths += [ langdir / 'multi' / testconfig['dir'] / file + '.' + info['ext'] ]
        endforeach

        where = testdir / path + '.' + info['ext']
        test(langname + ' modules ' + name, harness,
            args : [ name.replace(' ', '-') ] + paths,
            suite : [ langname, 'module' ],
            should_fail : testconfig.get('should_fail', false)
        )
    endforeach

    foreach case, success : crashes
        where = langdir / 'crashes' / case + '.' + info['ext']
        test(langname + ' crash ' + case, harness,
            args : [ 'crash-' + case, where ],
            suite : [ langname, 'crash' ],
            should_fail : not success
        )
    endforeach
endforeach
