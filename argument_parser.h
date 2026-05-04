/*
    Name: Argument Parser for C
    Author: Neeraj R Rugi
    Description: A simple argument parser for C that supports string, integer, float, and boolean arguments, as well as required and multiple arguments. It provides a simple API for defining arguments.
    LICENSE: GNU General Public License v3.0 (GPL-3.0)
    Copyright: (c) 2026 Neeraj R Rugi. All rights reserved.

*/

#ifndef ARGUMENT_PARSER
#define ARGUMENT_PARSER

/*
    * A simple argument parser for C that supports string, integer, float, and boolean arguments, as well as required and multiple arguments.
    * It provides a simple API for defining arguments, parsing command-line input, and retrieving argument values.
*/
/*
    * TYPE: Enumeration (enum)
    * FIELDS:
    * - ARGUMENT_TYPE_STRING: Represents a string argument type.
    * - ARGUMENT_TYPE_INTEGER: Represents an integer argument type.
    * - ARGUMENT_TYPE_FLOAT: Represents a float argument type.
    * - ARGUMENT_TYPE_BOOLEAN: Represents a boolean argument type.
    * - ARGUMENT_TYPE_REQUIRED: Modifier indicating that the argument is required.
    * - ARGUMENT_TYPE_MULTIPLE: Modifier indicating that the argument can accept multiple values.
    * DESCRIPTION:
    * This enumeration defines the various types and modifiers that can be associated with command-line arguments in
    * the argument parser.
*/
typedef enum ARGUMENT_TYPE{
    ARGUMENT_TYPE_STRING        = 1 << 0,
    ARGUMENT_TYPE_INTEGER       = 1 << 1,
    ARGUMENT_TYPE_FLOAT         = 1 << 2,
    ARGUMENT_TYPE_BOOLEAN       = 1 << 3,
    ARGUMENT_TYPE_REQUIRED      = 1 << 4,
    ARGUMENT_TYPE_MULTIPLE      = 1 << 5
} arg_type;


typedef enum ARGUMENT_PARRSER_ERROR{
    ARGUMENT_ERROR_NONE                 = 1 << 0,
    ARGUMENT_ERROR_MULTIPLE_PROVIDED    = 1 << 1,
    ARGUMENT_ERROR_UNKNOWN_ARGUMENT     = 1 << 2,
    ARGUMENT_ERROR_MISSING_VALUE        = 1 << 3,
    ARGUMENT_ERROR_INVALID_TYPE         = 1 << 4
} arg_error;

/*
    * STRUCTURES:
    * - ARGUMENT_OPTIONS: Represents the details of a single command-line argument, including its name, type, value, and help text.
    * - ARGUMENT_TABLE: Represents the collection of all defined arguments, along with counts of total, user-provided, and required arguments.
    * DESCRIPTION:
    * These structures are used to store information about the defined command-line arguments and their values after parsing. The ARGUMENT_OPTIONS structure holds details for each individual argument, while the ARGUMENT_TABLE structure manages the overall collection of arguments.
*/
typedef struct ARGUMENT_OPTIONS{
    char * argument_name_long;
    char * argument_name_short;
    void * argument_value;
    void ** multiple_argument_values;
    arg_type argument_type;
    char * help_text;
    int is_present;
    int argument_count;
} arg_opt;


typedef struct ARGUMENT_TABLE{
    arg_opt ** arguments;
    int total_arguments;
    int user_arguments;
    int required_arguments;
}arg_table;



/*
    * CAUTION: This function will terminate the program. Use it only for critical errors in the argument parser.
    * Input: A formatted error message and additional arguments (similar to printf)
    * Output: None (the function will terminate the program)
    * Description:
    * This function is used to handle critical errors in the argument parser. It takes a formatted
    * error message and additional arguments, prints the error message to stderr, and then terminates the program with a failure status.
*/
void argument_parser_panic(const char * message, ...);

/*
    * CAUTION: This function will terminate the program. Use it for non-critical errors in the argument parser.
    * Input: A formatted error message and additional arguments (similar to printf)
    * Output: None (the function will terminate the program)
    * Description:
    * This function is used to raise errors in the argument passed by the user. It takes a formatted
    * error message and additional arguments, prints the error message to stderr, and then terminates the program with a failure status.
*/
void argument_parser_error(const char *fmt, ...);

