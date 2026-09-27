

class Calculator [
    let firstNumber = 0
    let secondNumber = 0

    func init <
        firstNumber, 
        secondNumber
    > [
        set<this.firstNumber, firstNumber>
        set<this.secondNumber, secondNumber>
    ]

    func getSum<> [
        return add<this.firstNumber, this.secondNumber>
    ]

    func getDifference<> [
        return sub<this.firstNumber, this.secondNumber>
    ]

    func getProduct<> [
        return mul<this.firstNumber, this.secondNumber>
    ]

    func getQuotient<> [
        if eq<this.secondNumber, 0> [
            return 0
        ]
        return div<this.firstNumber, this.secondNumber>
    ]

    func getModulos<> [
        return mod<this.firstNumber, this.secondNumber>
    ]

]


func checkOperator<operator, calculator> [
    if eq<operator, "+"> [
        let result = calculator.getSum<>
        print<"Sum: {result}">
    ] else if eq<operator, "-"> [
        let result = calculator.getDifference<>
        print<"Difference: {result}">
    ] else if eq<operator, "*"> [
        let result = calculator.getProduct<>
        print<"Product: {result}">
    ] else if eq<operator, "/"> [
        let result = calculator.getQuotient<>
        print<"Quotient: {result}">
    ] else [
        let result = calculator.getModulos<>
        print<"Modulos: {result}">
    ]
]


try [
    let firstNumber = to_number<input<"Enter First number: ">>
    let secondNumber = to_number<input<"Enter Second Number: ">>

    let operator = input<"Enter Operator[+, -, *, /, %]: ">
    if not<inside<operator, {"+", "-", "*", "/", "%"}>> [
        raise<"Must input an operator">
    ]

    let calculator = Calculator<firstNumber, secondNumber>
    checkOperator<operator, calculator>
] catch <err> [
    print<"Error: {err}">
]

