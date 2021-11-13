/******************************************************************************
 *                                  IFJ21
 *                            precedence_parser.h
 * 
 *          Authors: Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 *            Purpose:  Header file of precedence parsing functions
 * 
 *                    Last change:
 *****************************************************************************/ 

#include "scanner.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "dstack.h"

#ifndef PRECEDENCE_PARSER_H
#define PRECEDENCE_PARSER_H

#define PP_ERROR -1

/**
 * @brief Return codes
 */ 

#define UNDEFINED -1
#define EXPRESSION_SUCCESS 0
#define LEXICAL_ERROR 1
#define EXPRESSION_FAILURE 2
#define INTERNAL_ERROR 99

/**
 * @brief Type of element in expression (and in operator grammar)
 */ 
typedef enum grm_sym_type {
    HASH, MINUS, MULT, DIV, INT_DIV, ADD, SUB, CONCAT,
    LT, LTE, GT, GTE, EQ, NOTEQ, L_PAR, R_PAR, OPERAND, 
    STOP_SYM, TERM_NUM
} grm_sym_type_t;

#define NON_TERM TERM_NUM /**< Nonterminal symbol */
#define LOWER 
#define GREATER 
#define SAME 

#define NONE -1 /**< Says that operator doesn't have any operands */

/**
 * @brief Structure that represents element in expression
 */ 
typedef struct exp_el {
    grm_sym_type_t type; /**< Determines type of element (and it is index to precedence table) */
    void *value; /**< Value of element (or pointer to symbol table) */
    int left_op_type, right_op_type; /**< Data type of left and right operand (MAX number of operands is 2)*/
} exp_el_t;

int parse_expression(scanner_t *sc);

#endif


/***                     End of precedence_parser.h                        ***/
