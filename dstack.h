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
#include "symtable.h"


#define INIT_SIZE 16 /**< Initial size of stack (after initialization)*/

/**
 * @brief Generates declaration of stack (of functions and structure) with specific name and data type
 * @param NAME prefix that will be added to opearations and structure name
 * @param TYPE data type of elements in the stack
 * @note Look at dstack.c to get better imagination how does it work and to generate stack definition
 */ 
#define DSTACK_DECL(TYPE, NAME)                             \
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
 */                                                         \
TYPE NAME##_pop(NAME##_stack_t *s);                         \
                                                            \
/**                                                         \
 * @brief Returns element of the top of the stack (and keeps it there)\
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


DSTACK_DECL(expr_el_t, pp)

DSTACK_DECL(tree_node_t*, ts)

DSTACK_DECL(symtab_t, symtabs)

#endif

/***                              End of dstack.h                          ***/
