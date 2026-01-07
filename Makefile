CC := gcc
MKDIR := mkdir -p

SRC_DIR := src
INC_DIR := include
BUILD_DIR := build
BIN_DIR := bin

CI_SRCS := $(shell find $(SRC_DIR)/ci -name '*.c')
HT_SRCS := $(SRC_DIR)/hashtable/hashtable.c
ARGPARSE_SRCS := $(shell find $(SRC_DIR)/argparse -name '*.c')
SRCS := $(CI_SRCS) $(HT_SRCS) $(ARGPARSE_SRCS)

CI_OBJS := $(patsubst $(SRC_DIR)/ci/%.c,$(BUILD_DIR)/ci/%.o,$(CI_SRCS))
HT_OBJS := $(patsubst $(SRC_DIR)/hashtable/%.c,$(BUILD_DIR)/hashtable/%.o,$(HT_SRCS))
ARGPARSE_OBJS := $(patsubst $(SRC_DIR)/argparse/%.c,$(BUILD_DIR)/argparse/%.o,$(ARGPARSE_SRCS))
OBJS := $(CI_OBJS) $(HT_OBJS) $(ARGPARSE_OBJS)

CFLAGS := -I$(INC_DIR) \
          -I$(INC_DIR)/ci \
          -std=c11 \
          -Wall \
          -Wextra \
          -Wpedantic \
          -Werror \
          -Wformat=2 \
          -Wshadow \
          -Wwrite-strings \
          -Wstrict-prototypes \
          -Wold-style-definition \
          -Wredundant-decls \
          -Wnested-externs \
          -Wmissing-include-dirs \
          -Wjump-misses-init \
          -Wlogical-op \
          -Wduplicated-cond \
          -Wduplicated-branches \
          -Wformat-security \
          -Wformat-signedness \
          -Wimplicit-fallthrough=5 \
          -fstack-protector-strong \
          -Wno-unused-function \
          -Wno-unused-parameter

RELEASE_FLAGS := -O3

DEBUG_FLAGS := -g3 -DDEBUG -O0

.PHONY: all
all: CFLAGS += $(RELEASE_FLAGS)
all: $(BIN_DIR)/ci $(BIN_DIR)/ht_test

.PHONY: test
test: $(BIN_DIR)/ci
	@-./run_tests.sh || echo "FAILED!"

.PHONY: test_week2
test_week2: $(BIN_DIR)/ci
	@-./run_tests.sh 2 || echo "FAILED!"

.PHONY: test_week3
test_week3: $(BIN_DIR)/ci
	@-./run_tests.sh 3 || echo "FAILED!"

.PHONY: test_week4
test_week4: $(BIN_DIR)/ci
	@-./run_tests.sh 4 || echo "FAILED!"

HT_TEST_OBJ := $(BUILD_DIR)/hashtable/ht_trace_test.o

$(BIN_DIR)/ht_test: $(BUILD_DIR)/hashtable/hashtable.o $(HT_TEST_OBJ) | $(BIN_DIR)
	$(CC) $^ -o $@

$(BUILD_DIR)/hashtable/ht_trace_test.o: $(SRC_DIR)/hashtable/ht_trace_test.c | $(BUILD_DIR)/hashtable
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: test_hashtable
test_hashtable: $(BIN_DIR)/ht_test
	@for f in testcases/hashtable/*.trace; do \
		echo "Running $$f..."; \
		$(BIN_DIR)/ht_test "$$f" || exit 1; \
	done
	@echo "All hashtable tests passed!"

.PHONY: debug
debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(BIN_DIR)/ci $(BIN_DIR)/ht_test

$(BIN_DIR):
	$(MKDIR) $(BIN_DIR)

$(BIN_DIR)/ci: $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) -o $@

$(BUILD_DIR)/ci:
	$(MKDIR) $(BUILD_DIR)/ci

$(BUILD_DIR)/hashtable:
	$(MKDIR) $(BUILD_DIR)/hashtable

$(BUILD_DIR)/argparse:
	$(MKDIR) $(BUILD_DIR)/argparse

$(BUILD_DIR)/ci/%.o: $(SRC_DIR)/ci/%.c | $(BUILD_DIR)/ci
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/hashtable/%.o: $(SRC_DIR)/hashtable/%.c | $(BUILD_DIR)/hashtable
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/argparse/%.o: $(SRC_DIR)/argparse/%.c | $(BUILD_DIR)/argparse
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJS) $(BIN_DIR)/ci
	rm -rf $(BIN_DIR) $(BUILD_DIR)

.PHONY: format
format:
	@find . \
		\( -path './.git' -o -path './build' -o -path './bin' -o -path './test_output' -o -path './include/argparse' -o -path './src/argparse' \) -prune -o \
		\( -name '*.c' -o -name '*.h' \) -print0 | xargs -0 clang-format -i

.PHONY: docs
docs:
	doxygen Doxyfile

.PHONY: docs-pdf
docs-pdf: docs
	$(MAKE) -C build/docs/latex
	@if [ -f build/docs/latex/refman.pdf ]; then \
		cp build/docs/latex/refman.pdf build/docs/latex/ci_reference.pdf; \
		echo "Wrote build/docs/latex/ci_reference.pdf"; \
	fi