/*
    * Input: None
    * Output: Pointer to the initialized argument table
    * Description:
    * Initializes the argument parser by allocating memory for the argument table and setting initial values.
    * Returns a pointer to the initialized argument table.
*/
arg_table * init_argument_parser();

/*
    * Input: Pointer to the argument table, long name of the argument, short name of the argument, type of the argument, and help text for the argument
    * Output: Pointer to the updated argument table
    * Description:
    * Adds a new argument to the argument table with the specified long name, short name, type, and help text.
    * The function allocates memory for the new argument and updates the total count of arguments in the table.
*/
arg_table * add_argument(arg_table * table, const char * long_name, const char * short_name, arg_type type, const char * help_text);

/*
    * Input: Pointer to the argument table and the name of the program (for usage display)
    * Output: None (the function will print the help message and terminate the program)
    * Description:
    * Prints a help message that includes usage instructions and details about each argument defined in the argument table.
    * The function iterates through all arguments in the table and displays their long name, short name, type, modifiers (required/multiple), and help text.
*/
void print_help(arg_table * table, const char * program_name);

/*
    * Input: Pointer to the argument table, argument count (argc), and argument vector (argv)
    * Output: Pointer to the updated argument table with parsed values
    * Description:
    * Parses the command-line arguments based on the definitions in the argument table. It iterates through the provided arguments,
    * matches them against the defined arguments in the table, and stores their values accordingly. The function also checks for required arguments
    * and handles errors such as unknown arguments or missing values. If the user requests help (e.g., by using --help), it will display the help message and exit. 
*/
arg_table * parse_all_arguments(arg_table * table, int argc, char ** argv, enum ARGUMENT_PARRSER_ERROR * out_error);

/*
    * Input: Pointer to the argument table and the name of the argument to retrieve
    * Output: Pointer to the argument option structure corresponding to the specified name
    * Description:
    * Retrieves the argument option structure from the argument table based on the provided name (either long or short). If the argument is not found, it raises an error.
*/
arg_opt * arg_get(arg_table * table, const char * name);
/*
    * Input: Pointer to the argument table and the name of the argument to retrieve
    * Output: The string value of the specified argument
    * Description:
    * Retrieves the value of a string argument from the argument table. It checks if the argument exists, is of the correct type, and has a value before returning it. If any of these conditions are not met, it raises an error.
    * Note: Useful for retrieving string arguments directly with metadata.
    */
char * arg_get_string(arg_table * table, const char * name, const char * default_value);
/*
    * Input: Pointer to the argument table and the name of the argument to retrieve
    * Output: The integer value of the specified argument
    * Description:
    * Retrieves the value of an integer argument from the argument table. It checks if the argument exists, is of the correct type, and has a value before returning it. If any of these
    * conditions are not met, it raises an error.
*/
int arg_get_int(arg_table * table, const char * name, int default_value);
/*
    * Input: Pointer to the argument table and the name of the argument to retrieve
    * Output: The float value of the specified argument
    * Description:
    * Retrieves the value of a float argument from the argument table. It checks if the argument
    * exists, is of the correct type, and has a value before returning it. If any of these conditions are not met, it raises an error.
*/
float arg_get_float(arg_table * table, const char * name, float default_value);
/*
    * Input: Pointer to the argument table and the name of the argument to retrieve
    * Output: The boolean value of the specified argument (1 for true, 0 for
    *  false)
    * Description:
    * Retrieves the value of a boolean argument from the argument table. It checks if the argument
    * exists and is of the correct type. If the argument is not present, it returns 0 (false). If the argument is present, it returns its value (1 for true).
*/
int arg_get_bool(arg_table * table, const char * name);
/*
    * Input: Pointer to the argument table, the name of the argument to retrieve, and a pointer to an integer to store the count of values
    * Output: An array of pointers to the values of the specified multiple argument
    * Description:
    * Retrieves the values of a multiple argument from the argument table. It checks if the argument exists, is of the correct type, and has values before returning them. The function also fills the provided integer pointer with the count of values. If any of these conditions are not met, it raises an error.
*/
void ** arg_get_multiple(arg_table * table, const char * name, int * out_count);
/*
    * Input: Pointer to the argument table, the name of the argument to retrieve, and a pointer to an integer to store the count of values
    * Output: An array of integers corresponding to the values of the specified multiple integer argument
    * Description:
    * Retrieves the values of a multiple integer argument from the argument table. It checks if the
    * argument exists, is of the correct type, and has values before returning them. The function also fills the provided integer pointer with the count of values. If any of these conditions are not met, it raises an error.
*/
int * arg_get_multiple_int(arg_table * table, const char * name, int * out_count, int * default_values);
/*
    * Input: Pointer to the argument table, the name of the argument to retrieve, and a pointer to an integer to store the count of values
    * Output: An array of floats corresponding to the values of the specified multiple float argument
    * Description:
    * Retrieves the values of a multiple float argument from the argument table. It checks if the
    * argument exists, is of the correct type, and has values before returning them. The function also fills the provided integer pointer with the count of values. If any of these conditions are not met, it raises an error.
*/
float * arg_get_multiple_float(arg_table * table, const char * name, int * out_count, float * default_values);
/*
    * Input: Pointer to the argument table, the name of the argument to retrieve, and a pointer to an integer to store the count of values
    * Output: An array of strings corresponding to the values of the specified multiple string argument
    * Description:
    * Retrieves the values of a multiple string argument from the argument table. It checks if the argument exists, is of the correct type, and has values before returning them. The function also fills
    * the provided integer pointer with the count of values. If any of these conditions are not met, it raises an error.
*/
char ** arg_get_multiple_string(arg_table * table, const char * name, int * out_count, char ** default_values);

