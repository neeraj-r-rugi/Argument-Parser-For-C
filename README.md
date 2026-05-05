# Command Line Argument Parser for C
```text
  /$$$$$$  /$$$$$$$   /$$$$$$           /$$$$$$ 
 /$$__  $$| $$__  $$ /$$__  $$         /$$__  $$
| $$  \ $$| $$  \ $$| $$  \__/        | $$  \__/
| $$$$$$$$| $$$$$$$/| $$ /$$$$ /$$$$$$| $$      
| $$__  $$| $$__  $$| $$|_  $$|______/| $$      
| $$  | $$| $$  \ $$| $$  \ $$        | $$    $$
| $$  | $$| $$  | $$|  $$$$$$/        |  $$$$$$/
|__/  |__/|__/  |__/ \______/          \______/ 
```

A lightweight, single-header C argument parser. Drop one file into your project and get typed flags, multi-value support, required-argument enforcement, inline default values, and auto-generated help with zero dependencies.

**Author:** Neeraj R Rugi  
**License:** GPL-3.0

---

## Integration

In **exactly one** `.c` file (typically `main.c`):

```c
#define LOAD_ARGUMENT_PARSER
#include "argument_parser.h"
```

In every other file that only needs the types or function signatures:

```c
#include "argument_parser.h"
```

> Defining `LOAD_ARGUMENT_PARSER` in more than one translation unit will cause linker errors.

---

## Quick Start

```c
#define LOAD_ARGUMENT_PARSER
#define ARGUMENT_PARSER_EXIT_ON_ERROR
#include "argument_parser.h"

int main(int argc, char **argv) {
    arg_table *table = init_argument_parser();

    add_argument(table, "--host",    "-H", ARGUMENT_TYPE_STRING | ARGUMENT_TYPE_REQUIRED, "Server hostname");
    add_argument(table, "--port",    "-p", ARGUMENT_TYPE_INTEGER,                          "Port number (default 8080)");
    add_argument(table, "--verbose", "-v", ARGUMENT_TYPE_BOOLEAN,                          "Enable verbose output");
    add_argument(table, "--tags",    "-t", ARGUMENT_TYPE_STRING | ARGUMENT_TYPE_MULTIPLE,  "One or more tags");
    add_argument(table, "--timeout", NULL, ARGUMENT_TYPE_FLOAT,                            "Timeout in seconds");
    add_argument(table, "--val",     NULL, ARGUMENT_TYPE_INTEGER | ARGUMENT_TYPE_MULTIPLE, "One or more integer values");

    //Error handling can be done by checking the return value of parse_all_arguments and the out_error code, 
    //but with ARGUMENT_PARSER_EXIT_ON_ERROR defined, 
    //the parser will exit on error, so we can ignore the return value here.
    parse_all_arguments(table, argc, argv, NULL);

    // Default values are passed directly into the getter — no is_present check needed
    char *host    = arg_get_string(table, "--host", NULL);
    int   port    = arg_get_int(table,    "--port",    8080);
    float timeout = arg_get_float(table,  "--timeout", 30.0f);
    int   verbose = arg_get_bool(table,   "--verbose");

    int tag_count;
    char **tags = arg_get_multiple_string(table, "--tags", &tag_count, NULL);
    if (tag_count > 0) {
        printf("Tags:\n");
        for (int i = 0; i < tag_count; i++)
            printf(" - %s\n", tags[i]);
        
    }

    int val_count;
    int *vals = arg_get_multiple_int(table, "--val", &val_count, NULL);
    if (val_count > 0) {
        printf("Values:\n");
        for (int i = 0; i < val_count; i++)
            printf(" - %d\n", vals[i]);
        
    }

    printf("Host: %s  Port: %d  Verbose: %d  Timeout: %.2f\n", host, port, verbose, timeout);
    #ifndef LEAK_MEMORY
        free_multiple_strings(&tags, tag_count);
        free_single_string(&host);
        free_multiple_ints(&vals);
        free_argument_table(&table);
    #endif
    return 0;
}
```

---

## Argument Types

