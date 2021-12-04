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

#define MAX_CNT_LEN 32

/*** Structures and functions for handling internal representation of code ***/
/**For example of usage @see gen_test.cpp**/

/**
 * @brief Element of DLL that represents instruction of target code
 */
typedef struct instruction {
    string_t content; /**< Content of given instruction */
    struct instruction *next; /**< Next instruction */
    struct instruction *prev; /**< Instruction before */
} instr_t;


DSTACK_DECL(instr_t *, instr)

/**
 * @brief Double linked list that represents target program 
 * @warning Not all operations with DLL behaves same as in other implementations
 */
typedef struct program {
    instr_t *first_instr; /**< First instruction of program (DLL) */
    instr_t *last_instr;  /**< Last instruction of program (DLL) */
    size_t while_nest_lvl;   //number of whiles we are currently in
    instr_t *while_topmost; //pointer to the while which is first in the current scope
} prog_t;

DSTACK_DECL(prog_t, prog)


/**
 * @brief Creates new empty intruction
 * @return Pointer to newly created instruction or null if an error occured
 */ 
instr_t *new_instruction();

/**
 * @brief Sets content of instruction (uses dynamic string @see dstring.h)
 * @return EXIT_SUCCESS, if everything was OK (otherwise INTERNAL_ERROR)
 */ 
int set_instruction(instr_t *instr, const char *const _Format, va_list args);


/**
 * @brief Correctly frees all resources held by intruction
 */ 
void instr_dtor(instr_t *instr);


/**
 * @brief Inits new program structure
 * @param program Pointer to program that will be initialized
 */ 
void init_new_prog(prog_t *program);


/**
 * @brief Frees all resources held by given program structure
 * @note After calling it, program will have same state as it has after initialization
 * @param program Pointer to program that will be deleted
 */ 
void program_dtor(prog_t *program);


/**
 * @brief Returns pointer to last instruction of given program
 * @note If program is empty, returns NULL
 */ 
instr_t *get_last(prog_t *program);


/**
 * @brief Returns pointer to first instruction of given program
 * @note If program is empty, returns NULL
 */
instr_t *get_first(prog_t *program);


/**
 * @brief Returns pointer to right neighbour of given instruction (next instruction)
 * @note If fiven instruction is last, it returns NULL
 */
instr_t *get_next(instr_t *instr);

/**
 * @brief Returns last instruction of given program
 * @note If fiven instruction is last, it returns NULL
 */
instr_t *get_prev(instr_t *instr);

/**
 * @brief Appends new instruction to given program
 * @note It's variadic function, so it can be used as standard printf
 */ 
int app_instr(prog_t *dst, const char *const _Format, ...);

/**
 * @brief Appends program (instruction sequence) to another program
 * @warning Appended program cannot be destroyed! (something can be double freed)
 */ 
void app_prog(prog_t *dst, prog_t *prog);

/**
 * @brief Inserts new instruction after given instruction
 * @note It's variadic function, so it can be used as standard printf
 * @warning If given pointer to instruction in argument is NULL it does nothing
 */ 
int ins_after(prog_t *dst, instr_t *instr, const char *const _Format, ...);


/**
 * @brief Inserts new instruction before given instruction
 * @note It's variadic function, so it can be used as standard printf
 * @warning If given pointer to instruction in argument is NULL it does nothing
 */
int ins_before(prog_t *dst, instr_t *instr, const char *const _Format, ...);


/**
 * @brief Reverts instructions in given interval (including borders)
 * @note Interval is given by to pointers to instructions, use 
 *       get_next/get_prev to make reversion only inside this interval
 * @warning If an error occured during the reversing, it does nothing
 */
void revert(prog_t *dst, instr_t *from, instr_t *to);

/**
 * @brief Prints program to standard output and adds '\n' after every instruction 
 */
void print_program(prog_t *source);



/***              End of internal repre. functions and structures          ***/




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
void generate_init(prog_t *dst);

/**
 * *---------FUNCTIONS---------
 */ 

/**
 * @brief generates the start of function definition
 * @name function name
 */ 
void generate_start_function(prog_t *dst,const char * name);

/**
 * @brief generates parameters of a function
 * @note they are declared reversely because they're popped from stack
 * @param param_names TYPE: tok_stack_t - the stack of tokens with parameter names
 */ 
void generate_parameters(prog_t *dst, void *sym_stack,symtab_t *symtab , void *param_names, scanner_t * scanner);

/**
 * @brief generates one parameter
 * @param name of the parameter
 */ 
void generate_parameter(prog_t *dst,const char * name);

/**
 * @brief generates the code needed to end a function definition
 */ 
void generate_end_function(prog_t *dst,const char * name);

/**
 * 
 */ 
void generate_call_function(prog_t *dst,const char * name);

/**
 * @brief pushes NILs to stack
 * @param n how many NIL returns to push
 */
void generate_additional_returns(prog_t *dst,size_t n); 

/**
 * @brief generates return statement
 */ 
void generate_return(prog_t *dst);

