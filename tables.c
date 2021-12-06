/******************************************************************************
 *                                  IFJ21
 *                                tables.c
 * 
 *                     Authors: Vojtěch Dvořák (xdvora3o)
 * Purpose: Functions, that contain static variables with defined keywords,...
 * 
 *                      Last change: 25. 11. 2021
 *****************************************************************************/ 

/**
 * @file tables.c
 * @brief Functions, that contain static variables with defined keywords,...
 * 
 * @authors Vojtěch Dvořák (xdvora3o)
 */ 

#include "tables.h"

/**
 * @brief Contains static array of allowed keywords in IFJ21 
 *        (sorted by ASCII - alphabetically, same sorting as in dictionary)
 * @return Pointer to keyword with index from argument    
 */ 
char * get_keyword(unsigned int index) {
    static char * keyword_table[KEYWORD_TABLE_SIZE] = 
    {
        "do", "else", "end", "function", 
        "global", "if", "integer", "local", 
        "nil", "number", "require", "return", 
        "string", "then", "while"
    };

    return keyword_table[index];
}


/**
 * @brief Contains static array of allowed operators in IFJ21 
 *        (sorted by ASCII value)
 * @return Pointer to operator with index from argument    
 */ 
char * get_operator(unsigned int index) {
    static char * operator_table[OPERATOR_TABLE_SIZE] = 
    {
        "#", "%", "*", "+", "-", "..", 
        "/", "//", "<", "<=", "=", 
        "==", ">", ">=", "^", "~="
    };

    return operator_table[index];
}

/**
 * @brief Contains static array of allowed separators in IFJ21
 *        (sorted by ASCII value)
 * @return Pointer to separator with index from argument    
 */ 
char * get_separator(unsigned int index) {
    static char * separator_table[SEPARATOR_TABLE_SIZE] = 
    {
        "(", ")", ",", ":"
    };
                            
    return separator_table[index];
}


/**
 * @brief Tries to find string in table (Implemented as binary search)
 * @param str string to be found
 * @param table_func pointer to function, that contains static array
 * @return pointer to found string in static array or NULL
 */ 
char * match(char * str, char * (*table_func)(unsigned int), size_t tab_size) {

    int middle = tab_size / 2;
    int left_b = 0, right_b = tab_size - 1;
    char * found = NULL;
    do {
        char * cur_str = table_func(middle);
        int cmp_result = str_cmp(str, cur_str);

        if(cmp_result == 0) {
            found = cur_str;
            break;
        }
        else if(cmp_result < 0) {
            right_b = middle - 1;
        }
        else {
            left_b = middle + 1;
        }

        middle = (right_b - left_b) / 2 + left_b;
    } while(left_b <= right_b);
    

    return found;
}


/***                             End of tables.c                           ***/
