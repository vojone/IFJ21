/******************************************************************************
 *                                  IFJ21
 *                                scanner.c
 * 
 *          Authors: Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 *           Purpose: Source file with implementation of lexer (scanner)
 * 
 *                  Last change: 
 *****************************************************************************/ 

/**
 * @file scanner.c
 * @brief Source file with implementation of lexer (scanner)
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 */ 

#include "scanner.h"
#include <ctype.h>

void scanner_init(scanner_t *scanner) {
    scanner->state = INIT;
}

token_t get_next_token(scanner_t *scanner) {
    token_t result;
    result.token_type = IDENTIFIER;
    result.attr = NULL;
    return result;
}




/***                             End of scanner.c                        ***/
