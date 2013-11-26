
LDLIBS = -lcurl
LDDEPS = -Ideps -Isrc
TESTS = $(wildcard test/*.c)
SOURCES = $(wildcard src/*.c)
SOURCES += $(wildcard deps/*.c)

test: $(TESTS)
	@$(foreach test, $(TESTS), ./$(basename $(test)) && echo " ✓ $(basename $(test))";)

$(TESTS):
	$(CC) -std=c99 $(SOURCES) $@ $(LDLIBS) $(LDDEPS) -o $(basename $@)

.PHONY: test $(TESTS)
