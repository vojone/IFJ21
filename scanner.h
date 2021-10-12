/******************************************************************************
 *                                  IFJ21
 *                                scanner.h
 * 
 *          Authors: Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 *   Purposer: File, that contains declarations of lexer (scanner) functions
 * 
 *                    Last change: 
 *****************************************************************************/ 

/**
 * @file scanner.h
 * @brief File, that contains declarations of lexer (scanner) functions
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 */ 

#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define STR_ALOWED_ASCII_START 31

typedef enum token_type {
    UNKNOWN,
    IDENTIFIER, 
    KEYWORD,
    INTEGER, 
    NUMBER, 
    STRING, 
    OPERATOR,
    SEPARATOR,
    EOF_TYPE,
    ERROR_TYPE
} token_type_t;

typedef enum fsm_state {
    INIT,
    ID_F,
    INT_F,
    NUM_1, NUM_2, NUM_3, NUM_F,
    COM_1, COM_2, COM_3, COM_4, COM_F,
    STR_1, STR_2, STR_3, STR_4, STR_F,
    SEP_F,
    OP_1, OP_2, OP_F1, OP_F2, OP_F3, OP_F4,
    EOF_F
} fsm_state_t;

typedef struct token {
    token_type_t token_type;
    size_t attr_size;
    size_t allocated;
    void * attr;
} token_t;

typedef struct scanner {
    fsm_state_t state;
} scanner_t;


token_t get_next_token(scanner_t *scanner);

void scanner_init(scanner_t *scanner);


#endif

/***                             End of scanner.h                        ***/
