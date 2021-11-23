/******************************************************************************
 *                                  IFJ21
 *                                dstring.h
 * 
 *       Authors: Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 *Purpose: Declaration of basic functions to process (dynamic) string and chars
 *                    Last change: 24. 10. 2021
 *****************************************************************************/ 

/**
 * @file dstring.h
 * @brief Declaration of basic functions to process (dynamic) string and chars
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 */ 

#ifndef DSTRING_H
#define DSTRING_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define STR_INIT_SPACE 8 /**< Initial allocated size for string */

//Return codes of string processing functions
#define STR_SUCCESS 0
#define STR_FAILURE 1

 
//String is implemented as dynamic array
typedef struct string {
    size_t length; /**< Length of string */
    size_t alloc_size; /**< Allocated space for string */
    char * str; /**< String data */
} string_t;


enum char_type {
    ALPHA, DIGIT, WHITESPACE, CONTROL, OTHER_CHARACTER
};

/**
 * @brief Initializes string_t structure
 * @param string dynamic string to initialize
 * @return STR_SUCCESS if everything goes well
 */
int str_init(string_t *string);

/**
 * @brief Appends character to end of string
 * @param c character to append
 * @param string target string 
 * @return STR_SUCCESS if everything goes well
 */
int app_char(char c, string_t *string);

int prep_char(char c, string_t *string);

int prep_str(string_t *dst, char *src);

/**
 * @brief Cuts string to given length
 * @note if given length is same or greater than current length of string it does nothing
 */ 
void cut_string(string_t *string, size_t new_length);

/**
 * @brief Brings string to the state after initialization  
 * @note Allocated space doesn't change!
 * @param string target string
 */ 
void str_clear(string_t *string);

/**
 * @brief Destroys string and frees all resources
 */
void str_dtor(string_t *string);

/**
 * @brief Returns pointer to array of characters
 */ 
char * to_str(string_t *string);

/**
 * @brief Tries to find character in string
 * @return true if character was found
 */
bool str_search(const char c, const char *str);

/**
 * @brief Copies array of characters to another place in memory
 * @param dst addres of pointer to target destination (IT SHOULD BE NULL!)
 * @param src source array of characters
 * @param length length of final string located at dst pointer
 * @return STR_SUCCESS if everything goes well
 */
int str_cpy(char **dst, const char *src, size_t length);

/**
 * @brief Copies array of characters (cstr) to string structure
 * @param length length of source cstr from start that will be copied to string
 */
int str_cpy_tostring(string_t* dst, const char *src, size_t length);

/**
 * @brief Makes hard copy of two strings
 */ 
int cpy_strings(string_t* dst, string_t *src);

/**
 * @brief returns type of character
 */
int get_chtype(const char c);


/**
 * @brief Compares to strings and returns result as an integer (same as strcmp)
 * @return 0 if strings are same
 */ 
int str_cmp(const char *str1, const char *str2);


size_t len(string_t *str);


int dstring_cmp(string_t* str1, string_t* str2);


#endif

/***                             End of dstring.h                        ***/
