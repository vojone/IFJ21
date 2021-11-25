/******************************************************************************
 *                                  IFJ21
 *                                symtable.h
 * 
 *        Authors: Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 *        Purpose: Declaration of symbol table functions and structures
 * 
 *                  Last change:
 *****************************************************************************/

#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dstring.h"
#include "dstack.h"

#define BUILTIN_TABLE_SIZE 8

#define UNDEFINED -1
#define UNSET -1


/**
 * @brief Specifies type of symbol
 */ 
typedef enum sym_type {
    FUNC, VAR, VAL
} sym_type_t;

/**
 * @brief Specifies data type of variable or return type of function 
 */ 
typedef enum sym_dtype {
    INT, NUM, STR, NIL, BOOL
} sym_dtype_t;


/**
 * @brief Specifies status of symbol (whether it was used, declared, defined)
 */ 
typedef enum sym_status {
    DECLARED, DEFINED
} sym_status_t;

/**
 * @brief Structure with all necessary data
 */ 
typedef struct sym_data {
    string_t name;
    sym_type_t type;
    string_t ret_types;
    string_t params;
    sym_dtype_t dtype;
    sym_status_t status;
    bool was_used;
} sym_data_t;

/**
 * @brief Element of symbol table (that is, in our case implemented as BST)
 */ 
typedef struct tree_node {
    char * key; 
    sym_data_t data;
    struct tree_node *l_ptr;
    struct tree_node *r_ptr;
} tree_node_t;


DSTACK_DECL(tree_node_t*, ts)


/**
 * @brief Symbol table data type for basic data storing
 */
typedef struct symtab {
    tree_node_t * t;
    int parent_ind; /**< Is used for switching contexts int parser */
} symtab_t; 

DSTACK_DECL(symtab_t, symtabs) /** Stack of symbol tables */

/**
 * @brief Symbol table data type, that can support superimposing of symbols 
 */ 
typedef struct symbol_tables {
    symtabs_stack_t symtab_st; /**< Stack for saving symbol tables */
    symtab_t global; /**< Global symbol table for functions */
    symtab_t symtab; /**< Current symbol table*/
} symbol_tables_t;

/**
 * @brief Initializes symbol table
 * @param symbol table to be initialized
 */ 
void init_tab(symtab_t *tab);

/**
 * @brief Initializes data structure of symbol
 */ 
int init_data(sym_data_t *new_data);

/**
 * @brief Frees all resources that data holds and set it to the state before initialization
 */ 
void data_dtor(sym_data_t *new_data);

/**
 * @brief Inserts a new element into existing symbol table or updates existing node
 * @param tab destination table
 * @param key key of new element
 */ 
void insert_sym(symtab_t *tab, const char *key, sym_data_t new_data);

/**
 * @brief Deletes element with specific key and frees all its resources
 * @param tab destination table
 * @param key key of element to be deleted
 */ 
void delete_sym(symtab_t *tab, const char *key);

/**
 * @brief Replaces deleted element with two children by rightmost element
 * @param tab destination table
 * @param target replaced element
 */
void replace_rightmost(symtab_t *tab, symtab_t *target);

/**
 * @brief Deletetes the entire symbol table and correctly frees its resources
 * @param tab symbol table to be deleted
 */ 
void destroy_tab(symtab_t *tab);

/**
 * @brief Searches for symbol table with specific key
 * @param tab symbol table in which should be searching executed
 * @param key key of element that should be found
 * @return Pointer to found symbol or NULL
 */ 
tree_node_t *search(symtab_t *tab, const char *key);

/**
 * @brief Converts character to sym_dtype enum
 */
sym_dtype_t char_to_dtype(char type_c);


/**
 * @brief Converts enum type used in symtable to correspoding character symbol
 */ 
char dtype_to_char(sym_dtype_t type);


/**
 * @brief inserts all builtin functions into given symbol table
 * @note Inseted functions will have same name as key in symbol table
 */ 
void load_builtin_f(symtab_t *dst);


/**
 * @brief Tries to find function by name in table of builtin functions
 * @return Pointer to function data in static table if function is found, other wise NULL
 */ 
sym_data_t* search_builtin(const char *f_name);


/**
 * @brief Performs searching in stack of symtabs
 * @return If nothing is found returns NULL otherwise returns pointer to first occurence
 */
tree_node_t * search_in_tables(symtabs_stack_t *sym_stack, 
                               symtab_t *start_symtab, 
                               char *key);


#endif


/***                          End of symtable.h                            ***/
