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