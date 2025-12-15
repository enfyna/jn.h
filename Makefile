default: example compile

CFLAGS=-Wall -Wextra -O0 -fsanitize=address,undefined,leak,integer -ggdb -pedantic

BUILD_PATH=build

example: example.c $(BUILD_PATH)
	clang $(CFLAGS) -o $(BUILD_PATH)/$@ $<

compile: compile.c $(BUILD_PATH)
	clang $(CFLAGS) -o $(BUILD_PATH)/$@ $<

build:
	mkdir -p ./$@

.PHONY=clean
clean:
	rm build/example
