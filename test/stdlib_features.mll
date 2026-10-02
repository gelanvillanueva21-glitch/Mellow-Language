let items = list<1, 2, 3>
items.push<4>
if not<equal<items.len<>, 4>> [
    raise<"list push/len failed">
]
if not<equal<len<items>, 4>> [
    raise<"len should work for lists">
]
if not<equal<items.get<2>, 3>> [
    raise<"list get failed">
]
items.remove<0>
if not<equal<items.len<>, 3>> [
    raise<"list remove failed">
]

let scores = dict<>
scores["player"] = "Mellow"
if not<equal<scores.get<"player">, "Mellow">> [
    raise<"dictionary set/get failed">
]
if not<equal<scores.len<>, 1>> [
    raise<"dictionary len failed">
]
if not<equal<len<scores>, 1>> [
    raise<"len should work for dictionaries">
]
if not<equal<len<"Hello World!">, 12>> [
    raise<"len should work for strings">
]

let value = random<>
if not<and<greater_equal<value, 0>, less_equal<value, 1>>> [
    raise<"random must be in the [0, 1] range">
]

if not<equal<subtract<10, 3>, 7>> [
    raise<"subtract should use the long-form builtin name">
]
if not<equal<multiply<4, 5>, 20>> [
    raise<"multiply should use the long-form builtin name">
]
if not<equal<divide<12, 3>, 4>> [
    raise<"divide should use the long-form builtin name">
]
if not<equal<modulo<10, 3>, 1>> [
    raise<"modulo should use the long-form builtin name">
]
if not<equal<power<2, 3>, 8>> [
    raise<"power should use the long-form builtin name">
]
if not<greater_than<5, 3>> [
    raise<"greater_than should use the long-form builtin name">
]
if not<less_than<3, 5>> [
    raise<"less_than should use the long-form builtin name">
]
if not<not_equal<5, 6>> [
    raise<"not_equal should use the long-form builtin name">
]

print<"stdlib checks passed">
