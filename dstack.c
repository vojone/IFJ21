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

#define DSTACK(TYPE, NAME, FORMAT_STR)                          \
void NAME##_stack_init(NAME##_stack_t *s) {                     \
    s->top = 0;                                                 \
    s->data = (TYPE *)malloc(sizeof(TYPE)*INIT_SIZE);           \
    if(!s->data) {                                              \
        fprintf(stderr, "Stack: Can't allocate!\n");            \
        return;                                                 \
    }                                                           \
    s->allocated = INIT_SIZE;                                   \
}                                               \
                                                \
bool NAME##_is_empty(NAME##_stack_t *s) {       \
    return s->top == 0;                         \
}                                               \
                                                \
void NAME##_push(NAME##_stack_t *s, TYPE  newdata) {                                \
    if(s->top == s->allocated) {                                                    \
        s->data = (TYPE *)realloc(s->data, sizeof(TYPE)*s->allocated*2);            \
        if(!s->data) {                                                              \
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
    return s->data[s->top - 1];           }     \
                                                \
void NAME##_show(NAME##_stack_t *s) {           \
    int i = 0;                                  \
    fprintf(stderr, "STACK: |");                \
    while(i != s->top) {                        \
        fprintf(stderr, FORMAT_STR , s->data[i]);\
        i++;                                    \
    }                                           \
    fprintf(stderr, "<= top\n");                \
}                                               \

DSTACK(int, pp, "%d ")


/***                        End of dstack.c                                ***/