Combine a **base type** with optional **modifiers** using bitwise OR.

### Base Types

| Type | Flag | Accepted values |
|---|---|---|
| `ARGUMENT_TYPE_STRING` | `--flag value` | Any string |
| `ARGUMENT_TYPE_INTEGER` | `--flag 42` | Signed integer within `INT_MIN`/`INT_MAX` |
| `ARGUMENT_TYPE_FLOAT` | `--flag 3.14` | Floating-point number |
| `ARGUMENT_TYPE_BOOLEAN` | `--flag` | No value — presence means `true` |

### Modifiers

| Modifier | Effect |
|---|---|
| `ARGUMENT_TYPE_REQUIRED` | Parser exits with an error if the flag is absent |
| `ARGUMENT_TYPE_MULTIPLE` | Flag accepts one or more space-separated values |

### Examples

```c
ARGUMENT_TYPE_STRING  | ARGUMENT_TYPE_REQUIRED                          // required string
ARGUMENT_TYPE_INTEGER | ARGUMENT_TYPE_MULTIPLE                           // one or more ints
ARGUMENT_TYPE_FLOAT   | ARGUMENT_TYPE_MULTIPLE | ARGUMENT_TYPE_REQUIRED  // required multi-float
ARGUMENT_TYPE_BOOLEAN                                                     // optional flag (no value)
```

---

## Flag Syntax

| Style | Supported |
|---|---|
| `--flag value` | Yes |
| `-f value` | Yes |
| `--flag=value` | No |
| `-fvalue` | No |
| Combined short flags (`-vxz`) | No |

Flags must always include the dash(es) in the name you register:

```c
add_argument(table, "--output", "-o", ...);   // correct
add_argument(table, "output",   "o",  ...);   // will never match argv
```

Negative numbers are treated as values, not flags. `--val -3 -7` works correctly for a `MULTIPLE INTEGER` argument.

---

## The `--` Sentinel

The bare `--` token has special meaning in this parser. Its behaviour differs depending on whether the preceding flag accepts a single value or multiple values.

### Single-value arguments: escaping a dash-prefixed value

For single-value flags, placing `--` immediately after the flag name signals that the **next token is a literal value**, even if it begins with `-`.

```bash
./test --host -- -myserver
```

Here `--` tells the parser: *the next word is the value for `--host`, not a new flag*. The stored value is `-myserver`.

### Multiple-value arguments: dash-safe window

For `ARGUMENT_TYPE_MULTIPLE` flags, `--` serves two purposes simultaneously:

1. **Opens a dash-safe value window** — all tokens after `--` that begin with `-` are treated as literal values, not flags.
2. **Closes the window on the second `--`** — a second bare `--` terminates the value chain and resumes normal flag parsing.

```bash
./test -H localhost -t -- -tag1 -tag2 -tag3 -- --verbose
```

Output:
```
Tag 1: -tag1
Tag 2: -tag2
Tag 3: -tag3
Host: localhost  Port: 8080  Verbose: 1 Timeout: 30.00
```

| Token | Interpretation |
|---|---|
| `-H localhost` | `--host` = `"localhost"` |
| `-t` | begin collecting values for `--tags` |
| `--` | enter dash-safe window for `-t` |
| `-tag1 -tag2 -tag3` | literal string values for `--tags` |
| `--` | close dash-safe window; resume normal parsing |
| `--verbose` | parsed normally as the boolean flag |

### Summary

| Context | `--` effect |
|---|---|
| After a single-value flag | Next token is treated as the value verbatim, even if it starts with `-` |
| After a `MULTIPLE` flag | Opens a dash-safe collection window; a second `--` closes it and resumes flag parsing |

---

## Error Handling

The parser offers two modes of error handling. The built-in automatic mode is recommended for most use cases.

### Recommended: Automatic exit on error (`ARGUMENT_PARSER_EXIT_ON_ERROR`)

Define `ARGUMENT_PARSER_EXIT_ON_ERROR` before including the header to have the parser automatically print an error message to `stderr` and exit the program on any parsing error. This is the simplest and most common approach.

