/**
 * @file parser-topdown.h
 * @brief Header file for recursive descent parser
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 */ 


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "dstring.h"
#include "scanner.h"
#include <stdarg.h>
#include "precedence_parser.h"
#include "symtable.h"
#include "generator.h"

typedef struct result{
    bool success;
    bool reachedEnd;
} result_t;


/**
 * @brief Return codes
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


typedef struct parser {
    symtabs_stack_t symtabs;
    int return_code;
    bool reached_EOF;
} parser_t;

int error_rule();

typedef struct rule {
    int (* rule_function)();
    token_t rule_first;
    bool attrib_relevant;
} rule_t;

/**
 * @brief Finds the appropriate rule
 * @return Returns reference to rule function
 * @param t Token based on which is going to be decided what rule to use
 * @param ruleset is an array of rules for particular context (ie. global or inside a function) 
 */ 
rule_t determine_rule(token_t t, rule_t ruleset[]);

/**
 * @brief sets the parser and scanner to use
 * @note there can be only one instance i guess, because of the parser static var
 */ 
void parser_setup(parser_t *p, scanner_t *s);

/**
 * @brief Starts parsing
 */ 
int parse_program();

/**
 * @brief Parses statements list outside of a function
 */
int global_statement_list();

/**
 * @brief Parses statements outside of a function
 */
int global_statement();

/**
 * @brief Parses statements list inside of a function
 */
int statement_list();
/**
 * @brief Parses statements inside of a function
 */
int statement();

/**
 * @brief parses an identifier inside a function
 * @note decides wheter the rule to use is a function call or an assignment
 */
int parse_identifier(); 

/**
 * @brief parses an identifier in global scope
 * @note workaround because parse_function_call expects the identifier to be already read
 */
int parse_global_identifier(); 

/**
 * @brief parses an EOF inside a function
 * @note basically an error rule with additional error message
 */ 
int EOF_fun_rule();

/**
 * @brief parses an EOF inside a function
 * @note basically an error rule with additional error message
 */ 
int EOF_global_rule();

/**
 * @brief Parses local variable definitions
 */
int parse_local_var();

/**
 * @brief Parses global variable definitions
 */
int parse_global_var();

/**
 * @brief Parses else branch 
 */
int parse_else();

/**
 * @brief Parses return statement
 */
int parse_return();

/**
 * @brief Parses end statement
 */
int parse_end();

/**
 * @brief checks value assignment when defining a variable
 */ 
bool value_assignment();

//int assigment();

int assignment();

/**
 * @brief Parses if statement
 * @note Expects that the 'if' is already read and the first token is going to be a condition (expression)
 * @note Also handles else branch currently
 */ 
int parse_if();

/**
 * @note might use in future 
 */ 
int else_branch();

int parse_while();

int param_list();
int param_list_1();

int type_list();
int type_list_1();

/**
 * expression_list & expression_list_1 are currently handled in multiple_assignment()
 * 
 */ 
int expression_list();
int expression_list_1();

/**
 * @brief Shows error if next token is not a STRING
 */ 
int parse_require();

/**
 * @brief Parses declaration of function (so just the signature)
 */ 
int parse_function_dec();

/**
 * @brief Parses function definition, checks for signature and then parses the inside of the function
 * 
 */ 
int parse_function_def();

/**
 * @brief parses the function call.
 * @note presumes that the function IDENTIFIER was already read and the first token is going to be opening bracket
 */ 
int parse_function_call();

/**
 * @brief parses the function arguments till the closing bracket
 *
 */ 
int parse_function_arguments();

/**
 * @brief Displays error about getting a different token than expected
 * @param expected the token which is expected
 * @param t the token which we got
 * */
void error_unexpected_token(char * expected, token_t t);

/**
 * @brief Displays error from semantic check 
 * @param message the text which is displayed
 * @param t the token which we got
 * */
void error_semantic(const char * _Format, ...);

/**
 * @param expecting The token type to expect to be the next
 * @brief Checks wheter the expected token type is at the next place
 * @return returns true the expected token type is there
 **/
bool lookahead_token(token_type_t expecting);

/**
 * @param expecting The token type to expect to be the next
 * @brief Checks wheter the next token has the specified type and attribute
 * @return returns true if both expected type and attribute are equal to real ones
 **/
bool lookahead_token_attr(token_type_t expecting, char * expecting_attr);

/**
 * @param expecting The token type to expect
 * @brief Shows error if there is an unexpected token type
 * @return returns true the expected token type is there
 **/
bool check_next_token(token_type_t expecting);

/**
 * @param expecting The token type to expect
 * @param expecting_attr The token attribute to expect
 * @brief Shows error if there is an unexpected token type or attribute
 * @return returns true the expected token is there
 **/
bool check_next_token_attr(token_type_t expecting_type, char * expecting_attr);

/**
 * @param expecting The token type to expect
 * @brief Compares the expected token type to token 't'
 * @return returns true the expected token types are equal
 **/
bool compare_token(token_t t,token_type_t expecting);

/**
 * @param expecting_type The token type to expect
 * @param expecting_attr The token attribute to expect
 * @brief Compares the expected token type & parameter to token 't'
 * @return returns true the token types & parameters are equal
 **/
bool compare_token_attr(token_t t, token_type_t expecting_type, char * expecting_attr);

/**
 * @return True if token is datatype
 */ 
bool is_datatype(token_t t);

/**
 * @brief checks if the next token is datatype
 */ 
int parse_datatype();

/**
 * !temporary
 * @brief shows error if token is not considered expression
 * @note checks only for INTEGER, NUMBER or STRING
 * 
 **/
// int parse_expression();

/**
 * !temporary
 * @brief shows error if token is not considered expression
 * @param t the token which will be checked
 * @return returns true if token one of types below 
 * @note checks only for INTEGER, NUMBER or STRING
 */ 
bool is_expression(token_t t);

/**
 * @brief debug printing
 * @note can be disabled by DEBUG static global variable
 * @note to disable the "Got token at:" and "^ Lookahead ^", delete it in the scanner.c file. My apologies for the inconvenience
 */
void debug_print(const char *const _Format, ...);
