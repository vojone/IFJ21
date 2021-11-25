/******************************************************************************
 *                                  IFJ21
 *                               generator.h
 * 
 *          Authors: Juraj Dědič (xdedic07), Tomáš Dvořák (xdvora3r)
 *                    Purpose: Header file of code generator
 * 
 *                    Last change: 25. 11. 2021
 *****************************************************************************/

/**
 * @file generator.h
 * @brief Header file of code generator
 * 
 * @authors Juraj Dědič (xdedic07), Tomáš Dvořák (xdvora3r)
 */ 

#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdio.h>
#include <stdlib.h>
#include "dstack.h"
#include <stdbool.h>
#include <stdarg.h>
#include "symtable.h"
#include "scanner.h"
#include "precedence_parser.h"

typedef struct gen {
    void *declare_variable;
} gen_t;

typedef struct char_mapping{
    char input[3];
    char output[4];
} char_mapping_t;

/**
 * @brief inicializes the code
 */ 
void generate_init();

/**
 * *---------FUNCTIONS---------
 */ 

/**
 * @brief generates the start of function definition
 * @name function name
 */ 
void generate_start_function(const char * name);

/**
 * @brief generates parameters of a function
 * @note they are declared reversely because they're popped from stack
 * @param param_names TYPE: tok_stack_t - the stack of tokens with parameter names
 */ 
void generate_parameters( void *sym_stack,symtab_t *symtab , void *param_names, scanner_t * scanner);

/**
 * @brief generates one parameter
 * @param name of the parameter
 */ 
void generate_parameter(const char * name);

/**
 * @brief generates the code needed to end a function definition
 */ 
void generate_end_function(const char * name);

/**
 * 
 */ 
void generate_call_function(const char * name);

/**
 * *---------VARIABLES---------
 */

/**
 * @brief declares a variable
 * @param var_id identifier of the variable
 * @note we will search the actual unique identifier in the symtab
 */ 
void generate_declare_variable( void *sym_stack,symtab_t *symtab , token_t *var_id, scanner_t * scanner);

/**
 * @brief assigns variables from stack to variables
 * @param param_names TYPE: tok_stack_t - the stack of tokens with variable names
 */ 
void generate_multiple_assignment( void *sym_stack,symtab_t *symtab , void *param_names, scanner_t * scanner);

/**
 * @brief pops the value from top of stack into variable
 * @param name name of the variable
 */ 
void generate_assign_value(const char * name); 


/**
 * @brief pushes a value (variable or static) to the stack
 * @param name identifier or the actual value to push
 * @note used in expression parsing
 */ 
void generate_value_push( sym_type_t type, sym_dtype_t dtype, const char * name );


/**
 * *--------CONDIONS--------
 */ 
void generate_if_condition(size_t n);

void generate_if_end(size_t n);

void generate_else_end(size_t n);


/**
 * *--------LOOPS--------
 */ 
void generate_while_condition_beginning(size_t n);

void generate_while_condition_evaluate(size_t n);

void generate_while_end(size_t n);


/**
 * *---------OPERATIONS---------
 */ 
void generate_operation_add();
void generate_operation_sub();
void generate_operation_mul();
void generate_operation_div();
void generate_operation_idiv();

void generate_operation_eq();
void generate_operation_gt();
void generate_operation_lt();
void generate_operation_gte();
void generate_operation_lte();

void generate_operation_concat();
void generate_operation_strlen();


/**
 * *---------BUILTIN---------
 */ 
/**
 * @brief generates prebuilt write function
 * @note supports exactly one argument
 */ 
void generate_write_function();


/**
 * *---------UTILITY---------
 */ 

/**
 * @brief gets the unique identifier from symtable
 * @param var_id the token which we're going to search
 */ 
string_t get_unique_name( void *sym_stack,symtab_t *symtab , token_t *var_id, scanner_t * scanner );

/**
 * @brief printf the code
 * @note ends each call witn \n
 */ 
void code_print(const char *const _Format, ...);

/**
 * @brief converts dtype into format used in ifjcode21
 * @param dtype input data type used in symtable
 * @return string with type for code generation
 */ 
const char *convert_type(sym_dtype_t dtype);

/**
 * @brief converts string from token format ie. "this is a string" to "this\032is\032string"
 * @param str the input string
 * @param out pointer to the output string
 */ 
void to_ascii(const char * str, string_t * out);




#endif

/***                            End of generator.h                         ***/