```c
#define LOAD_ARGUMENT_PARSER
#define ARGUMENT_PARSER_EXIT_ON_ERROR   // parser exits automatically on any error
#include "argument_parser.h"

int main(int argc, char **argv) {
    arg_table *table = init_argument_parser();
    // ... add_argument calls ...

    // Pass NULL for out_error — errors are handled automatically
    parse_all_arguments(table, argc, argv, NULL);

    // Safe to use getters directly — if we reach this line, parsing succeeded
    char *host = arg_get_string(table, "--host", NULL);
    // ...
}
```

### Manual: Inspecting errors via `out_error`

If `ARGUMENT_PARSER_EXIT_ON_ERROR` is **not** defined, `argument_parser_error()` will print the error message to `stderr` but **return instead of exiting**. You can then inspect the error code written to the `out_error` pointer passed to `parse_all_arguments` and decide how to respond.

```c
#define LOAD_ARGUMENT_PARSER
// ARGUMENT_PARSER_EXIT_ON_ERROR is NOT defined
#include "argument_parser.h"

int main(int argc, char **argv) {
    arg_table *table = init_argument_parser();
    // ... add_argument calls ...

    arg_error err = ARGUMENT_ERROR_NONE;
    parse_all_arguments(table, argc, argv, &err);

    if (err != ARGUMENT_ERROR_NONE) {
        // Inspect the error code and handle it yourself
        switch (err) {
            case ARGUMENT_ERROR_UNKNOWN_ARGUMENT:
                fprintf(stderr, "Unrecognised flag — check your input.\n");
                break;
            case ARGUMENT_ERROR_MISSING_VALUE:
                fprintf(stderr, "A required value or argument is missing.\n");
                break;
            case ARGUMENT_ERROR_MULTIPLE_PROVIDED:
                fprintf(stderr, "A flag was supplied more than once.\n");
                break;
            case ARGUMENT_ERROR_INVALID_TYPE:
                fprintf(stderr, "A value could not be converted to the expected type.\n");
                break;
            default:
                break;
        }
        free_argument_table(&table);
        return EXIT_FAILURE;
    }

    // Parsing succeeded — safe to use getters
    char *host = arg_get_string(table, "--host", NULL);
    // ...
}
```

### `arg_error` codes

| Code | Meaning |
|---|---|
| `ARGUMENT_ERROR_NONE` | Parsing completed successfully |
| `ARGUMENT_ERROR_UNKNOWN_ARGUMENT` | An unregistered flag was encountered |
| `ARGUMENT_ERROR_MULTIPLE_PROVIDED` | The same flag appeared more than once |
| `ARGUMENT_ERROR_MISSING_VALUE` | A flag expected a value but none followed, or a required flag was absent |
| `ARGUMENT_ERROR_INVALID_TYPE` | A value could not be converted to the flag's declared type |

> **Note:** `ARGUMENT_PARSER_EXIT_ON_ERROR` only governs `argument_parser_error()` — recoverable parsing errors. Internal invariant violations (null table, malloc failures, etc.) always call `argument_parser_panic()`, which exits unconditionally regardless of this setting.

### Error messages

All error messages are printed to `stderr`. The two categories are:

| Situation | Prefix |
|---|---|
| Recoverable parsing errors (unknown flag, duplicate, bad value, missing required) | `An Error Occurred While Parsing Arguments:` |
| Internal invariant violations (null pointer, malloc failure) | `Argument Parser PANIC:` |

Full message examples:

| Situation | Error message |
|---|---|
| Unknown flag | `An Error Occurred While Parsing Arguments: Unknown argument: --unknown` |
| Duplicate flag | `An Error Occurred While Parsing Arguments: Argument '...' is provided multiple times.` |
| Non-numeric value for integer flag | `An Error Occurred While Parsing Arguments: Invalid integer value: 'not_a_number'` |
| Non-numeric value for float flag | `An Error Occurred While Parsing Arguments: Invalid float value: 'not_a_float'` |
| Integer exceeds `INT_MAX`/`INT_MIN` | `An Error Occurred While Parsing Arguments: Invalid integer value(Bound Exceeded): '999999999999'` |
| `MULTIPLE` flag with no following values | `An Error Occurred While Parsing Arguments: Expected at least one value after '-t'` |
| Required argument missing | `An Error Occurred While Parsing Arguments: Required argument '...' was not provided.` |
| Internal error (null pointer, malloc fail) | `Argument Parser PANIC: ...` |

