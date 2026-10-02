#!/bin/sh
set -eu

mellow=${1:-./mellow}
temporary_directory=$(mktemp -d "${TMPDIR:-/tmp}/mellow-deprecated.XXXXXX")
trap 'rm -rf "$temporary_directory"' EXIT HUP INT TERM

while read -r old_name replacement; do
    [ -n "$old_name" ] || continue
    printf '%s<10, 3>\n' "$old_name" > "$temporary_directory/case.mll"

    if "$mellow" "$temporary_directory/case.mll" > "$temporary_directory/output" 2>&1; then
        printf "Expected deprecated builtin '%s' to fail\n" "$old_name" >&2
        exit 1
    fi

    expected="builtin '$old_name' is deprecated; use '$replacement' instead"
    if ! grep -F "$expected" "$temporary_directory/output" >/dev/null; then
        printf "Expected diagnostic not found for '%s':\n" "$old_name" >&2
        cat "$temporary_directory/output" >&2
        exit 1
    fi
done <<'ALIASES'
sub subtract
mul multiply
div divide
mod modulo
pow power
eq equal
neq not_equal
lt less_than
gt greater_than
lte less_equal
gte greater_equal
less less_than
greater greater_than
sum add
addition add
minus subtract
times multiply
quotient divide
remainder modulo
ALIASES

printf '%s\n' 'deprecated builtin checks passed'
