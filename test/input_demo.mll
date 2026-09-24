let raw = input<"Enter a number: ">
let amount = to_number<raw>
let answer = add<amount, 0.5>
print<"Answer: ", answer>
let flag = input<"Enter a flag: ">
print<"Boolean: ", to_bool<flag>>
