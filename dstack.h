/******************************************************************************
 *                                  IFJ21
 *                                 dstack.h
 * 
 *          Authors: Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 *          Purpose: Header file with declarations of auxiliary stacks
 * 
 *                    Last change: 11. 11. 2021
 *****************************************************************************/ 

/**
 * @file dstack.h
 * @brief Header file of auxiliary stacks (implemented as dynamic array)
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 */ 

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#ifndef H_DSTACK
#define H_DSTACK

#include "precedence_parser.h"

#define ST_ERROR -1

#define INIT_SIZE 16

#define DSTACK_DECL(TYPE, NAME, FORMAT)                     \
                                                            \
typedef struct NAME##_stack {                               \
    TYPE  *data;                                            \
    unsigned int top;                                       \
    size_t allocated;                                       \
} NAME##_stack_t;                                           \
                                                            \
void NAME##_stack_init(NAME##_stack_t *s);                  \
                                                            \
bool NAME##_is_empty(NAME##_stack_t *s);                    \
                                                            \
void NAME##_push(NAME##_stack_t *s, TYPE newdata);          \
                                                            \
TYPE NAME##_pop(NAME##_stack_t *s);                         \
                                                            \
TYPE NAME##_top(NAME##_stack_t *s);                         \
                                                            \
void NAME##_show(NAME##_stack_t *s);                        \

DSTACK_DECL(int, pp, "%d ")

#endif

/***                              End of dstack.h                          ***/
