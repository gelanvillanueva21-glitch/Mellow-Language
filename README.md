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

See [SPEC.md](SPEC.md) for the grammar and semantics. The next architectural
step is replacing the direct evaluator with an explicit AST and scoped object
model for dictionaries, indexing, closures, classes, modules, and structured
exception recovery.