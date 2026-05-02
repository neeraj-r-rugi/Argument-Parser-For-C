# Command Line Argument Parser for C

A single-header, zero-dependency C argument parser. Drop one file into your project, define `LOAD_ARGUMENT_PARSER` in exactly one translation unit, and you have a fully functional `--flag value` CLI parser with typed getters, required-argument enforcement, multi-value support, and auto-generated help.

---

## Table of Contents

- [Integration](#integration)
- [Quick Start](#quick-start)
- [How It Works](#how-it-works)
- [Argument Types](#argument-types)
- [Type Modifiers](#type-modifiers)
- [Valid Type Combinations](#valid-type-combinations)
- [API Reference](#api-reference)
  - [Lifecycle](#lifecycle)
  - [Single-Value Getters](#single-value-getters)
  - [Multi-Value Getters](#multi-value-getters)
  - [Error Helpers](#error-helpers)
- [Flag Syntax Rules](#flag-syntax-rules)
- [Help System](#help-system)
- [Error Behaviour](#error-behaviour)
- [Limitations](#limitations)
- [Memory Notes](#memory-notes)

---

## Integration

The header is split into two regions separated by a preprocessor guard:

```
#ifndef ARGUMENT_PARSER   ← declarations (always included)
...
#endif

#ifdef LOAD_ARGUMENT_PARSER   ← definitions (included once)
...
#endif
```

**In exactly one `.c` file** (your `main.c` is the natural choice):

```c
#define LOAD_ARGUMENT_PARSER
#include "argument_parser.h"
```

Every other file that only needs the types or function prototypes:

```c
#include "argument_parser.h"
```

Defining `LOAD_ARGUMENT_PARSER` in more than one translation unit will cause linker errors (duplicate symbol definitions). This is intentional — it is the standard single-header pattern.

---

## Quick Start

```c
#define LOAD_ARGUMENT_PARSER
#include "argument_parser.h"

int main(int argc, char **argv) {
    arg_table *table = init_argument_parser();

    add_argument(table, "--host",    "-H", ARGUMENT_TYPE_STRING | ARGUMENT_TYPE_REQUIRED, "Server hostname");
    add_argument(table, "--port",    "-p", ARGUMENT_TYPE_INTEGER,                          "Port number (default 8080)");
    add_argument(table, "--verbose", "-v", ARGUMENT_TYPE_BOOLEAN,                          "Enable verbose output");
    add_argument(table, "--tags",    "-t", ARGUMENT_TYPE_STRING | ARGUMENT_TYPE_MULTIPLE,  "One or more tags");

    parse_all_arguments(table, argc, argv);

    char *host    = arg_get_string(table, "--host");
    int   verbose = arg_get_bool(table,   "--verbose");

    int port = 8080;
    if (arg_get(table, "--port")->is_present)
        port = arg_get_int(table, "--port");

    int tag_count;
    char **tags = NULL;
    if (arg_get(table, "--tags")->is_present)
        tags = arg_get_multiple_string(table, "--tags", &tag_count);
        for(int i = 0; i < tag_count; i++) {
            printf("Tag %d: %s\n", i + 1, tags[i]);
        }

    printf("Host: %s  Port: %d  Verbose: %d\n", host, port, verbose);
    return 0;
}
```

Example invocations:

```
./app --host localhost --port 9000 --verbose --tags alpha beta gamma
./app -H localhost -p 9000 -v -t alpha beta
./app --help
```

---

## How It Works

### Initialisation

`init_argument_parser()` heap-allocates an `arg_table` and zeroes all fields. The `arguments` pointer starts as `NULL`; capacity grows on demand via `realloc` each time `add_argument` is called.

### Argument Registration

Each call to `add_argument` does:

1. `realloc` the `arg_opt **arguments` array to one larger.
2. `malloc` a new `arg_opt`, `strdup` the long name, short name, and help text into it.
3. Set `is_present = 0`, `argument_value = NULL`, `argument_count = 0`.
4. If `ARGUMENT_TYPE_REQUIRED` is set, increment `table->required_arguments`.

### Parsing

`parse_all_arguments` does a **linear scan** over `argv[1..argc-1]`:

1. **Help pre-scan** — iterates the full `argv` first. If `--help` or `-h` appears *anywhere*, it calls `print_help` and exits immediately, before any other parsing.
2. **Main loop** — for each token:
   - Tries to match the token against every registered argument's long name or short name (exact string match, `strcmp`).
   - If no match is found, calls `print_help` then `argument_parser_error` → program exits.
   - If already seen (`is_present == 1`), calls `argument_parser_error` → duplicate flag → program exits.
3. **Value consumption** (per matched type):
   - `BOOLEAN` — no value token consumed; sets `argument_value` to a heap `int` of `1`.
   - `MULTIPLE` — greedily consumes all subsequent tokens that do **not** look like a flag (see [Flag Syntax Rules](#flag-syntax-rules)). Requires at least one value.
   - Everything else — consumes exactly the next token. Errors if it is missing or looks like a flag.
4. **Required check** — after the loop, iterates all registered arguments; if any `REQUIRED` argument has `is_present == 0`, calls `print_help` then `argument_parser_panic` → program exits.

### Type Conversion

| Declared type | Conversion used |
|---|---|
| `STRING` | `strdup(val)` — raw copy, no validation |
| `INTEGER` | `atoi(val)` — no error detection |
| `FLOAT` | `atof(val)` — no error detection |
| `BOOLEAN` | flag presence only; no value token |

---

## Argument Types

These are bit-flags defined in `enum ARGUMENT_TYPE` and combined with `|`.

| Constant | Bit | Meaning |
|---|---|---|
| `ARGUMENT_TYPE_STRING` | `1 << 0` | Value is a `char *` string |
| `ARGUMENT_TYPE_INTEGER` | `1 << 1` | Value is parsed as `int` via `atoi` |
| `ARGUMENT_TYPE_FLOAT` | `1 << 2` | Value is parsed as `float` via `atof` |
| `ARGUMENT_TYPE_BOOLEAN` | `1 << 3` | Flag with no value token; presence = true |
| `ARGUMENT_TYPE_REQUIRED` | `1 << 4` | Modifier: argument must be present or the parser aborts |
| `ARGUMENT_TYPE_MULTIPLE` | `1 << 5` | Modifier: argument accepts one or more space-separated values |

---

## Type Modifiers

`ARGUMENT_TYPE_REQUIRED` and `ARGUMENT_TYPE_MULTIPLE` are **modifiers**, not base types. They must always be combined with exactly one base type using bitwise OR.

```c
// ✔ correct
ARGUMENT_TYPE_INTEGER | ARGUMENT_TYPE_REQUIRED
ARGUMENT_TYPE_STRING  | ARGUMENT_TYPE_MULTIPLE
ARGUMENT_TYPE_FLOAT   | ARGUMENT_TYPE_MULTIPLE | ARGUMENT_TYPE_REQUIRED

// ✘ wrong — no base type
ARGUMENT_TYPE_REQUIRED
ARGUMENT_TYPE_MULTIPLE
```

Combining `ARGUMENT_TYPE_BOOLEAN` with `ARGUMENT_TYPE_MULTIPLE` is structurally possible but semantically undefined — the boolean branch is checked first and no value tokens are consumed, so the MULTIPLE machinery never runs.

---

## Valid Type Combinations

| Combination | Getter |
|---|---|
| `ARGUMENT_TYPE_STRING` | `arg_get_string` |
| `ARGUMENT_TYPE_INTEGER` | `arg_get_int` |
| `ARGUMENT_TYPE_FLOAT` | `arg_get_float` |
| `ARGUMENT_TYPE_BOOLEAN` | `arg_get_bool` |
| `ARGUMENT_TYPE_STRING \| ARGUMENT_TYPE_REQUIRED` | `arg_get_string` |
| `ARGUMENT_TYPE_INTEGER \| ARGUMENT_TYPE_REQUIRED` | `arg_get_int` |
| `ARGUMENT_TYPE_FLOAT \| ARGUMENT_TYPE_REQUIRED` | `arg_get_float` |
| `ARGUMENT_TYPE_BOOLEAN \| ARGUMENT_TYPE_REQUIRED` | `arg_get_bool` |
| `ARGUMENT_TYPE_STRING \| ARGUMENT_TYPE_MULTIPLE` | `arg_get_multiple_string` |
| `ARGUMENT_TYPE_INTEGER \| ARGUMENT_TYPE_MULTIPLE` | `arg_get_multiple_int` |
| `ARGUMENT_TYPE_FLOAT \| ARGUMENT_TYPE_MULTIPLE` | `arg_get_multiple_float` |
| Any of the above `\| ARGUMENT_TYPE_REQUIRED` | same getter |

---

## API Reference

### Lifecycle

#### `arg_table * init_argument_parser()`
Allocates and returns a zeroed argument table. Must be called before any other function. Panics and exits on `malloc` failure.

#### `arg_table * add_argument(arg_table *table, const char *long_name, const char *short_name, arg_type type, const char *help_text)`
Registers a new argument. Both names are `strdup`'d internally. Returns the same `table` pointer for optional chaining. Panics on `realloc`/`malloc` failure.

- `long_name` — full flag string, e.g. `"--output"`. Must include the dashes.
- `short_name` — abbreviated flag, e.g. `"-o"`. Must include the dash.
- `type` — bitwise OR of base type + optional modifiers.
- `help_text` — displayed in `--help` output.

#### `arg_table * parse_all_arguments(arg_table *table, int argc, char **argv)`
Parses the command line. Handles `--help` / `-h`, unknown flags, duplicates, missing values, and required-argument enforcement. Calls `exit` on any error. Returns `table` on success.

---

### Single-Value Getters

All single-value getters panic and exit if:
- The argument name is not found in the table.
- The argument's declared type does not match the getter.
- The argument was not present on the command line (except `arg_get_bool`, which returns `0` instead).

#### `char * arg_get_string(arg_table *table, const char *name)`
Returns the raw string value. The pointer is owned by the parser; do not `free` it.

#### `int arg_get_int(arg_table *table, const char *name)`
Returns the integer value. Parsed at call time of `parse_all_arguments` via `atoi`.

#### `float arg_get_float(arg_table *table, const char *name)`
Returns the float value. Parsed via `atof`.

#### `int arg_get_bool(arg_table *table, const char *name)`
Returns `1` if the flag was present, `0` if absent. The only getter that does **not** panic on absence.

#### `arg_opt * arg_get(arg_table *table, const char *name)`
Low-level lookup; returns the raw `arg_opt *`. Useful to check `->is_present` before calling a typed getter on an optional argument, avoiding a panic.

```c
// Safe pattern for optional integer
if (arg_get(table, "--port")->is_present)
    port = arg_get_int(table, "--port");
```

---

### Multi-Value Getters

All multi-value getters panic and exit if the argument is not declared with `ARGUMENT_TYPE_MULTIPLE`, was not present, or has no values. They also write the value count to `*out_count` if non-NULL.

#### `void ** arg_get_multiple(arg_table *table, const char *name, int *out_count)`
Returns the raw `void **` array. You are responsible for casting each element to the correct pointer type.

#### `int * arg_get_multiple_int(arg_table *table, const char *name, int *out_count)`
Returns a **newly `malloc`'d** `int[]` flattened from the internal `void **` store. **You must `free` this array.** The internal `void *` pointers are not freed by this call.

#### `float * arg_get_multiple_float(arg_table *table, const char *name, int *out_count)`
Same ownership rules as `arg_get_multiple_int`. Returns a **newly `malloc`'d** `float[]`. **You must `free` this array.**

#### `char ** arg_get_multiple_string(arg_table *table, const char *name, int *out_count)`
Returns the internal `char **` directly — no new allocation. Do **not** `free` the array or any of its strings.

---

### Error Helpers

These are public but intended for internal use. Both print to `stderr` and call `exit(EXIT_FAILURE)`.

#### `void argument_parser_panic(const char *message, ...)`
For internal/programmer errors (null table, malloc failure, wrong type in getter). Output prefix: `Argument Parser PANIC:` in bold red.

#### `void argument_parser_error(const char *fmt, ...)`
For user-facing input errors (unknown flag, duplicate flag, missing value). Output prefix: `An Error Occured While Parsing Arguments:` in bold red.

Both functions use `vfprintf` and accept `printf`-style format strings.

---

## Flag Syntax Rules

The parser uses an internal `is_flag()` heuristic to distinguish flags from values when consuming tokens:

| Token pattern | Classified as |
|---|---|
| Starts with `--` | flag |
| Starts with `-` followed by a letter | flag |
| Starts with `-` followed by a digit | **value** (negative number) |
| Starts with `-` followed by `.` | **value** (e.g. `-.5`) |
| Starts with `-` alone (`-\0`) | value |
| Anything else | value |

This means `--numbers -3 -7 -1` is valid: `-3`, `-7`, `-1` are consumed as three integer values for a MULTIPLE INTEGER argument.

**However, the parser cannot disambiguate** `-3` as a negative number versus an unregistered short flag in all contexts. If you have a short flag `-3` registered, it will be matched as a flag, not a value.

Flags must be provided as **full tokens** — the parser does not support `--port=9000` or `-p9000` syntax. The space-separated form `--port 9000` is the only accepted style.

---

## Help System

`print_help` is automatically invoked when `--help` or `-h` appears anywhere in `argv`, and also before most error exits. It prints to `stdout` in the following format:

```
Usage: ./myapp [options]

Options:
  --host               -H    <string>    [required]             Server hostname
  --port               -p    <int>       [optional]             Port number
  --verbose            -v    <bool>      [optional]             Enable verbose output
  --tags               -t    <string>    [optional]  [multiple] One or more tags
```

`[required]` is highlighted in bold red via ANSI escape codes. Terminals that do not support ANSI will display the raw escape sequences.

---

## Error Behaviour

Every error — whether a programmer mistake or bad user input — calls `exit(EXIT_FAILURE)`. There is no error-code return path and no way to recover from a parse error at runtime.

| Situation | Function called | Exits? |
|---|---|---|
| `malloc`/`realloc` failure | `argument_parser_panic` | Yes |
| `NULL` table passed to any function | `argument_parser_panic` | Yes |
| Wrong getter type (e.g. `arg_get_int` on a STRING) | `argument_parser_panic` | Yes |
| Argument name not found in table | `argument_parser_panic` | Yes |
| Unknown flag on command line | `argument_parser_error` + `print_help` | Yes |
| Duplicate flag on command line | `argument_parser_error` + `print_help` | Yes |
| Required argument missing | `argument_parser_panic` + `print_help` | Yes |
| Missing value after a non-boolean flag | `argument_parser_error` | Yes |
| MULTIPLE flag with zero following values | `argument_parser_error` | Yes |
| `--help` / `-h` | `print_help` + `exit(EXIT_SUCCESS)` | Yes |

---

## Limitations

These are not bugs — they are deliberate design constraints or known gaps to be aware of:

**No `=` syntax.** `--port=9000` is not supported. Only `--port 9000` works.

**No short-flag combining.** `-vxz` (combining multiple single-character flags) is not supported. Each flag must be a separate token.

**No default values.** The parser has no built-in mechanism for defaults. You must check `->is_present` and apply a default yourself in application code.

**No integer/float validation.** `atoi` and `atof` return `0` silently for non-numeric input. `"--port abc"` will parse as port `0` with no warning.

**No integer overflow detection.** `atoi` has undefined behaviour on overflow. Values outside `INT_MIN`/`INT_MAX` are unsafe.

**Duplicate flag = hard error.** Passing `--verbose --verbose` exits the program. There is no `last-wins` or `count` mode.

**BOOLEAN + MULTIPLE is undefined.** The boolean branch fires first and consumes no value tokens; the MULTIPLE logic is never reached. Do not combine them.

**No subcommand support.** There is no concept of `git commit --message "..."` style subcommands. All flags are flat.

**Positional arguments are not supported.** Every argument must be preceded by its flag. Bare words that do not match a registered flag cause an error.

**Memory is never freed.** There is no `free_argument_parser()` function. The `arg_table`, all `arg_opt` structs, all `strdup`'d strings, and all parsed values remain allocated until process exit. For short-lived CLI programs this is fine; for library use or long-running processes it would be a leak.

**`arg_get_multiple_int` and `arg_get_multiple_float` allocate; caller must free.** The returned arrays are heap-allocated. `arg_get_multiple_string` returns the internal pointer and must **not** be freed.

**`print_help` uses ANSI escape codes unconditionally.** No `isatty()` check is performed. Redirecting output to a file will include raw escape sequences.

**Thread safety: none.** The parser performs no synchronisation. It must be used only from a single thread.

**`argument_name_long` and `argument_name_short` must include dashes.** The parser does a raw `strcmp` against `argv` tokens, so `"port"` will never match `"--port"`. Always pass `"--port"` and `"-p"`.

---

## Memory Notes

| Allocation | Owner | Must free? |
|---|---|---|
| `arg_table` itself | parser | No (no destructor) |
| `arg_opt` structs | parser | No |
| `strdup`'d names and help text | parser | No |
| `argument_value` (STRING) | parser | No |
| `argument_value` (INTEGER/FLOAT/BOOLEAN) | parser | No |
| `multiple_argument_values` (STRING) | parser | No |
| `multiple_argument_values[i]` (STRING) | parser | No |
| `multiple_argument_values[i]` (INT/FLOAT) | parser | No |
| Array returned by `arg_get_multiple_int` | **caller** | **Yes — `free(result)`** |
| Array returned by `arg_get_multiple_float` | **caller** | **Yes — `free(result)`** |
| Array returned by `arg_get_multiple_string` | parser | No |