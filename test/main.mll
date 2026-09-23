const title = "Mellow demo"
let name = "Gelan"

func factorial<n> [
    if lte<n, 1> [
        return 1
    ]

    return mul<n, factorial<dec<n>>>
]

let numbers = {1, 2, 3, 4, 5}
let fruits = list<"apple", "banana", "cherry">
let answer = factorial<5>

print<"{title} for {name}">
print<"5 factorial is {answer}">
print<"numbers: ", numbers>
print<"fruit count: ", len<fruits>>

if gte<answer, 100> [
    print<"The answer is large.">
] else [
    print<"The answer is small.">
]

let countdown = 3
while gt<countdown, 0> [
    print<"countdown: ", countdown>
    set<countdown, dec<countdown>>
]


