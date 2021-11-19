#include "parser_topdown.h"
#include "scanner.h"

/**
 * @brief Used for custom input into the parser
 */ 

symtab_t symtab;

int main(){
    scanner_t scanner;
    scanner_init(&scanner);
    parser_t parser;
    parser_setup(&parser, &scanner, &symtab);
    return parse_program();
}