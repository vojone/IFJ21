/******************************************************************************
 *                                  IFJ21
 *                                scanner.h
 * 
 *                   Authors: Vojtěch Dvořák (xdvora3o)
 *   Purpose: File, that contains declarations of lexer (scanner) functions
 * 
 *                    Last change: 25. 11. 2021
 *****************************************************************************/ 

/**
 * @file scanner.h
 * @brief File, that contains declarations of lexer (scanner) functions
 * 
 * @authors Vojtěch Dvořák (xdvora3o)
 */ 

#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "dstring.h"
#include "tables.h"

#define UNSET -1
 
#define INTERNAL_ERROR 99 /**< Means e.g. allocation error */
#define LEXICAL_ERROR 1 

/**
 * @brief All possible token types
 */
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
    ERROR_TYPE,
    INT_ERR_TYPE,
    TOK_TYPE_NUM
} token_type_t;

/**
 * @brief Token structure
 */ 
typedef struct token {
    token_type_t token_type;
    size_t first_ch_index;
    void * attr;
} token_t;

/**
 * @brief All possible states of FSM ( = base of scanner impelemntation)
 * @note If token ends with _F suffix, that means, that state is finite
 */ 
typedef enum fsm_state {
    INIT,
    ID_F,
    INT_F,
    NUM_1, NUM_2, NUM_3, NUM_F1, NUM_F2,
    COM_1, COM_2, COM_3, COM_F1, COM_F2, COM_F3,
    STR_1, STR_2, STR_3, STR_4, STR_F,
    SEP_F,
    OP_1, OP_2, OP_F1, OP_F2, OP_F3, OP_F4,
    EOF_F,
    STATE_NUM
} fsm_state_t;

/**
 * @brief Data types for cursor coordinates
 */ 
typedef long unsigned int pos_t;

typedef enum pos_indexes {
    ROW, COL, COORD_NUM
} pos_indexes_t;

/**
 * @brief Scanner structure
 */ 
typedef struct scanner {
    string_t str_buffer; /**< String buffer used for collecting value of tokens */
    size_t first_ch_index; /**< Index of first character of currently processed token */

    char input_buffer; /**< Sometimes is necessary to "push" character back to stdin*/
    bool is_input_buffer_full; /**< Flag that signalizes validity of data in input_buffer */

    token_t tok_buffer; /**< Additional buffer for one token */
    bool is_tok_buffer_full; /**< Flag that signalizes validity of data in tok_buffer */

    pos_t cursor_pos[COORD_NUM]; /**< Current cursor position (position of char, that will be processed)*/

    fsm_state_t state; /** Current state of FSM */
} scanner_t;

/**
 * @brief Transition function (type) that contains rules for changing 
 *        automata state or choosing token type
 */ 
typedef void (*trans_func_t)(char, token_t *, scanner_t *);


/**
 * @brief Sets initial values to token structure
 */ 
void token_init(token_t *token);

/**
 * @brief Finds attribute of given token
 * @return Pointer to token attribute (string)
 */ 
char * get_attr(token_t *token, scanner_t *sc);

/**
 * @brief Reads characters from stdin (or from buffer) and tries to make token from it
 * @param scanner Structure that contains necessary buffers and variables to scan input correctly
 * @return Token structure with type of token and its attribute
 * @note If type of returned token is ERROR_TYPE, that means a lexical error occurence
 */
token_t get_next_token(scanner_t *scanner);

/**
 * @brief Reads token from stdin (or from buffer) AND RETURNS IT TO SCANNER BUFFER
 * @param scanner Structure that contains necessary buffers and variables to scan input correctly
 * @return Token structure with type of token and its attribute 
 * @note Is important NOT TO FREE token attribute when token is got by lookahead
 * @note If type of returned token is ERROR_TYPE, that means a lexical error occurence
 */
token_t lookahead(scanner_t *scanner);

/**
 * @brief Converts enumeration type to cstring, where is described meaning of token type
 * @note Can be used e.g. in suggestions and in error messages
 * @param tok_type type that should be translated to a normal language
 * @return pointer to string (static alocated -> DONT FREE IT), or null if tok_type is not valid 
 */ 
char *tok_type_to_str(token_type_t tok_type);

/**
 * @brief Inits scanner structure
 * @return If it returns INTERNAL_ERROR error ocurred during initialization
 */ 
int scanner_init(scanner_t *scanner);

/**
 * @brief Destroys scanner structure
 */ 
void scanner_dtor(scanner_t *scanner);


#endif

/***                             End of scanner.h                        ***/
