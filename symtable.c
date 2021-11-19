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
	tab->t = NULL;
    tab->parent_ind = UNSET;
}

/**
 * @brief Searches for symbol table with specific key
 * @param tab symbol table in which should be searching executed
 * @param key key of element that should be found
 * @return Pointer to found symbol or NULL
 */ 
tree_node_t *search(symtab_t *tab, const char *key) {
    tree_node_t * cur_node = tab->t;
	while(cur_node) {
        int comparison_result = str_cmp(cur_node->key, key);
        if(comparison_result == 0) {
            break;
        }
        else if(comparison_result > 0) {
            cur_node = cur_node->l_ptr;
        }
        else {
            cur_node = cur_node->r_ptr;
        }
    }
    
    return cur_node;
}

/**
 * @brief Inserts a new element into existing symbol table 
 * @param tab destination table
 * @param key key of new element
 */ 
void insert_sym(symtab_t *tab, const char *key, sym_data_t newdata) {
    tree_node_t **cur_node = &(tab->t);

    bool was_inserted = false;
    while(*cur_node && !was_inserted) { //Find place for new node
        int comparison_result = str_cmp((*cur_node)->key, key);
        if(comparison_result == 0) {
            (*cur_node)->data = newdata;
            was_inserted = true;
        }
        else if(comparison_result > 0) {
            cur_node = &(*cur_node)->l_ptr;
        }
        else {
            cur_node = &(*cur_node)->r_ptr;
        }
    }

    if(!was_inserted) { //Create new node and allocate memory for it
        *cur_node = (tree_node_t *)malloc(sizeof(tree_node_t));
        if(*cur_node == NULL) {
            return;
        }
        int ret = str_cpy(&(*cur_node)->key, key, strlen(key));
        if(ret == STR_FAILURE) {
            *cur_node = NULL;
            return;
        }

        (*cur_node)->data = newdata;
        (*cur_node)->l_ptr = NULL;
        (*cur_node)->r_ptr = NULL;
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
    tree_node_t **cur_node = &(tab->t);

	while(*cur_node) {
        int comparison_result = str_cmp((*cur_node)->key, key);
        if(comparison_result == 0) { //Node was found
            tree_node_t *to_be_deleted = *cur_node;
            if(to_be_deleted->l_ptr && to_be_deleted->r_ptr) {
                replace_by_rightmost(to_be_deleted, &(to_be_deleted)->l_ptr);
            }
            else {
                if(to_be_deleted->l_ptr) {
                    *cur_node = to_be_deleted->l_ptr;
                }
                else if(to_be_deleted->r_ptr) {
                    *cur_node = to_be_deleted->r_ptr;
                }
                else {
                    *cur_node = NULL;
                }

                free(to_be_deleted->key);
                free(to_be_deleted);
            }
        }
        else if(comparison_result > 0) {
            cur_node = &(*cur_node)->l_ptr;
        }
        else {
            cur_node = &(*cur_node)->r_ptr;
        }
    }
}

/**
 * @brief Deletetes the entire symbol table and correctly frees its resources
 * @param tab symbol table to be deleted
 */ 
void destroy_tab(symtab_t *tab) {
    ts_stack_t stack;
    ts_stack_init(&stack);
    
    tree_node_t *curr_node = tab->t;
    do {
        if(curr_node == NULL) {
            if(!ts_is_empty(&stack)) {
                curr_node = ts_pop(&stack);
            }
        }
        else {
            tree_node_t *tmp = curr_node;
            if(curr_node->r_ptr != NULL) {
                ts_push(&stack, curr_node->r_ptr);
            }
            curr_node = curr_node->l_ptr;

            free(tmp->key);
            free(tmp);
        }

  } while(curr_node != NULL || !ts_is_empty(&stack));

  ts_stack_dtor(&stack);

  tab->t = NULL;
}

/**
 * @brief Converts character used in operand type grammar in get_rule() to sym_dtype enum
 */ 
sym_dtype_t char_to_dtype(char type_c) {
    sym_dtype_t type;
    switch (type_c)
    {
    case 'n':
        type = NUM;
        break;
    case 'i':
        type = INT;
        break;
    case 's':
        type = STR;
        break;
    default:
        type = UNDEFINED;
    }

    return type;
}

/***                          End of symtable.c                            ***/
