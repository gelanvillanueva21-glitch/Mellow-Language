# Mellow Language

Mellow is an English-influenced language implemented in C. Its defining syntax is:

- Function and method calls use angle brackets: `print<"hello">`.
- Code blocks use square brackets: `if condition [ print<"yes"> ]`.
- Curly braces are data literals, never code blocks.
- Arithmetic and comparison are named builtins, not operators.

## Current milestone

The project currently contains a source-aware lexer and the first interpreter
slice. Build and run it with:

```sh
make
./mellow --tokens test/main.mll
./mellow test/main.mll
./mellow --repl
```

The lexer supports identifiers, numbers, strings, chars, keywords, comments,
newlines, call delimiters, block delimiters, collection punctuation, and
assignment. The runtime supports dynamic values, optionally initialized
variables and fields, named functions, classes with static members and
visibility, checked return annotations, arrays, lists, dictionaries, loops,
imports, typed `try`/`catch`, and automatic `main<>` execution. Calls use angle
brackets, blocks use square brackets, and arithmetic/comparison use named
builtins. `--tokens` remains available as a development diagnostic mode, and
`--repl` starts an interactive session.

Current limitations include indexing syntax, default parameters, anonymous
functions, closures, and automatic garbage collection. See
[SPEC.md](SPEC.md) for the implemented grammar and semantics.

## Design decisions

Mellow uses dynamic runtime values. Optional function return annotations are
checked when a function or method returns; they do not make variable bindings
statically typed.

Arrays use `{1, 2, 3}`, lists use `list<1, 2, 3>`, and non-empty dictionaries
use string-keyed colon pairs such as `{"name": "Alice"}`. Empty `{}` is an
array; `dict<>` creates an empty dictionary. Use `get`, `has`, and `put` to
access and mutate dictionary entries.

The current execution pipeline is:

```text
source -> lexer -> direct evaluator -> runtime values
```

## Source layout

```text
src/
	lexer.c, lexer.h       tokenization and source locations
	mellow.c               command-line entry point and REPL
	runtime.c              statement execution and expression dispatch
	runtime/
		value.c, value.h     value ownership, collections, equality, display
		model.h              runtime state, variables, functions, classes, and instances
	utils/
		file.c, file.h       source file loading
```

The current runtime supports class declarations, `init` constructors, fields,
`this` access, methods, single inheritance, `super`, and `destroy` with optional
`free` destructors. The next architecture step is an explicit environment
module for nested lexical scopes, followed by visibility, static members, and
automatic garbage collection.

Modules are currently loaded with `import "path/to/file.mll" <name, OtherClass>`.

Terminal input is available through `input<>`. It always starts as text, then
can be converted explicitly:

```mellow
let raw = input<"Enter a number: ">
let amount = to_number<raw>
print<add<amount, 10>>
```

Text expressions can be evaluated with normal MDAS precedence:

```mellow
let expression = input<"Expression: ">
print<evaluate<expression>>
```

Collections support dictionaries, lookup, mutation, ranges, and iteration:

```mellow
let scores = {"Gelan": 90, "Mellow": 85}
put<scores, "Ada", 95>
for score in range<1, 4> [
	print<score>
]
```

Errors and string manipulation are also available:

```mellow
try [
	raise<"invalid input">
] catch<error> [
	print<"Handled: ", error>
]

print<replace<trim<"  mellow  ">, "mellow", "Mellow">>
```
Only selected functions and classes are imported; module variables and top-level
executable statements are ignored. The imported declarations execute in the
same runtime, and paths are relative to the process working directory. The
legacy `import "path/to/file.mll" <>` form imports every function and class.

Inside classes, `private func` methods are accessible from the declaring class
and subclasses, but not from outside callers or unrelated classes. Nested
classes do not automatically receive private access.

Terminal input is available through `input<>`. Input starts as text and can be
converted explicitly:

```mellow
let raw = input<"Enter a number: ">
let amount = to_number<raw>
print<add<amount, 10>>
```

See [SPEC.md](SPEC.md) for the grammar and semantics. An explicit AST and
lexical-scope model are possible future architecture improvements.