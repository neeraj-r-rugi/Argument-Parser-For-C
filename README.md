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


A lightweight, single-header C argument parser. Drop one file into your project and get typed flags, multi-value support, required-argument enforcement, and auto-generated help with zero dependencies.

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
#include "argument_parser.h"

int main(int argc, char **argv) {
    arg_table *table = init_argument_parser();

    add_argument(table, "--host",    "-H", ARGUMENT_TYPE_STRING | ARGUMENT_TYPE_REQUIRED, "Server hostname");
    add_argument(table, "--port",    "-p", ARGUMENT_TYPE_INTEGER,                          "Port number (default 8080)");
    add_argument(table, "--verbose", "-v", ARGUMENT_TYPE_BOOLEAN,                          "Enable verbose output");
    add_argument(table, "--tags",    "-t", ARGUMENT_TYPE_STRING | ARGUMENT_TYPE_MULTIPLE,  "One or more tags");
    add_argument(table, "--timeout", NULL, ARGUMENT_TYPE_FLOAT,                            "Timeout in seconds");
    add_argument(table, "--val", NULL, ARGUMENT_TYPE_INTEGER | ARGUMENT_TYPE_MULTIPLE, "One or more integer values");

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
        free_multiple_strings(tags, tag_count); // Free the allocated array for string values

    if (arg_get(table, "--val")->is_present) {
        int val_count;
        int *vals = arg_get_multiple_int(table, "--val", &val_count);
        for(int i = 0; i < val_count; i++) {
            printf("Val %d: %d\n", i + 1, vals[i]);
        }
        free_multiple_ints(vals); // Free the allocated array for integer values
    }

    float timeout = 0.0f;
    if (arg_get(table, "--timeout")->is_present)
        timeout = arg_get_float(table, "--timeout");
    printf("Host: %s  Port: %d  Verbose: %d Timeout: %.2f\n", host, port, verbose, timeout);
    free_argument_table(table);
    table = NULL; // prevent dangling pointer
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
| `ARGUMENT_TYPE_BOOLEAN` | `--flag` | No value. Presence of argument means `true` |

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

Least Redundant Syntax Philosophy was kept as spirit while deciding the syntax.

Flags must always include the dash(es) in the name you register:

```c
add_argument(table, "--output", "-o", ...);   // correct
add_argument(table, "output",   "o",  ...);   // will never match argv
```

Negative numbers are treated as values, not flags. `--val -3 -7` works correctly for a MULTIPLE INTEGER argument.

---

## The `--` Sentinel

The bare `--` token has special meaning in this parser. Its behaviour differs depending on whether the preceding flag accepts a single value or multiple values.

### Single-value arguments: escaping a dash-prefixed value

For single-value flags, placing `--` immediately after the flag name signals that the **next token is a literal value**, even if it begins with `-`. This lets you pass values that would otherwise be mistaken for flags.

```bash
./test --host -- -myserver
```

Here `--` tells the parser: *the next word is the value for `--host`, not a new flag*. The stored value is `-myserver`.

Without `--`, a token starting with `-` following a single-value flag would be rejected as an unknown flag or cause a parse error.

### Multiple-value arguments: dash-prefixed values and chain termination

For `ARGUMENT_TYPE_MULTIPLE` flags, `--` serves two purposes simultaneously:

1. **Opens a dash-safe value window** — all tokens after `--` that begin with `-` are treated as literal values, not flags.
2. **Closes the window on the second `--`** — a second bare `--` terminates the value chain and resumes normal flag parsing for the rest of the command line.

```bash
./test -H localhost -t -- -tag1 -tag2 -tag3 -- --verbose
```

Output:
```
Tag 1: -tag1
Tag 2: -tag2
Tag 3: -tag3
Host: localhost  Port: 8080  Verbose: 1 Timeout: 0.00
```

**What happens step by step:**

| Token | Interpretation |
|---|---|
| `-H localhost` | `--host` = `"localhost"` |
| `-t` | begin collecting values for `--tags` |
| `--` | enter dash-safe window for `-t` |
| `-tag1 -tag2 -tag3` | literal string values for `--tags` |
| `--` | close dash-safe window; resume normal parsing |
| `--verbose` | parsed normally as the `--verbose` boolean flag |

> Without the closing `--`, `--verbose` would be consumed as another value for `--tags` instead of being recognised as its own flag.

### Summary

| Context | `--` effect |
|---|---|
| After a single-value flag | Next token is treated as the value verbatim, even if it starts with `-` |
| After a `MULTIPLE` flag | Opens a dash-safe collection window; a second `--` closes it and resumes flag parsing |

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
arg_table *parse_all_arguments(arg_table *table, int argc, char **argv);
```
Parses the command line. Handles `--help`/`-h`, unknown flags, duplicates, missing values, type errors, and required-argument checks. Exits the program on any error.

```c
void free_argument_table(arg_table *table);
```
Frees all memory allocated by the parser. Call when done.

---

### Retrieving Values

#### Check presence first for optional arguments

```c
arg_opt *arg_get(arg_table *table, const char *name);
```

Use `->is_present` before calling a typed getter on an optional argument to avoid a panic:

```c
int port = 8080;
if (arg_get(table, "--port")->is_present)
    port = arg_get_int(table, "--port");
```

#### Single-value getters

```c
char  *arg_get_string(arg_table *table, const char *name);
int    arg_get_int   (arg_table *table, const char *name);
float  arg_get_float (arg_table *table, const char *name);
int    arg_get_bool  (arg_table *table, const char *name);  // returns 0 if absent — no panic
```

#### Multi-value getters

All three write the element count to `*out_count`.

```c
char **arg_get_multiple_string(arg_table *table, const char *name, int *out_count);
int   *arg_get_multiple_int   (arg_table *table, const char *name, int *out_count);
float *arg_get_multiple_float (arg_table *table, const char *name, int *out_count);
```

> `arg_get_multiple_int`, `arg_get_multiple_float`, and `arg_get_multiple_string` return a **newly allocated array, you must `free` it** using the provided `free_multiple_ints()`, `free_multiple_floats()`, `free_multiple_strings()` functions. 

```c
int count;
char **tags = arg_get_multiple_string(table, "--tags", &count);
for (int i = 0; i < count; i++)
    printf("Tag %d: %s\n", i + 1, tags[i]);
free_multiple_strings(tags, count);

int *vals = arg_get_multiple_int(table, "--val", &count);
for (int i = 0; i < count; i++)
    printf("Val %d: %d\n", i + 1, vals[i]);
free_multiple_ints(vals);  // required
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

---

## Error Handling

All errors print to `stderr` and exit with a failure code. There is no error-return path.

| Situation | Error message |
|---|---|
| Unknown flag | `An Error Occured While Parsing Arguments: Unknown argument: --unknown` |
| Duplicate flag | `An Error Occured While Parsing Arguments: Argument '...' is provided multiple times.` |
| Non-numeric value for integer flag | `An Error Occured While Parsing Arguments: Invalid integer value: 'not_a_number'` |
| Non-numeric value for float flag | `An Error Occured While Parsing Arguments: Invalid float value: 'not_a_float'` |
| Integer exceeds `INT_MAX`/`INT_MIN` | `An Error Occured While Parsing Arguments: Invalid integer value(Bound Excceded): '999999999999'` |
| MULTIPLE flag with no following values | `An Error Occured While Parsing Arguments: Expected at least one value after '-t'` |
| Required argument missing | `Argument Parser PANIC: Required argument '...' was not provided.` |
| Internal error (null pointer, malloc fail) | `Argument Parser PANIC: ...` |

---

## Running the Tests

A test script is included. Compile `test.c` first, then run:

```bash
# Happy-path tests
./test.sh

# Error-handling tests
./test.sh --fail
```

The happy-path run exercises strings, integers, floats, booleans, multi-value flags, and mixed combinations. The `--fail` run covers invalid integers, invalid floats, missing multi-values, unknown flags, and integer overflow — all of which should exit with a clear error message.

---

## Limitations and Design Choices

- **`--flag=value` syntax Omitted.** Space-separated form only. Single Syntax Design Choice
- **No combined short flags.** `-vp 9000` is not supported; use `-v -p 9000`. Single Syntax Design Choice
- **No built-in defaults.** Intentional design decision to let the user to check `->is_present` and apply defaults to their own code as they see fit. 
- **No positional arguments.** Every value must be preceded by its flag. Design choice to embrace Explicitness
- **No subcommands.** All flags are flat; there is no `git commit`-style dispatch yet.
- **`BOOLEAN | MULTIPLE` is Not Allowed.**
- **No thread safety.** Update from a single thread only, preferably the one with `main()`.
- **ANSI escape codes are always emitted as of now.** Redirecting help output to a file will include raw colour sequences.