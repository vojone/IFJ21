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

#ifndef PRECEDENCE_PARSER_H
#define PRECEDENCE_PARSER_H

#define STACK_MAX 100

#define PP_ERROR -1

enum terminals {
    HASH, MINUS, MULT, DIV, INT_DIV, ADD, SUB, CONCAT,
    LT, LTE, GT, GTE, EQ, NOTEQ, L_PAR, R_PAR, OPERAND, 
    STOP_SYM, TERM_NUM
} terminals_t;

#define NON_TERM TERM_NUM

typedef struct dstack {
    int data[STACK_MAX];
    int top;
} dstack_t;

bool parse_expression(scanner_t *sc);

#endif


/***                     End of precedence_parser.h                        ***/
