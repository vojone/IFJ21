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
#include "generator.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "dstack.h"
#include "symtable.h"

#ifndef PRECEDENCE_PARSER_H
#define PRECEDENCE_PARSER_H

/**
 * @brief Return codes
 */ 
#define EXPRESSION_SUCCESS 0
#define LEXICAL_ERROR 1
#define EXPRESSION_FAILURE 2
#define SYNTAX_ERROR_IN_EXPR 2
#define MISSING_EXPRESSION -2
#define UNDECLARED_IDENTIFIER 3
#define SEM_ERROR_IN_EXPR 6
#define NIL_ERROR 8
#define DIV_BY_ZERO 9
#define INTERNAL_ERROR 99


/**
 * @brief Structure, that groups token buffer with given scanner to get tokens and their attributes easily
 */ 
typedef struct tok_buffer {
    scanner_t *scanner;
    token_t last;
    token_t current;
} tok_buffer_t;


/**
 * @brief Type of element in expression (and in operator grammar)
 */ 
typedef enum grm_sym_type {
    HASH, MINUS, MULT, DIV, INT_DIV, ADD, SUB, CONCAT,
    LT, LTE, GT, GTE, EQ, NOTEQ, L_PAR, R_PAR, OPERAND, 
    STOP_SYM, TERM_NUM
} grm_sym_type_t;


#define NON_TERM TERM_NUM /**< Nonterminal symbol */

/**
 * @brief Structure that represents element in expression
 */ 
typedef struct expr_el {
    grm_sym_type_t type; /**< Determines type of element (and it is index to precedence table) */
    sym_dtype_t dtype; /**< Data type of element in expression (string, integer, number) */
    bool is_zero;
    void *value; /**< Value of element (or pointer to symbol table) */
} expr_el_t;

#define UNDEFINED -1
#define ORIGIN -1 /**< Says that after reduction has nonterminal oriiginal data typ (e.g. ("abc") -> E.type = STR) */

typedef enum zero_prop_flags {
    NONE, FIRST, SECOND, ALL, ONE
} zero_prop_flags_t;

#define REDUCTION_RULES_NUM 16

typedef struct expr_rule {
    char * right_side;
    char * operator_types; /**< Specification of operator types (see get_rule() in .c file)*/
    sym_dtype_t return_type;
    zero_prop_flags_t zero_prop; /**< Specifies how is zero propagated */
    char *error_message; /**< Error message that is showed when semantic error occured*/
    void (*generator_function)();
} expr_rule_t;

int parse_expression(scanner_t *sc, void *sym_stack,
                     symtab_t *symtab, sym_dtype_t *ret_type);

#endif


/***                     End of precedence_parser.h                        ***/
