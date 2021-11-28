/******************************************************************************
 *                                  IFJ21
 *                                dstring.c
 * 
 *                  Authors: Vojtech Dvorak (xdvora3o)
 * Purpose: Source file of functions to process (dynamic) strings and chars
 * 
 *                    Last change: 25. 11. 2021
 *****************************************************************************/ 

/**
 * @file dstring.h
 * @brief Source file of functions to process (dynamic) strings and chars
 * @see dstring.h to get functions description
 * 
 * @authors Vojtech Dvorak (xdvora3o)
 */

#include "dstring.h"

int str_init(string_t *string) {
    string->str = NULL;
    string->str = (char *)malloc(sizeof(char)*STR_INIT_SPACE);
    if(!string->str) {
        fprintf(stderr, "dstring: str_init: Cannot allocate memory!\n");
        return STR_FAILURE;
    }

    string->alloc_size = STR_INIT_SPACE;
    string->str[0] = '\0';
    string->length = 0;

    return STR_SUCCESS;
}


int extend_string(string_t *string) {
    size_t new_size = string->alloc_size*2;
    string->str = (char *)realloc(string->str, sizeof(char)*new_size);
    if(!string->str) {
        fprintf(stderr, "dstring: str_init: Cannot extend string!\n");
        return STR_FAILURE;
    } 

    string->alloc_size = new_size;

    return STR_SUCCESS;
}


int app_char(char c, string_t *string) {
    //There must be always place for '\0' character
    if(string->alloc_size - 1 < string->length + 1) {
        size_t new_size = string->alloc_size*2;
        string->str = (char *)realloc(string->str, sizeof(char)*new_size);
        if(!string->str) {
            fprintf(stderr, "dstring: str_init: Cannot extend string!\n");
            return STR_FAILURE;
        } 

        string->alloc_size = new_size;
    }

    string->str[string->length++] = c;
    string->str[string->length] = '\0';
    
    return STR_SUCCESS;
}


int app_str(string_t *dst, const char *src) {
    for(int i = 0; i < strlen(src); i++) {
        int ret = app_char(src[i], dst);
        if(ret == STR_FAILURE) {
            return STR_FAILURE;
        }
    }

    return STR_SUCCESS;
}


void cut_string(string_t *string, size_t new_length) {
    if(new_length >= string->length) {
        return;
    }

    string->length = new_length;
    string->str[string->length] = '\0';
}


int prep_char(char c, string_t *string) {
    if(string->alloc_size - 1 < string->length + 1) {
        size_t new_size = string->alloc_size*2;
        string->str = (char *)realloc(string->str, sizeof(char)*new_size);
        if(!string->str) {
            fprintf(stderr, "dstring: str_init: Cannot extend string!\n");
            return STR_FAILURE;
        } 
        string->alloc_size = new_size;
    }

    for(int i = strlen(string->str); i >= 0; i--) {
        string->str[i + 1] = string->str[i];
    }

    string->str[0] = c;

    return STR_SUCCESS;
}


int prep_str(string_t *dst, const char *src) {
    for(int i = strlen(src) - 1; i >= 0; i--) {
        int ret = prep_char(src[i], dst);
        if(ret == STR_FAILURE) {
            return STR_FAILURE;
        }
    }

    return STR_SUCCESS;
}


void str_clear(string_t *string) {
    string->str[0] = '\0';
    string->length = 0;
}


void str_dtor(string_t *string) {
    if(string->alloc_size > 0 && string->str != NULL) {
        free(string->str);
    }

    string->str = NULL;
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
        fprintf(stderr, "dstring: str_cpy: Cannot copy a string!\n");
        return STR_FAILURE;
    }

    for(int i = 0; i <= length; i++) {
        (*dst)[i] = src[i];
    }


    return STR_SUCCESS;
}

int str_cpy_tostring(string_t* dst, const char *src, size_t length) {
    str_clear(dst); //Length of source + \0
    for(size_t i = 0; i < length; i++) {
        if(app_char(src[i], dst) == STR_FAILURE) {
            fprintf(stderr, "dstring: str_cpy: Cannot copy a string!\n");
            return STR_FAILURE;
        }
    }
    
    return STR_SUCCESS;
}


int cpy_strings(string_t* dst, string_t *src, bool zero_term) {
    str_clear(dst); //Length of source + \0
    for(size_t i = 0; zero_term ? src->str[i] != '\0' : i < src->length; i++) {
        if(app_char(src->str[i], dst) == STR_FAILURE) {
            fprintf(stderr, "dstring: str_cpy: Cannot copy a string!\n");
            return STR_FAILURE;
        }
    }
    
    return STR_SUCCESS;
}


int str_cmp(const char *str1, const char *str2) {
    return strcmp(str1, str2);
}

size_t len(string_t *str) {
    return str->length;
}



int dstring_cmp(string_t* str1, string_t* str2) {
    return strcmp(str1->str, str2->str);
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
