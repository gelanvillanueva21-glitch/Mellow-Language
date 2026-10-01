func mdas<> [
	// 1 mul 3 add 7 div 2 sub 8 add 8
	// Multiplication and division are grouped first.
	let multiplied = mul<1, 3>
	let divided = div<7, 2>
	let additions = add<multiplied, divided>
	let without_eight = sub<additions, 8>
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
if not<eq<len<emptyDictionary>, 0>> [
	raise<"a new dictionary must be empty">
]
put<emptyDictionary, "answer", 42>
if not<eq<get<emptyDictionary, "answer">, 42>> [
	raise<"dictionary entries must be readable after insertion">
]