#include "parser_topdown.h"
#include "scanner.h"

/**
 * @brief User for custom input into the parser
 */ 

int main() {
    scanner_t scanner;
    scanner_init(&scanner);
    parser_t parser;
    parser_setup(&parser, &scanner);
    return parse_program();
}
