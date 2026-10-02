." Mellow Language Cheat Sheet
.TL
Mellow Language Cheat Sheet
.AU
Canonical builtins, keywords, libraries, and runtime rules
.AI
Mellow Language project
.NH 1
Quick syntax
.PP
Function calls use angle brackets:
.DS
print<"Hello, Mellow!">
add<2, 3>
if greater_than<score, 10> [
    print<"high score">
]
.DE
.PP
Blocks use square brackets. Comments use // and /* ... */.
.NH 1
Keywords and control flow
.TP
\fBtrue\fP / \fBfalse\fP
Boolean literals.
.TP
\fBnull\fP
Null value.
.TP
\fBlet\fP / \fBconst\fP
Declare variables. The form \fBlet\fP creates a variable that starts as null unless initialized.
.TP
\fBif\fP / \fBelse\fP
Conditional blocks.
.TP
\fBwhile\fP / \fBloop\fP / \fBbreak\fP / \fBcontinue\fP
Repeated execution and control.
.TP
\fBfor\fP
Iterate over arrays or lists with \fBfor x in collection [ ... ]\fP.
.TP
\fBreturn\fP
Return a value from a function or method.
.TP
\fBfunc\fP / \fBfn\fP
Define a named function.
.TP
\fBclass\fP
Declare a class with fields and methods.
.TP
\fBpublic\fP / \fBprivate\fP / \fBstatic\fP
Visibility and class-level data.
.TP
\fBimport\fP
Load declarations from another Mellow file.
.TP
\fBtry\fP / \fBcatch\fP
Handle runtime errors. \fBcatch<>\fP catches all; \fBcatch<ValueErr>\fP catches a specific type.
.TP
\fBraise\fP
Raise a runtime error with a message.
.TP
\fBthis\fP / \fBsuper\fP
Class instance and parent access.
.TP
\fBand\fP / \fBor\fP / \fBnot\fP
Boolean operators as named builtins.
.NH 1
Builtins: arithmetic
.TP
\fBadd<a, b>\fP
Adds numbers; concatenates strings or same-kind collections.
.TP
\fBsubtract<a, b>\fP
Numeric subtraction.
.TP
\fBmultiply<a, b>\fP
Numeric multiplication.
.TP
\fBdivide<a, b>\fP
Numeric division. Zero divisor raises \fBDivisionErr\fP.
.TP
\fBmodulo<a, b>\fP
Floating-point remainder.
.TP
\fBpower<a, b>\fP
Raise a to the power b.
.TP
\fBsqrt<a>\fP
Square root.
.TP
\fBinc<a>\fP
Return a + 1.
.TP
\fBdec<a>\fP
Return a - 1.
.PP
Deprecated aliases such as \fBsub\fP, \fBmul\fP, \fBdiv\fP, \fBmod\fP,
\fBpow\fP, \fBeq\fP, \fBlt\fP, \fBgt\fP, \fBlte\fP, \fBgte\fP,
\fBless\fP, \fBgreater\fP, \fBsum\fP, \fBminus\fP, \fBtimes\fP,
\fBquotient\fP, and \fBremainder\fP are rejected with a \fBValueErr\fP that points to the canonical replacement.
.NH 1
Builtins: comparisons and logic
.TP
\fBequal<a, b>\fP
True when values are equal.
.TP
\fBnot_equal<a, b>\fP
True when values differ.
.TP
\fBless_than<a, b>\fP
Numeric less-than test.
.TP
\fBgreater_than<a, b>\fP
Numeric greater-than test.
.TP
\fBless_equal<a, b>\fP
Numeric less-than-or-equal test.
.TP
\fBgreater_equal<a, b>\fP
Numeric greater-than-or-equal test.
.TP
\fBand<a, b>\fP
Boolean conjunction.
.TP
\fBor<a, b>\fP
Boolean disjunction.
.TP
\fBnot<a>\fP
Boolean negation.
.TP
\fBinside<value, container>\fP
Checks for string membership, array/list membership, or dictionary key presence. \fBcontains\fP is an alias.
.NH 1
Builtins: strings and text
.TP
\fBtrim<string>\fP
Remove surrounding whitespace.
.TP
\fBupper<string>\fP
Uppercase conversion.
.TP
\fBlower<string>\fP
Lowercase conversion.
.TP
\fBreplace<text, old, new>\fP
Replace all occurrences of a substring.
.TP
\fBstarts_with<text, prefix>\fP
Check a prefix.
.TP
\fBends_with<text, suffix>\fP
Check a suffix.
.TP
\fBsplit<text, delimiter>\fP
Split a string into a list.
.TP
\fBjoin<list, delimiter>\fP
Join a list of strings with a delimiter.
.TP
\fBinput<>\fP
Read one line from stdin.
.TP
\fBinput<"Prompt: ">\fP
Print a prompt, then read input.
.TP
\fBto_number<string|number>\fP
Convert text to a number.
.TP
\fBto_float<string|number>\fP
Convert to a floating-point number.
.TP
\fBto_int<string|number>\fP
Convert and truncate to an integer-valued number.
.TP
\fBto_bool<value>\fP
Convert true/false, 1/0, or string booleans.
.NH 1
Builtins: collections and values
.TP
\fBlist<item, item, ...>\fP
Create a list.
.TP
\fBdict<>\fP
Create an empty dictionary.
.TP
\fBget<container, key>\fP
Read a dictionary entry or collection index.
.TP
\fBhas<container, key>\fP
Check whether a dictionary key or collection index exists.
.TP
\fBput<dictionary, key, value>\fP
Add or replace a dictionary entry.
.TP
\fBlen<value>\fP
String length or collection size.
.TP
\fBtype<value>\fP
Return the runtime type name.
.TP
\fBto_string<value>\fP
Convert a value to text.
.TP
\fBrange<start, end[, step]>\fP
Create a numeric list ending before \fBend\fP.
.TP
\fBset<name, value>\fP
Assign a mutable variable.
.TP
\fBprint<value, ...>\fP
Print values separated by spaces and finish with a newline.
.TP
\fBdestroy<instance>\fP
Destroy an instance and call its \fBfree<>\fP method if present.
.NH 1
Builtins: numeric helpers and predicates
.TP
\fBabs<a>\fP
Absolute value.
.TP
\fBfloor<a>\fP
Floor value.
.TP
\fBceil<a>\fP
Ceiling value.
.TP
\fBround<a>\fP
Rounded value.
.TP
\fBmin<a, b>\fP
Smaller numeric value.
.TP
\fBmax<a, b>\fP
Larger numeric value.
.TP
\fBclamp<value, min, max>\fP
Clamp to range.
.TP
\fBis_null<value>\fP
Check for null.
.TP
\fBis_number<value>\fP
Check for number.
.TP
\fBis_string<value>\fP
Check for string.
.TP
\fBis_array<value>\fP
Check for array.
.TP
\fBis_list<value>\fP
Check for list.
.TP
\fBis_dict<value>\fP
Check for dictionary.
.NH 1
Error types
.TP
\fBValueErr\fP
General value or runtime mismatch error.
.TP
\fBRecurErr\fP
Recursion-related error.
.TP
\fBDivisionErr\fP
Division-by-zero or invalid division path.
.TP
\fBSyntaxErr\fP
Syntax or parser-level issue.
.PP
A \fBcatch<error: ValueErr>\fP style handler binds a message and filters by type.
.NH 1
Collection methods
.PP
These methods work on lists and dictionaries:
.TP
\fBlist.push<value>\fP
Append one item.
.TP
\fBlist.pop<>\fP
Remove and return the last value.
.TP
\fBlist.get<index>\fP
Read an item by index.
.TP
\fBlist.set<index, value>\fP
Replace an item by index.
.TP
\fBlist.remove<index>\fP
Remove an item by index.
.TP
\fBlist.clear<>\fP
Empty the list.
.TP
\fBlist.has<value>\fP
Check whether a list contains the value.
.TP
\fBdict.get<key>\fP
Read a dictionary value by string key.
.TP
\fBdict.put<key, value>\fP
Set or replace a dictionary entry.
.TP
\fBdict.has<key>\fP
Check whether a string key exists.
.TP
\fBdict.remove<key>\fP
Remove and return a key/value pair.
.TP
\fBdict.keys<>\fP
Return a list of keys.
.TP
\fBdict.values<>\fP
Return a list of values.
.TP
\fBdict.clear<>\fP
Empty the dictionary.
.NH 1
Import and libraries
.PP
Mellow loads other files with \fBimport\fP:
.DS
import "math.mll" <random>
import "path/to/file.mll" <hello, Parser>
.DE
.PP
The current runtime supports import-based code loading and builtins such as
\fBrandom<>\fP and \fBrand<>\fP. The active implementation does not require a
separate predeclared module namespace for list or dictionary helpers; those are
provided by collection builtins and methods.
.NH 1
Examples
.DS
let name = "Mellow"
let total = add<2, multiply<3, 4>>
print<"Total: ", total>

try [
    divide<5, 0>
] catch<err: DivisionErr> [
    print<"division failed: ", err>
]
.DE
.PP
The canonical naming style is long-form and explicit: \fBadd\fP, \fBsubtract\fP,
\fBmultiply\fP, \fBdivide\fP, \fBmodulo\fP, \fBequal\fP,
\fBgreater_than\fP, \fBless_than\fP, \fBgreater_equal\fP,
\fBless_equal\fP, and \fBnot_equal\fP.
.BM
Created for the Mellow language reference and quick lookup.
.EM