/*
    * Input: A string value to convert and a pointer to an integer to store the result
    * Output: None (the function will store the converted integer in the provided pointer)
    * Description:
    * Converts a string value to an integer. The function checks for valid integer format, handles overflow and underflow, and raises errors for invalid input. If the conversion is successful, it stores the result in the provided integer pointer.
*/
void cast_to_int(const char * val, int * out);
/*
    * Input: A string value to convert and a pointer to a double to store the result
    * Output: None (the function will store the converted float in the provided pointer)
    * Description:
    * Converts a string value to a float (double). The function checks for valid float format, handles overflow and underflow, and raises errors for invalid input. If the conversion is successful, it stores the result in the provided double pointer.
*/
void cast_to_float(const char * val, float * out);

/*
    * CAUTION: This function will free all memory allocated for the argument table. Use it when the argument table is no longer needed to prevent memory leaks.
    * Input: Pointer to the argument table
    * Output: None (the function will free all allocated memory for the argument table)
    * Description:
    * Frees all memory allocated for the argument table, including the argument options and their values. This function should be called when the argument table is no longer needed to prevent memory leaks.
*/
void free_argument_table(arg_table ** table);

/*
    * CAUTION: This function will free all memory allocated for an array of strings. Use it when the array of strings is no longer needed to prevent memory leaks.
    * Input: An array of strings and the count of strings in the array
    * Output: None (the function will free all allocated memory for the array of strings)
    * Description:
    * Frees all memory allocated for an array of strings, including each individual string and the array itself. This function should be called when the array of strings is no longer needed to prevent memory leaks.
*/
void free_multiple_strings(char *** strings, int count);
/*
    * CAUTION: This function will free all memory allocated for an array of integers. Use it when the array of integers is no longer needed to prevent memory leaks.
    * Input: An array of integers
    * Output: None (the function will free all allocated memory for the array of integers)
    * Description:
    * Frees all memory allocated for an array of integers. This function should be called when the array of integers is no longer needed to prevent memory leaks.
*/
void free_multiple_ints(int ** ints);
/*
    * CAUTION: This function will free all memory allocated for an array of floats. Use it when the array of floats is no longer needed to prevent memory leaks.
    * Input: An array of floats
    * Output: None (the function will free all allocated memory for the array of floats)
    * Description:
    * Frees all memory allocated for an array of floats. This function should be called when the array of floats is no longer needed to prevent memory leaks.
*/
void free_multiple_floats(float ** floats);

#endif




#ifdef LOAD_ARGUMENT_PARSER
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>

