fuzzers = {}

foreach name, lib : langs
    interface = executable(name, 'interface/main.c',
        dependencies : [ generic, lib ]
    )

    fuzzers += { name: interface }
endforeach
