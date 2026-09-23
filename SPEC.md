# Mellow Language Specification

This document describes the implemented Mellow core and the reserved syntax for
the planned production layers.

## Lexical grammar

```ebnf
program       = { statement } ;
statement     = declaration | control | expression [ ";" ] ;
declaration   = ( "let" | "const" ) identifier "=" expression
              | "func" identifier "<" [ parameters ] ">" block ;
parameters    = identifier { "," identifier } ;
block         = "[" { statement } "]" ;
control       = "if" expression block [ "else" ( "if" expression block | block ) ]
              | "while" expression block
              | "loop" block
              | "break" | "continue"
              | "return" [ expression ] ;
expression    = literal | identifier | call | array ;
call          = identifier "<" [ expression { "," expression } ] ">" ;
array         = "{" [ expression { "," expression } ] "}" ;
literal       = number | string | character | "true" | "false" | "null" ;
```

Whitespace and comments are ignored. Newlines are statement separators outside
calls and blocks, and are permitted anywhere inside calls and collection
literals. `+`, `-`, `*`, `/`, `%`, and comparison operators are not language
operators; named builtins provide those operations.

## Values and semantics

Mellow currently uses dynamic values: `null`, numbers, strings, booleans,
arrays, lists, and functions. `let` binds a mutable variable. `const` binds a
name that cannot be reassigned or passed to `set`. Variables are resolved in
the current function environment. Functions are first-class at the runtime
boundary and named functions support recursion.

Arrays use `{1, 2, 3}`. Lists use `list<1, 2, 3>`. Dictionaries are reserved
for the next collection layer because `{key: value}` needs a distinct value
representation and indexing semantics.

## Builtins

`print` writes values. `add`, `sub`, `mul`, `div`, `mod`, `pow`, `sqrt`, `inc`,
and `dec` operate on numbers; `add` also concatenates strings and same-kind
collections. `eq`, `neq`, `lt`, `gt`, `lte`, and `gte` return booleans.
`and`, `or`, and `not` combine booleans. `len`, `type`, and `to_string` inspect
values. `set<name, value>` mutates a variable. `raise<message>` reports a
runtime exception and exits with a nonzero status.

## Examples

```mellow
const greeting = "Hello"
let name = "Mellow"
print<"{greeting}, {name}!">

func factorial<n> [
    if lte<n, 1> [ return 1 ]
    return mul<n, factorial<dec<n>>>
]

print<factorial<5>>
```

## Reserved production syntax

The lexer reserves `class`, `public`, `private`, `this`, `super`, `import`,
`try`, and `catch`. The planned object model uses square-bracket class bodies,
single inheritance, dynamic method dispatch, and `init`/`free` lifecycle
methods. These tokens are accepted lexically but are not yet executable in the
current core runtime.