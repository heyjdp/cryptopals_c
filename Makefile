CC ?= cc
CFLAGS ?= -Wall -Wextra -O2
CPPFLAGS ?=

HEADER_DIR := header
LIB_DIR := lib
TOOLS_DIR := tools
TESTS_DIR := tests
BUILD_DIR := build
BIN_DIR := bin
TEST_BIN_DIR := $(BIN_DIR)/tests
CRYPT_DIR := cryptopals

CPPFLAGS += -I$(HEADER_DIR)

LIBS := utils hex2b64 fixed_xor score_english_hex
TOOLS := hex2b64 fixed_xor
TESTS := hex2b64 fixed_xor utils
CRYPT_SOURCES := $(wildcard $(CRYPT_DIR)/*.c)
CRYPT_TARGETS := $(patsubst $(CRYPT_DIR)/%.c,$(BIN_DIR)/cryptopals_%,$(CRYPT_SOURCES))

LIB_OBJS := $(patsubst %, $(BUILD_DIR)/lib/%.o, $(LIBS))
TOOL_OBJS := $(patsubst %, $(BUILD_DIR)/tools/%_main.o, $(TOOLS))
TOOL_BINS := $(patsubst %, $(BIN_DIR)/%, $(TOOLS))
TEST_OBJS := $(patsubst %, $(BUILD_DIR)/tests/test_%.o, $(TESTS))
TEST_BINS := $(patsubst %, $(TEST_BIN_DIR)/test_%, $(TESTS))

.SECONDARY: $(LIB_OBJS) $(TOOL_OBJS) $(TEST_OBJS)

all: $(TOOL_BINS) $(CRYPT_TARGETS)

.PHONY: all build tools tests test cryptopals clean docs

all:
	$(MAKE) clean
	$(MAKE) build
	$(MAKE) docs
	$(MAKE) test

build: $(TOOL_BINS) $(CRYPT_TARGETS)

tools: $(TOOL_BINS)

cryptopals: $(CRYPT_TARGETS)

$(BIN_DIR)/%: $(BUILD_DIR)/tools/%_main.o $(BUILD_DIR)/lib/%.o $(BUILD_DIR)/lib/utils.o $(BUILD_DIR)/lib/fixed_xor.o $(BUILD_DIR)/lib/score_english_hex.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/tools/%_main.o: $(TOOLS_DIR)/%_main.c | $(BUILD_DIR)/tools
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/lib/%.o: $(LIB_DIR)/%.c $(HEADER_DIR)/%.h | $(BUILD_DIR)/lib
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/lib $(BUILD_DIR)/tools $(BUILD_DIR)/tests $(BIN_DIR) $(TEST_BIN_DIR) $(BIN_DIR)/cryptopals:
	@mkdir -p $@

$(TEST_BIN_DIR)/test_%: $(BUILD_DIR)/tests/test_%.o $(BUILD_DIR)/lib/%.o $(BUILD_DIR)/lib/utils.o $(BUILD_DIR)/lib/fixed_xor.o $(BUILD_DIR)/lib/score_english_hex.o | $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/tests/test_%.o: $(TESTS_DIR)/test_%.c | $(BUILD_DIR)/tests
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/cryptopals_%: $(CRYPT_DIR)/%.c $(LIB_OBJS) | $(BIN_DIR)/cryptopals
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< $(LIB_OBJS)

# Aggregate rules for tests
tests: $(TEST_BINS)

# Run every test binary
TEST_COMMAND := for t in $(TEST_BINS); do echo "Running $$t"; $$t || exit $$?; done

test: tests
	@$(TEST_COMMAND)

docs:
	doxygen Doxyfile
	$(MAKE) -C docs/latex
	mv docs/latex/refman.pdf docs/refman.pdf
	rm -rf docs/latex

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
