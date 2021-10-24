/******************************************************************************
 *                                  IFJ21
 *                                tables.h
 * 
 *        Authors: Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 * Purpose: Functions, that contain static variables with defined keywords,...
 * 
 *                    Last change: 24. 10. 2021
 *****************************************************************************/ 

/**
 * @file tables.h
 * @brief Functions, that contain static variables with defined keywords,...
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 */ 


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#ifndef TABLES_H
#define TABLES_H

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


char * get_bfunc(unsigned int index);

/**
 * @brief Tries to find string in table
 * @param str string to be found
 * @param table_func pointer to function, that contains static array
 * @return pointer to found string in static array or NULL
 */ 
char * match(char * str, char * (*table_func)(unsigned int));

#endif

/***                             End of tables.h                           ***/
