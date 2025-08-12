default: example

CFLAGS=-Wall -Wextra -O0 -fsanitize=address,undefined,leak,integer -ggdb -pedantic

example: example.c
	clang $(CFLAGS) -o ./build/$@ $^

.PHONY=clean
clean:
	rm build/example
