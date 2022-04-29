# TODO items

## Blocking items

* complete wat support
  * how do strings work

* add wasm support
  * run wat2wasm on the output is not great

* document more stuff
  * other people have to contribute now

* extract most of `generic` into subprojects
  * better seperation of code into more logical blocks
  * makes it easier to generate variations of the compiler
  * required for plugins

* create plugin system
  * this would allow our secret sauce to be kept closed source
  * while the majority of the framework would be open source
    * other people can fix our bugs for us
    * brownie points for being open source
    * means we can use libraries like libgccjit
    * my work will remain available to me even after the summer ends
  * this would also allow for codegen shims to be added in
    * makes gathering metrics possible and exensible

* get this all running inside aws lambda
  * i dont know how s3 buckets work
  * i am too scared of spending my life savings in 5 seconds

* properly abstract file IO
  * current file IO doesnt work on windows
  * need to abstract over in memory files and on disk files better
  * need to create distinction between binary files and text files

* general code cleanup
  * this is a forever task

## Optional items

* add ssa emitter
  * direct asm/wasm output would be preferrable to relying on C/wat2wasm

* optimisation passes
  * more speed never hurts
  * would get us on the compiler framework map so to speak
  * would save us money in the long run with lambda runtimes

* static analysis support
  * makes it easier to find code defects
  * helps optimisations
  * if we get a C frontend it will help us find defects in the compiler itself

* rework command line parsing
  * instead of manually scanning the command line we should use flex & bison
    * a touch overkill perhaps
    * much easier to add/remove flags
    * would allow drivers to add their own flags
  * also allow passing other formats as flags
    * xml
    * json
    * yaml (ideally not this)

* make util functions more verbose
  * its getting a bit hard to reason about what some functions are doing based on name alone now

* move over to an existing style guide
  * the vulkan one looks decent
  * maybe microsoft

* properly prefix the entire compiler
  * will make writing plugins easier in the future
