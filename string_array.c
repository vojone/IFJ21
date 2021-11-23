#include "string_array.h"


void string_array_init(string_array_t *array){
    array->arr = NULL;
}


void string_array_append(string_array_t *array, const char * str){
    array = realloc(array->arr, sizeof(&array)+sizeof(str));
    array[array->arr_count] = str;
    array->arr_count = array->arr_count++;
}

//"a&b&c&"