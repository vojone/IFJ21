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
#include <stdbool.h>
#include "dstring.h"
#include <stdarg.h>
#include "symtable.h"

typedef struct gen {
    void *declare_variable;
} gen_t;

void generate_init();
// void generate_main();


void generate_start_function(const char * name);
void generate_parameters(string_t params, int param_count);
void generate_parameter(const char * name);
void generate_end_function(const char * name);
void generate_call_function(const char * name);

/**
 * Prebuild functions
 */ 
void generate_write_function();


void generate_declare_variable(const char * name);
void generate_value_push( sym_type_t type, sym_dtype_t dtype, const char * name );
void generate_assign_variable( const char *name );
void generate_operation(grm_sym_type_t type);



const char *convert_type(sym_dtype_t dtype);
void code_print(const char *const _Format, ...);




#endif

/***                            End of generator.h                         ***/
