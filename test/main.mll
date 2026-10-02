func mdas<> [
	// 1 multiply 3 add 7 divide 2 subtract 8 add 8
	// Multiplication and division are grouped first.
	let multiplied = multiply<1, 3>
	let divided = divide<7, 2>
	let additions = add<multiplied, divided>
	let without_eight = subtract<additions, 8>
	return add<without_eight, 8>
]

print<"Mellow MDAS expression result:">
print<mdas<>>

try [
	print<"Hello World">
	raise<"example error">
] catch<error> [
	print<"Handled: ", error>
]

let phrase = "  mellow language  "
print<upper<trim<phrase>>>
print<replace<trim<phrase>, "language", "runtime">>
print<join<split<trim<phrase>, " ">, "-">>

let firstNumber
if not<is_null<firstNumber>> [
	raise<"bare let must initialize to null">
]
set<firstNumber, 42>
if not<is_number<firstNumber>> [
	raise<"variable must accept a number after declaration">
]
set<firstNumber, "forty-two">
if not<is_string<firstNumber>> [
	raise<"variable must accept a different value type">
]

let emptyDictionary = dict<>
if not<is_dict<emptyDictionary>> [
	raise<"dict<> must create an empty dictionary">
]
	if not<equal<len<emptyDictionary>, 0>> [
	raise<"a new dictionary must be empty">
]
put<emptyDictionary, "answer", 42>
	if not<equal<get<emptyDictionary, "answer">, 42>> [
	raise<"dictionary entries must be readable after insertion">
]

let caughtValueError = false
try [
	raise<"expected value error">
] catch<DivisionErr> [
	raise<"wrong catch handler ran">
] catch<error: ValueErr> [
	set<caughtValueError, true>
] catch<> [
	raise<"later catch handler must not run">
]
if not<caughtValueError> [
	raise<"matching catch handler did not run">
]

public func typedNumber<> -> Number [
	return 3.5
]
public func typedNothing<> -> Nothing [
	return
]
	if not<equal<typedNumber<>, 3.5>> [
	raise<"Number return type must accept fractional values">
]
typedNothing<>

func wrongReturn<> -> String [
	return 12
]
let caughtReturnError = false
try [
	wrongReturn<>
] catch<returnError: ValueErr> [
	set<caughtReturnError, true>
]
if not<caughtReturnError> [
	raise<"return type mismatch must raise ValueErr">
]

class Counter [
	private let hidden = "private"
	public let label = "before"
	static let total = 1
	private static func increment<> -> Number [
		set<Counter.total, inc<Counter.total>>
		return Counter.total
	]
	public static func next<> -> Number [
		return Counter.increment<>
	]
	public func reveal<> -> String [
		return this.hidden
	]
]
let counter = Counter<>
	if not<equal<Counter.next<>, 2>> [
	raise<"static method must update static field">
]
	if not<equal<counter.reveal<>, "private">> [
	raise<"class methods must access private fields">
]
set<counter.label, "after">
	if not<equal<counter.label, "after">> [
	raise<"public instance fields must be mutable through set">
]
let caughtPrivateError = false
try [
	counter.hidden
] catch<privateError: ValueErr> [
	set<caughtPrivateError, true>
]
if not<caughtPrivateError> [
	raise<"private field access must be rejected outside the class">
]
let caughtPrivateWrite = false
try [
	set<counter.hidden, "exposed">
] catch<privateWriteError: ValueErr> [
	set<caughtPrivateWrite, true>
]
if not<caughtPrivateWrite> [
	raise<"set must not bypass private field access">
]