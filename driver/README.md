# Language driver layout

all drivers must conform to the same dir structure

* driver_name/
    * src/
    * include/
    * meson.build

## build file requirements

to be detected by interfaces each build file must add itself to `langs`

```py
src = [ 'src/main.c' ]

ex = library('example', src,
    dependencies : [ mediator, interop, tree ],
    c_args : user_args,
    include_directories : [ 'src' ]
)

langs += {
    'example': {
        'dep': declare_dependency(link_with : ex), # dependency object
        'mod': 'kExampleModule', # module definition exported from the object

        # an optional fuzz corpus to opt in to fuzzing tools
        'fuzz_corpus': meson.current_source_dir() / 'fuzz_corpus'
    }
}
```
