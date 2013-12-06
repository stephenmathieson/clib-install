
CC ?= cc
BIN ?= clib-install
CFLAGS = -std=c99 -Wall -Ideps -Isrc
LDFLAGS = -lcurl
SRC = $(filter-out src/main.c, $(wildcard src/*.c))
SRC += $(wildcard deps/*.c)
TESTS = $(wildcard test/*.c)

build: SRC += src/main.c
build: $(BIN)

$(BIN): $(SRC)
	$(CC) $(SRC) $(CFLAGS) $(LDFLAGS) -o $(BIN)

test: clean $(TESTS)

$(TESTS):
	$(CC) $@ $(SRC) $(CFLAGS) $(LDFLAGS) -o $(basename $@)
	./$(basename $@)

clean:
	$(foreach test, $(TESTS), rm -f $(basename $(test));)

.PHONY: test $(TESTS)
