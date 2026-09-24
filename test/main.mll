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
	raise<"example error">
] catch<error> [
	print<"Handled: ", error>
]

let phrase = "  mellow language  "
print<upper<trim<phrase>>>
print<replace<trim<phrase>, "language", "runtime">>
print<join<split<trim<phrase>, " ">, "-">>