---

## API Reference

### Setup

```c
arg_table *init_argument_parser();
```
Allocates and returns an empty argument table. Call this before anything else.

```c
arg_table *add_argument(arg_table *table,
                        const char *long_name,
                        const char *short_name,
                        arg_type    type,
                        const char *help_text);
```
Registers a flag. Pass `NULL` for `short_name` if no short form is needed.

```c
arg_table *parse_all_arguments(arg_table *table, int argc, char **argv, arg_error *out_error);
```
Parses the command line. Handles `--help`/`-h`, unknown flags, duplicates, missing values, type errors, and required-argument checks.

If `ARGUMENT_PARSER_EXIT_ON_ERROR` is defined, the program exits on any error. Otherwise, the error code is written to `*out_error` (if non-NULL) and the function returns early — always check `out_error` in that mode.

Pass `NULL` for `out_error` when using `ARGUMENT_PARSER_EXIT_ON_ERROR`.

```c
void free_argument_table(arg_table **table);
```
Frees all memory allocated by the parser and sets the pointer to `NULL`, preventing dangling pointer access. Call when done.

---

### Retrieving Values

#### Single-value getters

All single-value getters accept a **`default_value`** parameter. If the argument was not supplied on the command line, the default is returned directly. No manual `is_present` check required.

```c
char  *arg_get_string(arg_table *table, const char *name, const char *default_value);
int    arg_get_int   (arg_table *table, const char *name, int         default_value);
float  arg_get_float (arg_table *table, const char *name, float       default_value);
int    arg_get_bool  (arg_table *table, const char *name);  // always returns 0 if absent — no panic
```

**Examples:**

```c
// Returns "localhost" if --host was not provided
char *host = arg_get_string(table, "--host", "localhost");

// Returns 8080 if --port was not provided
int port = arg_get_int(table, "--port", 8080);

// Returns 30.0f if --timeout was not provided
float timeout = arg_get_float(table, "--timeout", 30.0f);

// Always safe — returns 0 if --verbose is absent
int verbose = arg_get_bool(table, "--verbose");
```

> Passing `NULL` as the default for `arg_get_string` is valid when the flag is `ARGUMENT_TYPE_REQUIRED`, since the parser will have already exited (or returned an error) before the getter is reached if the flag is absent.

>`char  *arg_get_string(arg_table *table, const char *name, const char *default_value);` returns a newly allocated array everytime, even in the default case to maintain design simplicity, and must always be freed after use using `free_single_string()` 
#### Checking presence explicitly

If you need to distinguish between "not provided" and "provided with a value equal to the default", use `arg_get` and check `->is_present`:

```c
arg_opt *opt = arg_get(table, "--port");
if (opt->is_present)
    printf("User explicitly set port to %d\n", arg_get_int(table, "--port", 0));
```

#### Multi-value getters

All three multi-value getters accept a **`default_values`** parameter. When the argument is absent, the getter writes `0` to `*out_count` and returns the provided default pointer. No allocation takes place in that case.

```c
char **arg_get_multiple_string(arg_table *table, const char *name, int *out_count, char **default_values);
int   *arg_get_multiple_int   (arg_table *table, const char *name, int *out_count, int  *default_values);
float *arg_get_multiple_float (arg_table *table, const char *name, int *out_count, float *default_values);
```

When the argument **is** present, these functions return a **newly allocated array** that you must free using the provided helpers.

**Examples:**

