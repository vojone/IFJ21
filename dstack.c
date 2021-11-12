/******************************************************************************
 *                                  IFJ21
 *                                 dstack.c
 * 
 *          Authors: Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 *   Purpose: Implementation of auxiliary stacks (implemented as dynamic array)
 * 
 *                    Last change: 11. 11. 2021
 *****************************************************************************/ 

/**
 * @file dstack.c
 * @brief Implementation of auxiliary stacks (implemented as dynamic array)
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 */ 

/**
 * @brief Generates stack as dynamic array of elements of TYPE 
 * @param FORMATSTR is not important only for printing stack content (use format symbol e.g. "%d" for integer stack)
 */ 

#include "dstack.h"

/**
 * @brief Generates definition of stack (of related functions) with specific name and data type
 * @param NAME prefix that will be added to opearations and structure name
 * @param TYPE data type of elements in the stack
 * @param PRINT_CMD Command that is used for printing data in the stack (not important, used only in show)
 * @note PRINT_CMD Can be omitted if NAME##_show is not needed
 */ 
#define DSTACK(TYPE, NAME, PRINT_CMD)                           \
void NAME##_stack_init(NAME##_stack_t *s) {                     \
    s->top = 0;                                                 \
    s->data = (TYPE *)malloc(sizeof(TYPE)*INIT_SIZE);           \
    if(!s->data) {                                              \
        fprintf(stderr, "Stack: Can't allocate!\n");            \
        return;                                                 \
    }                                                           \
    s->allocated = INIT_SIZE;                                   \
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
void NAME##_push(NAME##_stack_t *s, TYPE  newdata) {                                \
    if(s->top == s->allocated) { /*Allocated memory is full -> extend it*/          \
        s->data = (TYPE *)realloc(s->data, sizeof(TYPE)*s->allocated*2);            \
        if(!s->data) {                                                              \
            fprintf(stderr, "Stack: Can't allocate!\n");                            \
            return;                                                                 \
        }                                                                           \
        s->allocated *= 2;                                                          \
                                                                                    \
    }                                           \
                                                \
    s->data[s->top++] = newdata;                \
}                                               \
                                                \
TYPE NAME##_pop(NAME##_stack_t *s) {            \
    if(NAME##_is_empty(s)) {                    \
        return ST_ERROR;                        \
    }                                           \
                                                \
    s->top -= 1;                                \
    return s->data[s->top];                     \
}                                               \
                                                \
TYPE NAME##_top(NAME##_stack_t *s) {            \
    if(NAME##_is_empty(s)) {                    \
        return ST_ERROR;                        \
    }                                           \
                                                \
    return s->data[s->top - 1];                 \
}                                               \
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

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~STACK DEFINITIONS~~~~~~~~~~~~~~~~~~~~~~~~~~~~

DSTACK(int, pp, fprintf(stderr," %d",s->data[i]))


/***                        End of dstack.c                                ***/
