/******************************************************************************
 *                                  IFJ21
 *                             parser_topdown.h
 * 
 *          Authors: Radek Marek (xmarek77), Vojtěch Dvořák (xdvora3o), 
 *                  Juraj Dědič (xdedic07), Tomáš Dvořák (xdvora3r)
 * 
 *              Purpose: Header file for recursive descent parser
 * 
 *                       Last change: 25. 11. 2021
 *****************************************************************************/

/**
 * @file parser-topdown.h
 * @brief Header file for recursive descent parser
 * 
 * @authors Radek Marek (xmarek77), Vojtěch Dvořák (xdvora3o), 
 *          Juraj Dědič (xdedic07), Tomáš Dvořák (xdvora3r)
 */ 

#ifndef TOPDOWN_H
#define TOPDOWN_H


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "dstring.h"
#include "scanner.h"
#include "dstack.h"
#include "precedence_parser.h"
#include <stdarg.h>
#include "symtable.h"
#include "generator.h"
#include "dstack.h"


#define DEBUG true /**< If true, prints debug log to stderr */
#define PRINT_WARNINGS true /**< If true, prints warning to stderr about some implicit actions (see documentation for more)*/

/**
 * @brief Return codes of parser 
 */ 
typedef enum return_codes {
    PARSE_SUCCESS = 0,
    LEXICAL_ERROR_ = 1,
    SYNTAX_ERROR  = 2,
    SEMANTIC_ERROR_DEFINITION = 3,
    SEMANTIC_ERROR_ASSIGNMENT = 4,
    SEMANTIC_ERROR_PARAMETERS = 5,
    SEMANTIC_ERROR_EXPRESSION = 6,
    SEMANTIC_ERROR_OTHER      = 7,
    UNEXPECTED_NIL_ERROR      = 8,
    DIVISION_BY_ZERO_ERROR    = 9,
    INTERNAL_ERROR_ = 99
} return_codes_t;


#define DECLARATION_COUNTER_MAX_LEN 32 /**< Number of digits that can the declaration counter reach */

/**
 * @brief Holds data that are needs to be propagate during parsing
 */ 
typedef struct parser {
    token_t * curr_func_id; /**< Pointer to token with identifier of function, that is currently parsed */
    size_t decl_cnt; /**< Declaration counter for making unique identifiers in target code */
    size_t cond_cnt; /**< Condition counter for making unique labels for if else in target code */
    size_t loop_cnt; /**< Condition counter for making unique labels for while loops in target code */
    bool found_return; /**< Flag for propagation info about found returns */

    tok_stack_t decl_func; /**< Stack with declared functions (to check if they were defined)*/

    int return_code;
    bool reached_EOF;

    scanner_t * scanner;
    symbol_tables_t sym;
    prog_t dst_code;
} parser_t;

typedef struct rule {
    int (* rule_function)(parser_t *);
    token_t rule_first;
    bool attrib_relevant;
} rule_t;

int error_rule();

/**
 * @return Pointer to rule that corresponds to given index (or NULL if index is to high)
 */
rule_t * get_global_rule(size_t index);

/**
 * @return Pointer to rule that corresponds to given index (or NULL if index is to high)
 */
rule_t * get_inside_rule(size_t index);

/**
 * @brief Sets symtab to elder symbol table of old outer context 
 */ 
void to_outer_ctx(parser_t *parser);

/**
 * @brief Creates new symbol table (new context) and sets symtable to it
 */ 
void to_inner_ctx(parser_t *parser);

/**
 * @brief Recognizes if current context is global context (due to symbol table stack)
 * @return True if parser is in global context
 */ 
bool is_global_ctx(parser_t *parser);

/**
 * @brief sets the parser and scanner to use
 * @note there can be only one instance i guess, because of the parser static var
 */ 
void parser_setup(parser_t *parser, scanner_t *scanner);

/**
 * @brief Frees all resources hold by parser and its components
 */ 
void parser_dtor(parser_t *parser);


/**
 * @brief Checks if all declared (and called) functions were defined
 * @return If called function was not defined, it returns error code, if function was not called, it prints warning
 */ 
int check_if_defined(parser_t *parser);


/**
 * @brief Starts parsing
 */ 
int parse_program(parser_t *parser);


/**
 * @brief Parses statements list outside of a function
 */
int global_statement_list(parser_t *parser);


/**
 * @brief Parses statements list inside of a function
 */
int statement_list(parser_t *parser);


/**
 * @brief Parses statements outside of a function
 */
int global_statement(parser_t *parser);


/**
 * @brief Parses statements inside of a function
 */
int statement(parser_t *parser);


/**
 * @brief Check if data type at r side can be converted to variable data type
 * @return True if yes
 */ 
bool is_convertable(sym_dtype_t var_type, sym_dtype_t r_side_type);


/**
 * @brief Resolves compatibility of data types in assignment
 */ 
bool is_valid_assign(prog_t *dst_code, sym_dtype_t var_type, 
                     sym_dtype_t r_side_type);


