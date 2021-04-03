# Expressions

## Operator precedence

| Precedence |     Operator      |                 Description                 | Associativity |
| :--------- | :---------------: | :-----------------------------------------: | ------------: |
| 1          |       `::`        |             `Scope resolution`              | left to right |
| 2          |       `()`        |               `Function call`               | right to Left |
| 2          |       `[]`        |                 `Subscript`                 | right to left |
| 3          |     `.` `->`      |               `Member access`               | right to left |
| 4          |    `*` `/` `%`    |   `Multiplication`, `Division`, `Modulo`    | left to right |
| 5          |      `+` `-`      |          `Addition`, `Subtraction`          | left to right |
| 6          |     `<<` `>>`     | `Bitwise shift left`, `Bitwise shift right` | left to right |
| 7          | `<` `<=` `>` `>=` |           `Relational comparison`           | left to right |
| 8          |     `==` `!=`     |            `Equality comparison`            | left to right |
| 9          |        `&`        |                `Bitwise and`                | left to right |
| 10         |        `^`        |                `Bitwise xor`                | left to right |
| 11         |       `\|`        |                `Bitwise or`                 | left to right |
| 12         |       `&&`        |                `Logical and`                | left to right |
| 13         |      `\|\|`       |                `Logical or`                 | left to right |
| 14         |       `?:`        |                  `Ternary`                  | right to left |

## Function call expressions

function calls can be invoked on any expression that evaluates to a `closure` type.
* named parameters may only be used when invoking functions without indirection
* after a named parameter is used no additional positional parameters may be used
* invoking a `closure` may only be done with positional parameters, in addition default arguments will not be provided as this information is erased when taking a reference to a function
  * named arguments appear as `.ident = expr`
  * positional arguments appear as `expr`
  * expressions are evaluated in the order they are declared at the callsite. not the order they are declared at the function definition

## ASM expressions

`asm` expressions allow users to embed assembly instructions directly into functions
* assembly expressions in the form of `asm(reg)` evaluate to the current value of `reg` with a type relevant to `reg`
  * assigning to this expression will preserve its value for the remainder of the current scope.
  * assigning from this expression will leave its value unchanged.
  * `asm(reg)` expressions may also appear in the format of `asm!(seg:reg)` on platforms where segmentation is available.
* assembly expressions in the form of `asm {}` may contain an list of valid assembly opcodes
  * valid opcodes change depending on target platform
  * instructions are destination first then source
  * variable substitution is performed with `$name`. i.e. `asm! { mov rax, $num }`
  * block expressions evaluate to `void`
