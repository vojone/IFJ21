/**
 * @file parser-topdown.h
 * @brief Header file for recursive descent parser
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 */ 


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "dstring.h"
#include "scanner.h"

typedef struct result{
    bool success;
    bool reachedEnd;
} result_t;


int main();

/**
 * @brief Parses statements list outside of a function
 */
bool global_statement_list();

/**
 * @brief Parses statements outside of a function
 */
bool global_statement();

/**
 * @brief Parses statements list inside of a function
 */
bool statement_list();
/**
 * @brief Parses statements inside of a function
 */
bool statement();

/**
 * @brief Parses variable definitions
 */
bool parse_variable_def();

/**
 * @brief checks value assignment when defining a variable
 */ 
bool value_assignment();

//bool assigment();

bool multiple_assignment();

/**
 * @brief Parses if statement
 * @note Expects that the 'if' is already read and the first token is going to be a condition (expression)
 * @note Also handles else branch currently
 */ 
bool parse_if();

/**
 * @note might use in future 
 */ 
bool else_branch();

bool parse_while();

bool param_list();
bool param_list_1();

bool type_list();
bool type_list_1();

/**
 * expression_list & expression_list_1 are currently handled in multiple_assignment()
 * 
 */ 
bool expression_list();
bool expression_list_1();

/**
 * @brief Shows error if next token is not a STRING
 * @return true if token is string
 */ 
bool parse_str();

/**
 * @brief Parses function definition, checks for signature and then parses the inside of the function
 * 
 */ 
bool parse_function_def();
/**
 * @brief parses the function call.
 * @note presumes that the function IDENTIFIER was already read and the first token is going to be opening bracket
 */ 
bool parse_function_call();
/**
 * @brief parses the function arguments till the closing bracket
 *
 */ 
bool parse_function_arguments();

/**
 * @brief Displays error about getting a different token than expected
 * @param expected the token which is expected
 * @param t the token which we got
 * */
void incorrect_token(char * expected, token_t t, scanner_t * scanner);

/**
 * @param expecting The token type to expect to be the next
 * @brief Checks wheter the expected token type is at the next place
 * @return returns true the expected token type is there
 **/
bool lookahead_token(scanner_t * scanner,token_type_t expecting);
/**
 * @param expecting The token type to expect to be the next
 * @brief Checks wheter the next token has the specified type and attribute
 * @return returns true if both expected type and attribute are equal to real ones
 **/
bool lookahead_token_attr(scanner_t * scanner,token_type_t expecting, char * expecting_attr);
/**
 * @param expecting The token type to expect
 * @brief Shows error if there is an unexpected token type
 * @return returns true the expected token type is there
 **/
bool check_next_token(scanner_t * scanner,token_type_t expecting);
/**
 * @param expecting The token type to expect
 * @param expecting_attr The token attribute to expect
 * @brief Shows error if there is an unexpected token type or attribute
 * @return returns true the expected token is there
 **/
bool check_next_token_attr(scanner_t * scanner,token_type_t expecting, char * expecting_attr);

/**
 * @brief checks if the next token is datatype
 */ 
bool parse_datatype();

/**
 * !temporary
 * @brief shows error if token is not considered expression
 * @note checks only for INTEGER, NUMBER or STRING
 * 
 **/
bool parse_expression();

/**
 * !temporary
 * @brief shows error if token is not considered expression
 * @param t the token which will be checked
 * @return returns true if token one of types below 
 * @note checks only for INTEGER, NUMBER or STRING
 */ 
bool is_expression(token_t t);