/******************************************************************************
 *                                  IFJ21
 *                                symtable.c
 * 
 *       Authors: Vojtěch Dvořák (xdvora3o), Tomáš Dvořák (xdvora3r)
 *        Purpose: Implementation of symbol table used in compiler
 * 
 *              Based on our solutions of second IAL project
 * 
 *                      Last change: 7. 12. 2021
 *****************************************************************************/

/**
 * @file symtable.c
 * @brief Implementation of symbol table used in compiler
 * 
 * @authors Vojtěch Dvořák (xdvora3o), Tomáš Dvořák (xdvora3r)
 */ 


#include "symtable.h"

DSTACK(tree_node_t*, ts,) /**< Operations with tree nodes stack (used in destroy tab function) */

DSTACK(symtab_t, symtabs, fprintf(stderr," %s", s->data[i].t->key)) /**< Operations with stack of symtabs */

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
            
            data_dtor(&(*cur_node)->data);
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

                data_dtor(&to_be_deleted->data);
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

            data_dtor(&tmp->data);
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
    case 'z':
        type = NIL;
        break;
    case 'b':
        type = BOOL;
        break;
    default:
        type = UNDEFINED;
    }

    return type;
}


/**
 * @brief Converts enum type used in symtable to correspoding character symbol
 */ 
char dtype_to_char(sym_dtype_t type) {
    char type_c;
    switch (type)
    {
    case NUM:
        type_c = 'n';
        break;
    case INT:
        type_c = 'i';
        break;
    case STR:
        type_c = 's';
        break;
    case NIL:
        type_c = 'z';
        break;
    case BOOL:
        type_c = 'b';
        break;
    default:
        type_c = ' ';
    }

    return type_c;
}


/**
 * @brief Initializes strings data structure of symbol
 * @return If there were error during initialization returns EXIT_FAILURE, otherwise EXIT_SUCCESS
 */ 
int init_data(sym_data_t *new_data) {
    if(str_init(&new_data->name) != STR_SUCCESS) {
        return EXIT_FAILURE;
    }

    if(str_init(&new_data->params) != STR_SUCCESS) {
        return EXIT_FAILURE;
    }

    if(str_init(&new_data->ret_types) != STR_SUCCESS) {
        return EXIT_FAILURE;
    }

    new_data->was_used = false;

    return EXIT_SUCCESS;
}


/**
 * @brief Frees all resources that data holds and sets it to the state before initialization
 */ 
void data_dtor(sym_data_t *data) {
    str_dtor(&data->name);
    str_dtor(&data->params);
    str_dtor(&data->ret_types);
}


/**
 * @brief Contains static array with builtin functions and its attributes (parameter, return types)
 */ 
sym_data_t* builtin_functions(unsigned int index) {
    if (index >= BUILTIN_TABLE_SIZE) {
        return NULL;
    }

    static sym_data_t builtin_functions[BUILTIN_TABLE_SIZE] = {
    //Name                Return types   Parameters
    {{0, 0, "chr"}, FUNC, {0, 0, "s"}, {0, 0, "i"}, UNSET, DEFINED, false},
    {{0, 0, "ord"}, FUNC, {0, 0, "i"}, {0, 0, "si"}, UNSET, DEFINED, false},
    {{0, 0, "readi"}, FUNC, {0, 0, "i"}, {0, 0, ""}, UNSET, DEFINED, false},
    {{0, 0, "readn"}, FUNC, {0, 0, "n"}, {0, 0, ""}, UNSET, DEFINED, false},
    {{0, 0, "reads"}, FUNC, {0, 0, "s"}, {0, 0, ""}, UNSET, DEFINED, false},
    {{0, 0, "substr"}, FUNC, {0, 0, "s"}, {0, 0, "snn"}, UNSET, DEFINED, false},
    {{0, 0, "tointeger"}, FUNC, {0, 0, "i"}, {0, 0, "n"}, UNSET, DEFINED, false},
    {{0, 0, "write"}, FUNC, {0, 0, ""}, {0, 0, "%"}, UNSET, DEFINED, false}};

    return &builtin_functions[index];
}


/**
 * @brief Inserts all builtin functions into given symbol table
 * @note Inseted functions will have same name as key in symbol table
 */ 
void load_builtin_f(symtab_t *dst) {
    for(int i = 0; builtin_functions(i); i++) {
        char * f_name = to_str(&builtin_functions(i)->name);
        insert_sym(dst, f_name, *builtin_functions(i));
    }
}


/**
 * @brief Tries to find function by name in table of builtin functions
 * @return Pointer to function data in static table if function is found, other wise NULL
 */ 
sym_data_t* search_builtin(const char *f_name) {
    //If function name start with 'z' (for example) -> search from the end 
    int i = f_name[0] < 'z' - 'a' / 2 ? 0 : BUILTIN_TABLE_SIZE - 1;
    int inc = f_name[0] < 'z' - 'a' / 2 ? 1 : -1;

    for(; builtin_functions(i) != NULL; i += inc) {
        if(str_cmp(to_str(&builtin_functions(i)->name), f_name) == 0) {
            return builtin_functions(i);
        }
    }

    return NULL;
}


/**
 * @brief Checks if key identifies any of builtin functions, if yes puts it into given symtable
 * @return True if symbol was found in builtin functions table otherwise false
 */
bool check_builtin(char *key, symtab_t *dst) {
    sym_data_t *bfunc_data_ptr = search_builtin(key);

    if(bfunc_data_ptr) {
        insert_sym(dst, to_str(&bfunc_data_ptr->name), *bfunc_data_ptr);
        return true;
    }
    else {

        return false;
    }
}


/**
 * @brief Performs searching in stack of symtabs
 * @return If nothing is found returns NULL otherwise returns pointer to first occurence
 */
tree_node_t * deep_search(symtabs_stack_t *sym_stack, 
                          symtab_t *start_symtab, 
                          char *key) {
                              
    symtab_t *curr_tab = start_symtab;
    
    while(curr_tab != NULL) {
        tree_node_t * result_of_searching = search(curr_tab, key);
        if(result_of_searching) { //If something is found return pointer
            return result_of_searching;
        }
        else { //If not, try to search it in 'parent' symbol table
            curr_tab = symtabs_get_ptr(sym_stack, curr_tab->parent_ind);
        }
    }

    return NULL;
}


/***                          End of symtable.c                            ***/