/**
 * @brief generates pops for eliminating redundant values on stack 
 */ 
void generate_dump_values(prog_t *dst, size_t save_n, size_t delete_n);

/**
 * *---------VARIABLES---------
 */

/**
 * @brief declares a variable
 * @param var_id identifier of the variable
 * @note we will search the actual unique identifier in the symtab
 */ 
void generate_declare_variable( prog_t *dst,void *sym_stack,symtab_t *symtab , token_t *var_id, scanner_t * scanner);

/**
 * @brief assigns variables from stack to variables
 * @param param_names TYPE: tok_stack_t - the stack of tokens with variable names
 */ 
void generate_multiple_assignment(prog_t *dst, void *sym_stack,symtab_t *symtab , void *param_names, scanner_t * scanner);

/**
 * @brief pops the value from top of stack into variable
 * @param name name of the variable
 */ 
void generate_assign_value(prog_t *dst,const char * name); 


/**
 * @brief pushes a value (variable or static) to the stack
 * @param name identifier or the actual value to push
 * @note used in expression parsing
 */ 
void generate_value_push( prog_t *dst,sym_type_t type, sym_dtype_t dtype, const char * name );


/**
 * *--------CONDIONS--------
 */ 
void generate_if_condition(prog_t *dst,size_t n);

void generate_if_end(prog_t *dst,size_t n);

void generate_else_end(prog_t *dst, size_t n);


/**
 * *--------LOOPS--------
 */ 
void generate_while_condition_beginning(prog_t *dst,size_t n);

void generate_while_condition_evaluate(prog_t *dst,size_t n);

void generate_while_end(prog_t *dst,size_t n);


/**
 * *---------OPERATIONS---------
 */ 
void generate_operation_add(prog_t *dst);
void generate_operation_sub(prog_t *dst);
void generate_operation_mul(prog_t *dst);
void generate_operation_div(prog_t *dst);
void generate_operation_div(prog_t *dst);
void generate_operation_idiv(prog_t *dst);
void generate_operation_unary_minus(prog_t *dst);

void generate_operation_eq(prog_t *dst);
void generate_operation_neq(prog_t *dst);

void generate_operation_gt(prog_t *dst);
void generate_operation_lt(prog_t *dst);
void generate_operation_gte(prog_t *dst);
void generate_operation_lte(prog_t *dst);

void generate_operation_concat(prog_t *dst);
void generate_operation_strlen(prog_t *dst);


/**
 * *---------BUILTIN---------
 */ 

/**
 * @brief generates prebuilt write function
 * @note supports exactly one argument
 */ 
void generate_write_function(prog_t *dst);

/**
 * @brief generates prebuilt reads function
 */ 
void generate_reads_function(prog_t *dst);

/**
 * @brief generates prebuilt readi function
 */ 
void generate_readi_function(prog_t *dst);

/**
 * @brief generates prebuilt readn function
 */ 
void generate_readn_function(prog_t *dst);

/**
 * @brief generates prebuilt tointeger function
 */ 
void generate_tointeger_function(prog_t *dst);

/**
 * @brief generates prebuilt chr function
 */ 
void generate_chr_function(prog_t *dst);

/**
 * @brief generates prebuilt ord function
 */ 
void generate_ord_function(prog_t *dst);

/**
 * @brief generates prebuilt substr function
 */ 
void generate_substr_function(prog_t *dst);


/**
 * *BUILTIN INTERNAL FUNCTIONS - cannot be called from IFJ21 source code
 */ 

/**
 * @brief checks the operand on top of the stack for 0
 * @note if there is a zero it will exit exit with error 9
 */ 
void generate_checkzero_function_int(prog_t *dst);

/**
 * @brief checks the operand on top of the stack for 0
 * @note if there is a zero it will exit exit with error 9
 */ 
void generate_checkzero_function_float(prog_t *dst);

/**
 * @brief checks one operand on top of the stack for nil
 * @note if there is a nil it will exit exit with error 8
 */ 
void generate_checknil_function_single(prog_t *dst);

/**
 * @brief checks two operands on top of the stack for nil
 * @note if there is a nil it will exit exit with error 8
 */
void generate_checknil_function_double(prog_t *dst);

/**
 * @brief function to get negative value on top of stack
 */
void generate_unaryminus_function(prog_t *dst);

/**
 * @brief function to convert values on top of the stack to same types
 * @note if there is float and int it will make 2 floats 
 */
void generate_same_types(prog_t *dst);

/**
 * @brief convert 2 values on top of the stack to floats
 */
void generate_force_floats(prog_t *dst);

/**
 * @brief convert 2 values on top of the stack to ints
 */
void generate_force_ints(prog_t *dst);

/**
 * @brief if there is not bool on top of stack it will convert the value to bool
 */
void generate_tobool(prog_t *dst_code);


/**
 * *---------VARIOUS---------
 */

/**
 * @brief converts the value to number
 */ 
void generate_int2num(prog_t *dst_code);



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
void code_print(prog_t *dst, const char *const _Format, ...);

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
