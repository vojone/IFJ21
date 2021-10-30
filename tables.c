/******************************************************************************
 *                                  IFJ21
 *                                tables.c
 * 
 *        Authors: Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 * Purpose: Functions, that contain static variables with defined keywords,...
 * 
 *                    Last change: 24. 10. 2021
 *****************************************************************************/ 

/**
 * @file tables.c
 * @brief Functions, that contain static variables with defined keywords,...
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 */ 

#include "tables.h"

/**
 * @brief Contains static array of allowed keywords in IFJ21 (sorted by ASCII value - alphabetically)
 * @return Pointer to keyword with index from argument    
 */ 
char * get_keyword(unsigned int index) {
    static char * keyword_table[] = {"do", "else", "end", "function", 
                                     "global", "if", "integer", "local", 
                                     "nil", "number", "require", "return", 
                                     "string", "then", "while", NULL};

    return keyword_table[index];
}

/**
 * @brief Contains static array of allowed operators in IFJ21 (sorted by ASCII value)
 * @return Pointer to operator with index from argument    
 */ 
char * get_operator(unsigned int index) {
    static char * operator_table[] = {"#", "*", "+", "-", "..", 
                                      "/", "//", "<", "<=", "=", 
                                      "==", ">", ">=", "~=", NULL};

    return operator_table[index];
}

/**
 * @brief Contains static array of allowed separators in IFJ21 (sorted by ASCII value)
 * @return Pointer to separators with index from argument    
 */ 
char * get_separator(unsigned int index) {
    static char * separator_table[] = {",", ":", "(", ")", NULL};
                                     
    return separator_table[index];
}


char * get_builtin(unsigned int index) {
    return NULL;
}

/**
 * @brief Tries to find string in table
 * @param str string to be found
 * @param table_func pointer to function, that contains static array
 * @return pointer to found string in static array or NULL
 */ 
char * match(char * str, char * (*table_func)(unsigned int)) {

    int i = 0;
    char * cur_keyword = NULL;
    while((cur_keyword = (*table_func)(i)) != NULL) {
        if(!strcmp(cur_keyword, str)) {
            return cur_keyword;
        }

        i++;
    }

    return NULL;
}


/***                             End of tables.c                           ***/
