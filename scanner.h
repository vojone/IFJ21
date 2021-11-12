/******************************************************************************
 *                                  IFJ21
 *                                scanner.h
 * 
 *          Authors: Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 *   Purpose: File, that contains declarations of lexer (scanner) functions
 * 
 *                    Last change: 30. 10. 2021
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
#include "dstring.h"
#include "tables.h"

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
    ERROR_TYPE
} token_type_t;

/**
 * @brief Token structure
 */ 
typedef struct token {
    token_type_t token_type;
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
    NUM_1, NUM_2, NUM_3, NUM_F,
    COM_F1, COM_F2, COM_F3, COM_F4, COM_F5,
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
    string_t str_buffer; /**< Temporary string buffer used for collecting value of token */

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
 * @brief Inits scanner structure
 */ 
void scanner_init(scanner_t *scanner);

/**
 * @brief Destroys scanner structure
 */ 
void scanner_dtor(scanner_t *scanner);


#endif

/***                             End of scanner.h                        ***/
