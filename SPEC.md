# Mellow Language Specification

This document describes the syntax and behavior implemented by the current
Mellow interpreter.

## Lexical grammar

```ebnf
program       = { statement } ;
statement     = declaration | control | class_decl | import_stmt
              | expression [ ";" ] ;
declaration   = ( "let" | "const" ) identifier [ "=" expression ]
              | "func" identifier "<" [ parameters ] ">" block ;
parameters    = identifier { "," identifier } ;
block         = "[" { statement } "]" ;
control       = "if" expression block [ "else" ( "if" expression block | block ) ]
              | "while" expression block
              | "loop" block
              | "for" identifier "in" expression block
              | "try" block "catch" "<" [ catch_spec ] ">" block
              | "break" | "continue"
              | "return" [ expression ] ;
class_decl    = "class" identifier [ ":" identifier ] "["
                { field_decl | method_decl } "]" ;
field_decl    = "let" identifier [ "=" expression ] ;
method_decl   = [ "private" ] "func" identifier "<" [ parameters ] ">" block ;
import_stmt   = "import" string "<" [ identifier { "," identifier } ] ">" ;
catch_spec    = identifier [ ":" error_type ] | error_type ;
error_type    = "ValueErr" | "RecurErr" | "DivisionErr" | "SyntaxErr" ;
expression    = literal | identifier | call | collection | member_access ;
call          = identifier "<" [ expression { "," expression } ] ">" ;
member_access = identifier "." identifier [ "<" [ arguments ] ">" ] ;
arguments     = expression { "," expression } ;
collection    = "{" [ expression { "," expression } ] "}" ;
literal       = number | string | character | "true" | "false" | "null" ;
```

Newlines and semicolons separate statements. Newlines are permitted between
call arguments and inside collection literals. An `else` or `catch` belongs to
the immediately preceding block; blank lines are allowed, but another statement
cannot intervene. `+`, `-`, `*`, `/`, `%`, and comparison operators are not
language operators; named builtins provide those operations.

## Values and semantics

Mellow uses dynamic values: `null`, numbers, strings, booleans, arrays, lists,
dictionaries, and class instances. `let name` initializes a mutable variable to
`null`; `let name = expression` initializes it to that expression's value.
Variables can later hold values of any type. `const` accepts the same optional
initializer but cannot be reassigned. `set<name, value>` assigns a new value to
a mutable variable. Class fields follow the same `let` rules. Named functions
support recursion.

Arrays use `{1, 2, 3}` and lists use `list<1, 2, 3>`. A non-empty dictionary
literal uses string keys and colons, for example `{"name": "Ada"}`. Empty
braces `{}` mean an empty array; use `dict<>` for an empty dictionary.

## Builtins

Readable comparison aliases are also supported: `equal`, `not_equal`, `less`,
`greater`, `less_equal`, and `greater_equal`.

`print` writes values. `add`, `sub`, `mul`, `div`, `mod`, `pow`, `sqrt`, `inc`,
and `dec` operate on numbers; `add` also concatenates strings and same-kind
collections. `eq`, `neq`, `lt`, `gt`, `lte`, and `gte` return booleans.
`and`, `or`, and `not` combine booleans. `abs`, `min`, `max`, `clamp`, `floor`,
`ceil`, and `round` provide common numeric helpers. `is_null`, `is_number`,
`is_string`, `is_array`, `is_list`, and `is_dict` return type predicates. `len`,
`type`, and `to_string` inspect values. `get<container, key>` reads a dictionary
entry or collection index; `has<container, key>` tests for an entry or index;
`put<dictionary, key, value>` adds or replaces a dictionary entry. Dictionary
keys must be strings. `dict<>` creates an empty dictionary. `inside` and its
alias `contains` test strings, arrays, lists, or dictionary keys.
`set<name, value>` mutates a variable. `raise<message>` reports a runtime
exception and exits with a nonzero status unless caught.

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

Errors can be recovered with `try` and `catch`. Catch-all and typed forms are
supported. A named catch variable receives the error message:

```mellow
try [
    raise<"invalid value">
] catch<error> [
    print<"Handled: ", error>
]
```

Use `catch<>` for an unbound catch-all, `catch<DivisionErr>` to handle only a
particular error type, or `catch<error: DivisionErr>` to filter and bind the
message. Available types are `ValueErr`, `RecurErr`, `DivisionErr`, and
`SyntaxErr`.

String helpers include `trim`, `upper`, `lower`, `replace`, `starts_with`,
`ends_with`, `split`, and `join`:

```mellow
let phrase = "  mellow language  "
let words = split<trim<phrase>, " ">
print<join<words, "-">>
```

## Object-oriented programming

Classes use square-bracket bodies. Fields are declared with `let` and start as
`null` when no initializer is provided. Constructors use `init`, and methods
use the same angle-bracket call syntax as functions.

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

## Program entry point

If a top-level `func main<> [...]` exists, declarations and imports are
collected first and `main<>` is called automatically. Other top-level
executable statements are skipped in this mode. Without `main`, statements run
from top to bottom.

## Current limitations

Indexing syntax, default parameters, anonymous functions, closures, static
members, and automatic garbage collection are not yet implemented.