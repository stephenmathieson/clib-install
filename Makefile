
CC ?= cc
BIN ?= clib-install
PREFIX ?= /usr/local
CFLAGS = -std=c99 -Wall -Ideps -Isrc
LDFLAGS = -lcurl
SRC = $(filter-out src/main.c, $(wildcard src/*.c))
SRC += $(wildcard deps/*/*.c)

$(BIN): $(SRC) src/main.c
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $(BIN)

test: clean $(BIN)
	./$(BIN) --out tmp --dev --quite stephenmathieson/clib-package visionmedia/mon

clean:
	rm -f $(BIN)
	$(foreach test, $(TESTS), rm -f $(basename $(test));)

install: $(BIN)
	mkdir -p $(PREFIX)/bin
	cp -f $(BIN) $(PREFIX)/bin/

uninstall:
	rm -f $(PREFIX)/bin/$(BIN)

.PHONY: test clean install uninstall
