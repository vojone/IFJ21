/******************************************************************************
 *                                  IFJ21
 *                            precedence_parser.h
 * 
 *          Authors: Vojtěch Dvořák (xdvora3o), Juraj Dědič (xdedic07)
 *            Purpose: Header file of precedence parsing functions
 * 
 *                      Last change: 7. 12. 2021
 *****************************************************************************/ 

/**
 * @file precedence_parser.c
 * @brief Header file of precedence parsing functions
 * 
 * @authors Vojtěch Dvořák (xdvora3o), Juraj Dědič (xdedic07)
 *
 */


#ifndef PRECEDENCE_PARSER_H
#define PRECEDENCE_PARSER_H

#include "scanner.h"
#include "generator.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "dstack.h"
#include "symtable.h"

/**
 * @brief Return codes
 */ 
#define EXPRESSION_SUCCESS 0
#define EXPRESSION_FAILURE 2
#define SYNTAX_ERROR_IN_EXPR 2
#define MISSING_EXPRESSION 12
#define UNDECLARED_IDENTIFIER 3
#define SEMANTIC_ERROR_PARAMETERS_EXPR 5
#define SEM_ERROR_IN_EXPR 6
#define NIL_ERROR 8
#define DIV_BY_ZERO 9


#define PREVENT_ZERO_DIV true /**< If it is true it turns on semantic control of division by zero (there is also simple propagation of zero) */
#define PREVENT_NIL true /**< If it is true it turns on semantic control of using nil in expressions */
#define PRINT_EXPR_WARNINGS true /**< If it is true, precedence parser prints warnings when are some implicit actions performed */


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
    HASH, MINUS, POW, MOD, MULT, DIV, INT_DIV, ADD, SUB, CONCAT,
    LT, LTE, GT, GTE, EQ, NOTEQ, L_PAR, R_PAR, OPERAND, 
    STOP_SYM, TERM_NUM
} grm_sym_type_t;


#define NON_TERM TERM_NUM /**< Nonterminal symbol */

/**
 * @brief Structure that represents element in expression
 */ 
typedef struct expr_el {
    grm_sym_type_t type; /**< Determines type of element (and it is index to precedence table) */
    string_t dtype; /**< Data type of element in expression (string, integer, number) */
    bool is_zero;
    void *value; /**< Value of element (or pointer to symbol table) */
    bool is_fcall;
} expr_el_t;


DSTACK_DECL(expr_el_t, pp) /**< Declares stack with expr_els and its operations */


/**
 * @brief Structure of precedence parser
 */ 
typedef struct p_parser {
    expr_el_t on_top;
    expr_el_t on_input;

    pp_stack_t stack; /**< Main stack where reductions are performed */
    pp_stack_t garbage; /**< Garbage collecting stack where is stored every symbol that should be (or its part) freed */
    bool stop_flag; /**< Flag that stops main cycle in precedence parsing if there is probably end of expression */
    bool empty_expr; /**< Flag that signalizes empty expression */
    bool empty_cycle; /**< Flag that determines whether was performed any action (push to main stack or reduction) during main cycle */

    bool was_f_call;
    bool only_f_was_called; /**< If there is any other operantion than calling function in epxression it is set to false */

    size_t last_call_ret_num; /**< Number of return values of last called function */

    prog_t *dst_code; /**< Poiter to code where should be printed instructions */
} p_parser_t;


#define UNDEFINED -1
#define ORIGIN -1 /**< Says that after reduction has nonterminal oriiginal data typ (e.g. ("abc") -> E.type = STR) */

typedef enum zero_prop_flags {
    NONE, FIRST, SECOND, ALL, ONE
} zero_prop_flags_t;


#define REDUCTION_RULES_NUM 18

typedef struct expr_rule {
    char * right_side; /**< Right side of reduction rule */
    char * operator_types; /**< Specification of operator types (@see get_rule() in .c file)*/
    sym_dtype_t return_type; /**< Result type of operation */
    zero_prop_flags_t zero_prop; /**< Specifies how is zero propagated */
    char *error_message; /**< Error message that is showed when semantic error occured*/
    void (*generator_function)();
} expr_rule_t;


/**
 * @brief Determines if is token separator, that can be used in expressions
 */ 
bool is_allowed_separator(scanner_t *sc, token_t *token);


/**
 * @brief Determines whether is token nil keyword or not
 */ 
bool is_nil(scanner_t *sc, token_t *token);


/**
 * @brief Determines if operand (token) represents zero (number zero or empty string)
 */ 
bool is_zero(scanner_t *sc, token_t *token);


/**
 * @brief Resolves if token can be part of expression
 */ 
bool is_EOE(scanner_t *sc, token_t *token);


/**
 * @brief Tries to determine if current token is unary or binary minus operator (due to last_token)
 */ 
bool is_unary_minus(tok_buffer_t *tok_b);


/**
 * @brief Resolves operator type
 */ 
grm_sym_type_t operator_type(char first_c, char sec_c, tok_buffer_t *tok_b);


