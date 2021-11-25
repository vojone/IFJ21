/******************************************************************************
 *                                  IFJ21
 *                                 dstack.h
 * 
 *                  Authors: Vojtech Dvorak (xdvora3o)
 *  Purpose: Header file with macros that can generate auxiliary stacks
 *              Contains also stack declarations used in multiple files 
 * 
 *                        Last change: 25. 11. 2021
 *****************************************************************************/ 

/**
 * @file dstack.h
 * @brief Header file with macros that can generate auxiliary stacks
 *        Contains also stack declarations used in multiple files 
 * 
 * @authors Vojtech Dvorak (xdvora3o)
 */ 


#ifndef H_DSTACK
#define H_DSTACK

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "scanner.h"


#define INIT_SIZE 16 /**< Initial size of stack (after initialization)*/

/**
 * @brief Generates declaration of stack (of functions and structure) with specific name and data type
 * @param NAME prefix that will be added to opearations and structure name
 * @param TYPE data type of elements in the stack
 * @note Look at dstack.c to get better imagination how does it work and to generate stack definition
 */ 
#define DSTACK_DECL(TYPE, NAME)                             \
                                                            \
                                                            \
/**                                                         \
 * @brief Structure that represents stack                   \
 * @note Stack is implemented as dynamic array              \
 */                                                         \
typedef struct NAME##_stack {                               \
    TYPE  *data;                                            \
    unsigned int top;                                       \
    size_t allocated;                                       \
} NAME##_stack_t;                                           \
                                                            \
/**                                                         \
 * @brief Allocates initial memory space for stack          \
 */                                                         \
bool NAME##_stack_init(NAME##_stack_t *s);                  \
                                                            \
/**                                                         \
 * @brief Deallocates all resources of                      \
 */                                                         \
void NAME##_stack_dtor(NAME##_stack_t *s);                  \
                                                            \
                                                            \
/**                                                         \
 * @brief Determines whether stack is empty or not          \
 */                                                         \
bool NAME##_is_empty(NAME##_stack_t *s);                    \
                                                            \
                                                            \
/**                                                         \
 * @brief Pushes element to the stack                       \
 */                                                         \
bool NAME##_push(NAME##_stack_t *s, TYPE newdata);          \
                                                            \
                                                            \
/**                                                         \
 * @brief Pops element on top of the stack (and return it)  \
 * @warning You should check if stack is not empty before call this function\
 */                                                         \
TYPE NAME##_pop(NAME##_stack_t *s);                         \
                                                            \
/**                                                         \
 * @brief Returns element of the top of the stack (and keeps it there)\
 * @warning You should check if stack is not empty before call this function\
 */                                                         \
TYPE NAME##_top(NAME##_stack_t *s);                         \
                                                            \
/**                                                         \
 *@brief Return pointer to the element in the stack         \
 *@note Pointer couldn't be stored if there is possibility of pushing new elements (the stack can be reallocated)\
 */                                                         \
TYPE * NAME##_get_ptr(NAME##_stack_t *s, int index);        \
                                                            \
/**                                                         \
 * @brief Returns index of element, that is currently on the top \
 * @note This index is not changing unless element is popped (in contrast with pointer)\
 */                                                         \
unsigned int NAME##_get_top_ind(NAME##_stack_t *s);         \
                                                            \
/**                                                         \
 * @brief Prints content of stack (Just for easier debugging)\
 */                                                         \
void NAME##_show(NAME##_stack_t *s);                        \



/**
 * @brief Generates definition of stack (of related functions) with specific name and data type
 * @param NAME prefix that will be added to opearations and structure name
 * @param TYPE data type of elements in the stack
 * @param PRINT_CMD Command that is used for printing data in the stack (not important, used only in show)
 * @note PRINT_CMD Can be omitted if NAME##_show is not needed
 */ 
#define DSTACK(TYPE, NAME, PRINT_CMD)                           \
bool NAME##_stack_init(NAME##_stack_t *s) {                     \
    s->top = 0;                                                 \
    s->data = (TYPE *)malloc(sizeof(TYPE)*INIT_SIZE);           \
    if(!s->data) {                                              \
        fprintf(stderr, "Stack: Can't allocate!\n");            \
        return false;                                           \
    }                                                           \
    s->allocated = INIT_SIZE;                                   \
                                                                \
    return true;                                                \
}                                                               \
                                                                \
                                                                \
void NAME##_stack_dtor(NAME##_stack_t *s) {                     \
    free(s->data);                                              \
    s->allocated = 0;                                           \
}                                               \
                                                \
bool NAME##_is_empty(NAME##_stack_t *s) {       \
    return s->top == 0;                         \
}                                               \
                                                \
bool NAME##_push(NAME##_stack_t *s, TYPE  newdata) {                                \
    if(s->top == s->allocated) { /*Allocated memory is full -> extend it*/          \
        s->data = (TYPE *)realloc(s->data, sizeof(TYPE)*s->allocated*2);            \
        if(!s->data) {                                                              \
            fprintf(stderr, "Stack: Can't allocate!\n");                            \
            return false;                                                           \
        }                                                                           \
        s->allocated *= 2;                                                          \
                                                                                    \
    }                                           \
                                                \
    s->data[s->top++] = newdata;                \
                                                \
    return true;                                \
}                                               \
                                                \
TYPE NAME##_pop(NAME##_stack_t *s) {            \
    s->top -= 1;                                \
    return s->data[s->top];                     \
}                                               \
                                                \
TYPE NAME##_top(NAME##_stack_t *s) {            \
    return s->data[s->top - 1];                 \
}                                               \
                                                \
TYPE * NAME##_get_ptr(NAME##_stack_t *s, int index) {               \
    if(index > s->top - 1 || index < 0) {                           \
        return NULL;                                                \
    }                                                               \
    else {                                                          \
        return &(s->data[index]);                                   \
    }                                                               \
}                                                                   \
                                                                    \
                                                                    \
unsigned int NAME##_get_top_ind(NAME##_stack_t *s) {                \
        return s->top - 1;                                          \
}                                                                   \
                                                \
void NAME##_show(NAME##_stack_t *s) {           \
    int i = 0;                                  \
    fprintf(stderr, "STACK: |");                \
    while(i != s->top) {                        \
        PRINT_CMD;                              \
        i++;                                    \
    }                                           \
    fprintf(stderr, "<= top\n");                \
}                                               \


DSTACK_DECL(token_t, tok)


#endif

/***                              End of dstack.h                          ***/
