/******************************************************************************
 *                                  IFJ21
 *                                symtable.h
 * 
 *        Authors: Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 *        Purpose: Declaration of symbol table functions and structures
 * 
 *                  Last change:
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dstring.h"

#ifndef SYMTABLE_H
#define SYMTABLE_H

/**
 * @brief Specifies type of symbol
 */ 
typedef enum sym_type {
    FUNC, VAR
} sym_type_t;

/**
 * @brief Specifies data type of variable or return type of function 
 */ 
typedef enum sym_dtype {
    INTEGER, NUMBER, STRING
} sym_dtype_t;


/**
 * @brief Specifies status of symbol (whether it was used, declared, defined)
 */ 
typedef enum sym_status {
    DECLARED, DEFINED, USED
} sym_status_t;

/**
 * @brief Structure with all necessary data
 */ 
typedef struct sym_data {
    char * name;
    sym_type_t type;
    sym_dtype_t dtype;
    sym_status_t status;
} sym_data_t;

/**
 * @brief Element of symbol table (that is, in our case implemented as BST)
 */ 
typedef struct tree_node {
    char * key; 
    sym_data_t data;
    struct tree_node *lPtr;
    struct tree_node *rPtr;
} tree_node_t;

/**
 * @brief Symbol table data type
 */
typedef tree_node_t* symtab_t; 

/**
 * @brief Initializes symbol table
 * @param symbol table to be initialized
 */ 
void init_tab(symtab_t *tab);

/**
 * @brief Inserts a new element into existing symbol table 
 * @param tab destination table
 * @param key key of new element
 */ 
void insert_sym(symtab_t *tab, char *key);

/**
 * @brief Deletes element with specific key and frees all its resources
 * @param tab destination table
 * @param key key of element to be deleted
 */ 
void delete_sym(symtab_t *tab, char *key);

/**
 * @brief Deletetes whole symbol table and correctly frees its resources
 * @param tab symbol table to be deleted
 */ 
void destroy_tab(symtab_t *tab);

/**
 * @brief Searches for symbol table with specific key
 * @param tab symbol table in which should be searching executed
 * @param key key of element that should be found
 * @return Pointer to found symbol or NULL
 */ 
tree_node_t *search(symtab_t *tab, char *key);

/**
 * @brief Changes data of element with given key (if it exists)
 * @param tab destination table
 * @param key key, that specifies element to be changed
 * @param new_data new data of symbol
 */ 
void set_sym(symtab_t *tab, char *key, sym_data_t new_data);

#endif


/***                          End of symtable.h                            ***/
