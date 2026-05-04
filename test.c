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

    char *host    = arg_get_string(table, "--host", NULL);
    int   verbose = arg_get_bool(table,   "--verbose");

    int port = arg_get_int(table, "--port", 8080); // default port is 8080 if not provided

    int tag_count;
    char **tags = NULL;
    tags = arg_get_multiple_string(table, "--tags", &tag_count, NULL);
    if(tag_count > 0) {
        printf("Tags:\n");
        for(int i = 0; i < tag_count; i++) {
            printf(" - %s\n", tags[i]);
        }
        free_multiple_strings(&tags, tag_count); // Free the allocated array for string values
    }
    int val_count;
    int * vals = arg_get_multiple_int(table, "--val", &val_count, NULL);
    if(val_count > 0) {
        printf("Values:\n");
        for(int i = 0; i < val_count; i++) {
            printf(" - %d\n", vals[i]);
        }
        free_multiple_ints(&vals); // Free the allocated array for int values
    }

    float timeout = arg_get_float(table, "--timeout", 30.0f); // default timeout is 30 seconds if not provided
    printf("Host: %s  Port: %d  Verbose: %d Timeout: %.2f\n", host, port, verbose, timeout);
    free_argument_table(&table);
    return 0;
}