void print_help(arg_table *table, const char *program_name) {
    printf("\nUsage: %s [options]\n\n", program_name);
    printf("Options:\n");
    for (int i = 0; i < table->total_arguments; i++) {
        arg_opt *arg = table->arguments[i];

        // names
        printf("  %-20s %-6s",
               arg->argument_name_long,
               arg->argument_name_short);

        // base type
        if      (arg->argument_type & ARGUMENT_TYPE_STRING)  printf("  %-10s", "<string>");
        else if (arg->argument_type & ARGUMENT_TYPE_INTEGER) printf("  %-10s", "<int>");
        else if (arg->argument_type & ARGUMENT_TYPE_FLOAT)   printf("  %-10s", "<float>");
        else if (arg->argument_type & ARGUMENT_TYPE_BOOLEAN) printf("  %-10s", "<bool>");

        // modifiers
        #ifndef ARGUMENT_PARSER_NO_COLOR_HELP
            printf("  %s", (arg->argument_type & ARGUMENT_TYPE_REQUIRED) ? "\033[1;31m[required]\033[0m" : "[optional]");
            printf("  %s", (arg->argument_type & ARGUMENT_TYPE_MULTIPLE) ? "[multiple]" : "          ");
        #elif
            printf("  %s", (arg->argument_type & ARGUMENT_TYPE_REQUIRED) ? "[required]" : "[optional]");
            printf("  %s", (arg->argument_type & ARGUMENT_TYPE_MULTIPLE) ? "[multiple]" : "          ");
        #endif

        // help text
        printf("  %s\n", arg->help_text ? arg->help_text : "");
    }

    printf("\n");
}

void argument_parser_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    #ifndef ARGUMENT_PARSER_NO_COLOR_ERROR
        fprintf(stderr, "\033[1;31mAn Error Occurred While Parsing Arguments:\033[0m ");
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
    #elif
        fprintf(stderr, "An Error Occurred While Parsing Arguments: ");
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
    #endif
    va_end(args);

    #ifdef ARGUMENT_PARSER_EXIT_ON_ERROR
    exit(EXIT_FAILURE);
    #endif
}


void argument_parser_panic(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    #ifndef ARGUMENT_PARSER_NO_COLOR_ERROR
        fprintf(stderr, "\033[1;31mArgument Parser PANIC:\033[0m ");
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
    #elif
        fprintf(stderr, "Argument Parser PANIC: ");
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
    #endif
    va_end(args);
    exit(EXIT_FAILURE);
}

arg_table * init_argument_parser(){
    arg_table * table = (arg_table *)malloc(sizeof(arg_table));
    if(table == NULL){
        argument_parser_panic("Memory allocation failed for argument table.");
    }
    table->arguments = NULL;
    table->total_arguments = 0;
    table->user_arguments = 0;
    table->required_arguments = 0;
    return table;
}

arg_table *add_argument(arg_table *table, const char *verbose_name, const char *short_name,
                        arg_type type, const char *help_text) {
    if (table == NULL)
        argument_parser_panic("Argument table is not initialized.");

    if(verbose_name == NULL || strlen(verbose_name) == 0)
        argument_parser_panic("Argument long name cannot be NULL or empty.");
    if(type & ARGUMENT_TYPE_BOOLEAN && type & ARGUMENT_TYPE_MULTIPLE)
        argument_parser_panic("Argument '%s' cannot be both BOOLEAN and MULTIPLE.", verbose_name);
    
    if(strcmp(verbose_name, "--help") == 0 || strcmp(verbose_name, "-h") == 0) {
        argument_parser_panic("Cannot use reserved argument name '%s'.", verbose_name);
    }
    if(short_name == NULL){
        short_name = "";
    }
    if(help_text == NULL){
        help_text = "<Help text not provided>";
    }
    for (int i = 0; i < table->total_arguments; i++) {
        arg_opt *arg = table->arguments[i];
        if ((strcmp(arg->argument_name_long, verbose_name) == 0 ||
            strcmp(arg->argument_name_short, short_name) == 0) && strcmp(short_name, "") != 0)
            argument_parser_panic("Argument name '%s' or short name '%s' is already in use.", verbose_name, short_name);
    }

    int new_count = table->total_arguments + 1;

    arg_opt **new_args = (arg_opt **)realloc(table->arguments, sizeof(arg_opt *) * new_count);
    if (new_args == NULL)
        argument_parser_panic("Memory reallocation failed when adding argument '%s'.", verbose_name);

    table->arguments = new_args;

    arg_opt *new_arg = (arg_opt *)malloc(sizeof(arg_opt));
    if (new_arg == NULL)
        argument_parser_panic("Memory allocation failed for argument '%s'.", verbose_name);

    new_arg->argument_name_long     = strdup(verbose_name);
    new_arg->argument_name_short    = strdup(short_name);
    new_arg->argument_type          = type;
    new_arg->help_text              = strdup(help_text);
    new_arg->argument_value         = NULL;
    new_arg->multiple_argument_values = NULL;
    new_arg->is_present             = 0;
    new_arg->argument_count         = 0;

    table->arguments[table->total_arguments] = new_arg;
    table->total_arguments = new_count;
    if(new_arg->argument_type & ARGUMENT_TYPE_REQUIRED) {
        table->required_arguments++;
    }

    return table;
}