/**
 * @brief Parses lside of assignment (supports also multiple assignment)
 * @param start_id Pointer to token with first identifier
 * @param id_types Pointer to initalized string where will be written data types of all identifiers
 * @param id_number Pointer to integer where will be written amount of identifiers on left side of assignment
 */ 
int assignment_lside(parser_t *parser, token_t* start_id, 
                     string_t *id_types, tok_stack_t *var_names);


/**
 * @brief Parses right side of assignment
 */ 
int assignment_rside(parser_t *parser, string_t *id_types, string_t *rside);


/**
 * @brief Frees everything inside given stack of programs
 */
void prog_stack_deep_dtor(prog_stack_t *prog);


/**
 * @note This function expects that the current token is identifier
 */ 
int assignment(parser_t *parser, token_t t);


/**
 * @brief Converts keyword (must be data type) to enum type
 * @note If keyword is not recognized as valid data type specifier returns INT
 */ 
sym_dtype_t keyword_to_dtype(token_t * t, scanner_t *sc);


/**
 * @brief Inverse to keyword_to_dtype
 */ 
char *dtype_to_keyword(sym_dtype_t dtype);


/**
 * @brief Inserts variable into current symbol table
 */ 
void ins_var(parser_t *parser, token_t *id_token, 
             sym_status_t status, sym_dtype_t dtype);


/**
 * @brief Updates status of variable defined by pointer to symbol table
 * @note If pointer is NULL it does nothing
 */ 
void set_var_status(tree_node_t *var, sym_status_t new_stat);


/**
 * @brief Updates was_used flag of given symbol
 * @note If pointer is NULL it does nothing
 */ 
void set_use_flag(tree_node_t *symbol, bool new_s);


/**
 * @brief Parses assignment after declaration of variable in function
 * @warning This function expects that the current token was get by lookahead
 * @note If everything went well, changes variable status to defined
 */
int local_var_assignment(parser_t *parser, sym_status_t *status, 
                         sym_dtype_t dtype, token_t *var_id);


/**
 * @brief Parses data type part variable declaration (e.g ": integer")
 * @return PARSE SUCCESS if everthing was OK
 */ 
int local_var_datatype(parser_t *p, token_t *curr_tok, sym_dtype_t *var_type);


/**
 * @brief Parses declaration of local variable in function
 */ 
int parse_local_var(parser_t *parser);


/**
 * @brief Parses prolog at the start of the program
 */ 
int parse_require(parser_t *parser);


/**
 * @brief Inserts function into symbol table or updates existing element in current symbol table with given data
 */ 
void ins_func(parser_t *parser, token_t *id_token, sym_data_t *data);


/**
 * @brief Parses function return data types (in declaration of function)
 */ 
int func_dec_returns(parser_t *parser, string_t *returns);


/**
 * @brief Parses function parameter list in declaration of function
 */ 
int func_dec_params(parser_t *parser, string_t *params);


/**
 * @brief Parses declaration of function in global scope (in other scopes it is not valid)
 */ 
int parse_function_dec(parser_t *parser);


/**
 * @brief Searches for function in current symbol table and initializes symbol data due to result
 * @note If symbol was declared initializes structure passed as argument to values from symbol table
 * @return SEMANTIC_ERROR_DEFINITION if function was declared and defined (and data structure is not initialized), 
 *         otherwise returns PARSE_SUCCESS
 */ 
int check_if_declared(parser_t *parser, bool *was_decl, tree_node_t *id_tok,
                      token_t *func_id, sym_data_t *sym_data);


/**
 * @brief Parses prolog of function parameters (parameter name and ':')
 */
int func_def_params_prolog(parser_t *parser, token_t *param_id);


/**
 * @brief Parses parameters in function definition (and makes semantics checks)
 */ 
int func_def_params(parser_t *p, token_t *id_token, 
                    bool was_decl, sym_data_t *f_data);


/**
 * @brief Checks validity of function signature
 * @param id if true, function identifier was found
 * @param left_bracket if true, left bracket was found
 */ 
int check_function_signature(parser_t *parser, bool id, bool left_bracket);


/**
 * @brief Parses function return values when function is defined
 */ 
int func_def_returns(parser_t *p, token_t *id_token, 
                     bool was_decl, sym_data_t *f_data);


/**
 * @brief Parses function tail (end keyword)
 */ 
int func_def_epilogue(parser_t *parser);


/**
 * @brief Checks if function has return statement inside (if function returns something)
 * @warning If warnings are turned off, it does nothing
 */ 
int check_return(parser_t *parser, tree_node_t *id_fc);


/**
 * @brief Parses function definition 
 */ 
int parse_function_def(parser_t *parser);


/**
 * @brief Parses function arguments when function is called
 * @param func_sym Pointer to smbol table where is the function (it cannot be NULL)
 * @note Functon argument can be also expression as well as variable or immediate value
 */ 
int parse_function_arguments(parser_t *parser, tree_node_t *func_sym);


