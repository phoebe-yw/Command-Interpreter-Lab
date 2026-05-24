# CI Lab — ASML Interpreter

An interpreter for **ASML (A Simple Machine Language)**, written in C as part of CS 429 (Computer Architecture) at UT Austin. The project implements the full pipeline from source text to execution: lexing (provided), parsing, and interpreting.

## Overview

ASML is a flat, register-based assembly language with:
- **32 mutable 64-bit registers** (`x0`–`x31`)
- **1 KB of interpreter memory** (byte-addressable)
- **20 commands** covering arithmetic, bitwise logic, shifts, memory, branches, and function calls
- **Numeric literals** in decimal, hex (`0X`), and binary (`0B`)
- **Labels** for branching and calling, resolved at parse time via a custom hash table

## What I Implemented

| Component | File | Description |
|---|---|---|
| Parser | `src/ci/parser.c` | Recursive-descent parser; converts token stream into a linked list of `Command` structs |
| Interpreter | `src/ci/interpreter.c` | Walks the command list and executes each instruction; manages registers, flags, memory, and call stack |
| Command cleanup | `src/ci/command.c` | `free_command()` — walks and frees the full command linked list |
| Hash table | `src/hashtable/hashtable.c` | Generic chained hash table (DJB2 hash) used to map label strings to command pointers |

## Commands Supported

| Week | Commands |
|---|---|
| 2 | `add`, `sub`, `mov`, `nop`, `cmp`, `cmp_u`, `print` (numeric) |
| 3 | `and`, `orr`, `eor`, `lsl`, `lsr`, `asr`, `load`, `store`, `put`, `print` (string) |
| 4 | `b`, `b.<cond>`, `call`, `ret` |

Branch conditions: `eq`, `ne`, `gt`, `ge`, `lt`, `le`

## Building and Running

Requires GCC and Make on a Linux machine (tested on UTCS Ubuntu 20.04).

```bash
# Build
make all

# Run a program file
bin/ci -i testcases/week2/add.s

# Run and write output to file
bin/ci -i testcases/week4/call.s -o out.txt

# Interactive REPL
bin/ci

# Debug build (for use with GDB)
make debug

# Run tests
make test_week2
make test_week3
make test_week4
make test_hashtable
```

## Error Handling

- **Syntactic errors** — the parser halts at the bad token, sets `had_error`, and returns any successfully parsed commands up to that point
- **Interpretation errors** — the interpreter sets `had_error` on invalid memory access, bad byte counts, or out-of-range operations
- Multiple commands on the same line without a semicolon separator are treated as a syntax error

## Hash Table

A generic hash table in `src/hashtable/hashtable.c` maps `char *` keys to `void *` values using DJB2 hashing with chaining for collision resolution. Supports `ht_init`, `ht_put`, `ht_get`, `ht_delete`, `ht_size`, and `ht_free` with an optional value-freeing callback.

## Project Structure

```
src/
  ci/
    parser.c       # Token → Command linked list
    interpreter.c  # Command execution engine
    command.c      # Command memory management
  hashtable/
    hashtable.c    # Generic hash table
include/           # Header files (provided)
testcases/         # 198 test cases across weeks 2–4 + hash table
```

## Tools Used

- **GCC** with `-Wall -Wextra -Werror` and address sanitizers
- **GDB** for runtime debugging
- **Valgrind** for memory leak detection
- **Gradescope** autograder for automated testing
