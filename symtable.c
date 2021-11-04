/******************************************************************************
 *                                  IFJ21
 *                                symtable.c
 * 
 *        Authors: Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 *        Purpose: Implementation of symbol table used in compiler
 * 
 *                  Last change:
 *****************************************************************************/

#include "symtable.h"

/**
 * @brief Initializes symbol table
 * @param symbol table to be initialized
 */ 
void init_tab(symtab_t *tab) {

}

/**
 * @brief Inserts a new element into existing symbol table 
 * @param tab destination table
 * @param key key of new element
 */ 
void insert_sym(symtab_t *tab, char *key) {

}

/**
 * @brief Deletes element with specific key and frees all its resources
 * @param tab destination table
 * @param key key of element to be deleted
 */ 
void delete_sym(symtab_t *tab, char *key) {

}

/**
 * @brief Deletetes whole symbol table and correctly frees its resources
 * @param tab symbol table to be deleted
 */ 
void destroy_tab(symtab_t *tab) {
    
}

/**
 * @brief Searches for symbol table with specific key
 * @param tab symbol table in which should be searching executed
 * @param key key of element that should be found
 * @return Pointer to found symbol or NULL
 */ 
tree_node_t *search(symtab_t *tab, char *key) {
    return NULL;
}

/**
 * @brief Changes data of element with given key (if it exists)
 * @param tab destination table
 * @param key key, that specifies element to be changed
 * @param new_data new data of symbol
 */ 
void set_sym(symtab_t *tab, char *key, sym_data_t new_data) {

}


/***                          End of symtable.c                            ***/
