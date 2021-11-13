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
#include <stdbool.h>
#include <string.h>

/**
 * @brief Initializes symbol table
 * @param symbol table to be initialized
 */ 
void init_tab(symtab_t *tab) {
	*tab = NULL;
}

/**
 * @brief Searches for symbol table with specific key
 * @param tab symbol table in which should be searching executed
 * @param key key of element that should be found
 * @return Pointer to found symbol or NULL
 */ 
tree_node_t *search(symtab_t *tab, const char *key) {
	while(*tab) {
        int comparison_result = str_cmp((*tab)->key, key);
        if(comparison_result == 0) {
            break;
        }
        else if(comparison_result > 0) {
            tab = &(*tab)->l_ptr;
        }
        else {
            tab = &(*tab)->r_ptr;
        }
    }
    
    return *tab;
}

/**
 * @brief Inserts a new element into existing symbol table 
 * @param tab destination table
 * @param key key of new element
 */ 
void insert_sym(symtab_t *tab, const char *key, sym_data_t newdata) {
    bool was_inserted = false;
    while(*tab && !was_inserted) { //Find place for new node
        int comparison_result = str_cmp((*tab)->key, key);
        if(comparison_result == 0) {
            (*tab)->data = newdata;
            was_inserted = true;
        }
        else if(comparison_result > 0) {
            tab = &(*tab)->l_ptr;
        }
        else {
            tab = &(*tab)->r_ptr;
        }
    }

    if(!was_inserted) { //Create new node and allocate memory for it
        *tab = (tree_node_t *)malloc(sizeof(tree_node_t));
        if(*tab == NULL) {
            return;
        }
        int ret = str_cpy(&(*tab)->key, key, strlen(key));
        if(ret == STR_FAILURE) {
            *tab = NULL;
            return;
        }

        (*tab)->data = newdata;
        (*tab)->l_ptr = NULL;
        (*tab)->r_ptr = NULL;
    }
}

/**
 * @brief Replaces deleted element with two children by rightmost element
 * @param tab destination table
 * @param target replaced element
 */
void replace_by_rightmost(tree_node_t *target, tree_node_t **tab) {
    if(*tab == NULL) {
        return;
    }

    while((*tab)->r_ptr) {
        tab = &(*tab)->r_ptr;
    }

    tree_node_t *temp = *tab;
    target->data = (*tab)->data;

    free(target->key);
    target->key = (*tab)->key;
    
    if((*tab)->l_ptr != NULL) {
        *tab = (*tab)->l_ptr;
    }
    else { 
        *tab = NULL;
    }

    free(temp);
}

/**
 * @brief Deletes element with specific key and frees all its resources
 * @param tab destination table
 * @param key key of element to be deleted
 */ 
void delete_sym(symtab_t *tab, const char *key) {
	while(*tab) {
        int comparison_result = str_cmp((*tab)->key, key);
        if(comparison_result == 0) { //Node was found
            tree_node_t *to_be_deleted = *tab;
            if(to_be_deleted->l_ptr && to_be_deleted->r_ptr) {
                replace_by_rightmost(to_be_deleted, &(to_be_deleted)->l_ptr);
            }
            else {
                if(to_be_deleted->l_ptr) {
                    *tab = to_be_deleted->l_ptr;
                }
                else if(to_be_deleted->r_ptr) {
                    *tab = to_be_deleted->r_ptr;
                }
                else {
                    *tab = NULL;
                }

                free(to_be_deleted->key);
                free(to_be_deleted);
            }
        }
        else if(comparison_result > 0) {
            tab = &(*tab)->l_ptr;
        }
        else {
            tab = &(*tab)->r_ptr;
        }
    }
}

/**
 * @brief Deletetes the entire symbol table and correctly frees its resources
 * @param tab symbol table to be deleted
 */ 
void destroy_tab(tree_node_t **tab) {
    ts_stack_t stack;
    ts_stack_init(&stack);

    do {
        if(*tab == NULL) {
            if(!ts_is_empty(&stack)) {
                *tab = ts_pop(&stack);
            }
        }
        else {
            tree_node_t *tmp = *tab;
            if((*tab)->r_ptr != NULL) {
                ts_push(&stack, (*tab)->r_ptr);
            }
            *tab = (*tab)->l_ptr;

            free(tmp->key);
            free(tmp);
        }
  } while(*tab != NULL || !ts_is_empty(&stack));

  ts_stack_dtor(&stack);
}

/***                          End of symtable.c                            ***/
