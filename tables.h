/******************************************************************************
 *                                  IFJ21
 *                                tables.h
 * 
 *                     Authors: Vojtěch Dvořák (xdvora3o)
 * Purpose: Functions, that contain static variables with defined keywords,...
 * 
 *                    Last change: 25. 11. 2021
 *****************************************************************************/ 

/**
 * @file tables.h
 * @brief Functions, that contain static variables with defined keywords,...
 * 
 * @authors Vojtěch Dvořák (xdvora3o)
 */ 


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "dstring.h"

#ifndef TABLES_H
#define TABLES_H


#define KEYWORD_TABLE_SIZE 15
#define OPERATOR_TABLE_SIZE 15
#define SEPARATOR_TABLE_SIZE 4

/**
 * @brief Returns pointer to string in static array with keywords
 * @param index to array with keywords
 */
char * get_keyword(unsigned int index);

/**
 * @brief Returns pointer to string in static array with operators
 * @param index to array with operators
 */
char * get_operator(unsigned int index);

/**
 * @brief Returns pointer to string in static array with separators
 * @param index to array with separators
 */
char * get_separator(unsigned int index);

/**
 * @brief Contains static array of supported builtin functions
 * @return Pointer to builtin function name 
 */ 
char * get_builtin(unsigned int index);

/**
 * @brief Tries to find string in table (binary search)
 * @param str string to be found
 * @param table_func pointer to function, that contains static array
 * @param tab_size size of table in which be searching executed
 * @return pointer to found string in static array or NULL
 */ 
char * match(char * str, char * (*table_func)(unsigned int), size_t tab_size);

#endif

/***                             End of tables.h                           ***/