int is_flag(const char *token) {
    if (token[0] != '-') return 0;
    if (token[1] == '-') return 1;
    if (token[1] == '\0') return 0;
    if (token[1] >= '0' && token[1] <= '9') return 0;
    if (token[1] == '.') return 0;
    return 1;
}

arg_table *parse_all_arguments(arg_table *table, int argc, char **argv, enum ARGUMENT_PARRSER_ERROR * out_error) {
    if (table == NULL)
        argument_parser_panic("Argument table is not initialized.");
    if (argv == NULL)
        argument_parser_panic("Argument vector (argv) is NULL.");


    for(int temp = 1; temp < argc; temp++) {
        if (strcmp(argv[temp], "--help") == 0 || strcmp(argv[temp], "-h") == 0) {
            print_help(table, argv[0]);
            exit(EXIT_SUCCESS);
        }
    }

    for (int current_argument = 1; current_argument < argc; current_argument++) {
        char *current_word = argv[current_argument];
        int matched = 0;

        for (int i = 0; i < table->total_arguments; i++) {
            arg_opt *arg = table->arguments[i];

            if (strcmp(current_word, arg->argument_name_long) != 0 &&
                strcmp(current_word, arg->argument_name_short) != 0)
                continue;

            matched = 1;
            if(arg->is_present) {
                print_help(table, argv[0]);
                argument_parser_error("Argument '%s' is provided multiple times.", arg->argument_name_long);
                if(out_error) {
                    *out_error = ARGUMENT_ERROR_MULTIPLE_PROVIDED;
                }
                return table;
            }
            arg->is_present = 1;

            if (arg->argument_type & ARGUMENT_TYPE_BOOLEAN) {
                int *b = malloc(sizeof(int));
                if (!b) argument_parser_panic("malloc failed for '%s'", current_word);
                *b = 1;
                arg->argument_value = b;

            } else if (arg->argument_type & ARGUMENT_TYPE_MULTIPLE) {
                int count = 0;
                int start = current_argument + 1;
                int is_special_case = 0;
                int end_special_case = 0;
                if(!(start >= argc) && (strcmp(argv[start], "--") == 0)) {start++;is_special_case = 1;}
                while (start + count < argc && (!is_flag(argv[start + count]) || is_special_case && !end_special_case)) {
                    if(strcmp(argv[start + count], "--") == 0){
                        end_special_case = 1;
                        continue;
                    }
                    count++;
                } 
                    
                if (count == 0){
                    argument_parser_error("Expected at least one value after '%s'", current_word);
                    if(out_error) {
                        *out_error = ARGUMENT_ERROR_MISSING_VALUE;
                    }
                    return table;
                }
                    
                

                arg->multiple_argument_values = malloc(sizeof(void *) * count);
                if (!arg->multiple_argument_values)
                    argument_parser_panic("malloc failed for '%s'", current_word);

                for (int j = 0; j < count; j++) {
                    char *val = argv[start + j];

                    if (arg->argument_type & ARGUMENT_TYPE_INTEGER) {
                        int *n = malloc(sizeof(int));
                        if (!n) argument_parser_panic("malloc failed for multiple int '%s'", current_word);
                        cast_to_int(val, n);
                        arg->multiple_argument_values[j] = n;

                    } else if (arg->argument_type & ARGUMENT_TYPE_FLOAT) {
                        float *f = malloc(sizeof(float));
                        if (!f) argument_parser_panic("malloc failed for multiple float '%s'", current_word);
                        cast_to_float(val, f);
                        arg->multiple_argument_values[j] = f;

                    } else {
                        arg->multiple_argument_values[j] = strdup(val);
                    }
                }

                arg->argument_count = count;
                current_argument += (is_special_case)? count + 1 : count;
                current_argument += (end_special_case)? 1 : 0;

            } else {
                if (current_argument + 1 >= argc){
                    argument_parser_error("Expected value after '%s'", current_word);
                    if(out_error) {
                        *out_error = ARGUMENT_ERROR_MISSING_VALUE;
                    }
                    return table;
                }
                    


                char *val = argv[++current_argument];

                if (is_flag(val) && !(strcmp(val, "--") == 0)){
                    argument_parser_error("Expected value after '%s', got flag '%s'", current_word, val);
                    if(out_error) {
                        *out_error = ARGUMENT_ERROR_MISSING_VALUE;
                    }
                    return table;
                }
                    
                
                if((strcmp(val, "--") == 0))
                    val = argv[++current_argument];
                    
                if (arg->argument_type & ARGUMENT_TYPE_STRING) {
                    arg->argument_value = strdup(val);

                } else if (arg->argument_type & ARGUMENT_TYPE_INTEGER) {
                    int *n = malloc(sizeof(int));
                    if (!n) argument_parser_panic("malloc failed for '%s'", current_word);
                    cast_to_int(val, n);
                    arg->argument_value = n;

                } else if (arg->argument_type & ARGUMENT_TYPE_FLOAT) {
                    float *f = malloc(sizeof(float));
                    if (!f) argument_parser_panic("malloc failed for '%s'", current_word);
                    cast_to_float(val, f);
                    arg->argument_value = f;

                } else {
                    argument_parser_error("Unsupported type for '%s'", current_word);
                    if(out_error) {
                        *out_error = ARGUMENT_ERROR_INVALID_TYPE;
                    }
                    return table;
                }
            }
            break;
        }

        if (!matched) {
            print_help(table, argv[0]);
            argument_parser_error("Unknown argument: %s", current_word);
            if(out_error) {
                *out_error = ARGUMENT_ERROR_UNKNOWN_ARGUMENT;
            }
            return table;
        }
    }

    for (int i = 0; i < table->total_arguments; i++) {
        arg_opt *arg = table->arguments[i];
        if ((arg->argument_type & ARGUMENT_TYPE_REQUIRED) && !arg->is_present) {
            print_help(table, argv[0]);
            argument_parser_error("Required argument '%s' was not provided.", arg->argument_name_long);
            if(out_error) {
                *out_error = ARGUMENT_ERROR_MISSING_VALUE;
            }
            return table;
        }
    }

    return table;
}

