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

bool value_assignment();
bool assigment();

bool multiple_assigment();
bool else_branch();

bool loop_while();

bool param_list();
bool param_list_1();

bool type_list();
bool type_list_1();

bool expression_list();
bool expression_list_1();

bool parse_str();
bool parse_function_def();

/**
 * @brief Displays error about getting a different token than expected
 * @param expected the token which is expected
 * @param got the token type which we got
 * */
void incorrectToken(char * expected, int got, scanner_t * scanner);

/**
 * @param expecting The token type to expect to be the next
 * @brief Checks wheter the expected token type is at the next place
 * @return returns true the expected token type is there
 **/
bool lookaheadToken(scanner_t * scanner,token_type_t expecting);
/**
 * @param expecting The token type to expect to be the next
 * @brief Checks wheter the next token has the specified type and attribute
 * @return returns true if both expected type and attribute are equal to real ones
 **/
bool lookaheadTokenAttr(scanner_t * scanner,token_type_t expecting, char * expecting_attr);

/**
 * @param expecting The token type to expect
 * @brief Shows error if there is an unexpected token type
 * @return returns true the expected token type is there
 **/
bool checkNextToken(scanner_t * scanner,token_type_t expecting);
/**
 * @param expecting The token type to expect
 * @param expecting_attr The token attribute to expect
 * @brief Shows error if there is an unexpected token type or attribute
 * @return returns true the expected token is there
 **/
bool checkNextTokenAttr(scanner_t * scanner,token_type_t expecting, char * expecting_attr);

/**
 * !temporary
 * @note checks only for INTEGER, NUMBER or STRING
 **/
bool parseExpression();