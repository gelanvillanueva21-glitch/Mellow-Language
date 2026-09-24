let user = {"name": "Gelan", "age": 25}
put<user, "role", "developer">

print<get<user, "name">>
print<get<user, "role">>
print<has<user, "age">>
print<len<user>>
print<inside<"name", user>>

let total = 0
for number in range<1, 5> [
	set<total, add<total, number>>
]

print<"range total: ", total>
print<"has 3: ", inside<3, range<1, 5>>>