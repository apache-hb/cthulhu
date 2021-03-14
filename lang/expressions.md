# Expressions

## Operator precedence

| Precedence | Operator |    Description     | Associativity |
| :--------- | :------: | :----------------: | ------------: |
| 1          |   `::`   | `Scope resolution` | left to right |
| 2          |   `()`   |  `Function call`   | right to Left |
| 2          |   `[]`   |    `Subscript`     | right to left |
| 2          | `.` `->` |  `Member access`   | right to left |
| 3          | `*` `/` `%` | `Multiplication`, `Division`, `Modulo` | left to right
| 4          | `+` `-` | `Addition`, `Subtraction` | left to right
| 5          | `<<` `>>` | `Bitwise shift left`, `Bitwise shift right` | left to right
| 6 | `<` `<=` `>` `>=` | `Relational comparison` | left to right
| 7 | `==` `!=` | `Equality comparison` | left to right
| 8 | `&` | `Bitwise and` | left to right
| 9 | `^` | `Bitwise xor` | left to right
| 10 | `\|` | `Bitwise or` | left to right
| 11 | `&&` | `Logical and` | left to right
| 12 | `||` | `Logical or` | left to right
| 13 | `?:` | `Ternary` | right to left
