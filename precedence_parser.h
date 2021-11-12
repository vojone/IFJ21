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

typedef enum grm_sym_type {
    HASH, MINUS, MULT, DIV, INT_DIV, ADD, SUB, CONCAT,
    LT, LTE, GT, GTE, EQ, NOTEQ, L_PAR, R_PAR, OPERAND, 
    STOP_SYM, TERM_NUM
} grm_sym_type_t;


typedef struct exp_el {
    grm_sym_type_t type; 
} exp_el_t;


#define NON_TERM TERM_NUM

bool parse_expression(scanner_t *sc);

#endif


/***                     End of precedence_parser.h                        ***/