```c
// --- String multiple ---
int tag_count;
char **tags = arg_get_multiple_string(table, "--tags", &tag_count, NULL);
if (tag_count > 0) {
    for (int i = 0; i < tag_count; i++)
        printf("Tag %d: %s\n", i + 1, tags[i]);
    free_multiple_strings(&tags, tag_count);
}

// --- Integer multiple ---
int val_count;
int *vals = arg_get_multiple_int(table, "--val", &val_count, NULL);
if (val_count > 0) {
    for (int i = 0; i < val_count; i++)
        printf("Val %d: %d\n", i + 1, vals[i]);
    free_multiple_ints(&vals);
}

// --- Float multiple with a static default ---
float fallback[] = {1.0f, 2.0f};
int weight_count;
float *weights = arg_get_multiple_float(table, "--weights", &weight_count, fallback);
// If --weights was not given: weight_count == 0, weights == fallback (do NOT free)
// If --weights was given:     weight_count >  0, weights is heap-allocated (MUST free)
if (weight_count > 0)
    free_multiple_floats(&weights);
```

> **Important:** Only free the returned pointer when `out_count > 0`. When the argument is absent the returned pointer is your own `default_values` — freeing it is your responsibility under your own rules, not the parser's.

---

### Memory Management and Dangling Pointer Prevention

All `free_*` helpers take a **pointer-to-pointer** and set the inner pointer to `NULL` after freeing. This prevents use-after-free bugs from stale pointers.

```c
void free_argument_table  (arg_table ***table);              // sets *table   = NULL
void free_multiple_ints   (int       **ints);                // sets *ints    = NULL
void free_multiple_floats (float     **floats);              // sets *floats  = NULL
void free_multiple_strings(char      ***strings, int count); // sets *strings = NULL
void free_single_string(char ** str);
```

The pointer is zeroed immediately after the free, so any subsequent accidental dereference will segfault loudly rather than producing undefined behaviour silently.

```c
// Correct usage — pass the address of your pointer
free_argument_table(&table);             // table   is now NULL
free_multiple_ints(&vals);               // vals    is now NULL
free_multiple_floats(&weights);          // weights is now NULL
free_multiple_strings(&tags, tag_count); // tags    is now NULL
```

---

## Help Output

Pass `--help` or `-h` anywhere on the command line to print usage and exit:

```
Usage: ./test [options]

Options:
  --host               -H      <string>    [required]              Server hostname
  --port               -p      <int>       [optional]              Port number (default 8080)
  --verbose            -v      <bool>      [optional]              Enable verbose output
  --tags               -t      <string>    [optional]  [multiple]  One or more tags
  --timeout                    <float>     [optional]              Timeout in seconds
  --val                        <int>       [optional]  [multiple]  One or more integer values
```

`[required]` is highlighted in bold red on terminals that support ANSI colour codes. Help is also printed automatically before most error exits.

### Printing to a File
If the output of the program is redirected to a file, then Include `ARGUMENT_PARSER_NO_COLOR_OUTPUT` definition at the top of the source file. As without it, **ANSI escape codes are always emitted.** Redirecting help output to a file will include raw colour sequences.

Use:
```C
#define LOAD_ARGUMENT_PARSER
#define ARGUMENT_PARSER_EXIT_ON_ERROR
#define ARGUMENT_PARSER_NO_COLOR_OUTPUT
//----Source File----
```

---

## Running the Tests

A test script is included. Compile `test.c` first, then run:

```bash
# Happy-path tests
./test.sh

# Error-handling tests
./test.sh --fail
```

The happy-path run exercises strings, integers, floats, booleans, multi-value flags, default values, and mixed combinations. The `--fail` run covers invalid integers, invalid floats, missing multi-values, unknown flags, and integer overflow — all of which should exit with a clear error message.

---

## Limitations and Design Choices

- **`--flag=value` syntax omitted.** Space-separated form only. 
- **No combined short flags.** `-vp 9000` is not supported; use `-v -p 9000`. 
- **No positional arguments.** Every value must be preceded by its flag. Similar to unix tools like `uname`
- **No subcommands support yet.** All flags are flat; there is no `git commit`-style dispatch. But can be implemented using flags and provided APIs.
- **`BOOLEAN | MULTIPLE` is not allowed.**
- **No thread safety.** Update from a single thread only, or ensure all updates to table occur in a lock-held crtical section, preferably the one with `main()`.
---

