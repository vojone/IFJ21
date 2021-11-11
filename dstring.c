/******************************************************************************
 *                                  IFJ21
 *                                dstring.c
 * 
 *          Authors: Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 * Purpose: Source file of functions to process (dynamic) strings and chars
 * 
 *                    Last change: 24. 10. 2021
 *****************************************************************************/ 

/**
 * @file dstring.h
 * @brief Source file of functions to process (dynamic) strings and chars
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 */ 

#include "dstring.h"

int str_init(string_t *string) {
    string->str = (char *)malloc(sizeof(char)*STR_INIT_SPACE);
    if(!string->str) {
        fprintf(stderr, "dstring: str_init: Cannot allocate memory!");
        return STR_FAILURE;
    }

    string->alloc_size = STR_INIT_SPACE;
    string->str[0] = '\0';
    string->length = 0;

    return STR_SUCCESS;
}


int app_char(char c, string_t *string) {
    //There must be always place for '\0' character
    if(string->alloc_size - 1 < string->length + 1) {
        size_t new_size = string->alloc_size*2;
        string->str = (char *)realloc(string->str, sizeof(char)*new_size);
        if(!string->str) {
            fprintf(stderr, "dstring: str_init: Cannot extend string!");
            return STR_FAILURE;
        } 
        string->alloc_size = new_size;
    }

    string->str[string->length++] = c;
    string->str[string->length] = '\0';
    
    return STR_SUCCESS;
}


void str_clear(string_t *string) {
    string->str[0] = '\0';
    string->length = 0;
}


void str_dtor(string_t *string) {
    free(string->str);
    string->length = 0;
    string->alloc_size = 0;
}


char * to_str(string_t *string) {
    return string->str;
}


bool str_search(const char c, const char *str) {
    if(strchr(str, c)) {
        return true;
    }
    else {
        return false;
    }
}


int str_cpy(char **dst, const char *src, size_t length) {
    *dst = (char *)malloc(sizeof(char) * (length + 1)); //Length of source + \0
    if(!(*dst)) {
        fprintf(stderr, "dstring: str_cpy: Cannot copy a string!");
        return STR_FAILURE;
    }

    for(int i = 0; i <= length; i++) {
        (*dst)[i] = src[i];
    }


    return STR_SUCCESS;
}


int str_cmp(const char *str1, const char *str2) {
    return strcmp(str1, str2);
}


int get_chtype(const char c) {
    if(isalpha(c)) {
        return ALPHA;
    }
    else if(isdigit(c)) {
        return DIGIT;
    }
    else if(isspace(c)) {
        return WHITESPACE;
    }
    else if(iscntrl(c)) {
        return CONTROL;
    }
    else {
        return OTHER_CHARACTER;
    }
}


/***                             End of dstring.c                          ***/
