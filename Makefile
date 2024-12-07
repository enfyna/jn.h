default: main

CFLAGS=-Wall -Wextra -O0 -fsanitize=address,undefined,leak,integer -ggdb -pedantic

main: main.c
	clang $(CFLAGS) -o ./build/$@ $^