arg_opt *arg_get(arg_table *table, const char *name) {
    if (table == NULL)
        argument_parser_panic("arg_get: table is NULL.");
    if (name == NULL)
        argument_parser_panic("arg_get: name is NULL.");

    for (int i = 0; i < table->total_arguments; i++) {
        arg_opt *arg = table->arguments[i];
        if (strcmp(arg->argument_name_long, name) == 0 ||
            (strcmp(arg->argument_name_short, name) == 0 && strcmp(arg->argument_name_short, "") != 0))
            return arg;
    }

    argument_parser_panic("arg_get: argument '%s' not found in table.", name);
    return NULL;
}

char *arg_get_string(arg_table *table, const char *name, const char *default_value) {
    arg_opt *arg = arg_get(table, name);


    if (!(arg->argument_type & ARGUMENT_TYPE_STRING))
        argument_parser_panic("arg_get_string: argument '%s' is not of type STRING.", name);
    if (!arg->is_present)
        return (char *)default_value;
    if (arg->argument_value == NULL)
        argument_parser_panic("arg_get_string: argument '%s' has no value.", name);
    
    return (char *)arg->argument_value;
}

int arg_get_int(arg_table *table, const char *name, int default_value) {
    arg_opt *arg = arg_get(table, name);

    if (!(arg->argument_type & ARGUMENT_TYPE_INTEGER))
        argument_parser_panic("arg_get_int: argument '%s' is not of type INTEGER.", name);
    if (!arg->is_present)
        return default_value;
    if (arg->argument_value == NULL)
        argument_parser_panic("arg_get_int: argument '%s' has no value.", name);

    return *(int *)arg->argument_value;
}

