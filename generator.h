/******************************************************************************
 *                                  IFJ21
 *                               generator.h
 * 
 *          Authors: Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 *                   Purpose: Header file of code generator
 * 
 *                    Last change:
 *****************************************************************************/

/**
 * @file generator.h
 * @brief Header file of code generator
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 */ 

#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdio.h>
#include <stdlib.h>
#include "dstack.h"
#include <stdbool.h>
#include <stdarg.h>
#include "symtable.h"

typedef struct gen {
    void *declare_variable;
} gen_t;

void generate_init();
// void generate_main();


void generate_start_function(const char * name);
void generate_parameters( void *sym_stack,symtab_t *symtab , void *param_names, scanner_t * scanner);
void generate_parameter(const char * name);
void generate_multiple_assignment( void *sym_stack,symtab_t *symtab , void *param_names, scanner_t * scanner);
void generate_assign_value(const char * name);
void generate_end_function(const char * name);
void generate_call_function(const char * name);

/**
 * Prebuild functions
 */ 
void generate_write_function();


void generate_declare_variable( void *sym_stack,symtab_t *symtab , token_t *var_id, scanner_t * scanner);
void generate_value_push( sym_type_t type, sym_dtype_t dtype, const char * name );
void generate_assign_variable( const char *name );

void generate_operation_add();
void generate_operation_sub();
void generate_operation_mul();
void generate_operation_div();
void generate_operation_idiv();

// void generate_operation(grm_sym_type_t type);


string_t get_unique_name( void *sym_stack,symtab_t *symtab , token_t *var_id, scanner_t * scanner );


const char *convert_type(sym_dtype_t dtype);
void code_print(const char *const _Format, ...);




#endif

/***                            End of generator.h                         ***/
