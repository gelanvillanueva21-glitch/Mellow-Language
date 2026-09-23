." Mellow Language Reference
." Generated with groff -ms and ps2pdf
.TL
Mellow Language Reference
.AU
Core syntax, keywords, and built-in functions
.AI
Mellow Language project
.NH 1
1. Overview
.PP
Mellow is an English-influenced programming language implemented in C. Function
calls use angle brackets, code blocks use square brackets, and curly braces are
used for array data. Arithmetic and comparison are expressed with named
built-in functions rather than operators.
.PP
This reference describes the current executable core. Keywords marked
"reserved" are recognized by the lexer but are not yet executable in the
interpreter.
.NH 1
2. Basic Syntax
.PP
Function calls:
.DS
print<"Hello, Mellow!">
add<2, 3>
print<add<2, mul<3, 4>>>
.DE
.PP
Blocks:
.DS
if gt<score, 10> [
    print<"high score">
]
.DE
.PP
Comments use // for one line and /* ... */ for multiple lines. Newlines may be
used between arguments and delimiters in calls and collection literals.
.NH 1
3. Values and Variables
.TP
.B null
The absence of a value. The literal is .B null.
.TP
.B number
The current runtime stores integer and floating-point literals as C double
values. Examples: .B 42 and .B 3.14159.
.TP
.B string
Text enclosed in double quotes. Example: .B "hello".
.TP
.B char
A single character enclosed in single quotes. Character values currently use
the same string representation at runtime.
.TP
.B bool
The boolean literals .B true and .B false.
.TP
.B array
An ordered collection written with curly braces. Example: .B {1, 2, 3}.
.TP
.B list
An ordered dynamic collection created with .B list<...>. Example:
\fBlist<"red", "green", "blue">\fP.
.PP
\fBlet\fP defines a mutable variable:
.DS
let name = "Mellow"
set<name, "Mellow Language">
.DE
.PP
\fBconst\fP defines a variable that cannot be reassigned or passed as the first
argument to .B set:
.DS
const PI = 3.14159
.DE
.NH 1
4. Implemented Keywords
.TP
.B true
Creates the boolean value true.
.TP
.B false
Creates the boolean value false.
.TP
.B null
Creates the null value.
.TP
.B let
Declares a mutable variable using .B let name = expression.
.TP
.B const
Declares an immutable variable using .B const name = expression.
.TP
.B if
Executes a block when an expression is truthy. Truthy values are true,
nonzero numbers, nonempty strings, and nonempty collections.
.DS
if eq<name, "Mellow"> [
    print<"matched">
]
.DE
.TP
.B else
Provides the alternative block for .B if. Chained .B else if syntax is
supported.
.TP
.B while
Repeats a block while its condition is truthy.
.DS
while gt<count, 0> [
    print<count>
    set<count, dec<count>>
]
.DE
.TP
.B loop
Repeats a block indefinitely until .B break or a runtime error occurs.
.TP
.B break
Leaves the nearest loop.
.TP
.B continue
Skips to the next iteration of the nearest loop.
.TP
.B return
Returns a value from a named function. With no expression it returns null.
.TP
.B func
Defines a named function with parameters and a square-bracket body.
.DS
func square<x> [
    return mul<x, x>
]
print<square<5>>
.DE
.TP
.B raise
Raises a runtime exception and exits the current program with a nonzero status.
The usual form is .B raise<"message">.
.NH 1
5. Arithmetic Builtins
.PP
Arithmetic operators such as +, -, *, /, and % do not exist in Mellow.
.TP
.B add<a, b>
Returns the numeric sum. It also concatenates two strings or two collections of
the same kind.
.TP
.B sub<a, b>
Returns a minus b for numbers.
.TP
.B mul<a, b>
Returns a times b for numbers.
.TP
.B div<a, b>
Returns a divided by b. Division by zero is a runtime error.
.TP
.B mod<a, b>
Returns the floating-point remainder of a divided by b.
.TP
.B pow<a, b>
Returns a raised to the power b.
.TP
.B sqrt<a>
Returns the square root of a.
.TP
.B inc<a>
Returns a plus one.
.TP
.B dec<a>
Returns a minus one.
.NH 1
6. Comparison and Logic Builtins
.PP
Comparison operators such as ==, !=, <, >, <=, and >= do not exist as
operators. The following functions return booleans.
.TP
.B eq<a, b>
True when the values are equal, including recursive collection equality.
.TP
.B neq<a, b>
True when the values are not equal.
.TP
.B lt<a, b>
True when numeric a is less than numeric b.
.TP
.B gt<a, b>
True when numeric a is greater than numeric b.
.TP
.B lte<a, b>
True when numeric a is less than or equal to numeric b.
.TP
.B gte<a, b>
True when numeric a is greater than or equal to numeric b.
.TP
.B equal<a, b>
Readable alias for .B eq<a, b>.
.TP
.B not_equal<a, b>
Readable alias for .B neq<a, b>.
.TP
.B less<a, b>
Readable alias for .B lt<a, b>.
.TP
.B greater<a, b>
Readable alias for .B gt<a, b>.
.TP
.B less_equal<a, b>
Readable alias for .B lte<a, b>.
.TP
.B greater_equal<a, b>
Readable alias for .B gte<a, b>.
.TP
.B and<a, b>
Returns the boolean conjunction of a and b.
.TP
.B or<a, b>
Returns the boolean disjunction of a and b.
.TP
.B not<a>
Returns the boolean negation of a.
.NH 1
7. Collection and Conversion Builtins
.TP
.B list<items>
Creates a list from zero or more values.
.TP
.B len<value>
Returns the character count of a string or the item count of an array/list.
.TP
.B type<value>
Returns a string naming the runtime type: null, number, string, bool, array,
or list.
.TP
.B to_string<value>
Converts primitive values to text. Collections currently return a collection
placeholder string and are displayed directly by .B print.
.TP
.B print<values>
Prints each argument separated by a space, followed by a newline.
.TP
.B set<name, value>
Replaces the value of a mutable variable. The first argument is a variable name,
not an evaluated expression.
.NH 1
8. Interpolation
.PP
Expressions inside a string using {name} are replaced with the current value of
the named variable:
.DS
let user = "Ada"
print<"Hello, {user}!">
.DE
.PP
Interpolation currently supports variables containing strings, numbers, and
booleans. Unknown names remain unchanged.
.NH 1
9. Functions and Recursion
.PP
Named functions support parameters, local variables, return values, and
recursion. Arguments are evaluated before the function is called.
.DS
func factorial<n> [
    if lte<n, 1> [
        return 1
    ]
    return mul<n, factorial<dec<n>>>
]
print<factorial<5>>
.DE
.NH 1
10. Command-Line Tools
.TP
.B make
Builds the interpreter. Run it from the project root.
.TP
.B ./mellow file.mll
Executes a Mellow source file.
.TP
.B ./mellow --tokens file.mll
Prints the lexer token stream with source locations.
.TP
.B ./mellow --repl
Starts the interactive read-evaluate-print loop. Type .B exit to leave it.
.NH 1
11. Reserved Keywords
.PP
The following words are lexed and reserved for the next runtime layers:
\fBfor\fP, \fBclass\fP, \fBpublic\fP, \fBprivate\fP, \fBimport\fP, \fBtry\fP, \fBcatch\fP, \fBfn\fP, \fBthis\fP, \fBsuper\fP, and.
.PP
.B for
is reserved for collection iteration. .B class, .B public, and .B private are
reserved for the object system. .B this and .B super are reserved for method
access. .B import is reserved for modules. .B try and .B catch are reserved
for structured error recovery. .B fn is reserved for anonymous functions and
closures. These features are
specified in the project roadmap but are not executable in the current core.
.NH 1
12. Current Limitations
.PP
Dictionaries, indexing, default parameters, lambdas, closures, structured
try/catch recovery, modules, classes, inheritance, and garbage collection are
not yet implemented. The lexer recognizes much of their vocabulary so these
features can be added without changing the central call and block syntax.
.bp