CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic -O2

SOURCES = src/mellow.c src/lexer.c src/runtime.c src/utils/file.c

.PHONY: all clean test tokens

all: mellow

mellow: $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -lm -o $@

tokens: mellow
	./mellow --tokens test/main.mll

test: mellow
	./mellow test/main.mll

clean:
	rm -f mellow