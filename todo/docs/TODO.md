# TODO items

## Urgent items

* wasm backend
  * this is going to need ssa

* get rid of hlir parent data from the frontend
  * belongs either in the middle or backend
  * probably collected in the backend with some utility provided by common tools

## Blocking items

* take logging in more places

* custom allocator support

* timing support

* custom flex + bison to support generating texmate/vim/emacs grammars
  * this will be a big one
  * should also have pretty big payoff
  * may even be able to drop them both as deps

* rework location tracking for reporting
  * allow nested underlines and multiple location spans

* rework logging 
  * need custom sinks for multiple interfaces
  * will these be intergrated with interfaces or a seperate entity

* document more stuff
  * other people have to contribute now

* extract most of `generic` into subprojects
  * better seperation of code into more logical blocks
  * makes it easier to generate variations of the compiler

* create plugin system
  * this would allow our secret sauce to be kept closed source
  * while the majority of the framework would be open source
    * other people can fix our bugs for us
    * brownie points for being open source
    * means we can use libraries like libgccjit
  * this would also allow for codegen shims to be added in
    * makes gathering metrics possible and exensible

* general code cleanup
  * this is a forever task

* make hlir reentrant
  * this will be an important thing to test so reentrant code is needed
  * this will also make it allocator aware

* add ssa emitter
  * direct asm/wasm output would be preferrable to relying on C/wat2wasm

* add better coverage support
  * either here in tree or into meson
  * its pretty shit and cant be configured right now

* extract the command line parser out into its own library
  * this means all the tools can share the same parser
  * blocked by extracting generic

## Optional items

* better debug tools

* more test coverage

* dedicated fuzzing frontends

* optimisation passes
  * more speed never hurts
  * would get us on the compiler framework map so to speak
  * would save us money in the long run with lambda runtimes

* static analysis support
  * makes it easier to find code defects
  * helps optimisations
  * if we get a C frontend it will help us find defects in the compiler itself

* properly prefix the entire compiler
  * will make writing plugins easier in the future