/**
 * @brief Transforms token to symbol used in precedence parser ( @see expr_el_t)
 */ 
int tok_to_type(tok_buffer_t *tok_b, bool *was_only_f_call);


/**
 * @brief Makes dynamic string from given character
 */ 
int make_type_str(string_t *dst, char type_c);


/**
 * @brief Returns primary type of return types (first return type)
 * @note When we use function in expression we will work only with primary return type in the rest of expr.
 */ 
sym_dtype_t prim_type(string_t *type_string);


/**
 * @brief Checks copatibility of data type string (returned from function or expression) and arg. type
 */ 
bool is_compatible_in_arg(prog_t *prog, char par_type, char arg_type);


/**
 * @brief Checks if expected token type end token type of fiven token are equal
 */
bool is_tok_type(token_type_t exp_type, token_t *t);


/**
 * @brief Checks whether expected token attribute end token attribute of fiven token are equal
 */
bool is_tok_attr(char *exp_attr, token_t *t, tok_buffer_t *tok_b);


/**
 * @brief Prints error msg to stderr
 */ 
void fcall_sem_error(tok_buffer_t *tok_b, char *f_name, char *msg);


/**
 * @brief Prints error msg to stderr
 */ 
void fcall_syn_error(tok_buffer_t *tok_b, char *f_name, char *msg);


/**
 * @brief Parses just one expression (argument) in function call
 */ 
int parse_arg_expr(size_t *arg_cnt, prog_t *dst_code, 
                   symbol_tables_t *syms, tok_buffer_t *tok_b, 
                   tree_node_t *symbol);

/**
 * @brief Parses character after argument (can be ')' or ',')
 * @return EXPRESSION_SUCCESS if there was expected character
 */ 
int after_argument(bool *closing_bracket, tree_node_t *symbol, 
                   size_t cnt, tok_buffer_t *tok_b);

/**
 * @brief Parses arguments in function call in expression (checks separators and expressions)
 */ 
int argument_parser(prog_t *dst_code, 
                    tree_node_t *symbol,
                    symbol_tables_t *syms,
                    tok_buffer_t *tok_b);

/**
 * @brief Parses function call inside expression (check if there is '(' whe nfunction is called)
 * @return EXPRESSION_SUCCESS if wverything was ok
 */ 
int fcall_parser(prog_t *dst_prog,
                 tree_node_t *symbol, 
                 symbol_tables_t *syms, 
                 tok_buffer_t *tok_b);


/**
 * @brief Processes identifier got on input
 */
int process_identifier(p_parser_t *pparser, 
                       tok_buffer_t *t_buff, 
                       symbol_tables_t *syms);


/**
 * @brief Creates expression element from current and last token
 */ 
int from_input_token(p_parser_t *pparser, 
                     tok_buffer_t *t_buff, 
                     symbol_tables_t *syms);


/**
 * @brief Creates stop symbol as expression element and initializes it
 */ 
expr_el_t stop_symbol();


/**
 * @brief Creates precedence sign expr. el. due to given parameter and initializes it
 */ 
expr_el_t prec_sign(char sign);


/**
 * @brief Creates operand expression element 
 */
expr_el_t operand();


/**
 * @brief Contains precedence table of operators in IFJ21
 */ 
char get_precedence(expr_el_t on_stack_top, expr_el_t on_input);


/**
 * @brief Additional function to get real expression element from enum type
 * @note Especially for better visualization of "to be reduced" sequence
 */ 
char *to_char_sequence(expr_el_t expression_element);


/**
 * @brief Contains rules of reduction on top of the pp_stack, operand possibilities and return data type of reduction
 * @note For explaning possible operands is used specific string:
 *       | separates possibilities for operands
 *       * every data type
 *       n number
 *       i integer
 *       s string
 *       z nil
 *       b bool
 *       ! inicates that cannot be zero
 *       ( types symbols after it defines type for resting operators (integer and number are compatible)
 *       ) returns from "must_be" mode to normal mode 
 *       Example: (nis|nis = First and sec operand can be number/integer/string and if it's e. g. string second must be string too
 */ 
expr_rule_t *get_rule(unsigned int index);


/**
 * @brief Pops operand stack if it is not empty
 */ 
expr_el_t safe_op_pop(bool *cur_ok, int *check_result, pp_stack_t *op_stack);


/**
 * @brief Returns true if two types are compatible
 */ 
bool is_compatible(string_t *dtypes1, string_t *dtypes2);


/**
 * @brief Resolves which data type attribut should have newly created nonterminal on stack
 * @return INTERNAL_ERROR if an error occurs otherwise EXPRESSION SUCCESS
 */ 
int resolve_res_type(string_t *res, expr_rule_t *rule, 
                      expr_el_t cur_op, bool cur_ok);


/**
 * @brief Resolve return code returned by type_check() function
 * @return SEM_ERROR_IN_EXPR or NIL_ERROR (if PREVENT_NIL is set to true and there is nil incompatibility)
 */ 
