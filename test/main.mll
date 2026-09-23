const app = "Mellow Report"
let student = "Gelan"

func celsius_to_fahrenheit<celsius> [
    return add<mul<celsius, div<9, 5>>, 32>
]

func grade_label<score> [
    if greater_equal<score, 90> [
        return "excellent"
    ] else if greater_equal<score, 70> [
        return "good"
    ] else [
        return "keep practicing"
    ]
]

let scores = {92, 84, 76}
let temperatures = list<0, 20, 37>
let score = 84
let fahrenheit = celsius_to_fahrenheit<20>
let label = grade_label<score>

print<"{app} for {student}">
print<"score: ", score>
print<"assessment: ", label>
print<"scores recorded: ", len<scores>>
print<"temperatures recorded: ", len<temperatures>>
print<"20 C in Fahrenheit: ", fahrenheit>

if and<greater_equal<score, 50>, less<score, 100>> [
    print<"The score is valid.">
]

let reminders = 3
while greater<reminders, 0> [
    print<"reminder ", reminders>
    set<reminders, dec<reminders>>
]