## Test Output

The following output is produced by running `./test.sh` against a correctly built binary. Two scenarios are shown: **without memory leaks** (all `free_*` calls active) and **with memory leaks** (`#define LEAK_MEMORY` suppresses cleanup, caught by AddressSanitizer).

### Happy-path tests (`./test.sh`)

Each block below corresponds to one `./test` invocation in `test.sh`, separated by `-----------------------------`.

```
# ./test -H "localhost"
Host: localhost  Port: 8080  Verbose: 0  Timeout: 30.00
-----------------------------
# ./test -H "192.168.1.17" --verbose
Host: 192.168.1.17  Port: 8080  Verbose: 1  Timeout: 30.00
-----------------------------
# ./test -H "localhost" -p 5000
Host: localhost  Port: 5000  Verbose: 0  Timeout: 30.00
-----------------------------
# ./test -H "localhost" --timeout 2.5
Host: localhost  Port: 8080  Verbose: 0  Timeout: 2.50
-----------------------------
# ./test -H "localhost" -t -- -tag1 -tag2 -tag3 -- --verbose
Tags:
 - -tag1
 - -tag2
 - -tag3
Host: localhost  Port: 8080  Verbose: 1  Timeout: 30.00
-----------------------------
# ./test -H "localhost" --timeout 40.0 -t -- -tag1 -tag2 -tag3 -tag4 -tag5 -tag6
Tags:
 - -tag1
 - -tag2
 - -tag3
 - -tag4
 - -tag5
 - -tag6
Host: localhost  Port: 8080  Verbose: 0  Timeout: 40.00
-----------------------------
# ./test -H "localhost" -t "tag 9" "tag 11" "tag 13" --timeout -3.5 --verbose -p 6007 --val 10 20 30
Tags:
 - tag 9
 - tag 11
 - tag 13
Values:
 - 10
 - 20
 - 30
Host: localhost  Port: 6007  Verbose: 1  Timeout: -3.50
-----------------------------
# ./test --help
Usage: ./test [options]

Options:
  --host               -H      <string>    [required]              Server hostname
  --port               -p      <int>       [optional]              Port number (default 8080)
  --verbose            -v      <bool>      [optional]              Enable verbose output
  --tags               -t      <string>    [optional]  [multiple]  One or more tags
  --timeout                    <float>     [optional]              Timeout in seconds
  --val                        <int>       [optional]  [multiple]  One or more integer values
```

No AddressSanitizer output — zero leaks when all `free_*` helpers are called correctly.

---

### Memory-leak run (with `LEAK_MEMORY` defined)

When the `#ifndef LEAK_MEMORY` cleanup block is skipped, AddressSanitizer reports every allocation that was never freed. The functional output is identical to the happy-path run; only the leak summary differs. Example excerpt from the last invocation (the most complex one):

```
==10777==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 24 byte(s) in 1 object(s) allocated from:
    #1 0x... in init_argument_parser argument_parser.h:337

Direct leak of 24 byte(s) in 1 object(s) allocated from:
    #1 0x... in arg_get_multiple_string argument_parser.h:745

Direct leak of 12 byte(s) in 1 object(s) allocated from:
    #1 0x... in arg_get_multiple_int argument_parser.h:689

Direct leak of 10 byte(s) in 1 object(s) allocated from:
    #1 0x... in arg_get_string argument_parser.h:623

... (indirect leaks from add_argument, parse_all_arguments, realloc) ...

SUMMARY: AddressSanitizer: 765 byte(s) leaked in 44 allocation(s).
```

The table below maps each leak site to the responsible `free_*` call that eliminates it:

| Leak origin | Freed by |
|---|---|
| `init_argument_parser` | `free_argument_table(&table)` |
| `arg_get_string` | `free_single_string(&host)` |
| `arg_get_multiple_string` | `free_multiple_strings(&tags, tag_count)` |
| `arg_get_multiple_int` | `free_multiple_ints(&vals)` |
| `add_argument` (indirect) | `free_argument_table(&table)` |
| `parse_all_arguments` (indirect) | `free_argument_table(&table)` |