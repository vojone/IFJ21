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
	(*tab) = NULL;
}

/**
 * @brief Inserts a new element into existing symbol table 
 * @param tab destination table
 * @param key key of new element
 */ 
void insert_sym(symtab_t *tab, const char *key) {

	tree_node_t* Node = malloc(sizeof(tree_node_t));
	if(Node == NULL)
		return;

	Node->key = malloc(sizeof());
	strcpy(Node->key, (char *)key);

	if((*tab) == NULL) {
		(*tab) = Node;
		return;
	}
	tree_node_t* SRCH;

	while(true) {
		if(strcmp((char *)key, SRCH->key) == 0) {
			break;
		}
		else if(strcmp((char *)key, SRCH->key) > 1 && SRCH->right == NULL) {
			SRCH->right = Node;
			break;
		}
		else if(strcmp((char *)key, (*tab)->key) < 1 && (*tab)->left == NULL) {
			SRCH->right = Node;
			break;
		}
		else {
			if(strcmp((char *)key, SRCH->key) > 1)
				SRCH = SRCH->right;
			else
				SRCH = SRCH->left;
		}

	}

}

/**
 * @brief Replaces deleted element with two children by rightmost element
 * @param tab destination table
 * @param target replaced element
 */
void replace_rightmost(symtab_t *tab, symtab_t *target) {

}

/**
 * @brief Deletes element with specific key and frees all its resources
 * @param tab destination table
 * @param key key of element to be deleted
 */ 
void delete_sym(symtab_t *tab,  const char *key) {
	if((*tab) == NULL)
		return;
	tree_node_t* PREV_NODE = (*tab);
	tree_node_t* SRCH = (*tab);
	

	while(true) {
		if(strcmp(SRCH->key, (char *)key) == 0) {

			if(SRCH->left != NULL && SRCH->right != NULL) {
				replace_rightmost(&(PREV_NODE), &(SRCH));
			}
			else if(SRCH->left != NULL && SRCH->right == NULL) {
				PREV_NODE->right = SRCH->left;
				free(SRCH);
			}
			else if(SRCH->left == NULL && SRCH->right != NULL) {
				PREV_NODE->right = SRCH->right;
				free(SRCH);
			}
			else {

				if(strcmp((char *)key, SRCH->key) > 1) {
					PREV_NODE = SRCH;
					SRCH = SRCH->right;
				}
				else {
					PREV_NODE = SRCH;
					SRCH = SRCH->left;
				}
			}
		}
	}
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
tree_node_t *search(symtab_t *tab, const char *key) {

	while(true) {
		if((*tab) == NULL) {
			return NULL;
		}
		if(strcmp((*tab)->key, (char *)key) == 0) {
			return (*tab);
		}
		else if(strcmp((char *)key, (*tab)->key) > 1) {
			(*tab) = (*tab)->right;
		}
		else {
			(*tab) = (*tab)->left;
		}
	}

}

/**
 * @brief Changes data of element with given key (if it exists)
 * @param tab destination table
 * @param key key, that specifies element to be changed
 * @param new_data new data of symbol
 */ 
void set_sym(symtab_t *tab, const char *key, sym_data_t new_data) {
	//while()
		(*tab)->data = new_data;
	//strcpy(new_data->name, (*tab)->data->name);
	//(*tab)->data->dtype = new_data->dtype;
	//(*tab)->data->status = new_data->status;
}


/***                          End of symtable.c                            ***/
