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
This reference describes the current executable core. The limitations section
lists features that are not yet implemented.
.NH 1
2. Basic Syntax
.PP
Function calls:
.DS
print<"Hello, Mellow!">
add<2, 3>
print<add<2, multiply<3, 4>>>
.DE
.PP
Blocks:
.DS
if greater_than<score, 10> [
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
Numeric values such as .B 42 and .B 3.14159.
.TP
.B string
Text enclosed in double quotes, such as .B "hello".
.TP
.B char
A single character enclosed in single quotes. Character values use the same
string representation at runtime.
.TP
.B bool
The boolean literals .B true and .B false.
.TP
.B array
An ordered collection written with curly braces, such as \fB{1, 2, 3}\fP. Empty
\fB{}\fP is an empty array.
.TP
.B list
An ordered collection created with .B list<...>.
.TP
.B dictionary
A string-keyed mapping written with colon pairs, such as
\fB{"name": "Ada"}\fP. Use .B dict<> for an empty dictionary.
.PP
\fBlet\fP defines a mutable variable. Without an initializer it starts as
.B null:
.DS
let name = "Mellow"
let unset
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
Declares a mutable variable using .B let name or .B let name = expression.
.TP
.B const
Declares an immutable variable using .B const name or .B const name = expression.
.TP
.B if
Executes a block when an expression is truthy. Truthy values are true,
nonzero numbers, nonempty strings, and nonempty collections.
.DS
if equal<name, "Mellow"> [
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
while greater_than<count, 0> [
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
    return multiply<x, x>
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
.B subtract<a, b>
Returns a minus b for numbers.
.TP
.B multiply<a, b>
Returns a times b for numbers.
.TP
.B divide<a, b>
Returns a divided by b. Division by zero is a runtime error.
.TP
.B modulo<a, b>
Returns the floating-point remainder of a divided by b.
.TP
.B power<a, b>
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
.PP
The retired names \fBsub\fP, \fBmul\fP, \fBdiv\fP, \fBmod\fP, and
\fBpow\fP raise a \fBValueErr\fP and recommend \fBsubtract\fP,
\fBmultiply\fP, \fBdivide\fP, \fBmodulo\fP, and \fBpower\fP, respectively.
The retired names \fBsum\fP, \fBaddition\fP, \fBminus\fP, \fBtimes\fP,
\fBquotient\fP, and \fBremainder\fP also raise a \fBValueErr\fP.
.NH 1
6. Comparison and Logic Builtins
.PP
Comparison operators such as ==, !=, <, >, <=, and >= do not exist as
operators. The following functions return booleans.
.TP
.B equal<a, b>
True when the values are equal, including recursive collection equality.
.TP
.B not_equal<a, b>
True when the values are not equal.
.TP
.B less_than<a, b>
True when numeric a is less than numeric b.
.TP
.B greater_than<a, b>
True when numeric a is greater than numeric b.
.TP
.B less_equal<a, b>
True when numeric a is less than or equal to numeric b.
.TP
.B greater_equal<a, b>
True when numeric a is greater than or equal to numeric b.
.PP
The retired names \fBeq\fP, \fBneq\fP, \fBlt\fP, \fBgt\fP, \fBlte\fP,
\fBgte\fP, \fBless\fP, and \fBgreater\fP raise a \fBValueErr\fP and
recommend their corresponding canonical comparison names.
.TP
.B and<a, b>
Returns the boolean conjunction of a and b.
.TP
.B or<a, b>
Returns the boolean disjunction of a and b.
.TP
.B not<a>
Returns the boolean negation of a.
.TP
.B inside<value, container>
Returns only a boolean. For strings, it checks whether the first string occurs
inside the second. For arrays and lists, it checks whether an equal item exists.
For dictionaries, it checks whether the string key exists. .B contains is an
alias for .B inside.
.NH 1
7. Collection and Conversion Builtins
.TP
.B list<items>
Creates a list from zero or more values.
.TP
.B len<value>
Returns the character count of a string or the item count of an array, list, or
dictionary.
.TP
.B type<value>
Returns a string naming the runtime type: null, number, string, bool, array,
list, dictionary, or instance.
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
.TP
dict<>, get<container, key>, has<container, key>, put<dictionary, key, value>
Read a dictionary key or collection index, test whether it exists, or add and
replace a dictionary entry. .B dict<> creates an empty dictionary, .B get
returns .B null for a missing key, and dictionary keys must be strings.
.TP
.B range<start, end, step>
Creates a numeric list with an exclusive end. The step is optional and defaults
to one.
.TP
for item in collection [ ... ]
Iterates over an array or list.
.TP
.B input<> or input<"prompt">
Reads one terminal line as a string. An optional string prompt is printed before
reading.
.TP
.B to_number<value>, to_float<value>, to_int<value>
Convert numeric text. .B to_int truncates the numeric value.
.TP
.B to_bool<value>
Converts booleans, nonzero numbers, or the strings true, false, 1, and 0.
.TP
.B trim<text>, upper<text>, lower<text>
Remove surrounding whitespace or change string case.
.TP
.B replace<text, old, new>
Replace every occurrence of old with new.
.TP
starts_with<text, prefix>, ends_with<text, suffix>
Return a boolean for prefix and suffix checks.
.TP
split<text, delimiter>, join<list, delimiter>
Split a string into a list or join a string collection into one string.
.NH 1
9. Errors
.PP
One .B try block may have multiple consecutive .B catch clauses. Only the first
matching handler runs. A named catch variable receives the error message;
\fBcatch<>\fP catches without binding;
\fBcatch<DivisionErr>\fP filters by type; \fBcatch<error: DivisionErr>\fP
filters and binds. Error types are \fBValueErr\fP, \fBRecurErr\fP,
\fBDivisionErr\fP, and \fBSyntaxErr\fP. \fBraise<message>\fP creates a
runtime error.
.TP
.B input<> or input<"prompt">
Reads one terminal line as a string. An optional string prompt is printed before
reading.
.TP
.B to_number<value>, to_float<value>, to_int<value>
Convert numeric text. .B to_int truncates the numeric value.
.TP
.B to_bool<value>
Converts booleans, nonzero numbers, or the strings true, false, 1, and 0.
.TP
abs<a>, min<a, b>, max<a, b>, clamp<a, low, high>
Common numeric helpers. abs returns magnitude; min and max select a number;
clamp limits a value to a range.
.TP
.B floor<a>, ceil<a>, round<a>
Return the corresponding rounded numeric value.
.TP
is_null<a>, is_number<a>, is_string<a>, is_array<a>, is_list<a>, is_dict<a>
Return a boolean indicating whether a value has the requested runtime type.
.NH 1
9. Modules and Imports
.PP
An import executes declarations from another Mellow file in the same runtime.
Use an angle-bracket selector to import only named functions or classes;
variables and top-level executable statements are ignored. Paths are relative
to the process working directory and may name files in subfolders.
.DS
import "test/lib/greetings.mll" <welcome>
print<welcome<"Mellow">>
.DE
.PP
The legacy form .B import "path.mll" <> imports every function and class.
Imported files may import other files. Import cycles are not yet detected.
.NH 1
10. Private Methods
.PP
Inside a class, .B private func declares a private method. It may be called by
methods of its declaring class or subclasses, including through .B this.method<>
or .B super.method<>. External callers and unrelated classes are rejected.
Nested classes do not automatically receive private access.
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
    if less_equal<n, 1> [
        return 1
    ]
    return multiply<n, factorial<dec<n>>>
]
print<factorial<5>>
.DE
.PP
Functions and methods may declare a runtime-checked return type after their
parameter list: \fB-> String\fP, \fB-> Number\fP, \fB-> Boolean\fP, or
\fB-> Nothing\fP. .B Number accepts integral and fractional numbers. .B Nothing
requires a null result. Without an annotation, return values remain dynamic.
.NH 1
10. Object-Oriented Programming
.PP
Classes use square-bracket bodies. Fields use .B let and default to .B null
without an initializer. Constructors use the method name .B init, and methods
use angle-bracket calls.
.DS
class Person [
    let name = ""
    func init<n> [
        set<this.name, n>
    ]
    func greet<> [
        print<"Hello, ", this.name>
    ]
]
let person = Person<"Ada">
person.greet<>
.DE
.PP
Single inheritance uses .B class Student : Person [...]. Inherited fields,
dynamic method lookup, overriding, and parent calls through .B super.method<>
are supported. .B destroy<instance> calls an optional .B free<> method before
releasing the instance.
.PP
Visibility defaults to .B public. .B private fields and methods are restricted
to their declaring class; private top-level functions and classes are limited
to their source file. .B static fields and methods are accessed through the
class name, for example .B Counter.total and .B Counter.next<>.
.NH 1
11. Command-Line Tools
.TP
.B make
Builds the interpreter from the project root.
.TP
.B ./mellow file.mll
Executes a Mellow source file.
.TP
.B ./mellow --tokens file.mll
Prints the token stream with source locations.
.TP
.B ./mellow --repl
Starts the interactive REPL. Type .B exit to leave it.
.NH 1
12. Program Entry Point
.PP
If a top-level .B func main<> [...] exists, declarations and imports are
collected and .B main<> runs automatically. Other top-level executable
statements are skipped in this mode. Without .B main, the program executes
top-to-bottom.
.NH 1
13. Current Limitations
.PP
Indexing syntax, default parameters, lambdas, closures, and automatic garbage
collection are not yet implemented.
.bp