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

char * get_keyword(unsigned int index) {
    static char * keyword_table[] = {"do", "else", "end", "function", 
                                     "global", "if", "local", "nil", 
                                     "require", "return", "then", "while",
                                     "integer", "number", "string",
                                     NULL};
    return keyword_table[index];
}


char * get_operator(unsigned int index) {
    static char * keyword_table[] = {"=", "#", "*", "/", 
                                     "//", "+", "-", "..", 
                                     "<", "<=",">", ">=",
                                     "==", "~=", NULL};

    return keyword_table[index];
}


char * get_separator(unsigned int index) {
    static char * keyword_table[] = {",", ":", "(", ")", 
                                     NULL};
                                     
    return keyword_table[index];
}


char * get_bfunc(unsigned int index) {
    return NULL;
}


char * match(char * str, char * (*table_func)(unsigned int)) {
    int i = 0;
    char * cur_keyword = NULL;
    while((cur_keyword = (*table_func)(i)) != NULL) {
        if(!strcmp(cur_keyword, str)) {
            return get_keyword(i);
        }

        i++;
    }

    return NULL;
}


/***                             End of tables.c                           ***/
