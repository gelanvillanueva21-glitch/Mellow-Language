CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic -O2

SOURCES = src/mellow.c src/lexer.c src/runtime.c src/runtime/value.c src/utils/file.c

.PHONY: all clean test tokens

all: mellow

mellow: $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -lm -o $@

tokens: mellow
	./mellow --tokens test/main.mll

test: mellow
	./mellow test/main.mll
	./mellow test/feature_entrypoint.mll
	./mellow test/stdlib_features.mll
	sh scripts/check_deprecated_builtins.sh ./mellow

clean:
	rm -f mellow