int get_tcheck_ret(expr_el_t *current_operand);


/**
 * @brief Checks current operand if it corresponds to expected data type (used in type_check function)
 * @return EXPRESSION_SUCCESS or INTERNAL_ERROR if there was an error in string operations
 */ 
int op_type_ch(char c, bool must_be_flag, bool *is_curr_ok, 
                   string_t *must_be, expr_el_t *current);


/**
 * @brief Performs type checking when precedence parser reducing the top of the stack
 * @note Type check is based on rules writen in get_rule() ( @see get_rule())
 */
int type_check(pp_stack_t op_stack, expr_rule_t *rule, string_t *res_type);


/**
 * @brief Gets from top of the stack sequence that will be reduced
 * @return Function can return INTERNAL_ERROR, so it should be checked
 */ 
int get_str_to_reduction(pp_stack_t *s, pp_stack_t *op, string_t *to_be_red);


/**
 * @brief Determines if result of reduction will be zero value (in some cases it is possible to find out it if is)
 */ 
bool resolve_res_zero(pp_stack_t operands, expr_rule_t *rule);


/**
 * @brief Determines if result of reduction will be zero value (in some cases it is possible to find out it if is)
 */ 
bool resolve_res_zero(pp_stack_t operands, expr_rule_t *rule);


/**
 * @brief Initializes given expression element as non-terminal (E) expression symbol (there is only one non-terminal in rules)
 */ 
int non_term(expr_el_t *non_terminal, string_t *data_type, bool is_zero);


/**
 * @brief Makes reduction of top of the main stack including type check and resolving errors
 * @param ops Stack with operands
 * @param rule Rule containing information about result type and operand data types
 */  
int reduce(p_parser_t *pparser, pp_stack_t ops, symbol_tables_t *syms, 
           expr_rule_t *rule, string_t *res_type);


/**
 * @brief Tries to reduce top of stack to non_terminal due to rules in get_rule()
 */ 
int reduce_top(p_parser_t *pparser, symbol_tables_t *syms,
               char ** failed_op_msg, string_t *ret_types);


/**
 * @brief Tries to reduce top of stack to non_terminal due to rules in get_rule()
 */ 
int reduce_top(p_parser_t *pparser, symbol_tables_t *syms,
               char ** failed_op_msg, string_t *ret_types);


/**
 * @brief Gets symbol from input (if it is valid as expression element otherwise is set to STOP_SYM)
 * @note Symbol on input adds to garbage stack to be freed at the end of expression parsing
 */ 
int get_input_symbol(p_parser_t *pparser, 
                     tok_buffer_t *t_buff, 
                     symbol_tables_t *symtabs,
                     prog_t *dst);


/**
 * @brief Gets first nonterminal from top of the stack (There can't be more of them due to operator grammar) 
 */ 
int get_top_symbol(p_parser_t *pparser);


/**
 * @brief Frees all resources (expecially dynamic allocated memory) of PP
 */ 
void free_everything(p_parser_t *pparser);

/**
 * @brief Makes last token from current and gets new token from input (from scanner)
 */ 
int token_aging(tok_buffer_t *token_buffer);


/**
 * @brief Does necessary actions when stack top has lower precedence than input
 */ 
int has_lower_prec(pp_stack_t *stack, expr_el_t on_input);


/**
 * @brief Prints error message due to return value
 * @note Also can corrects return code to expected value
 */ 
void print_err_message(int *return_value, tok_buffer_t *token_buffer, 
                      char **err_m);


/**
 * @brief Prints warning about undefined variable in expression
 */ 
void undefined_var_warning(tok_buffer_t *token_buffer, char *variable_name);


/**
 * @brief Inits all parts of auxiliary structure tok_buffer_t
 */ 
void prepare_buffer(scanner_t *sc, tok_buffer_t *tok_b);


/**
 * @brief Prepare necessary stacks before their usage in precedence parser
 * @param dst Pointer to program structure in which are instructions printed
 */ 
int prepare_pp(prog_t *dst, p_parser_t *pp);


/**
 * @brief Updates parser structs and current token int token_buffer in main cycle
 */ 
int update_structs(scanner_t *sc, symbol_tables_t *s, 
                   tok_buffer_t *tok_buff, p_parser_t *pparser, prog_t *dst);


/**
 * @brief Checks if there are $s on the top of stack an on the input
 * @note Also determines if expression is empty or not, and information about it is in return_value
 * @return True if there are
 */ 
bool is_end_of_parsing(p_parser_t *pparser, int *return_value);


/**
 * @brief Parses expression due to defined precedence table
 * @param dtypes Output parameter, p. parser will load it with string that 
 *               specifies return types of expression
 * @param is_only_f_call Output parameter, will be set to true, if was ONLY 
 *                       function called inside expression (there aren't any other operations)
 */
int parse_expression(scanner_t *sc, symbol_tables_t *s, 
                     string_t *dtypes, bool *is_only_f_call, prog_t *dst_code);


#endif


/***                     End of precedence_parser.h                        ***/
