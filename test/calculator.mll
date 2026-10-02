

class Calculator [
    let firstNumber
    let secondNumber

    func init <
        firstNumber, 
        secondNumber
    > [
        set<this.firstNumber, firstNumber>
        set<this.secondNumber, secondNumber>
    ]

    func getSum<> -> Number [
        return add<this.firstNumber, this.secondNumber>
    ]

    func getDifference<> -> Number [
        return subtract<this.firstNumber, this.secondNumber>
    ]

    func getProduct<> -> Number [
        return multiply<this.firstNumber, this.secondNumber>
    ]

    func getQuotient<> -> Number [
        if equal<this.secondNumber, 0> [
            return 0
        ]
        return divide<this.firstNumber, this.secondNumber>
    ]

    func getModulos<> -> Number [
        return modulo<this.firstNumber, this.secondNumber>
    ]

]


func checkOperator<operator, calculator> -> Nothing [
    if equal<operator, "+"> [
        let result = calculator.getSum<>
        print<"Sum: {result}">
    ] 
    
    else if equal<operator, "-"> [
        let result = calculator.getDifference<>
        print<"Difference: {result}">
    ] 
    
    else if equal<operator, "*"> [
        let result = calculator.getProduct<>
        print<"Product: {result}">
    ] 
    
    else if equal<operator, "/"> [
        let result = calculator.getQuotient<>
        print<"Quotient: {result}">
    ] 
    
    else [
        let result = calculator.getModulos<>
        print<"Modulos: {result}">
    ]
]


func main<> -> Nothing [
    try [
        let firstNumber = to_number<input<"Enter First number: ">>
        let secondNumber = to_number<input<"Enter Second Number: ">>

        let operator = input<"Enter Operator[+, -, *, /, %]: ">
        if not<inside<operator, {"+", "-", "*", "/", "%"}>> [
            raise<"Must input an operator">
        ]

        let calculator = Calculator<firstNumber, secondNumber>
        checkOperator<operator, calculator>
    ] 
    catch <validationError> [
        print<"Validation Error: {validationError}">
    ] 
    catch <conversionError> [
        print<"Conversion Error: {conversionError}">
    ] 

    catch <err> [
        print<"Error: {err}">
    ]

]