/**
 * @brief Parses if statement
 * @note Expects that the 'if' is already read and the first token is going to be a condition (expression)
 * @note Also handles else branch currently
 */  
int parse_if(parser_t *parser);


/**
 * @brief Parses return with following expression(s)
 */ 
int parse_return(parser_t *parser);


/**
 * @brief Parses while statement
 */ 
int parse_while(parser_t *parser);

/**
 * @brief Finds the appropriate rule
 * @return Returns reference to rule function
 * @param t Token based on which is going to be decided what rule to use
 * @param ruleset_function is a function, that contains array with ruleset
 */
rule_t *determine_rule(parser_t *p, token_t t, rule_t *(*ruleset_f)(size_t));

/**
 * @brief parses an identifier inside a function
 * @note decides wheter the rule to use is a function call or an assignment
 */
int parse_identifier(parser_t *parser); 

/**
 * @brief parses an identifier in global scope
 * @note workaround because parse_function_call expects the identifier to be already read
 */
int parse_global_identifier(parser_t *parser); 

/**
 * @brief parses an EOF inside a function
 * @note basically an error rule with additional error message
 */ 
int EOF_fun_rule(parser_t *parser);

/**
 * @brief parses an EOF inside a function
 * @note basically an error rule with additional error message
 */ 
int EOF_global_rule(parser_t *parser);

/**
 * @brief Parses global variable definitions
 */
int parse_global_var(parser_t *parser);

/**
 * @brief Parses else branch 
 */
int parse_else(parser_t *parser);

/**
 * @brief Parses end statement
 */
int parse_end(parser_t *parser);

/**
 * @brief checks value assignment when defining a variable
 */ 
bool value_assignment(parser_t *parser);

/**
 * @note might use in future 
 */ 
int else_branch(parser_t *parser);


/**
 * @brief parses the function call.
 * @param func_sym Pointer to smbol table where is the function (it cannot be NULL)
 * @note presumes that the function IDENTIFIER was already read and the first token is going to be opening bracket
 */ 
int parse_function_call(parser_t *parser, tree_node_t *func_sym);


/**
 * @brief Displays error about getting a different token than expected
 * @param expected the token which is expected
 * @param t the token which we got
 * */
void error_unexpected_token(parser_t *parser, char * expected, token_t t);

/**
 * @brief Displays error from semantic check 
 * @param message the text which is displayed
 * @param t the token which we got
 * */
void error_semantic(parser_t *parser, const char * _Format, ...);


/**
 * @brief Prints formated warnings to stderr
 */ 
void warn(parser_t *parser, const char * _Format, ...);

/**
 * @param expecting The token type to expect to be the next
 * @brief Checks wheter the expected token type is at the next place
 * @return returns true the expected token type is there
 **/
bool lookahead_token(parser_t *parser, token_type_t expecting);

/**
 * @param expecting The token type to expect to be the next
 * @brief Checks wheter the next token has the specified type and attribute
 * @return returns true if both expected type and attribute are equal to real ones
 **/
bool lookahead_token_attr(parser_t *p, token_type_t exp_type, char * exp_attr);

/**
 * @param expecting The token type to expect
 * @brief Shows error if there is an unexpected token type
 * @return returns true the expected token type is there
 **/
bool check_next_token(parser_t *parser, token_type_t expecting);

/**
 * @param expecting The token type to expect
 * @param expecting_attr The token attribute to expect
 * @brief Shows error if there is an unexpected token type or attribute
 * @return returns true the expected token is there
 **/
bool check_next_token_attr(parser_t *p, token_type_t exp_type, char * exp_attr);

/**
 * @param expecting The token type to expect
 * @brief Compares the expected token type to token 't'
 * @return returns true the expected token types are equal
 **/
bool compare_token(token_t t, token_type_t expecting);

/**
 * @param expecting_type The token type to expect
 * @param expecting_attr The token attribute to expect
 * @brief Compares the expected token type & parameter to token 't'
 * @return returns true the token types & parameters are equal
 **/
bool compare_token_attr(parser_t *parser, token_t t, 
                        token_type_t exp_type, char * exp_attr);

/**
 * @return True if token is datatype
 */ 
bool is_datatype(parser_t *parser, token_t t);

/**
 * @brief checks if the next token is datatype
 */ 
int parse_datatype(parser_t *parser);


/**
 * @brief shows error if token is not considered expression
 * @param t the token which will be checked
 * @return returns true if token one of types below 
 * @note checks only for INTEGER, NUMBER or STRING
 */ 
bool is_expression(parser_t *parser, token_t t);


/**
 * @brief Returns first character of given string (primary returned type of function)
 * @return Primary return type of function
 */ 
sym_dtype_t prim_dtype(string_t *type_string);


/**
 * @brief debug printing
 * @note can be disabled by DEBUG static global variable
 * @note to disable the "Got token at:" and "^ Lookahead ^", delete it in the scanner.c file. My apologies for the inconvenience
 */
void debug_print(const char *const _Format, ...);


#endif

/***                        End of parser_topdown.h                        ***/
