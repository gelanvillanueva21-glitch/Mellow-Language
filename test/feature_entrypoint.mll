raise<"top-level code should not run when main exists">

public func main<> -> Nothing [
	func fractional<> -> Number [
		return 2.75
	]
	if not<equal<fractional<>, 2.75>> [
		raise<"modified main or Number return annotation failed">
	]
	print<fractional<>>
]