float arg_get_float(arg_table *table, const char *name, float default_value) {
    arg_opt *arg = arg_get(table, name);

    if (!(arg->argument_type & ARGUMENT_TYPE_FLOAT))
        argument_parser_panic("arg_get_float: argument '%s' is not of type FLOAT.", name);
    if (!arg->is_present)
        return default_value;
    if (arg->argument_value == NULL)
        argument_parser_panic("arg_get_float: argument '%s' has no value.", name);

    return *(float *)arg->argument_value;
}

int arg_get_bool(arg_table *table, const char *name) {
    arg_opt *arg = arg_get(table, name);

    if (!(arg->argument_type & ARGUMENT_TYPE_BOOLEAN))
        argument_parser_panic("arg_get_bool: argument '%s' is not of type BOOLEAN.", name);

    // boolean is allowed to be absent — absence means false
    if (!arg->is_present) return 0;
    if (arg->argument_value == NULL) return 0;

    return *(int *)arg->argument_value;
}

// --- multiple getters ---

void **arg_get_multiple(arg_table *table, const char *name, int *out_count) {
    arg_opt *arg = arg_get(table, name);

    if (!(arg->argument_type & ARGUMENT_TYPE_MULTIPLE))
        argument_parser_panic("arg_get_multiple: argument '%s' is not of type MULTIPLE.", name);
    if (!arg->is_present)
        argument_parser_panic("arg_get_multiple: argument '%s' was not provided.", name);
    if (arg->multiple_argument_values == NULL)
        argument_parser_panic("arg_get_multiple: argument '%s' has no values.", name);

    if (out_count != NULL)
        *out_count = arg->argument_count;

    return arg->multiple_argument_values;
}

int *arg_get_multiple_int(arg_table *table, const char *name, int *out_count, int * default_values) {
    arg_opt *arg = arg_get(table, name);

    if (!(arg->argument_type & ARGUMENT_TYPE_MULTIPLE))
        argument_parser_panic("arg_get_multiple_int: argument '%s' is not MULTIPLE.", name);
    if (!(arg->argument_type & ARGUMENT_TYPE_INTEGER))
        argument_parser_panic("arg_get_multiple_int: argument '%s' is not of type INTEGER.", name);
    if (!arg->is_present){
        *out_count = 0;
        return default_values;
    }
        
    if (arg->multiple_argument_values == NULL)
        argument_parser_panic("arg_get_multiple_int: argument '%s' has no values.", name);

    if (out_count != NULL)
        *out_count = arg->argument_count;

    // flatten void** of int* into a plain int array for convenience
    int *result = malloc(sizeof(int) * arg->argument_count);
    if (!result)
        argument_parser_panic("arg_get_multiple_int: malloc failed for '%s'.", name);

    for (int i = 0; i < arg->argument_count; i++)
        result[i] = *(int *)arg->multiple_argument_values[i];

    return result;
}

float *arg_get_multiple_float(arg_table *table, const char *name, int *out_count, float * default_values) {
    arg_opt *arg = arg_get(table, name);

    if (!(arg->argument_type & ARGUMENT_TYPE_MULTIPLE))
        argument_parser_panic("arg_get_multiple_float: argument '%s' is not MULTIPLE.", name);
    if (!(arg->argument_type & ARGUMENT_TYPE_FLOAT))
        argument_parser_panic("arg_get_multiple_float: argument '%s' is not of type FLOAT.", name);
    if (!arg->is_present){
        *out_count = 0;
        return default_values;
    }
        
    if (arg->multiple_argument_values == NULL)
        argument_parser_panic("arg_get_multiple_float: argument '%s' has no values.", name);

    if (out_count != NULL)
        *out_count = arg->argument_count;

    float *result = malloc(sizeof(float) * arg->argument_count);
    if (!result)
        argument_parser_panic("arg_get_multiple_float: malloc failed for '%s'.", name);

    for (int i = 0; i < arg->argument_count; i++)
        result[i] = *(float *)arg->multiple_argument_values[i];

    return result;
}

