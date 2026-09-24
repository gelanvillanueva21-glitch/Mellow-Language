let should_not_be_imported = "module variable"
print<"This module top-level print should not run during import.">

func welcome<name> [
    return add<"Welcome, ", name>
]

func triple<number> [
    return mul<number, 3>
]func welcome<name> [
    return add<"Welcome, ", name>
]
func triple<number> [
    return mul<number, 3>
]
