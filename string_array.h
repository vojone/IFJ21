#include <stdlib.h>
#include <string.h>

typedef struct string_array
{
    const char **arr;
    size_t arr_count;
} string_array_t;

void string_array_init(string_array_t array);