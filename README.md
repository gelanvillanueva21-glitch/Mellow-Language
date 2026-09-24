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
newlines, call delimiters, block delimiters, collection punctuation, assignment,
and pipelines. The runtime currently supports literals, `let` and `const`,
variables, nested angle-bracket calls, `print`, `add`, `sub`, `mul`, `div`,
`mod`, `pow`, `sqrt`, `inc`, `dec`, all named comparisons and logical
functions, `len`, `type`, `to_string`, `raise`, arrays, lists, `if`, `else`,
`while`, `loop`, `break`, `continue`, named functions, recursion, `return`,
and string interpolation. `--tokens` remains available as a development
diagnostic mode, and `--repl` starts an interactive session.

The remaining production-grade layers are dictionaries and indexing, default
parameters, lambdas/closures, `for` iteration, structured `try/catch`, modules,
and the object system with inheritance and dynamic dispatch. They require the
runtime to move from the current direct evaluator to an AST and scoped object
model; the current implementation keeps its behavior testable while that
larger transition is built.

## Design decisions

Mellow uses dynamic runtime values with optional type annotations planned for a
later static-checking pass. This keeps closures, modules, and dynamic dispatch
straightforward in the first interpreter.

Because arrays and lists cannot be distinguished from identical `{...}` syntax,
the grammar uses `{1, 2, 3}` for arrays and `list<1, 2, 3>` for lists. A mapping
literal is recognized by its colon pairs, for example `{"name": "Alice"}`.

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

Collections support dictionaries, lookup, mutation, ranges, and iteration:

```mellow
let scores = {"Gelan": 90, "Mellow": 85}
put<scores, "Ada", 95>
for score in range<1, 4> [
	print<score>
]
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

See [SPEC.md](SPEC.md) for the grammar and semantics. The next architectural
step is replacing the direct evaluator with an explicit AST and scoped object
model for dictionaries, indexing, closures, classes, modules, and structured
exception recovery.