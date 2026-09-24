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

Readable comparison aliases are also supported: `equal`, `not_equal`, `less`,
`greater`, `less_equal`, and `greater_equal`.

`print` writes values. `add`, `sub`, `mul`, `div`, `mod`, `pow`, `sqrt`, `inc`,
and `dec` operate on numbers; `add` also concatenates strings and same-kind
collections. `eq`, `neq`, `lt`, `gt`, `lte`, and `gte` return booleans.
`and`, `or`, and `not` combine booleans. `abs`, `min`, `max`, `clamp`, `floor`,
`ceil`, and `round` provide common numeric helpers. `is_null`, `is_number`,
`is_string`, `is_array`, and `is_list` return type predicates. `len`, `type`, and `to_string` inspect
values. `inside<value, container>` returns only a boolean and checks whether a
string is contained in a string, or a value is equal to an item in an array or
list. `contains` is an alias. `set<name, value>` mutates a variable. `raise<message>` reports a
runtime exception and exits with a nonzero status.

`input<>` reads one line from the terminal and returns it as a string.
`input<"Prompt: ">` prints a prompt first. `to_number` and `to_float` parse
numeric text, `to_int` truncates to an integer-valued number, and `to_bool`
accepts `true`, `false`, `1`, or `0`.

## Modules and imports

An import executes declarations from another Mellow file in the same runtime.
Use an angle-bracket selector to import only named functions or classes;
variables and top-level executable statements cannot be imported.
Paths are resolved from the process working directory, so subfolders can be
referenced directly:

```mellow
import "test/lib/greetings.mll" <welcome>
print<welcome<"Mellow">>
```

`import "path.mll" <>` keeps the legacy form and imports every function/class
declaration. Imported files may import other files. Import cycles are not yet
detected.

## Private methods

Inside a class, `private func` declares a private method. A private method may
be called by methods of its declaring class or subclasses, including through
`this.method<>` or `super.method<>`. Calls from outside an instance and calls
from unrelated classes fail. Nested classes do not inherit access automatically.

`get<container, key>` reads a dictionary key or array/list index. `has<container,
key>` returns whether that key or index exists. `put<dictionary, key, value>`
adds or replaces a dictionary entry. `range<start, end[, step]>` returns a
list of numbers with an exclusive end. `for item in collection [...]` iterates
arrays and lists.

`input<>` reads one terminal line and returns it as a string. An optional
`input<"Prompt: ">` prints a prompt first. `to_number` and `to_float` parse
numeric text, `to_int` truncates it, and `to_bool` accepts `true`, `false`, `1`,
or `0`.

`evaluate<text>` parses a text expression containing numeric values and the
words `add`, `sub`, `mul`, and `div`. It applies normal MDAS precedence, so
`1 mul 3 add 7 div 2 sub 8 add 8` evaluates multiplication and division before
addition and subtraction. `calculate_expression` is an alias.

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

## Errors and strings

Errors can be recovered with `try` and `catch`. The catch variable receives the
error message:

```mellow
try [
    raise<"invalid value">
] catch<error> [
    print<"Handled: ", error>
]
```

String helpers include `trim`, `upper`, `lower`, `replace`, `starts_with`,
`ends_with`, `split`, and `join`:

```mellow
let phrase = "  mellow language  "
let words = split<trim<phrase>, " ">
print<join<words, "-">>
```

## Object-oriented programming

Classes use square-bracket bodies. Fields are declared with `let`, constructors
use `init`, and methods use the same angle-bracket call syntax as functions.

```mellow
class Person [
    let name = ""
    func init<n> [ set<this.name, n> ]
    func greet<> [ print<"Hello, ", this.name> ]
]

let person = Person<"Ada">
person.greet<>
```

Single inheritance, inherited fields, dynamic method lookup, overriding, and
parent calls through `super.method<>` are supported. `destroy<instance>` calls
an optional `free<>` method before releasing the instance.

## Reserved production syntax

The lexer reserves `public`, `private`, `import`, `try`, and `catch`. Visibility
modifiers, static members, modules, structured exception recovery, and automatic
garbage collection are not yet executable.