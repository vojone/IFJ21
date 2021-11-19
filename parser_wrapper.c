#include "parser_topdown.h"
#include "scanner.h"

/**
 * @brief User for custom input into the parser
 */ 
int main(){
    scanner_t scanner;
    scanner_init(&scanner);
    parser_t parser;
    symtab_t tab;
    parser_setup(&parser, &scanner, &tab);
    return parse_program();
}