char **arg_get_multiple_string(arg_table *table, const char *name, int *out_count, char ** default_values) {
    arg_opt *arg = arg_get(table, name);

    if (!(arg->argument_type & ARGUMENT_TYPE_MULTIPLE))
        argument_parser_panic("arg_get_multiple_string: argument '%s' is not MULTIPLE.", name);
    if (!(arg->argument_type & ARGUMENT_TYPE_STRING))
        argument_parser_panic("arg_get_multiple_string: argument '%s' is not of type STRING.", name);
    if (!arg->is_present){
        *out_count = 0;
        return default_values;
    }
        
    if (arg->multiple_argument_values == NULL)
        argument_parser_panic("arg_get_multiple_string: argument '%s' has no values.", name);

    if (out_count != NULL)
        *out_count = arg->argument_count;

    char ** result = malloc(sizeof(char *) * arg->argument_count);
    if (!result)
        argument_parser_panic("arg_get_multiple_string: malloc failed for '%s'.", name);
    for (int i = 0; i < arg->argument_count; i++) {
        result[i] = strdup((char *)arg->multiple_argument_values[i]);
        if (!result[i])
            argument_parser_panic("arg_get_multiple_string: malloc failed for string '%s' in argument '%s'.", (char *)arg->multiple_argument_values[i], name);
    }

    return result;
}


void free_argument_table(arg_table ** table) {
    if (*table == NULL) return;

    for (int i = 0; i < (*table)->total_arguments; i++) {
        arg_opt *arg = (*table)->arguments[i];
        if (arg) {
            free(arg->argument_name_long);
            free(arg->argument_name_short);
            free(arg->help_text);
            if (arg->argument_value) free(arg->argument_value);
            if (arg->multiple_argument_values) {
                for (int j = 0; j < arg->argument_count; j++) {
                    if (arg->multiple_argument_values[j]) free(arg->multiple_argument_values[j]);
                }
                free(arg->multiple_argument_values);
            }
            free(arg);
        }
    }
    free((*table)->arguments);
    free(*table);
    *table = NULL;
}

void free_multiple_ints(int ** ints) {
    if (ints) free(*ints);
    *ints = NULL;
}
void free_multiple_floats(float ** floats) {
    if (floats) free(*floats);
    *floats = NULL;
}

void free_multiple_strings(char *** strings, int count) {
    if (strings == NULL) return;
    for (int i = 0; i < count; i++) {
        if ((*strings)[i]) free((*strings)[i]);
    }
    free(*strings);
    *strings = NULL;
}   

void cast_to_int(const char *val, int *out) {
    char *end;
    errno = 0;

    long temp = strtol(val, &end, 10);
    // Check: no digits were found
    if (end == val){
        argument_parser_error("Invalid integer value: '%s'", val); 
        exit(EXIT_FAILURE);
    } 
         
    // Check: extra characters after number
    if (*end != '\0'){
        argument_parser_error("Invalid integer value (extra characters): '%s'", val);
        exit(EXIT_FAILURE);
    }
        
    // Check: overflow/underflow from strtol
    if (errno == ERANGE){
        argument_parser_error("Integer value out of range: '%s'", val);
        exit(EXIT_FAILURE);
    }
        
    // Check: fits in int
    if (temp < INT_MIN || temp > INT_MAX){
        argument_parser_error("Invalid integer value(Bound Exceeded): '%s'", val);
        exit(EXIT_FAILURE);
    }
        
    *out = (int)temp;
}


void cast_to_float(const char *val, float *out) {
    char *end;
    errno = 0;
    float temp = strtof(val, &end);
    // No conversion performed
    if (end == val){
        argument_parser_error("Invalid float value: '%s'", val);
        exit(EXIT_FAILURE);
    }
        
    // Extra characters after number
    if (*end != '\0'){
        argument_parser_error("Invalid float value (extra characters): '%s'", val);
        exit(EXIT_FAILURE);
    }
        
    // Overflow / underflow
    if (errno == ERANGE){
        argument_parser_error("Float value out of range: '%s'", val);
        exit(EXIT_FAILURE);
    }
        

    *out = temp;
}
#endif